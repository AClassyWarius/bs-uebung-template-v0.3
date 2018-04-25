//
//  blockdevice.h
//  myfs
//
//  Created by Oliver Waldhorst on 09.10.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef blockdevice_h
#define blockdevice_h

#include <stdio.h>
#include <cstdint>
#include <string>

#define BD_BLOCK_SIZE 512
#define INODE_START_BLOCK 3
#define NAME_LENGTH 255
#define NUM_DIR_ENTRIES 64

//struct zum befüllen des Superblocks
typedef struct superblock_st {
    int disk_size;
    int block_size;
    int block_number;
    int inode_start_address;
} superblock_st;

//struct zum befüllen der Inodes
typedef struct inode_st {
    char *filename;
    int size;
    int user_id;
    int grp_id;
    int protection;
    time_t atime;
    time_t mtime;
    time_t ctime;
    int block_pointer;
} inode_st;

//struct zum befüllen des Root-Verzeichnis
//num_of_files zählt die einträge und entspricht der inode number
typedef struct dir_st {
    std::string filenames;
    int num_of_files;
} dir_st;

class BlockDevice {
private:
    uint32_t blockSize;
    int contFile;
    uint32_t size;
public:
    int i = 0;                                          //hinzugefügt
    int getNumberOfDirectory() { return this->i;}       //hinzugefügt
    void setNumberOfDirectory() { i++;}                 //hinzugefügt
    BlockDevice(u_int32_t blockSize = 512);
    void resize(u_int32_t blockSize);
    int open(const char* path);
    int create(const char* path);    
    int close();
    int read(u_int32_t blockNo, char *buffer);
    int write(u_int32_t blockNo, char *buffer);
    uint32_t getSize() {return this->size;}
};

#endif /* blockdevice_h */
