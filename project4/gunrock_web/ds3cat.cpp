#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << argv[0] << ": diskImageFile inodeNumber" << endl;
    return 1;
  }

  // Parse command line arguments
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int inodeNumber = stoi(argv[2]);

  super_t super;
  fileSystem->readSuperBlock(&super);

  inode_t inode;
  fileSystem->stat(inodeNumber, &inode);

  if (inode.type != UFS_REGULAR_FILE) {
    cerr << "Error reading file" << endl;
    return 1;
  }
  
  // cout << inode.size << endl;
  int numBlocks = inode.size / UFS_BLOCK_SIZE;
  if (inode.size % UFS_BLOCK_SIZE) numBlocks ++;

  cout << "File blocks" << endl;
  for (int i = 0; i < numBlocks; i ++) {
    cout << inode.direct[i] << endl;
  }
  cout << endl;

  cout << "File data" << endl;
  char dataBuffer[inode.size];
  // cout << "size of dataBuffer: " << sizeof(dataBuffer) << endl;
  fileSystem->read(inodeNumber, dataBuffer, inode.size);

  for (int i = 0; i < (int) sizeof(dataBuffer); i ++) {
    cout << dataBuffer[i];
  }
  // cout << dataBuffer;

  delete fileSystem;
  delete disk;

  return 0;
}
