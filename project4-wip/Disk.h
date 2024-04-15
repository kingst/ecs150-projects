#ifndef _DISK_H_
#define _DISK_H_

#include <string>

class Disk {
 public:
  Disk(std::string imageFile, int blockSize);
  void readBlock(int blockNumber, void *buffer);
  void writeBlock(int blockNumber, void *buffer);
  int numberOfBlocks();

 private:
  std::string imageFile;
  int blockSize;
  int imageFileSize;
};

#endif
