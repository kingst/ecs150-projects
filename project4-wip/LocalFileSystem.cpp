#include <iostream>
#include <string>

#include "LocalFileSystem.h"
#include "ufs.h"

using namespace std;

LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}

int LocalFileSystem::lookup(int parentInodeNumber, string name) {
  return -1;
}

int LocalFileSystem::create(int parentInodeNumber, int type, string name) {
  return -1;
}
  
int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
  return -1;
}

int LocalFileSystem::read(int inodeNumber, const void *buffer) {
  return -1;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name) {
  return -1;
}

void LocalFileSystem::readSuperBlock(Disk *disk, super_t *super) {

}

void LocalFileSystem::readInodeBitmap(Disk *disk, super_t *super, unsigned char *inodeBitmap) {

}

void LocalFileSystem::writeInodeBitmap(Disk *disk, super_t *super, unsigned char *inodeBitmap) {

}

void LocalFileSystem::readDataBitmap(Disk *disk, super_t *super, unsigned char *dataBitmap) {

}

void LocalFileSystem::writeDataBitmap(Disk *disk, super_t *super, unsigned char *dataBitmap) {

}

void LocalFileSystem::readInodeRegion(Disk *disk, super_t *super, inode_t *inodes) {

}

void LocalFileSystem::writeInodeRegion(Disk *disk, super_t *super, inode_t *inodes) {

}
