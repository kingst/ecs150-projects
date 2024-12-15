#include <iostream>
#include <string>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

// test case 27 should return 0 (currently not returning 0)
int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << argv[0] << ": diskImageFile src_file dst_inode" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img dthread.cpp 3" << endl;
    return 1;
  }

  // Parse command line arguments
  
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  string srcFile = string(argv[2]);
  int dstInode = stoi(argv[3]);
  
  int fd = open(srcFile.c_str(), O_RDONLY);
  if (fd == -1) {
    cerr << "Couldn't open file" << endl;
    return 0;
  }

  int currentOffset = lseek(fd, 0, SEEK_CUR);
  int fileSize = lseek(fd, 0, SEEK_END);
  char writeBuf[fileSize];
  int idx = 0;
  
  int bytesToRead = fileSize;

  int ret;
  char readBuf[UFS_BLOCK_SIZE];

  lseek(fd, currentOffset, SEEK_SET);

  while ((ret = read(fd, readBuf, UFS_BLOCK_SIZE)) > 0) {
    if (ret == -1) {
      cerr << "Could not write to dst_file" << endl;
      return 1;
    }
    int putBytes = UFS_BLOCK_SIZE;
    if (bytesToRead < putBytes) putBytes = bytesToRead;
    memcpy(writeBuf + idx, readBuf, putBytes);
    bytesToRead -= putBytes;
    idx += putBytes;
  }
  close(fd);

  ret = fileSystem->write(dstInode, writeBuf, fileSize);

  delete fileSystem;
  delete disk;

  if (ret < 0) {
    cerr << "Could not write to dst_file" << endl;
    return 1;
  }
  
  return 0;
}
