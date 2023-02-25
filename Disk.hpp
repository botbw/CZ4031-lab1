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
    int poolSize;
    // total pool size in our simulated disk
    char *pool;
    // available T*
    queue<T *> avail;
    // allocated T*
    unordered_set<T *> allocated;
    // from node to block
    unordered_map<T *, Block *> blkOf;

public:
    Disk(int _poolSize) : poolSize{_poolSize}, pool{new char[poolSize]} {
        assert(poolSize >= sizeof(Block));
        assert(sizeof(Block) >= sizeof(T));
        int numBlock = poolSize / sizeof(Block);
        int numTPerBlk = sizeof(Block) / sizeof(T);
        Block *blkStart = (Block *) pool;
        for (int i = 0; i < numBlock; i++) {
            Block *curBlk = blkStart + i;
            T *tStart = (T *) curBlk;
            for (int j = 0; j < numTPerBlk; j++) {
                T *curT = tStart + j;
                avail.push(curT);
                blkOf[curT] = curBlk;
            }
        }
        assert(avail.size() == numBlock * numTPerBlk);
    }

    ~Disk() {
        delete[] pool;
    }

    T *allocate(const T &val) {
        if(avail.empty()) throw bad_alloc();
        T *ret = avail.front();
        avail.pop();
        new (ret) T(val);
        assert(allocated.count(ret) == 0);
        allocated.insert(ret);
        return ret;
    }

    void deallocate(T *p) {
        assert(allocated.count(p) != 0);
        allocated.erase(p);
        avail.push(p);
    }

    int countAccessed(vector<T *> node) {
        //  a set counter for blocks
        set<Block *> cntBlock;
        for (auto it = node.begin(); it != node.end(); it++) {
            Block *blk = blkOf[*it];
            cntBlock.insert(blk);
        }
        return cntBlock.size();
    }
//    int countUsed()
//    {
//        int cnt = 0;
//        for (int i = 0; i < totalCnt; i++)
//        {
//            curBlk = (Block *)pool + i;
//            if (left[curBlk] == sizeof(Block))
//            {
//                cnt++;
//            }
//        }
//        return cnt;
//    }
};

#endif // BPTREE_DISK_HPP
