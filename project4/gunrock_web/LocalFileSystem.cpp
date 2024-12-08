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
    int bytesToRead = bitsToRead / 8;
    if (bitsToRead % 8) bytesToRead ++;
    memcpy(inodeBitmap + i * 4096, buffer, bytesToRead);
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
    int bytesToRead = bitsToRead / 8;
    if (bitsToRead % 8) bytesToRead ++;
    memcpy(buffer, inodeBitmap + i * 4096, bytesToRead);
    this->disk->writeBlock(bitmapAddr, buffer);
    unreadBits -= bitsToRead;
  }
}

void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap) {
  int bitmapAddr = super->data_bitmap_addr;
  int bitmapBlocks = super->data_bitmap_len;
  int unreadBits = super->num_data;
  
  for (int i = 0; i < bitmapBlocks; i ++) {
    char buffer[4096];
    this->disk->readBlock(bitmapAddr + i, buffer);
    int bitsToRead = 4096;
    if (unreadBits < 4096) bitsToRead = unreadBits;
    int bytesToRead = bitsToRead / 8;
    if (bitsToRead % 8) bytesToRead ++;
    memcpy(dataBitmap + i * 4096, buffer, bytesToRead);
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
    int bytesToRead = bitsToRead / 8;
    if (bitsToRead % 8) bytesToRead ++;
    memcpy(buffer, dataBitmap + i * 4096, bytesToRead);
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

/**
 * returns first empty bit in data bitmap (first indirect block num of available data block)
 * if no available data blocks (bitmap is full), return -1
 * note: if num_data isn't multiple of 8, this can return a block num greater than num_data
 */
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

/**
 * TODO edge cases
 * [ X ] no more data blocks, create directory (need data block for . and ..)
 * [ X ] directory needs new block and there are more blocks
 * [ X ] no more data blocks but directory needs new block (implemented not tested)
 * [ X ] no more inodes, can't create file or directory
 * 
 */

int LocalFileSystem::create(int parentInodeNumber, int type, string name) {
  super_t super;
  readSuperBlock(&super);

  inode_t inodes[super.num_inodes];
  readInodeRegion(&super, inodes);
  inode_t parentInode = inodes[parentInodeNumber];

  if (name.length() < 1 || name.length() >= DIR_ENT_NAME_SIZE) {
    return -EINVALIDNAME;
  }

  // parent inode doesn't exist
  if (parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes) {
    return -EINVALIDINODE;
  }

  if (parentInode.type != UFS_DIRECTORY) {
    return -EINVALIDINODE;
  }

  int existingInodeNum = lookup(parentInodeNumber, name);

  // thing of that name already exists in parent directory
  if (existingInodeNum >= 0) {
    inode_t existingInode = inodes[existingInodeNum];
    // correct type
    if (existingInode.type == type) {
      return existingInodeNum;
    } 
    // incorrect type;
    else {
      return -EINVALIDTYPE;
    }
  }

  int dataBitmapSize = super.num_data / 8;
  if (super.num_data % 8) dataBitmapSize += 1;
  unsigned char dataBitmap[dataBitmapSize];
  readDataBitmap(&super, dataBitmap);  // this is modifying inodes??

  // get first unallocated bit in inode bitmap
  int inodeBitmapSize = super.num_inodes / 8;
  if (super.num_inodes % 8) inodeBitmapSize ++;
  unsigned char inodeBitmap[inodeBitmapSize];
  readInodeBitmap(&super, inodeBitmap);
  int freeInodeNum = firstEmptyBit(inodeBitmap, inodeBitmapSize);
  
  if (freeInodeNum < 0 || freeInodeNum >= super.num_inodes) {  // no more free inodes
    return -ENOTENOUGHSPACE;
  }

  // set up inode
  setBitmapBit(inodeBitmap, freeInodeNum, 1);
  inode_t newInode;
  newInode.type = type;
  newInode.size = 0;

  inodes[freeInodeNum] = newInode;

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
      int freeDataBit = firstEmptyBit(dataBitmap, dataBitmapSize);  // get first unallocated bit in data bitmap
      if (freeDataBit < 0 || freeDataBit >= super.num_data) {  // no more free data blocks
        return -ENOTENOUGHSPACE;
      }
      // allocate a new block to the parent
      // update data bitmap (not synced yet)
      setBitmapBit(dataBitmap, freeDataBit, 1);  // set data bitmap bit (allocate block)
      int newDirectoryBlockNum = freeDataBit + super.data_region_addr;
      // update parent direct pointer
      parentInode.direct[i] = newDirectoryBlockNum;
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

  // couldn't find block to write back to, parent is out of direct pointers
  if (wbBlock < 0) {
    return -ENOTENOUGHSPACE;
  }

  dir_ent_t newEntry;
  setEntryName(&newEntry, name);
  newEntry.inum = freeInodeNum;
  memcpy(blockBuffer + newEntryOffset, &newEntry, sizeof(dir_ent_t));
  parentInode.size += sizeof(dir_ent_t);
  
  // if we made a directory, we need to add entries . and ..
  if (type == UFS_DIRECTORY) {
    // claim a data block for the directory
    int freeDataBit = firstEmptyBit(dataBitmap, dataBitmapSize);
    if (freeDataBit < 0 || freeDataBit >= super.num_data) {  // no more free data blocks
      return -ENOTENOUGHSPACE;
    }
    setBitmapBit(dataBitmap, freeDataBit, 1);
    
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

  // sync data bitmap
  writeDataBitmap(&super, dataBitmap);

  // sync inode bitmap
  writeInodeBitmap(&super, inodeBitmap); 

  // sync inode region
  inodes[parentInodeNumber] = parentInode;
  writeInodeRegion(&super, inodes);

  // sync parent directory data region
  this->disk->writeBlock(wbBlock, blockBuffer);  // write into parent directory's data

  return freeInodeNum;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
  const char* readBuffer = static_cast<const char*>(buffer);

  super_t super;
  readSuperBlock(&super);

  inode_t inodes[super.num_inodes];
  readInodeRegion(&super, inodes);
  inode_t fileInode = inodes[inodeNumber];

  int dataBitmapSize = super.num_data / 8;
  if (super.num_data % 8) dataBitmapSize ++;
  unsigned char dataBitmap[dataBitmapSize];
  readDataBitmap(&super, dataBitmap);

  if (size < 0) {
    return -EINVALIDSIZE;
  }

  // the inode doesn't exist
  if (inodeNumber < 0 || inodeNumber >= super.num_inodes) {
    return -EINVALIDINODE;
  }

  // isn't a file (we can't write to directories)
  if (fileInode.type != UFS_REGULAR_FILE) {
    return -EINVALIDTYPE;
  }

  // calculate how many blocks needed
  int blocksNeeded = size / UFS_BLOCK_SIZE;
  if (size % UFS_BLOCK_SIZE) blocksNeeded ++;
  if (blocksNeeded > DIRECT_PTRS) blocksNeeded = DIRECT_PTRS;

  // check how many blocks currently used
  int blocksUsed = fileInode.size / UFS_BLOCK_SIZE;
  if (fileInode.size % UFS_BLOCK_SIZE) blocksUsed ++;

  // need to allocate more blocks
  if (blocksNeeded > blocksUsed) {
    for (int i = 0; i < blocksNeeded - blocksUsed; i ++) {
      // find first unused block
      int dataBit = firstEmptyBit(dataBitmap, dataBitmapSize);
      if (dataBit < 0 || dataBit >= super.num_data) {  // no more free data blocks
        break;
      }
      int dataBlockNum = super.data_region_addr + dataBit;

      // set bitmap bit
      setBitmapBit(dataBitmap, dataBit, 1);

      // set direct pointer
      int directIdx = blocksUsed + i;
      fileInode.direct[directIdx] = dataBlockNum;
    }
  } 
  // need to unallocate blocks
  else if (blocksNeeded < blocksUsed) {
    for (int i = 0; i < blocksUsed - blocksNeeded; i ++) {
      // find block to free
      int directIdx = blocksUsed - i - 1;
      int dataBlockNum = fileInode.direct[directIdx];
      int dataBit = dataBlockNum - super.data_region_addr;

      // set bitmap bit
      setBitmapBit(dataBitmap, dataBit, 0);

      // free direct pointer
      fileInode.direct[directIdx] = -1;
    }
  }

  // write to the data blocks

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
    bytesToWrite -= writingBytes;
  }

  int newFileSize = size - bytesToWrite;
  fileInode.size = newFileSize;

  inodes[inodeNumber] = fileInode;
  writeInodeRegion(&super, inodes);
  writeDataBitmap(&super, dataBitmap);

  return newFileSize;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name) {
  if (name == "." || name == "..") return -EUNLINKNOTALLOWED;

  if (name.length() < 1 || name.length() >= DIR_ENT_NAME_SIZE) return -EINVALIDNAME;

  super_t super;
  readSuperBlock(&super);
  inode_t inodes[super.num_inodes];
  readInodeRegion(&super, inodes);

  if (parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes) {
    return -EINVALIDINODE;
  }

  // get parent inode
  inode_t parentInode = inodes[parentInodeNumber];
  if (parentInode.type != UFS_DIRECTORY) {
    return -EINVALIDINODE;
  }

  // get item inode
  int inodeNum = lookup(parentInodeNumber, name);

  // item doesn't exist
  if (inodeNum < 0) {
    return 0;  // do nothing
  }
  inode_t inode = inodes[inodeNum];

  // if inode is a directory
  if (inode.type == UFS_DIRECTORY) {
    // check for contents (if directory isn't empty, it's an error)
    // empty directory has 2 entries: . and ..
    if (inode.size != 2 * sizeof(dir_ent_t)) {
      return -EDIRNOTEMPTY;
    }
  }

  // free data: data bitmap, direct pointer

  // iterate over direct pointers
  int fileBytes = inode.size;
  int fileBlocks = fileBytes / UFS_BLOCK_SIZE;
  if (fileBytes % UFS_BLOCK_SIZE) fileBlocks ++;

  int dataBitmapSize = super.num_data / 8;
  if (super.num_data % 8) dataBitmapSize ++;

  unsigned char dataBitmap[dataBitmapSize];
  readDataBitmap(&super, dataBitmap);

  for (int directIdx = 0; directIdx < fileBlocks; directIdx ++) {
    int directBlock = inode.direct[directIdx];
    
    // free block in bitmap
    int dataBit = directBlock - super.data_region_addr;
    setBitmapBit(dataBitmap, dataBit, 0);
  }

  // sync data bitmap changes
  writeDataBitmap(&super, dataBitmap);

  // set inode size to 0
  inode.size = 0;

  int inodeBitmapSize = super.num_inodes / 8;
  if (super.num_inodes % 8) inodeBitmapSize ++;

  // clear inode bitmap bit
  unsigned char inodeBitmap[inodeBitmapSize];
  readInodeBitmap(&super, inodeBitmap);
  setBitmapBit(inodeBitmap, inodeNum, 0);

  // sync inode bitmap changes
  writeInodeBitmap(&super, inodeBitmap);

  // remove dir_ent_t from parent
  // iterate over parent direct blocks
  int parentBlocks = parentInode.size / UFS_BLOCK_SIZE;
  if (parentInode.size % UFS_BLOCK_SIZE) parentBlocks ++;

  // need to remove the last block
  if (parentInode.size % UFS_BLOCK_SIZE == sizeof(dir_ent_t)) {
    
    // invalidate direct pointer
    parentInode.direct[parentBlocks - 1] = -1; // poop

    // clear bit in bitmap
    int parentBlockBit = parentBlocks - 1 + super.data_region_addr;
    setBitmapBit(dataBitmap, parentBlockBit, 0);
    writeDataBitmap(&super, dataBitmap);
  }

  for (int directIdx = 0; directIdx < parentBlocks; directIdx ++) {
    // read in block
    int blockNum = parentInode.direct[directIdx];
    char buffer[UFS_BLOCK_SIZE];
    this->disk->readBlock(blockNum, buffer);

    char newEntriesBlock[UFS_BLOCK_SIZE];
    int offset = 0;

    // iterate over directory entries in block
    for (int byteOffset = 0; byteOffset < UFS_BLOCK_SIZE; byteOffset += sizeof(dir_ent_t)) {
      
      dir_ent_t entry;
      memcpy(&entry, buffer + byteOffset, sizeof(dir_ent_t));
      if (entry.name == name) {
        offset = (int) -sizeof(dir_ent_t);
      } else {
        memcpy(newEntriesBlock + byteOffset + offset, buffer + byteOffset, sizeof(dir_ent_t));
      }
    }

    // update block
    this->disk->writeBlock(blockNum, newEntriesBlock);
  }

  // update parent size
  parentInode.size -= sizeof(dir_ent_t);

  // sync inodes
  inodes[parentInodeNumber] = parentInode;
  inodes[inodeNum] = inode;
  writeInodeRegion(&super, inodes);

  return 0;
}

