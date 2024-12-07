#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <cmath>

#include "LocalFileSystem.h"
#include "ufs.h"

using namespace std;


LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}

void LocalFileSystem::readSuperBlock(super_t *super) {
  char buffer[4096];
  this->disk->readBlock(0, buffer);
  memcpy(super, buffer, sizeof(super_t));
}

void printInodeSizes(inode_t *inodes, int numInodes) {
  for (int i = 0; i < numInodes; i ++) {
    cout << inodes[i].size << " ";
  }
  cout << endl;
}

void LocalFileSystem::readInodeBitmap(super_t *super, unsigned char *inodeBitmap) {

  int bitmapAddr = super->inode_bitmap_addr;
  int bitmapBlocks = super->inode_bitmap_len;
  int unreadBits = super->num_inodes;

  for (int i = 0; i < bitmapBlocks; i ++) {
    char buffer[4096];
    this->disk->readBlock(bitmapAddr, buffer);
    int bitsToRead = 4096;
    if (unreadBits < 4096) bitsToRead = unreadBits;
    memcpy(inodeBitmap + i * 4096, buffer, bitsToRead / 8);
    unreadBits -= bitsToRead;
  }
}

void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
  int bitmapAddr = super->inode_bitmap_addr;
  int bitmapBlocks = super->inode_bitmap_len;
  int unreadBits = super->num_inodes;

  for (int i = 0; i < bitmapBlocks; i ++) {
    char buffer[4096];
    this->disk->readBlock(bitmapAddr, buffer);
    int bitsToRead = 4096;
    if (unreadBits < 4096) bitsToRead = unreadBits;
    memcpy(buffer, inodeBitmap + i * 4096, (bitsToRead) / 8);
    this->disk->writeBlock(bitmapAddr, buffer);
    unreadBits -= bitsToRead;
  }
}

void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap) {
  inode_t inodes[super->num_inodes];
  readInodeRegion(super, inodes);

  int bitmapAddr = super->data_bitmap_addr;
  int bitmapBlocks = super->data_bitmap_len;
  int unreadBits = super->num_inodes;
  
  for (int i = 0; i < bitmapBlocks; i ++) {
    char buffer[4096];
    this->disk->readBlock(bitmapAddr + i, buffer);
    int bitsToRead = 4096;
    if (unreadBits < 4096) bitsToRead = unreadBits;
    memcpy(dataBitmap + i * 4096, buffer, bitsToRead / 8);
    unreadBits -= bitsToRead;
  }
}

void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap) {
  int bitmapAddr = super->data_bitmap_addr;
  int bitmapBlocks = super->data_bitmap_len;
  int unreadBits = super->num_data;

  for (int i = 0; i < bitmapBlocks; i ++) {
    char buffer[4096];
    this->disk->readBlock(bitmapAddr, buffer);
    int bitsToRead = 4096;
    if (unreadBits < 4096) bitsToRead = unreadBits;
    memcpy(buffer, dataBitmap + i * 4096, (bitsToRead) / 8);
    this->disk->writeBlock(bitmapAddr, buffer);
    unreadBits -= bitsToRead;
  }
}

void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes) {
  int numInodes = super->num_inodes;
  int blockInodes = 4096 / sizeof(inode_t);
  int inodeRegionAddr = super->inode_region_addr;
  int inodeIdx = 0;
  int lastBlock = -1;
  while (inodeIdx < numInodes) {
    char buffer[4096];
    int blockNum = inodeIdx / blockInodes;
    int offset = inodeIdx % blockInodes;
    if (blockNum != lastBlock) {
      this->disk->readBlock(inodeRegionAddr + blockNum, buffer);
    }
    memcpy(&inodes[inodeIdx], buffer + sizeof(inode_t) * offset, sizeof(inode_t));
    inodeIdx += 1;
  }
}

