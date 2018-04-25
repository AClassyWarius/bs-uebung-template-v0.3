//
//  mk.myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "myfs.h"
#include "blockdevice.h"
#include "macros.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>

using namespace std;

//Maximale Länge eines Dateinamens
#define NAME_LENGTH 255

//Logische Blockgröße
#define BLOCK_SIZE 512

//Anzahl der Verzeichniseinträge
#define NUM_DIR_ENTRIES 64

//Anzahl offener Datein pro MyFS Containerdatei
#define NUM_OPEN_FILES 64

//Disk-Size 32 768 Bytes
const int disk_size = 32768;


//Erstellt den Superblock mit Informationen über das Filesystem
superblock_st* createSuperblock() {
    superblock_st *sb = new superblock_st;
    sb->disk_size = disk_size;
    sb->block_size = BLOCK_SIZE;
    sb->block_number = sb->disk_size / sb->block_size;
    sb->inode_start_address = 3;                            //0 = SuperBlock , 1 = Directory , 2 = Start Inode
    return sb;
}

//Erstellt ein File-bezogenes Inode nach der vorge-
//gebenen Struktur
inode_st* createInode(char *file, BlockDevice container) {

    struct stat Status;
    stat(file, &Status);
    
    inode_st *inode = new inode_st;
    
    inode->filename = file;                 //Dateiname
    inode->size = Status.st_size;           //Dateigröße
    inode->user_id = Status.st_uid;         //Benutzer-ID
    inode->grp_id = Status.st_gid;          //Gruppen-ID
    
    Status.st_mode = (S_IFREG | 0444);      //Füllen der Zugriffsberechtigungen
    
    inode->protection = Status.st_mode;     //Zugriffsberechtigungen (mode)
    inode->atime = time(0);                 //Zeitpunkt letzter Zugriff
    inode->mtime = time(0);                 //letzte Veränderung
    inode->ctime = time(0);                 //letzte Statusänderung
    
    inode->block_pointer = (container.getNumberOfDirectory() + 7);              
                                            //Pointer auf den Datenblock der Datei

    return inode;
}

//Erstellt ein File-bezogenes Root-Verzeichnis auf der Container-Datei
dir_st* createDirectory(const char *file, BlockDevice &container) {     //&-Operator verwerft nicht die Referenz auf das Objekt

    dir_st* directory = new dir_st;
    
    directory->filenames = file;
    container.setNumberOfDirectory();
    directory->num_of_files = container.getNumberOfDirectory();
    
    return directory;
}

//Get-Methode um den Inhalt einer Datei auszulesen.
//Gibt ein char-Array zurück.
char* getData(const char *file) {
    
    FILE *f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    
    int fsize = ftell(f);
    
    fseek(f, 0, SEEK_SET);

    char *buffer = (char *)malloc(fsize + 1);
    fread(buffer, fsize, 1, f);
    fclose(f);

    buffer[fsize] = 0;
    
    return buffer;
}

//Überprüft ob ein Block leer ist.
//Return true, falls Block leer ist.
//Return false, falls Block voll ist.
bool blockIsEmpty(BlockDevice container, int blockNr) {
    char buffer[BLOCK_SIZE];
    char zeroArray[512];
    
    for (int i = 0; i < 512; i++) {
        zeroArray[i] = 0;
    }
    
    container.read(blockNr, buffer);
    if (strcmp(buffer, zeroArray) == 0) {
        cout << "Block ist leer" << endl;
        return true;
    } else {
        cout << "Block ist gefüllt" << endl;
        return false;
    }
}

//Schreibt einmalig den Superblock mit den Informationen über die Datei
void writeSuperblock(superblock_st *superblock, BlockDevice container) {
    if (blockIsEmpty(container, 0)) {
        
        char buffer[BLOCK_SIZE];
        container.read(0, buffer);
        
        string s1 = to_string(superblock->disk_size);
        string s2 = to_string(superblock->block_size);
        string s3 = to_string(superblock->block_number);
        string s4 = to_string(superblock->inode_start_address);
        
        char const *c1 = s1.c_str();
        char const *c2 = s2.c_str();
        char const *c3 = s3.c_str();
        char const *c4 = s4.c_str();
        
        int j = 0;
        int i = 0;
        
        while (j != s1.length()) {
            if (buffer[i] == 0) {
                buffer[i] = *c1;
                c1++;
                j++;
            }
            i++;
        }
        i = 64;
        j = 0;
        while (j != s2.length()) {
            if (buffer[i] == 0) {
                buffer[i] = *c2;
                c2++;
                j++;
            }
            i++;
        }
        i = 128;
        j = 0;
        while (j != s3.length()) {
            if (buffer[i] == 0) {
                buffer[i] = *c3;
                c3++;
                j++;
            }
            i++;
        }
        i = 192;
        j = 0;
        while (j != s4.length()) {
            if (buffer[i] == 0) {
                buffer[i] = *c4;
                c4++;
                j++;
            }
            i++;
        }
        container.write(0, buffer);
        
    } else {
        cout << "Superblock bereits angelegt" << endl;
    }
}


void writeDirectory(dir_st *dir, BlockDevice container) {
    
    char buffer[BLOCK_SIZE];
    container.read(1, buffer);
    
    int i;
    
    if (blockIsEmpty(container, 1)) {
        i = 0;
    } else {
        i = 32 * container.getNumberOfDirectory();
    }
        string s1 = dir->filenames;
        string s2 = to_string(dir->num_of_files);
        
        char const *c1 = s1.c_str();
        char const *c2 = s2.c_str();
        
        int j = 0;
        
        
        while(j != s1.length()) {
            if (buffer[i] == 0) {
                buffer[i] = *c1;
                c1++;
                j++;
            }  
            i++;
        }
        i += 1;
        j = 0;
        while (j != s2.length()) {
            if (buffer[i] == 0) {
                buffer[i] = *c2;
                c2++;
                j++;
            }
            i++;
        }
        container.write(1, buffer);
}


