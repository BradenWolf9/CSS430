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

  //if (numb == 1000)

  //{

    //bn--;

  //}

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



/*

  for (int i = 0; i < BYTESPERBLOCK; i++)

  {

    fprintf(stderr, "index: %d    value: %d\n", i, *((i8*)(buf + i)));

  }

*/



  /* get bytes from remaining blocks until last block *

  int readNumBlocks = numb / BYTESPERBLOCK; // get number of blocks to read after first block

  for (int i = 1; i < readNumBlocks; i++)

  {

    fread(buf + currBufIndex, 1, BYTESPERBLOCK, fp); // read block into buffer

    currBufIndex += BYTESPERBLOCK; // adjust buf index

  }

  */



  /* get bytes from remaining blocks */

  int readNumBlocks = numb / BYTESPERBLOCK; // get number of middle blocks to read

  for (int i = 1; i < readNumBlocks; i++)

  {

    fbn++;

    dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

    boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

    ret  = fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

    num = fread(buf + currBufIndex, 1, BYTESPERBLOCK, fp); // read block into buffer

    currBufIndex += BYTESPERBLOCK; // adjust buf index

  }



  /* get bytes from last block if last block if not all of last block*

  if (readNumBlocks > 0) // if number of blocks to read after first block is greater than zero

  {

    fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer

    int bytesLeft = numb - (readNumBlocks * BYTESPERBLOCK); // calculate bytes left to read in last block

    memcpy(buf + currBufIndex, currBlock, bytesLeft); // copy bytes into buf

  }

  free(currBlock); // free curr block

  */



  // if number of blocks to read after first block is greater than zero and

  // numb mod BYTESPERBLOCK is not 0 so the last block is not to be read fully

  if (readNumBlocks > 0 && numb % BYTESPERBLOCK != 0)

  {

    fbn++;

    dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

    boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

    ret  = fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

    num = fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer

    int bytesLeft = numb - currBufIndex; // calculate bytes left to read in last block

    memcpy(buf + currBufIndex, currBlock, bytesLeft); // copy bytes into buf

  }

  free(currBlock); // free curr block





  /* set cursor */

  bfsSetCursor(inum, cursor + numb); // update the cursor



  /* return */

  return numb; // return actual bytes reads

}



/*

i32 fsRead(i32 fd, i32 numb, void* buf) {

  fprintf(stderr, "numb: %d\n", numb);



  /* get basic info of file

  i32 inum = bfsFdToInum(fd); // convert fd to inum

  i32 cursor = bfsTell(fd); // get cursor of file

  i32 fbn = cursor / 512; // get the fbn cursor is pointing to

  i32 dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

  i32 boff = dbn * BYTESPERBLOCK; // get the offset of the dbn



  fprintf(stderr, "cursor: %d\n", cursor);

  fprintf(stderr, "fbn: %d\n", fbn);



  /* set up the file

  //FILE* fp = fopen(BFSDISK, "r"); // open file

  //i32 ret  = fseek(fp, boff, SEEK_SET); // set the disk cursor to the dbn



  /* check size of numb and change if bigger than remaining file size

  i32 size = bfsGetSize(inum); // get size of file

  fprintf(stderr, "size: %d\n", size);



  i32 fileLeft = size - cursor; // get size of remaining file after cursor

  if (fileLeft < numb) // if remaining file size is less than numb

  {

    numb = fileLeft; // change numb to equal remaining file size

  }

  if (numb == 0) { // if numb originally zero, or cursor at end of file

    return 0; // return read 0 bytes of data

  }



  /* get bytes from first block

  void* currBlock = malloc(BYTESPERBLOCK); // initialize buffer to store blocks

  //fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer

  bfsRead(inum, fbn, currBlock);

  int bufIndex = cursor % BYTESPERBLOCK; // get index of buffer where the cursor starts

  int bytesAfterCursor = BYTESPERBLOCK - bufIndex; // get num bytes after cursor in first block



  fprintf(stderr, "bufIndex: %d\n", bufIndex);

  fprintf(stderr, "bytesAfterCursor: %d\n", bytesAfterCursor);



  memcpy(buf, currBlock + bufIndex, bytesAfterCursor); // copy bytes into buf

  int currBufIndex = bytesAfterCursor; // set the index for buf



  /* get bytes from middle blocks

  int readNumBlocks = numb / BYTESPERBLOCK; // get number of blocks to read after first block

  for (int fbn = 1; fbn < readNumBlocks; fbn++)

  {

    //fread(buf + currBufIndex, 1, BYTESPERBLOCK, fp); // read block into buffer

    bfsRead(inum, fbn, buf + currBufIndex);

    currBufIndex += BYTESPERBLOCK; // adjust buf index

  }



  /* get bytes from last block

  if (readNumBlocks > 0) // if number of blocks to read after first block is greater than zero

  {

    fbn++;

    //fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer

    bfsRead(inum, fbn, currBlock);

    //int bytesLeft = numb - (readNumBlocks * BYTESPERBLOCK); // calculate bytes left to read in last block

    int bytesLeft = numb - currBufIndex;

    memcpy(buf + currBufIndex, currBlock, bytesLeft); // copy bytes into buf

  }

  free(currBlock); // free curr block



  /* set cursor

  bfsSetCursor(inum, cursor + numb); // update the cursor



  /* return

  return numb; // return actual bytes reads

}



*/



