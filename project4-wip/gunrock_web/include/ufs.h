#ifndef __ufs_h__
#define __ufs_h__

#define UFS_DIRECTORY (0)
#define UFS_REGULAR_FILE (1)

#define UFS_ROOT_DIRECTORY_INODE_NUMBER (0)

#define UFS_BLOCK_SIZE (4096)

#define DIRECT_PTRS (30)

#define MAX_FILE_SIZE (DIRECT_PTRS * UFS_BLOCK_SIZE)

// Note: Bitmap indexes identify disk blocks relative to the start of a region.

typedef struct {
    int type;   // UFS_DIRECTORY or UFS_REGULAR
    int size;   // bytes
    unsigned int direct[DIRECT_PTRS];
} inode_t;

#define DIR_ENT_NAME_SIZE (28)
typedef struct {
    char name[DIR_ENT_NAME_SIZE];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} dir_ent_t;

// presumed: block 0 is the super block
typedef struct __super {
    int inode_bitmap_addr; // block address (in blocks)
    int inode_bitmap_len;  // in blocks
    int data_bitmap_addr;  // block address (in blocks)
    int data_bitmap_len;   // in blocks
    int inode_region_addr; // block address (in blocks)
    int inode_region_len;  // in blocks
    int data_region_addr;  // block address (in blocks)
    int data_region_len;   // in blocks
    int num_inodes;        // just the number of inodes
    int num_data;          // and data blocks...
} super_t;


#endif // __ufs_h__
