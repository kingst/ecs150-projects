# Distributed File System

In this assignment, you will be developing a working distributed file
server. We provide you with only the `gunrock_web` HTTP framework; you
have to build the rest.

The goals of this project are:
- To learn the basics of on-disk structures for file systems
- To learn about caching correctness and performance
- To learn about distributed storage systems

This project consists of three main parts: on-disk storage, a disk
block caching layer, and synchronized concurrent access. We recommend
implementing your server in this order where you ensure that you have
a solid foundation before moving on to the next part.

## Distributed File System Background

The main idea behind a distribute file system is that you can have
multiple clients access the same file system at the same time. One
popular distributed file system, which serves as inspiration for this
project, is Amazon's S3 storage system. S3 is used widely and provides
clear semantics with a simple REST/HTTP API for accessing data. With
these basics in place, S3 provides the storage layer that powers many
of the modern apps we all use every day.

Like local file systems, distributed file systems support a number of
high level file system operations. In this project you'll implement
`read()`, `write()`, `delete()`, and `move()` operations.

One difficulty with distributed file systems is that clients connect
concurrently and over a network, so dealing with multiple clients
accessing the same objects at the same time must be predictable. This
difficulty is why we provide clear semantics for all of the operations
that your file system will support.

## HTTP/REST API

In your distributed file system you will have two different entities:
`files` and `directories`. You will access these entities using
standard HTTP/REST network calls. All URL paths begin with `/ds3/`,
which defines the root of your file system.

To create or update a file, use HTTP `PUT` method where the URL
defines the file name and path and the body of your PUT request
contains the entire contents of the file. If the file already exists,
the PUT call overwrites the contents with the new data sent via the
body.

In the system directories are created implicitly. If a client PUTs a
file located at `/ds3/a/b/c.txt` and directories `a` and `b` do not
already exist, you will create them as a part of handling the
request. If one of the directories on the path exists as a file
already, like `/a/b`, then it is an error.

To read a file, you use the HTTP `GET` method, specifying the file
location as the path of your URL and your server will return the
contents of the file as the body in the response. To read a directory,
you also use the HTTP `GET` method, but directories return directory
entries. To encode directory entries, you put each directory entry on
a new line and use web form encoding, where you separate keys and
values using `=`, omitting the entries for `.` and `..`. For example,
GET on `/ds3/a/b` will return:

`type=1&name=c.txt`

And a GET on `/ds3/a/` will return:

`type=0&name=b`

Check out the `WwwFormEncodedDict` class to help creating and parsing
these key/value pairs.

To delete a file, you use the HTTP `DELETE` method, specifying the
file location as the path of your URL. To delete a directory, you also
use DELETE but if you try to delete a directory that is not empty it
is an error.

To move a file or directory, you use the HTTP `MOVE` method. Since
this HTTP method is non-standard, you need to add it to gunrock
web. For this call you specify the file or directory you want to move
in the URL and include a header name `x-ds3-destination` and set the
value to the path where you want to move your file or directory
to. The same rules creating a file and dealing with existing resources
on your destination path apply.

## On-Disk File System: A Basic Unix File System

