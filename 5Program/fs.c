// ============================================================================
// fs.c - user FileSytem API
// ============================================================================

#include "bfs.h"
#include "fs.h"

// ============================================================================
// Close the file currently open on file descriptor 'fd'.
// ============================================================================
i32 fsClose(i32 fd) {
  i32 inum = bfsFdToInum(fd);
  bfsDerefOFT(inum);
  return 0;
}



// ============================================================================
// Create the file called 'fname'.  Overwrite, if it already exsists.
// On success, return its file descriptor.  On failure, EFNF
// ============================================================================
i32 fsCreate(str fname) {
  i32 inum = bfsCreateFile(fname);
  if (inum == EFNF) return EFNF;
  return bfsInumToFd(inum);
}



// ============================================================================
// Format the BFS disk by initializing the SuperBlock, Inodes, Directory and
// Freelist.  On succes, return 0.  On failure, abort
// ============================================================================
i32 fsFormat() {
  FILE* fp = fopen(BFSDISK, "w+b");
  if (fp == NULL) FATAL(EDISKCREATE);

  i32 ret = bfsInitSuper(fp);               // initialize Super block
  if (ret != 0) { fclose(fp); FATAL(ret); }

  ret = bfsInitInodes(fp);                  // initialize Inodes block
  if (ret != 0) { fclose(fp); FATAL(ret); }

  ret = bfsInitDir(fp);                     // initialize Dir block
  if (ret != 0) { fclose(fp); FATAL(ret); }

  ret = bfsInitFreeList();                  // initialize Freelist
  if (ret != 0) { fclose(fp); FATAL(ret); }

  fclose(fp);
  return 0;
}


// ============================================================================
// Mount the BFS disk.  It must already exist
// ============================================================================
i32 fsMount() {
  FILE* fp = fopen(BFSDISK, "rb");
  if (fp == NULL) FATAL(ENODISK);           // BFSDISK not found
  fclose(fp);
  return 0;
}



// ============================================================================
// Open the existing file called 'fname'.  On success, return its file
// descriptor.  On failure, return EFNF
// ============================================================================
i32 fsOpen(str fname) {
  i32 inum = bfsLookupFile(fname);        // lookup 'fname' in Directory
  if (inum == EFNF) return EFNF;
  return bfsInumToFd(inum);
}



// ============================================================================
// Read 'numb' bytes of data from the cursor in the file currently fsOpen'd on
// File Descriptor 'fd' into 'buf'.  On success, return actual number of bytes
// read (may be less than 'numb' if we hit EOF).  On failure, abort
// ============================================================================
i32 fsRead(i32 fd, i32 numb, void* buf) {
  /* get basic info of file */
  i32 inum = bfsFdToInum(fd); // convert fd to inum
  i32 cursor = bfsTell(fd); // get cursor of file
  i32 fbn = cursor / 512; // get the fbn cursor is pointing to
  i32 dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn
  i32 boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

  /* set up the file */
  FILE* fp = fopen(BFSDISK, "r"); // open file
  i32 ret  = fseek(fp, boff, SEEK_SET); // set the disk cursor to the dbn

  /* check size of numb and change if bigger than remaining file size */
  i32 size = bfsGetSize(inum); // get size of file
  i32 fileLeft = size - cursor; // get size of remaining file after cursor
  if (fileLeft < numb) // if remaining file size is less than numb
  {
    numb = fileLeft; // change numb to equal remaining file size
  }
  if (numb == 0) { // if numb originally zero, or cursor at end of file
    return 0; // return read 0 bytes of data
  }

  /* get bytes from first block */
  void* currBlock = malloc(BYTESPERBLOCK); // initialize buffer to store blocks
  i32 num = fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer
  int bufIndex = cursor % BYTESPERBLOCK; // get index of buffer where the cursor starts
  int bytesAfterCursor = BYTESPERBLOCK - bufIndex; // get num bytes after cursor in first block
  memcpy(buf, currBlock + bufIndex, bytesAfterCursor); // copy bytes into buf
  int currBufIndex = bytesAfterCursor; // set the index for buf

  /* get bytes from remaining blocks until last block */
  int readNumBlocks = numb / BYTESPERBLOCK; // get number of blocks to read after first block
  for (int i = 1; i < readNumBlocks; i++)
  {
    num = fread(buf + currBufIndex, 1, BYTESPERBLOCK, fp); // read block into buffer
    currBufIndex += BYTESPERBLOCK; // adjust buf index
  }

  /* get bytes from last block */
  if (readNumBlocks > 0) // if number of blocks to read after first block is greater than zero
  {
    num = fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer
    int bytesLeft = numb - (readNumBlocks * BYTESPERBLOCK); // calculate bytes left to read in last block
    memcpy(buf + currBufIndex, currBlock, bytesLeft); // copy bytes into buf
  }
  free(currBlock); // free curr block

  /* set cursor */
  bfsSetCursor(inum, cursor + numb); // update the cursor

  /* return */
  return numb; // return actual bytes reads
}


// ============================================================================
// Move the cursor for the file currently open on File Descriptor 'fd' to the
// byte-offset 'offset'.  'whence' can be any of:
//
//  SEEK_SET : set cursor to 'offset'
//  SEEK_CUR : add 'offset' to the current cursor
//  SEEK_END : add 'offset' to the size of the file
//
// On success, return 0.  On failure, abort
// ============================================================================
i32 fsSeek(i32 fd, i32 offset, i32 whence) {

  if (offset < 0) FATAL(EBADCURS);

  i32 inum = bfsFdToInum(fd);
  i32 ofte = bfsFindOFTE(inum);

  switch(whence) {
    case SEEK_SET:
      g_oft[ofte].curs = offset;
      break;
    case SEEK_CUR:
      g_oft[ofte].curs += offset;
      break;
    case SEEK_END: {
        i32 end = fsSize(fd);
        g_oft[ofte].curs = end + offset;
        break;
      }
    default:
        FATAL(EBADWHENCE);
  }
  return 0;
}



// ============================================================================
// Return the cursor position for the file open on File Descriptor 'fd'
// ============================================================================
i32 fsTell(i32 fd) {
  return bfsTell(fd);
}



// ============================================================================
// Retrieve the current file size in bytes.  This depends on the highest offset
// written to the file, or the highest offset set with the fsSeek function.  On
// success, return the file size.  On failure, abort
// ============================================================================
i32 fsSize(i32 fd) {
  i32 inum = bfsFdToInum(fd);
  return bfsGetSize(inum);
}



// ============================================================================
// Write 'numb' bytes of data from 'buf' into the file currently fsOpen'd on
// filedescriptor 'fd'.  The write starts at the current file offset for the
// destination file.  On success, return 0.  On failure, abort
// ============================================================================
i32 fsWrite(i32 fd, i32 numb, void* buf) {

  // ++++++++++++++++++++++++
  // Insert your code here
  // ++++++++++++++++++++++++

  FATAL(ENYI);                                  // Not Yet Implemented!
  return 0;
}
