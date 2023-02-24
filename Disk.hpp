#ifndef BPTREE_DISK_HPP
#define BPTREE_DISK_HPP

#include "Block.h"
#include <unordered_map>
#include <set>

// from 100 ~ 500 MB
const int MEM_TOTAL = 500 * 1024 * 1024;

using namespace std;

// <node, 100MB in bytes>
template <typename T, int poolSize = MEM_TOTAL>
class Disk
{
private:
    // current block
    Block *curBlk;
    // current offset in the block
    int curOffset;
    // total pool size in our simulated disk
    char *pool = new char[poolSize];
    // total number of blocks
    const static int totalCnt = int(poolSize / sizeof(Block));
    // true when the block is completely free, false when the block is partially/fully used
    bool empty[totalCnt];
    // check how much space is left of a block
    unordered_map<Block *, int> left;
    // from node to block
    unordered_map<T *, Block *> blkOf;

public:
    Disk()
    {
        for (int i = 0; i < totalCnt; i++)
        {
            curBlk = (Block *)pool + i;
            empty[i] = true;
            left[curBlk] = sizeof(Block);
        }
        blkOf.clear();
    }
    ~Disk()
    {
        delete[] pool;
    }
    T *allocate()
    {
        if (curOffset < sizeof(T)) // need to find a new block
        {
            curBlk = nullptr;
            for (int i = 0; i < totalCnt; i++)
            {
                curBlk = (Block *)pool + i;
                if (left[curBlk] >= sizeof(T))
                {
                    curOffset = sizeof(Block) - left[curBlk];
                    break;
                }
            }
            if (curBlk == nullptr)
            {
                cout << "No enough space in disk" << endl;
                exit(1);
            }
            // assign the address
            T *ret = (T *)(curBlk + curOffset);
            left[curBlk] -= sizeof(T);
            blkOf[ret] = curBlk;
            return ret;
        }
    }

    void deallocate(T *node)
    {
        Block *blk = blkOf[node];
        left[blk] += sizeof(node);
    }

    int countAccessed(vector<Block> node)
    {
        set<Block *> cntBlock;
        for (auto it = node.begin(); it != node.end(); it++)
        {
            Block *blk = blkOf[*it];
            cntBlock.insert(blk);
        }
        return cntBlock.size();
    }
    int countUsed()
    {
        int cnt = 0;
        for (int i = 0; i < totalCnt; i++)
        {
            if (!empty[i])
                cnt++;
        }
        return cnt;
    }
};
#endif // BPTREE_DISK_HPP
