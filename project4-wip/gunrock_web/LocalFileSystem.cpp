#include "LocalFileSystem.h"
#include "Disk.h"

LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}
