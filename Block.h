#ifndef BPTREE_BLOCK_H
#define BPTREE_BLOCK_H

// each block is 200 bytes (char)
const int BLOCK_SIZE = 200;
// use MB instead of MiB
const int DISK_MEM_SIZE = 500 * 1000 * 1000;
using Block = char[BLOCK_SIZE];

#endif // BPTREE_BLOCK_H