void writeInode(inode_st *inode, BlockDevice container) {
    
    int counter = 3; //Zähler für die Inode-Blöcke
    
    while(counter < container.getNumberOfDirectory() + 3) {
        
        char buffer[BLOCK_SIZE];
        container.read(counter, buffer);
    
        if (blockIsEmpty(container, counter) & !(strcmp(inode->filename, buffer) == 0)) {
            
            cout << "create Inode " << counter << endl;
            
            char buffer[BLOCK_SIZE];
            container.read(counter, buffer);

            string s1 = inode->filename;
            string s2 = to_string(inode->size);
            string s3 = to_string(inode->user_id);
            string s4 = to_string(inode->grp_id);
            string s5 = to_string(inode->protection);
            string s6 = to_string(inode->block_pointer);

            char const *c1 = s1.c_str();
            char const *c2 = s2.c_str();
            char const *c3 = s3.c_str();
            char const *c4 = s4.c_str();
            char const *c5 = s5.c_str();
            char const *c6 = s6.c_str();

            char *c7 = ctime(&inode->atime);
            char *c8 = ctime(&inode->mtime);
            char *c9 = ctime(&inode->ctime);

            if (s1.length() <= NAME_LENGTH) {
                for (int i = 0; i < s1.length(); i++) {
                    buffer[i] = *c1;
                    c1++;
                }
            } else {
                error("Filename is too long");
            }

            for (int i = 256; i < BLOCK_SIZE; i++) {
                if (i >= 256 & i < 272 & i < (s2.length() + 256)) {
                    buffer[i] = *c2;
                    c2++;
                }
                if (i >= 272 & i < 288 & i < (s3.length() + 272)) {
                    buffer[i] = *c3;
                    c3++;
                }
                if (i >= 288 & i < 304 & i < (s4.length() + 288)) {
                    buffer[i] = *c4;
                    c4++;
                }
                if (i >= 304 & i < 320 & i < (s5.length() + 304)) {
                    buffer[i] = *c5;
                    c5++;
                }
                if (i >= 320 & i < 336 & i < (s6.length() + 320)) {
                    buffer[i] = *c6;
                    c6++;
                }
                if (i >= 336 & i < 368 /*& i < (strlen(c7) + 336)*/) {
                    buffer[i] = *c7;
                    c7++;
                }
                if (i >= 368 & i < 400 /*& i < (strlen(c8) + 368)*/) {
                    buffer[i] = *c8;
                    c8++;
                }
                if (i >= 400 & i < 432 /*& i < (strlen(c9) + 400)*/) {
                    buffer[i] = *c9;
                    c9++;
                }
            }
            container.write(counter, buffer);
        }
        counter++;
    }
}

void writeData(char *data, BlockDevice container) {
    int i = container.getNumberOfDirectory() + 7;
    if (blockIsEmpty(container, i)) {
        container.write(i, data);
    }
}



void test(BlockDevice bd) {
    char *selectedText = NULL;
    if (blockIsEmpty(bd, INODE_START_BLOCK)) {
        cout << "KACKE" << endl;
    } else {
        int i = INODE_START_BLOCK;
        while(!blockIsEmpty(bd, i)) {
            char buffer[BD_BLOCK_SIZE];
            bd.read(i, buffer);
            char name[NAME_LENGTH];
            std::memcpy(name, buffer, NAME_LENGTH);

            //char iData[16];
            //std::memcpy(iData, buffer+320, 16);
            //int blockNo = atoi(iData);
            char data[BD_BLOCK_SIZE];
            bd.read(i+5, data);
            selectedText = data;
            cout << selectedText << endl;
            i++;
        }
    }
}

int getSize(BlockDevice bd, int i) {
    char sizeChar[16];
    char buffer[BD_BLOCK_SIZE];
    bd.read(i, buffer);
    std::memcpy(sizeChar, buffer+256, 16);
    int size = atoi(sizeChar);
    return size;
}



int main(int argc, char *argv[]) {
    
    BlockDevice bd;
    
    if (bd.open("container.bin") < 0) {
        
        bd.create("container.bin");
        
        cout << "64 Blocks werden erschaffen" << endl;
        
        char buffer[BLOCK_SIZE];
       
        for (int j = 0; j < 512; j++) {
                buffer[j] = 0;
            }
    
        for (int i = 0; i < 64; i++) {
            bd.write(i, buffer);
        }
        
        writeSuperblock(createSuperblock(), bd);
        
    } else {
        
        cout << "container existiert bereits" << endl;
    
    }
    
    if (blockIsEmpty(bd, 3)) {
        writeDirectory(createDirectory("file.txt", bd), bd);
        writeInode(createInode("file.txt", bd), bd);
        writeData(getData("file.txt"), bd);

        writeDirectory(createDirectory("test.txt", bd), bd);
        writeInode(createInode("test.txt", bd), bd);
        writeData(getData("test.txt"), bd);
        
        writeDirectory(createDirectory("neu.txt", bd), bd);
        writeInode(createInode("neu.txt", bd), bd);
        writeData(getData("neu.txt"), bd);
    }
    
    
    
    
    bd.close();
    
    return 0;
}


