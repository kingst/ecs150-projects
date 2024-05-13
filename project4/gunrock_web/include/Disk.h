#ifndef _DISK_H_
#define _DISK_H_

#include <string>
#include <deque>

struct UndoRecord {
  int blockNumber;
  unsigned char *blockData;
};

class Disk {
 public:
  Disk(std::string imageFile, int blockSize);
  void readBlock(int blockNumber, void *buffer);
  void writeBlock(int blockNumber, void *buffer);
  int numberOfBlocks();

  void beginTransaction();
  void commit();
  void rollback();
  
 private:
  std::string imageFile;
  int blockSize;
  int imageFileSize;
  bool isInTransaction;
  std::deque<struct UndoRecord> undoLog;
};

#endif
