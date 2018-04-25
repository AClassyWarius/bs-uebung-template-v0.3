//
//  myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 02.08.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include <iostream>
#include <cstring>
#include <cmath>
#include <time.h>

#include "myfs.h"
#include "myfs-info.h"



MyFS* MyFS::_instance = NULL;

#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

#define LOGF(fmt, ...) \
do { fprintf(this->logFile, fmt "\n", __VA_ARGS__); } while (0)

#define LOG(text) \
do { fprintf(this->logFile, text "\n"); } while (0)

#define LOGM() \
do { fprintf(this->logFile, "%s:%d:%s()\n", __FILE__, \
__LINE__, __func__); } while (0)


bool MyFS::blockIsEmpty(int blockNo) {
    char buffer[BD_BLOCK_SIZE];
    char zeroArray[512];
    
    for (int i = 0; i < 512; i++) {
        zeroArray[i] = 0;
    }
    
    bd.read(blockNo, buffer);
    if (strcmp(buffer, zeroArray) == 0) {
        return true;
    } else {
        return false;
    }
}



MyFS* MyFS::Instance() {
    if(_instance == NULL) {
        _instance = new MyFS();
    }
    return _instance;
}

MyFS::MyFS() {
    this->logFile= stderr;
}

MyFS::~MyFS() {
    
}

std::string MyFS::getName(int i) {
    char nameChar[NAME_LENGTH];
    char buffer[BD_BLOCK_SIZE];
    bd.read(i, buffer);
    std::memcpy(nameChar, buffer, NAME_LENGTH);
    std::string name = nameChar;
    return name;
}

int MyFS::getSize(int i) {
    char sizeChar[16];
    char buffer[BD_BLOCK_SIZE];
    bd.read(i, buffer);
    std::memcpy(sizeChar, buffer+256, 16);
    int size = atoi(sizeChar);
    return size;
}

int MyFS::getUid(int i) {
    char uidChar[16];
    char buffer[BD_BLOCK_SIZE];
    bd.read(i, buffer);
    std::memcpy(uidChar, buffer+272, 16);
    int uid = atoi(uidChar);
    return uid;
}

int MyFS::getGid(int i) {
    char gidChar[16];
    char buffer[BD_BLOCK_SIZE];
    bd.read(i, buffer);
    std::memcpy(gidChar, buffer+288, 16);
    int gid = atoi(gidChar);
    return gid;
}

int MyFS::getMode(int i) {
    char modeChar[16];
    char buffer[BD_BLOCK_SIZE];
    bd.read(i, buffer);
    std::memcpy(modeChar, buffer+304, 16);
    int mode = atoi(modeChar);
    return mode;
}

int MyFS::getDataBlockNo(int i) {
    char dataBlockNo[16];
    char buffer[BD_BLOCK_SIZE];
    bd.read(i, buffer);
    std::memcpy(dataBlockNo, buffer+320, 16);
    int dataNo = atoi(dataBlockNo);
    return dataNo;
}


//TODO zweiter Aufruf.
int MyFS::fuseGetattr(const char *path, struct stat *statbuf) {
    LOGM();
   
    LOGF("Path Fuse: %s", path);
    
    statbuf->st_uid = 510;
    statbuf->st_gid = 20;
    statbuf->st_atime = time ( NULL );
    statbuf->st_mtime = time ( NULL );
    
    if (strcmp(path, "/") == 0) {
        statbuf->st_mode = S_IFDIR | 0755;          
        statbuf->st_nlink = 2;
    }
    
    if (blockIsEmpty(INODE_START_BLOCK)) {
        LOG("Keine Inode gefunden, keine Daten zum Mounten gefunden");
        return -1;
    } else {
        LOG("Inode gefunden, Struktur Status wird mit Informationen aus der Inode befüllt");
        int i = INODE_START_BLOCK;
        while(!blockIsEmpty(i)) {
            std::string name = "/" + getName(i);
            if (strcmp(path, name.c_str()) == 0) {
                statbuf->st_uid = getUid(i);
                statbuf->st_gid = getGid(i);
                statbuf->st_mode = S_IFREG | 0644;
                statbuf->st_nlink = 1;                      
                statbuf->st_size = getSize(i);
            }
            i++;
        }
    }
    return 0;
}

