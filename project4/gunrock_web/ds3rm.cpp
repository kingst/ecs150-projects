#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

// test case 31 should return 0
int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << argv[0] << ": diskImageFile parentInode entryName" << endl;
    return 1;
  }

  // Parse command line arguments
  
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int parentInode = stoi(argv[2]);
  string entryName = string(argv[3]);
  
  int ret = fileSystem->unlink(parentInode, entryName);

  delete fileSystem;
  delete disk;

  if (ret < 0) {
    cerr << "Error removing entry" << endl;
    return 1;
  }

  return 0;
}
