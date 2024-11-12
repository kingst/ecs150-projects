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

// Note: If a function invocation has more than one error, return
// whichever error makes the most sense in your implementation and
// it will be considered correct.

// the operation failed because there wasn't enough space on the disk
#define ENOTENOUGHSPACE    (1)
// Unlinking a directory that is _not_ empty
#define EDIRNOTEMPTY       (2)
// The inode number is invalid
#define EINVALIDINODE      (3)
// The inode is valid but not allocated
#define ENOTALLOCATED      (4)
// The `size` for a read or write is invalid
#define EINVALIDSIZE       (5)
// Attempting to write to a directory
#define EWRITETODIR        (6)
// Lookup an entity that does not exist
#define ENOTFOUND          (7)
// Invalid name
#define EINVALIDNAME       (8)
// Creating an entity that exists with a different type or writing to a directory
#define EINVALIDTYPE       (9)
// Unlinking '.' or '..'
#define EUNLINKNOTALLOWED  (10)

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
   * Failure: return -ENOTFOUND, -EINVALIDINODE.
   * Failure modes: invalid parentInodeNumber, name does not exist.
   */
  int lookup(int parentInodeNumber, std::string name);

  /**
   * Read an inode.
   *
   * Given an inodeNumber this function will fill in the `inode` struct with
   * the type of the entry and the size of the data, in bytes, and direct blocks.
   *
   * Success: return 0
   * Failure: return -EINVALIDINODE
   * Failure modes: invalid inodeNumber
   */
  int stat(int inodeNumber, inode_t *inode);
  
  /**
   * Makes a file or directory.
   *
   * Makes a file (type == UFS_REGULAR_FILE) or directory (type == UFS_DIRECTORY)
   * in the parent directory specified by parentInodeNumber of name name.
   *
   * Success: return the inode number of the new file or directory
   * Failure: -EINVALIDINODE, -EINVALIDNAME, -EINVALIDTYPE, -ENOTENOUGHSPACE.
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
   * Failure: -EINVALIDINODE, -EINVALIDSIZE, -EINVALIDTYPE.
   * Failure modes: invalid inodeNumber, invalid size, not a regular file
   * (because you can't write to directories).
   */
  int write(int inodeNumber, const void *buffer, int size);

  /**
   * Read the contents of a file or directory.
   *
   * Reads up to `size` bytes of data into the buffer from file specified by
   * inodeNumber. The routine should work for either a file or directory;
   * directories should return data in the format specified by dir_ent_t.
   *
   * Success: number of bytes read
   * Failure: -EINVALIDINODE, -EINVALIDSIZE.
   * Failure modes: invalid inodeNumber, invalid size.
   */
  int read(int inodeNumber, void *buffer, int size);

  /**
   * Remove a file or directory.
   *
   * Removes the file or directory name from the directory specified by
   * parentInodeNumber.
   *
   * Success: 0
   * Failure: -EINVALIDINODE, -EDIRNOTEMPTY, -EINVALIDNAME, -EUNLINKNOTALLOWED
   * Failure modes: parentInodeNumber does not exist, directory is NOT
   * empty, or the name is invalid. Note that the name not existing is NOT
   * a failure by our definition. You can't unlink '.' or '..'
   */
  int unlink(int parentInodeNumber, std::string name);
  
  /**
   * Some helper functions that you need to implement and use in your
   * implementation of the higher-level functions. When you operate on
   * file system metadata, you must read/write the entire structure instead
   * of trying to identify individual disk blocks and accessing only these.
   */
  void readSuperBlock(super_t *super);

  // Helper functions, you should read/write the entire inode and bitmap regions
  void readInodeBitmap(super_t *super, unsigned char *inodeBitmap);
  void writeInodeBitmap(super_t *super, unsigned char *inodeBitmap);
  void readDataBitmap(super_t *super, unsigned char *dataBitmap);
  void writeDataBitmap(super_t *super, unsigned char *dataBitmap);
  void readInodeRegion(super_t *super, inode_t *inodes);
  void writeInodeRegion(super_t *super, inode_t *inodes);

  // Normally we'd mark this as private but we expose it so that you can access
  // it in a function you add that is not part of the LocalFileSystem object but
  // can still access the disk.
  Disk *disk;
};  

#endif
