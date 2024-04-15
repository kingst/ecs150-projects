#ifndef _LOCAL_FILE_SYSTEM_H_
#define _LOCAL_FILE_SYSTEM_H_

#include <string>

#include "Disk.h"
#include "ufs.h"

/**
 * The local file system interface.
 *
 * Implement this class so that your server can access the local file system that
 * you will implement. This class uses our Disk class for accessing disk blocks
 * on the underlying storage stack.
 *
 * One important aspect of this interface is that the buffers and sizes that
 * callers operate will not align on disk block boundaries, so your job is
 * to manage the interactions with the underlying storage to provide a higher
 * level of abstraction for any code that uses this class.
 */

// TODO: let's move away from returning -1 and have explicit error codes instead

class LocalFileSystem {
 public:
  LocalFileSystem(Disk *disk);
  /**
   * Lookup an inode.
   *
   * Takes the parent inode number (which should be the inode number
   * of a directory) and looks up the entry name in it. The inode
   * number of name is returned.
   *
   * Success: return inode number of name
   * Failure: return -1.
   * Failure modes: invalid parentInodeNumber, name does not exist.
   */
  int lookup(int parentInodeNumber, std::string name);

  /**
   * Makes a file or directory.
   *
   * Makes a file (type == UFS_REGULAR_FILE) or directory (type == UFS_DIRECTORY)
   * in the parent directory specified by parentInodeNumber of name name.
   *
   * Success: return the inode number of the new file or directory
   * Failure: -1
   * Failure modes: parentInodeNumber does not exist, or name is too long.
   * If name already exists and is of the correct type, return success, but
   * if the name already exists and is of the wrong type, return an error.
   */
  int create(int parentInodeNumber, int type, std::string name);

  /**
   * Write the contents of a file.
   *
   * Writes a buffer of size to the file, replacing any content that
   * already exists.
   *
   * Success: number of bytes written
   * Failure: -1
   * Failure modes: invalid inodeNumber, invalid size, not a regular file
   * (because you can't write to directories).
   */
  int write(int inodeNumber, const void *buffer, int size);

  /**
   * Read the contents of a file or directory.
   *
   * Reads up to MAX_FILE_SIZE bytes of data into the buffer from file specified by
   * inodeNumber. The routine should work for either a file or directory;
   * directories should return data in the format specified by dir_ent_t.
   *
   * Success: number of bytes read
   * Failure: -1
   * Failure modes: invalid inodeNumber, invalid size.
   */
  int read(int inodeNumber, const void *buffer);

  /**
   * Remove a file or directory.
   *
   * Removes the file or directory name from the directory specified by
   * parentInodeNumber.
   *
   * Success: 0
   * Failure: -1
   * Failure modes: parentInodeNumber does not exist, directory is NOT
   * empty. Note that the name not existing is NOT a failure by our definition.
   */
  int unlink(int parentInodeNumber, std::string name);

  /**
   * Some helper functions that you need to implement and use in your
   * implementation of the higher-level functions. When you operate on
   * file system metadata, you must read/write the entire structure instead
   * of trying to identify individual disk blocks and accessing only these.
   */
  void readSuperBlock(Disk *disk, super_t *super);
  void readInodeBitmap(Disk *disk, super_t *super, unsigned char *inodeBitmap);
  void writeInodeBitmap(Disk *disk, super_t *super, unsigned char *inodeBitmap);
  void readDataBitmap(Disk *disk, super_t *super, unsigned char *dataBitmap);
  void writeDataBitmap(Disk *disk, super_t *super, unsigned char *dataBitmap);
  void readInodeRegion(Disk *disk, super_t *super, inode_t *inodes);
  void writeInodeRegion(Disk *disk, super_t *super, inode_t *inodes);
  
 private:
  Disk *disk;
  
};  

#endif
