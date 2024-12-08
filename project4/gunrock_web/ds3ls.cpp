#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include "StringUtils.h"
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

vector<string> splitStringByDelimiter(string s, string delimiter) {
  vector<string> substrings;
  int idx = 0;
  while (idx < (int) s.size()) {
    idx = s.find(delimiter);
    if (idx == -1) break;
    string substring = s.substr(0, idx);
    if (substring.size() > 0) substrings.push_back(substring);
    s.erase(0, idx + delimiter.size());
  }
  if (s.length() != 0) substrings.push_back(s);
  return substrings;
}


// Use this function with std::sort for directory entries
bool compareByName(const dir_ent_t& a, const dir_ent_t& b) {
    return std::strcmp(a.name, b.name) < 0;
}


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << argv[0] << ": diskImageFile directory" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img /a/b" << endl;
    return 1;
  }

  // parse command line arguments
  
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  string directory = string(argv[2]);
  
  vector<string> pathDirectories = splitStringByDelimiter(directory, "/");

  int currInodeNum = 0;
  for (int i = 0; i < (int) pathDirectories.size(); i ++) {
    currInodeNum = fileSystem->lookup(currInodeNum, pathDirectories[i]);
    if (currInodeNum == -ENOTFOUND) {
      cerr << "Directory not found" << endl;
      return 1;
    }
  }

  super_t super;
  fileSystem->readSuperBlock(&super);

  inode_t currInode ;
  int ret = fileSystem->stat(currInodeNum, &currInode);

  if (ret == -1) {
    cerr << "Directory not found" << endl;
  }

  if (currInode.type == UFS_REGULAR_FILE) {
    cout << currInodeNum << "\t" << pathDirectories.back() << endl;
    return 0;
  }

  // cout << "here" << endl;

  // cout << "got to directory " << pathDirectories.back() << " with inode number " << currInodeNum << endl;
  // currInode is now the target directory

  vector<dir_ent_t> files;  // files in the directory
  int inodeSize = currInode.size;  // 
  char buffer[inodeSize];
  fileSystem->read(currInodeNum, buffer, inodeSize);  //

  int bytesToRead = currInode.size;  // this will be decremented as we read bytes
  // cout << "this directory has " << bytesToRead << " bytes to read" << endl;
  // cout << bytesToRead / sizeof(dir_ent_t) << " entries." << endl;

  int blocksToRead = inodeSize / 4096;  // number of blocks this inode uses
  if (currInode.size % 4096) blocksToRead ++; 

  // iterate over direct pointers
  for (int i = 0; i < blocksToRead; i ++) {

    // shouldn't get here
    if (currInode.direct[i] < 0 || currInode.direct[i] > MAX_FILE_SIZE) {
      break;
    }

    // read the block the direct pointer points to
    char blockBuffer[4096];
    disk->readBlock(currInode.direct[i], blockBuffer);

    // iterate over directory entries in the block
    for (int j = 0; j < 4096; j += sizeof(dir_ent_t)) {
      dir_ent_t entry; 
      memcpy(&entry, blockBuffer + j, sizeof(dir_ent_t));  // read in the entry
      files.push_back(entry);
      bytesToRead -= sizeof(dir_ent_t);
      if (bytesToRead <= 0) break;
    }
  }

  sort(files.begin(), files.end(), compareByName);

  for (int i = 0; i < (int) files.size(); i ++) {
    cout << files[i].inum << "\t" << files[i].name << endl;
  }

  delete fileSystem;
  delete disk;
  
  return 0;
}
