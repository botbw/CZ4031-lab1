//
// Created by haoxuanwang on 20/2/23.
//

#ifndef BPTREE_DISK_HPP
#define BPTREE_DISK_HPP

#include "Block.h"
#include "unordered_map"

const int MEM_TOTAL = 500 * 1024 * 1024;
const int MEM_NODE = 200 * 1024 * 1024;
const int MEM_RECORD = 300 * 1024 * 1024;

using namespace  std;

// <node, 100MB in bytes>
template <typename T, int poolSize>
class Disk {
    Block *curBlk;
    int curOffset;
    char raw[poolSize];
    bool available[poolSize / sizeof(Block)];
    // indexing
    unordered_map<Block *, int> nth;
    // check how much space is left of a blk
    unordered_map<Block *, int> left;
    // for delete
    unordered_map<T*, Block*> blkOf;

    T* allocate() {

    }

    void deallocate() {

    }

};
#endif //BPTREE_DISK_HPP