int MyFS::fuseReadlink(const char *path, char *link, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();
    return 0;
}

int MyFS::fuseMkdir(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseUnlink(const char *path) {
    LOGM();
    return 0;
}

int MyFS::fuseRmdir(const char *path) {
    LOGM();
    return 0;
}

int MyFS::fuseSymlink(const char *path, const char *link) {
    LOGM();
    return 0;
}

int MyFS::fuseRename(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseLink(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseChmod(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    LOGM();
    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t newSize) {
    LOGM();
    return 0;
}

int MyFS::fuseUtime(const char *path, struct utimbuf *ubuf) {
    LOGM();
    return 0;
}

//TODO
int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

//TODO
int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    char *selectedText = NULL;
    
    if (blockIsEmpty(INODE_START_BLOCK)) {
        LOG("Keine Inode gefunden, keine Daten zum Mounten gefunden");
        return -1;
    } else {
        LOG("Inode gefunden, fuseRead");
        int i = INODE_START_BLOCK;
        while(!blockIsEmpty(i)) {
            std::string name = "/" + getName(i);
            
            LOGF("Path von Fuse: %s", path);
            LOGF("Dateiname: %s", name.c_str());
            
            if (strcmp(path, name.c_str()) == 0) {
                LOG("path == name");
                char data[BD_BLOCK_SIZE];
                bd.read((getDataBlockNo(i)), data);
                selectedText = data;
                
                LOGF("inhalt datei: %s", selectedText);
                
                memcpy(buf, selectedText + offset, getSize(i));
                return strlen(selectedText) - offset;
            }
            i++;
        } 
    }
    return 0;
}


int MyFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

//Wird aufgerufen als erstes Aufgerufen kann aber ignoriert werden.
int MyFS::fuseStatfs(const char *path, struct statvfs *statInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseFlush(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

//TODO
int MyFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseFsync(const char *path, int datasync, struct fuse_file_info *fi) {
    LOGM();
    return 0;
}

int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
    LOGM();
    return 0;
}

int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseListxattr(const char *path, char *list, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseRemovexattr(const char *path, const char *name) {
    LOGM();
    return 0;
}

//TODO
int MyFS::fuseOpendir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

//TODO
int MyFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    LOGF("Path von Fuse: %s", path);
    
    filler(buf, ".", NULL, 0); //Current Directory
    filler(buf, "..", NULL, 0); //Parent Directory
    
    int i = INODE_START_BLOCK;
    
    while (!blockIsEmpty(i)) {
        if (strcmp(path, "/") == 0) {
            const char* name = getName(i).c_str();
            filler(buf, name, NULL, 0);
        }
        i++;
    }
    return 0;
}

//TODO
int MyFS::fuseReleasedir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    return 0;
}

int MyFS::fuseFsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

//TODO
int MyFS::fuseInit(struct fuse_conn_info *conn) {
    
    // Open logfile
    this->logFile= fopen(((MyFsInfo *) fuse_get_context()->private_data)->logFile, "w");
    if(this->logFile == NULL) {
        fprintf(stderr, "ERROR: Cannot open logfile %s\n", ((MyFsInfo *) fuse_get_context()->private_data)->logFile);
        return -1;
    }
    
    // turn of logfile buffering
    setvbuf(this->logFile, NULL, _IOLBF, 0);

    LOG("Starting logging...\n");
    LOGM();
        
    // you can get the containfer file name here:
    LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);
    
    // TODO: Enter your code here!
    
    
    bd.open(((MyFsInfo *) fuse_get_context()->private_data)->contFile);
    
    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

void MyFS::fuseDestroy() {
    LOGM();
}