The on-disk file system structures follow that of the
very simple file system discussed
[here](https://pages.cs.wisc.edu/~remzi/OSTEP/file-implementation.pdf). On-disk,
the structures are as follows:
- A single block (4KB) super block
- An inode bitmap (can be one or more 4KB blocks, depending on the number of inodes)
- A data bitmap (can be one or more 4KB blocks, depending on the number of data blocks)
- The inode table (a multiple of 4KB-sized blocks, depending on the number of inodes)
- The data region (some number of 4KB blocks, depending on the number of data blocks)

More details about on-disk structures can be found in the header
[ufs.h](ufs.h), which you should use. Specifically, this has a very
specific format for the super block, inode, and directory
entries. Bitmaps just have one bit per allocated unit as described in
the book.

As for directories, here is a little more detail.  Each directory has
an inode, and points to one or more data blocks that contain directory
entries. Each directory entry should be simple, and consist of 32
bytes: a name and an inode number pair. The name should be a
fixed-length field of size 28 bytes; the inode number is just an
integer (4 bytes). When a directory is created, it should contain two
entries: the name `.` (dot), which refers to this new directory's
inode number, and `..` (dot-dot), which refers to the parent
directory's inode number. For directory entries that are not yet in
use (in an allocated 4-KB directory block), the inode number should be
set to -1. This way, utilities can scan through the entries to check
if they are valid.

When your server is started, it is passed the name of the file system
image file. The image is created by a tool we provide, called `mkfs`.
It is pretty self-explanatory and can be found
[here](mkfs.c).

When booting off of an existing image, your server should read in the
superblock, bitmaps, and inode table, and keep in-memory versions of
these. When writing to the image, you should update these on-disk
structures accordingly.

One important aspect of your on-disk structure is that you need to
assume that your server can crash at any time, so all disk writes need
to leave your file system in a consistent state. To maintain
consistency you'll need to order your writes carefully to make sure
that if your system crashes your file system is always correct.

Importantly, you cannot change the file-system on-disk format.

## Bitmaps for block allocation
We use on-disk bitmaps to keep track of entries (inodes and data blocks) that
the file system has allocated. For our bitmaps for each byte, the least
significant bit (LSB) is considered the first bit, and the most significant
bit (MSB) is considered the last bit. So if the first bit of a two byte
bitmap is set, it will look like this in hex:

```
byte position:  0  1
hex value:     01 00

bit position   0                               15
bit value:     1 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0
```

and if the last bit is set it will look like this in hex:

```
byte position:  0  1
hex value:     00 80

bit position   0                               15
bit value:     0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 1
```

## Disk write ordering for correctness
To read data, you are welcome to make extra disk reads in and make them in any
order to implement your file system functions. In fact, to help simplify your
implementation we encorage you to read the entire inode table (region), data
bitmap, and inode bitmap structures when you need to access these. Although these
structures can span multiple disk blocks, to help simplify your implementation
we encourage you to read these data structures in their entirety when you need
to use them.

For disk writes, however, the order that you write data to disk is extremely
important for correctness. As a general principle, you need to make sure that
your file system is always in a consistent state after ALL disk writes. You can
get a crash at any time, so making sure that your disk is always consistent and
correct is an important part of this project.

Some important points of consistency are (1) making sure that all blocks in use
are marked as being allocated in the inode and data bitmaps and (2) All directories
have two default entires, "." and ".." which refer to itself and its parent directory
respectively.

In our system we don't have a notion of appending or modifying data, conceptually we
overwrite all data when we store something in our system. A sound strategy is to
create new data and then as a final write update references so that they point to
this new data.

## Caching for performance

On modern computer systems, disk I/O is slow relative to the
computation happening on a system. Every time you read a file you need
to do two I/O operations for each level in the file hierarchy just
to read the file.

To improve the performance of our file system, you will implement a
*least recently used (LRU)* in-memory cache for disk blocks. Each time
you read or write a disk block, you will add it to your cache. Your
cache will be a fixed size, specified on the command line of your
server, and once your cache becomes full you will evict existing
entries to make room for new blocks. The way that you pick the block
to evict is by keeping track of which block in your cache was used
least recently. Modern file systems use a more flexible method of
caching, but for this project we'll stick with this basic cache
allocation and policy to keep things simple.

Although many modern file systems buffer disk writes, we will store
any file system modifications immediately when the state of the file
system changes.

## Concurrent requests

On your file system you will enable concurrent requests using a
version of a reader/writer locking. All requests to the server must
start in the order that their connections were `accepted` by the
server, but they can finish in any order and you should maximize
concurrency when possible. To support threads you will use a thread
pool.

You can have concurrent `read` requests, but `write`, `delete`, and
`move` all modify the file system and you must enforce mutual
exclusion on your file system. Since all requests start in order, you
must wait until any pending requests finish before executing a file
system modification operation. While it executes, it must complete its
file system updates before allowing any subsequent requests to run.
