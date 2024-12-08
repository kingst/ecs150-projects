#include <iostream>
#include <string>
#include <vector>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

// 21: rc of 1
// 22: rc of 1
int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << argv[0] << ": diskImageFile parentInode fileName" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " a.img 0 a.txt" << endl;
    return 1;
  }

  // Parse command line arguments
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int parentInode = stoi(argv[2]);
  string fileName = string(argv[3]);

  int ret = fileSystem->create(parentInode, UFS_REGULAR_FILE, fileName);

  delete fileSystem;
  delete disk;

  if (ret < 0) {
    cerr << "Error creating file" << endl;
    return 1;
  }
  
  return 0;
}
