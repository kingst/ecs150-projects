#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

void printBitmap(unsigned char *bitmap, int bytes) {
  // cout << bytes << endl;
  for (int i = 0; i < bytes; i ++) {
    cout << (unsigned int) bitmap[i] << " ";
  }
  cout << endl;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << argv[0] << ": diskImageFile" << endl;
    return 1;
  }

  // Parse command line arguments
  
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  
  super_t super;
  fileSystem->readSuperBlock(&super);

  cout << "Super" << endl;
  cout << "inode_region_addr " << super.inode_region_addr << endl;
  cout << "inode_region_len " << super.inode_region_len << endl;
  cout << "num_inodes " << super.num_inodes << endl;
  cout << "data_region_addr " << super.data_region_addr << endl;
  cout << "data_region_len " << super.data_region_len << endl;
  cout << "num_data " << super.num_data << endl;
  cout << endl;

  unsigned char inodeBitmap[super.num_inodes];
  fileSystem->readInodeBitmap(&super, inodeBitmap);
  unsigned char dataBitmap[super.num_data];
  fileSystem->readDataBitmap(&super, dataBitmap);

  // cout << inodeBitmap << endl;
  // cout << dataBitmap << endl;

  cout << "Inode bitmap" << endl;
  printBitmap(inodeBitmap, super.num_inodes / 8);

  cout << endl;

  cout << "Data bitmap" << endl;
  printBitmap(dataBitmap, super.num_data / 8);

  delete fileSystem;
  delete disk;
  
  return 0;
}
