#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


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
  

 if (fileSystem->unlink(parentInode, entryName) != 0) {
  cerr << "Error unable to remove entry (look up the write string)" << endl;
  return 1;
 }

  delete fileSystem;
  delete disk;

  return 0;
}