void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes) {
  int numInodes = super->num_inodes;
  
  int blocks = (numInodes  * sizeof(inode_t)) / UFS_BLOCK_SIZE;
  int bytesToRead = numInodes * sizeof(inode_t);
  if (bytesToRead % UFS_BLOCK_SIZE) blocks += 1;

  // cout << "size of inode_t is " << sizeof(inode_t) << endl;
  // cout << blocks << " blocks with " << bytesToRead << " bytes" << endl;

  // iterate over blocks in inode region
  for (int block = 0; block < blocks; block += 1) {
    char writeBuf[4096];
    int inodesInBlock = UFS_BLOCK_SIZE / sizeof(inode_t);
    if (bytesToRead < 4096) inodesInBlock = bytesToRead / sizeof(inode_t);

    // iterate over inodes in the block
    for (int inodeIdx = 0; inodeIdx < inodesInBlock; inodeIdx ++) {
      inode_t currInode = inodes[block * (UFS_BLOCK_SIZE / sizeof(inode_t)) + inodeIdx];
      memcpy(writeBuf + inodeIdx * sizeof(inode_t), &currInode, sizeof(inode_t));
    }
    int blockNum = super->inode_region_addr + block;
    this->disk->writeBlock(blockNum, writeBuf);
  }
}

int LocalFileSystem::lookup(int parentInodeNumber, string name) {
  inode_t parentInode;
  stat(parentInodeNumber, &parentInode);
  if (parentInode.type != UFS_DIRECTORY) {
    return -1;
  }

  for (int i = 0; i < DIRECT_PTRS; i ++) {
    // cout << "reading ptr " << i << " to block " << parentInode.direct[i] << endl;
    if (parentInode.direct[i] < 0 || parentInode.direct[i] > MAX_FILE_SIZE) {
      // cout << "block num less than 1" << endl;
      break;
    }
    char buffer[4096];
    this->disk->readBlock(parentInode.direct[i], buffer);
    dir_ent_t entry; 

    for (int j = 0; j < 4096; j += sizeof(dir_ent_t)) {
      memcpy(&entry, buffer + j, sizeof(dir_ent_t));
      // cout << entry.name << ", ";
      
      if (strcmp(entry.name, name.c_str()) == 0) {
        // cout << "found " << entry.name << ", inode number = " << entry.inum << endl; 
        return entry.inum;
      }
    }
  }
  
  return -ENOTFOUND;
}

int LocalFileSystem::stat(int inodeNumber, inode_t *inode) {
  super_t super;
  readSuperBlock(&super);
  int numInodes = super.num_inodes;
  inode_t inodes[numInodes];
  readInodeRegion(&super, inodes);
  memcpy(inode, &inodes[inodeNumber], sizeof(inode_t));
  return 0;
}

bool bitIsSet(int index, unsigned char *bitmap) {
  int byte = index / 8; 
  int offset = index % 8;
  int chosenByte = bitmap[byte];
  int bit = chosenByte >> (8 - offset - 1);
  if (bit == 1) return true;
  return false;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size) {
  // todo: fix this super janky implementation it's not even correct

  super_t super;
  readSuperBlock(&super);
  int numInodes = super.num_inodes;

  inode_t inodes[numInodes];
  readInodeRegion(&super, inodes);
  inode_t inode = inodes[inodeNumber];

  char* charBuffer = static_cast<char*>(buffer);
  // cout << "size of charBuffer is " << sizeof(charBuffer) << endl;
  // cout << charBuffer << endl;

  int bytesToRead = inode.size;
  int blocksToRead = bytesToRead / 4096;
  if (bytesToRead % 4096) blocksToRead ++;

  // cout << "reading " << bytesToRead << " bytes from " << blocksToRead << " blocks" << endl;

  for (int i = 0; i < blocksToRead; i ++) {
    char blockBuffer[4096];
    this->disk->readBlock(inode.direct[i], blockBuffer);
    int readingBytes = 4096;
    if (bytesToRead < 4096) readingBytes = bytesToRead;
    // cout << readingBytes << " from block " << inode.direct[i] << endl;
    memcpy(charBuffer + i * 4096, blockBuffer, readingBytes);
    // cout << charBuffer << endl;
    bytesToRead -= readingBytes;
  }

  // cout << charBuffer << endl;
  // cout << "done with read" << endl;

  return size;
}

void printBitmapBinary(unsigned char *bitmap, int bytes) {
  for (int i = 0; i < bytes; i ++) {
    cout << (unsigned int) bitmap[i] << " ";
  }

  cout << endl;
}

int firstEmptyBit(unsigned char *bitmap, int bytes) {
  for (int i = 0; i < bytes; i ++) {
    unsigned int curr = bitmap[i];
    int pos = 0;
    while (curr % 2) {
      pos += 1;
      curr >>= 1;
    }
    if (pos != 8) return pos + i * 8;
  }
  return -1;
}

