#ifndef BPTREE_DISK_HPP
#define BPTREE_DISK_HPP

#include "Block.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>

// from 100 ~ 500 MB
using namespace std;

template<typename T>
class Disk {
private:
    const int poolSize;
    const int numBlock;
    const int numTPerBlk;
    // total pool size in our simulated disk
    char *pool;
    // available T*
    queue<T *> unallocated;
    // allocated T*
    unordered_set<T *> allocated;
    // from node to block
    unordered_map<T *, Block *> blkOf;
    // the index of first available block
    int blkId;

public:
    Disk(int _poolSize) : poolSize{_poolSize}, numBlock{poolSize / (int) sizeof(Block)},
                          numTPerBlk{sizeof(Block) / sizeof(T)}, pool{new char[poolSize]}, blkId{0} {
        assert(poolSize >= sizeof(Block));
        assert(sizeof(Block) >= sizeof(T));
    }

    ~Disk() {
        delete[] pool;
    }

    T *allocate(const T &val) {
        if (unallocated.empty()) { // no available space, try to lazily get blocks
            if (blkId == numBlock)
                throw runtime_error("No enough space in the disk!"); // no unallocated mem and no more block to use
            // if there are blocks left
            Block *blkStart = (Block *) pool;
            Block *curBlk = blkStart + blkId;
            T *tStart = (T *) curBlk;
            for (int j = 0; j < numTPerBlk; j++) {
                T *curT = tStart + j;
                unallocated.push(curT);
                blkOf[curT] = curBlk;
            }
            blkId++;
        }
        T *ret = unallocated.front();
        unallocated.pop();
        new(ret) T(val);
        assert(allocated.count(ret) == 0);
        allocated.insert(ret);
        return ret;
    }

    void deallocate(T *p) {
        assert(allocated.count(p) != 0);
        allocated.erase(p);
        unallocated.push(p);
    }

    int countAccessed(vector<T *> node) {
        //  a set counter for blocks
        unordered_set<Block *> cntBlock;
        for (auto it = node.begin(); it != node.end(); it++) {
            Block *blk = blkOf[*it];
            cntBlock.insert(blk);
        }
        return cntBlock.size();
    }

    int countUsed() {
        return allocated.size();
    }
};

#endif // BPTREE_DISK_HPP