/*

i32 fsRead(i32 fd, i32 numb, void* buf) {

  /* get basic info of file

  i32 inum = bfsFdToInum(fd); // convert fd to inum

  i32 cursor = bfsTell(fd); // get cursor of file

  i32 fbn = cursor / 512; // get the fbn cursor is pointing to

  i32 dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

  i32 boff = dbn * BYTESPERBLOCK; // get the offset of the dbn



  /* set up the file

  FILE* fp = fopen(BFSDISK, "r"); // open file

  i32 ret  = fseek(fp, boff, SEEK_SET); // set the disk cursor to the dbn



  /* check size of numb and change if bigger than remaining file size

  i32 size = bfsGetSize(inum); // get size of file

  i32 fileLeft = size - cursor; // get size of remaining file after cursor

  if (fileLeft < numb) // if remaining file size is less than numb

  {

    numb = fileLeft; // change numb to equal remaining file size

  }

  if (numb == 0) { // if numb originally zero, or cursor at end of file

    return 0; // return read 0 bytes of data

  }



  /* get bytes from first block

  void* currBlock = malloc(BYTESPERBLOCK); // initialize buffer to store blocks

  i32 num = fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer

  int bufIndex = cursor % BYTESPERBLOCK; // get index of buffer where the cursor starts

  int bytesAfterCursor = BYTESPERBLOCK - bufIndex; // get num bytes after cursor in first block

  memcpy(buf, currBlock + bufIndex, bytesAfterCursor); // copy bytes into buf

  int currBufIndex = bytesAfterCursor; // set the index for buf



  /* get bytes from middle blocks

  int readNumBlocks = numb / BYTESPERBLOCK; // get number of middle blocks to read

  for (int fbn = 1; fbn < readNumBlocks; fbn++)

  {

    dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

    boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

    ret  = fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

    num = fread(buf + currBufIndex, 1, BYTESPERBLOCK, fp); // read block into buffer

    currBufIndex += BYTESPERBLOCK; // adjust buf index

  }



  /* get bytes from last block

  if (readNumBlocks > 0) // if number of blocks to read after first block is greater than zero

  {

    fbn++;

    dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

    boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

    ret  = fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

    num = fread(currBlock, 1, BYTESPERBLOCK, fp); // read block into curr block buffer

    int bytesLeft = numb - (readNumBlocks * BYTESPERBLOCK); // calculate bytes left to read in last block

    memcpy(buf + currBufIndex, currBlock, bytesLeft); // copy bytes into buf

  }

  free(currBlock); // free curr block



  /* set cursor

  bfsSetCursor(inum, cursor + numb); // update the cursor



  /* return

  return numb; // return actual bytes reads

}

*/



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

  /* get basic info of file */

  i32 inum = bfsFdToInum(fd); // convert fd to inum

  i32 cursor = bfsTell(fd); // get cursor of file

  i32 fbn = cursor / 512; // get the fbn cursor is pointing to

  i32 dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

  i32 boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

  i32 size = bfsGetSize(inum);

  void* currBlock = malloc(BYTESPERBLOCK); // initialize buffer to store blocks

  //FILE* fp = fopen(BFSDISK, "w"); // open file



  if (cursor > size) // outside

  {

    /* extend file and fill zero blocks */

    int originalNumBlocks = (size / BYTESPERBLOCK) + 1;

    int moreBlocks = (cursor + numb) / BYTESPERBLOCK;

    if ((cursor + numb) % BYTESPERBLOCK != 0) {

      moreBlocks++;

    }

    bfsExtend(inum, moreBlocks);

    bfsSetSize(inum, cursor + numb);

    memset(currBlock, 0, BYTESPERBLOCK);

    for (int currFbn = originalNumBlocks + 1; currFbn <= moreBlocks; currFbn++) {

      dbn = bfsFbnToDbn(inum, currFbn); // get dbn associated with fbn

      //boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

      //fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

      bioWrite(dbn, currBlock);

    }



    /* write bytes to first block */

    dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

    //boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

    //fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

    bfsRead(inum, fbn, currBlock);

    int bufIndex = cursor % BYTESPERBLOCK; // get index of buffer where the cursor starts

    int bytesAfterCursor = BYTESPERBLOCK - bufIndex; // get num bytes after cursor in first block

    if (numb < bytesAfterCursor)

    {

      memcpy(currBlock + bufIndex, buf, numb);

    }

    else

    {

      memcpy(currBlock + bufIndex, buf, bytesAfterCursor);

    }

    bioWrite(dbn, currBlock);

    int currBufIndex = bytesAfterCursor; // set the index for buf



    /* write bytes to middle blocks */

    int readNumBlocks = numb / BYTESPERBLOCK; // get number of middle blocks to write

    for (int fbn = 1; fbn < readNumBlocks; fbn++)

    {

      dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

      //boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

      //fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

      bioWrite(dbn, buf + currBufIndex);

      currBufIndex += BYTESPERBLOCK; // adjust buf index

    }



    /* write bytes to last block */

    if (readNumBlocks > 0) // if number of blocks to read after first block is greater than zero

    {

      fbn++;

      bfsRead(inum, fbn, currBlock);

      //int bytesLeft = numb % BYTESPERBLOCK;

      int bytesLeft = numb - currBufIndex;

      memcpy(currBlock, buf + currBufIndex, bytesLeft);

      dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

      //boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

      //fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

      bioWrite(dbn, currBlock);

    }

  }



  else if (size - cursor >= numb) // inside

  {

    /* write bytes to first block */

    bfsRead(inum, fbn, currBlock);

    //for (int i = 0; i < BYTESPERBLOCK; i++)

    //{

      //fprintf(stderr, "index: %d    value: %d\n", i, *((i8*)(buf + i)));

    //}

    int bufIndex = cursor % BYTESPERBLOCK; // get index of buffer where the cursor starts

    int bytesAfterCursor = BYTESPERBLOCK - bufIndex; // get num bytes after cursor in first block

    if (numb < bytesAfterCursor)

    {

      memcpy(currBlock + bufIndex, buf, numb);

    }

    else

    {

      memcpy(currBlock + bufIndex, buf, bytesAfterCursor);

    }

    bioWrite(dbn, currBlock);

    int currBufIndex = bytesAfterCursor; // set the index for buf



    /* write bytes to middle blocks */

    int readNumBlocks = numb / BYTESPERBLOCK; // get number of middle blocks to write

    for (int fbn = 1; fbn < readNumBlocks; fbn++)

    {

      dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

      bioWrite(dbn, buf + currBufIndex);

      currBufIndex += BYTESPERBLOCK; // adjust buf index

    }



    /* write bytes to last block */

    if (readNumBlocks > 0) // if number of blocks to read after first block is greater than zero

    {

      fbn++;

      bfsRead(inum, fbn, currBlock);

      int bytesLeft = numb - currBufIndex;

      memset(currBlock, 11, BYTESPERBLOCK);

      memcpy(currBlock, buf + currBufIndex, bytesLeft);

      dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

      bioWrite(dbn, currBlock);

    }

  }



  else // overlap

  {

    /* extend file */

    int originalNumBlocks = (size / BYTESPERBLOCK) + 1;

    int moreBlocks = (cursor + numb) / BYTESPERBLOCK;

    if ((cursor + numb) % BYTESPERBLOCK != 0) {

      moreBlocks++;

    }

    bfsExtend(inum, moreBlocks);

    bfsSetSize(inum, cursor + numb);



    /* write bytes to first block */

    dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

    //boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

    //fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

    bfsRead(inum, fbn, currBlock);

    int bufIndex = cursor % BYTESPERBLOCK; // get index of buffer where the cursor starts

    int bytesAfterCursor = BYTESPERBLOCK - bufIndex; // get num bytes after cursor in first block

    if (numb < bytesAfterCursor)

    {

      memcpy(currBlock + bufIndex, buf, numb);

    }

    else

    {

      memcpy(currBlock + bufIndex, buf, bytesAfterCursor);

    }

    bioWrite(dbn, currBlock);

    int currBufIndex = bytesAfterCursor; // set the index for buf



    /* write bytes to middle blocks */

    int readNumBlocks = numb / BYTESPERBLOCK; // get number of middle blocks to write

    for (int fbn = 1; fbn < readNumBlocks; fbn++)

    {

      dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

      //boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

      //fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

      bioWrite(dbn, buf + currBufIndex);

      currBufIndex += BYTESPERBLOCK; // adjust buf index

    }



    /* write bytes to last block */

    if (readNumBlocks > 0) // if number of blocks to read after first block is greater than zero

    {

      fbn++;



      bfsRead(inum, fbn, currBlock);

      //int bytesLeft = numb % BYTESPERBLOCK;

      int bytesLeft = numb - currBufIndex;

      memcpy(currBlock, buf + currBufIndex, bytesLeft);

      dbn = bfsFbnToDbn(inum, fbn); // get dbn associated with fbn

      //boff = dbn * BYTESPERBLOCK; // get the offset of the dbn

      //fseek(fp, boff, SEEK_SET); // set the disk cursor to the fbn cursor

      bioWrite(dbn, currBlock);

    }

  }



  free(currBlock); // free curr block



  /* set cursor */

  bfsSetCursor(inum, cursor + numb); // update the cursor



  return 0; // return 0 for success

}

