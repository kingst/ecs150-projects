#include <iostream>
#include <unistd.h>

#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "Disk.h"
#include "dthread.h"

using namespace std;

Disk::Disk(string imageFile, int blockSize) {
  this->imageFile = imageFile;
  this->blockSize = blockSize;
  this->isInTransaction = false;
  
  struct stat stat;
  int imageFileDescriptor = open(imageFile.c_str(), O_RDONLY);
  if (imageFileDescriptor < 0) {
    cerr << "could not open " << imageFile << endl;
    exit(1);
  }
  int ret = fstat(imageFileDescriptor, &stat);
  if (ret != 0) {
    cerr << "Could not stat image file" << endl;
    exit(1);
  }
  close(imageFileDescriptor);
  
  this->imageFileSize = stat.st_size;

  if ((this->imageFileSize % this->blockSize) != 0 || this->blockSize == 0) {
    cerr << "Your disk image size must be a multiple of your block size" << endl;
    cerr << "  imageSize: " << this->imageFileSize << endl;
    cerr << "  blockSize: " << this->blockSize << endl;
    cerr << "  imageSize % blockSize: " << this->imageFileSize % this->blockSize << endl;
    exit(1);
  }
  
}

int Disk::numberOfBlocks() {
  return this->imageFileSize / this->blockSize;
}

void Disk::readBlock(int blockNumber, void *buffer) {
  if (blockNumber < 0 || blockNumber >= this->numberOfBlocks()) {
    cerr << "Invalid block number " << blockNumber << endl;
    exit(1);
  }

  int fd = open(this->imageFile.c_str(), O_RDONLY);
  if (fd < 0) {
    cerr << "Could not open image file " << this->imageFile << endl;
    exit(1);
  }

  int offset = blockNumber * this->blockSize;
  int ret = lseek(fd, offset, SEEK_SET);
  if (ret != offset) {
    perror("read::lseek");
    cerr << "Could not seek to file" << endl;
    exit(1);
  }

  ret = read(fd, buffer, this->blockSize);
  if (ret != this->blockSize) {
    cerr << "Could not read file" << endl;
    exit(1);
  }

  close(fd);
}

void Disk::writeBlock(int blockNumber, void *buffer) {  
  if (blockNumber < 0 || blockNumber >= this->numberOfBlocks()) {
    cerr << "Invalid block number " << blockNumber << endl;
    exit(1);
  }

  if (isInTransaction) {
    struct UndoRecord undoRecord;
    undoRecord.blockNumber = blockNumber;
    undoRecord.blockData = new unsigned char[blockSize];
    this->readBlock(blockNumber, undoRecord.blockData);
    undoLog.push_front(undoRecord);
  }
  
  int fd = open(this->imageFile.c_str(), O_RDWR);
  if (fd < 0) {
    cerr << "Could not open image file " << this->imageFile << endl;
    exit(1);
  }

  int offset = blockNumber * this->blockSize;
  int ret = lseek(fd, offset, SEEK_SET);
  if (ret != offset) {
    perror("write::lseek");
    cerr << "Could not seek to file" << endl;
    exit(1);
  }

  ret = write(fd, buffer, this->blockSize);
  if (ret != this->blockSize) {
    cerr << "Could not write file" << endl;
    exit(1);
  }
  fsync(fd);
  close(fd);
}

void Disk::beginTransaction() {
  if (isInTransaction) {
    cerr << "You can't start a new transaction: one already exists" << endl;
    exit(1);
  }
  isInTransaction = true;
}

void Disk::commit() {
  isInTransaction = false;
  deque<struct UndoRecord>::iterator iter;
  for (iter = undoLog.begin(); iter != undoLog.end(); iter++) {
    delete [] iter->blockData;
  }
  undoLog.clear();
}

void Disk::rollback() {
  isInTransaction = false;
  deque<struct UndoRecord>::iterator iter;
  for (iter = undoLog.begin(); iter != undoLog.end(); iter++) {
    this->writeBlock(iter->blockNumber, iter->blockData);
    delete [] iter->blockData;
  }
  undoLog.clear();
}