void setBitmapBit(unsigned char *bitmap, int pos, int value) {
  int byte = pos / 8;
  int offset = pos % 8;
  unsigned int curr = bitmap[byte];
  int currBitVal = (curr >> offset) % 2;
  int bitPlace = 1 << offset;
  if (currBitVal == 1 && value == 0) {
    bitmap[byte] -= bitPlace;
  } else if (currBitVal == 0 && value == 1) {
    bitmap[byte] += bitPlace;
  }
}

void setEntryName(dir_ent_t *entry, string name) {
  for (int i = 0; i < (int) name.length(); i ++) {
    entry->name[i] = name[i];
  }
  entry->name[name.length()] = '\0';
}

// todo: consider case where direct pointers are full
// todo: consider case where all inodes are used
// todo: consider case where all datas are used

int LocalFileSystem::create(int parentInodeNumber, int type, string name) {
  // cout << "starting create" << endl;
  super_t super;
  readSuperBlock(&super);
  int numInodes = super.num_inodes;

  inode_t inodes[numInodes];
  readInodeRegion(&super, inodes);

  int existingInodeNum = lookup(parentInodeNumber, name);
  if (existingInodeNum >= 0) {
    inode_t existingInode = inodes[existingInodeNum];
    if (existingInode.type == type) {
      return 0;
    } else {
      if (type == UFS_DIRECTORY) {
        cerr << "Error creating directory" << endl;
      } else {
        cerr << "Error creating file" << endl;
      }
      return 1;
    }
  }

  unsigned char dataBitmap[super.num_data / 8];
  readDataBitmap(&super, dataBitmap);  // this is modifying inodes??

  // get first unallocated bit in inode bitmap
  unsigned char inodeBitmap[super.num_inodes / 8];
  readInodeBitmap(&super, inodeBitmap);

  int freeInodeNum = firstEmptyBit(inodeBitmap, super.num_inodes);

  // set up inode
  setBitmapBit(inodeBitmap, freeInodeNum, 1);
  writeInodeBitmap(&super, inodeBitmap); 
  inode_t newInode;
  newInode.type = type;
  newInode.size = 0;

  inodes[freeInodeNum] = newInode; //

  inode_t parentInode = inodes[parentInodeNumber];
  int bytesToRead = parentInode.size;
  int blocksToRead = bytesToRead / 4096;
  if (bytesToRead % 4096) blocksToRead ++;
  // cout << "parent has " << blocksToRead << " blocks to read, total " << bytesToRead << " bytes" << endl;

  int wbBlock = -1;  // parent directory's data block that we will write into
  char blockBuffer[4096];
  int newEntryOffset = 0;

  // iterate through parent's direct pointers
  for (int i = 0; i < DIRECT_PTRS; i ++) {
    if (wbBlock != -1)  break;

    // direct pointer not allocated: previous direct pointers' blocks were full
    if (i >= blocksToRead) {
      // allocate new data block for direct pointer to point to:
      int freeDataBit = firstEmptyBit(dataBitmap, super.num_data);  // get first unallocated bit in data bitmap
      setBitmapBit(dataBitmap, freeDataBit, 1);  // set data bitmap bit (allocate block)
      writeDataBitmap(&super, dataBitmap);
      int newDirectoryBlockNum = freeDataBit + super.data_region_addr;
      // cout << "parent direct pointer " << i << " not set, allocating block " << newDirectoryBlockNum;

      // read in our new block
      this->disk->readBlock(newDirectoryBlockNum, blockBuffer);

      wbBlock = newDirectoryBlockNum;
      break;
    }
    
    // read in the block pointed to by this direct pointer
    this->disk->readBlock(parentInode.direct[i], blockBuffer);

    // iterate through directory entries in the block
    for (int j = 0; j < 4096; j += sizeof(dir_ent_t)) {
      // reached unallocated directory entry, add directory entry to this block
      if (i * 4096 + j >= bytesToRead) {
        newEntryOffset = j;
        wbBlock = parentInode.direct[i];
        break;
      }
    }
  }

  if (wbBlock == -1) {
    cerr << "how am i here" << endl;
    return 1;
  }

  dir_ent_t newEntry;
  setEntryName(&newEntry, name);
  newEntry.inum = freeInodeNum;
  memcpy(blockBuffer + newEntryOffset, &newEntry, sizeof(dir_ent_t));
  inodes[parentInodeNumber].size += sizeof(dir_ent_t);
  

  // if we made a directory, we need to add entries . and ..
  if (type == UFS_DIRECTORY) {
    // claim a data block for the directory
    int freeDataBit = firstEmptyBit(dataBitmap, super.num_data);
    setBitmapBit(dataBitmap, freeDataBit, 1);
    writeDataBitmap(&super, dataBitmap);
    int dataBlockAddr = freeDataBit + super.data_region_addr;

    // point first direct pointer to the new data block
    inodes[freeInodeNum].direct[0] = dataBlockAddr;

    // make entry for .
    dir_ent_t currDir;
    currDir.name[0] = '.';
    currDir.name[1] = '\0';
    currDir.inum = freeInodeNum;  // inode number is the directory itself's inode number

    // make entry for ..
    dir_ent_t parentDir;
    parentDir.name[0] = '.';
    parentDir.name[1] = '.';
    parentDir.name[2] = '\0';
    parentDir.inum = parentInodeNumber; // inode number is parent inode number

    // add entries to the new data block
    char dataBuffer[UFS_BLOCK_SIZE];
    memcpy(dataBuffer, &currDir, sizeof(dir_ent_t));
    memcpy(dataBuffer + sizeof(dir_ent_t), &parentDir, sizeof(dir_ent_t));
    this->disk->writeBlock(dataBlockAddr, dataBuffer);

    // increase size of the new directory in its inode
    inodes[freeInodeNum].size = 2 * sizeof(dir_ent_t);
  }

  writeInodeRegion(&super, inodes);
  this->disk->writeBlock(wbBlock, blockBuffer);  // write into parent directory's data

  return 0;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
  const char* readBuffer = static_cast<const char*>(buffer);

  // calculate how many blocks needed
  int blocksNeeded = size / UFS_BLOCK_SIZE;
  if (size % UFS_BLOCK_SIZE) blocksNeeded ++;

  super_t super;
  readSuperBlock(&super);

  inode_t inodes[super.num_inodes];
  readInodeRegion(&super, inodes);
  inode_t fileInode = inodes[inodeNumber];

  // check how many blocks currently used
  int blocksUsed = fileInode.size / UFS_BLOCK_SIZE;
  if (fileInode.size % UFS_BLOCK_SIZE) blocksUsed ++;

  unsigned char dataBitmap[super.num_data / 8];
  readDataBitmap(&super, dataBitmap);

  // need to allocate more blocks
  if (blocksNeeded > blocksUsed) {
    for (int i = 0; i < blocksNeeded - blocksUsed; i ++) {
      // find first unused block
      int dataBit = firstEmptyBit(dataBitmap, super.num_data);
      int dataBlockNum = super.data_region_addr + dataBit;

      // set bitmap bit
      setBitmapBit(dataBitmap, dataBit, 1);
      writeDataBitmap(&super, dataBitmap);

      // set direct pointer
      int directIdx = blocksUsed + i;
      fileInode.direct[directIdx] = dataBlockNum;
    }
  } 
  // need to unallocate blocks
  else if (blocksNeeded < blocksUsed) {
    for (int i = 0; i < blocksUsed - blocksNeeded; i ++) {
      // find block to free
      int directIdx = blocksUsed - i;
      int dataBlockNum = fileInode.direct[directIdx];
      int dataBit = dataBlockNum - super.data_bitmap_addr;

      // set bitmap bit
      setBitmapBit(dataBitmap, dataBit, 0);
      writeDataBitmap(&super, dataBitmap);

      // free direct pointer
      fileInode.direct[directIdx] = -1;
    }
  }

  // write to the data blocks

  fileInode.size = 0;
  int bytesToWrite = size;

  // iterate over direct pointers to data blocks
  for (int directIdx = 0; directIdx < blocksNeeded; directIdx ++) {
    int blockNum = fileInode.direct[directIdx];

    int writingBytes = UFS_BLOCK_SIZE;
    if (bytesToWrite < UFS_BLOCK_SIZE) writingBytes = bytesToWrite;

    // put this block's data in a buffer
    char writeBuf[UFS_BLOCK_SIZE];
    memcpy(writeBuf, readBuffer + directIdx * UFS_BLOCK_SIZE, writingBytes);

    this->disk->writeBlock(blockNum, writeBuf);
    fileInode.size += writingBytes;
    bytesToWrite -= writingBytes;
  }

  inodes[inodeNumber] = fileInode;
  writeInodeRegion(&super, inodes);

  return 0;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name) {
  return 0;
}

