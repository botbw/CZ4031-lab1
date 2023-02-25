#ifndef BPTREE_DISK_HPP
#define BPTREE_DISK_HPP

#include "Block.h"
#include <unordered_map>
#include <set>

// from 100 ~ 500 MB
const int MEM_TOTAL = 500 * 1024 * 1024;

using namespace std;
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
    // total slots in a block
    const static int totalSlot = int(sizeof(Block) / sizeof(T));
    // check how much space is left of a block
    unordered_map<Block *, int> left;
    // get empty slots in a block, false means empty, true means occupied
    unordered_map<Block *, bool[totalSlot]> emptySlot;
    // from node to block
    unordered_map<T *, Block *> blkOf;

public:
    Disk()
    {
        curOffset = 0;
        Block *tempBlk;
        for (int i = 0; i < totalCnt; i++)
        {
            tempBlk = (Block *)pool + i;
            left[tempBlk] = sizeof(Block);
            for (int j = 0; j < totalSlot; j++)
                emptySlot[tempBlk][j] = false;
        }
        curBlk = (Block *)pool;
    }
    ~Disk()
    {
        delete[] pool;
    }
    T *allocate(const T &val)
    {
        try
        {
            if (left[curBlk] < sizeof(T)) // need to find a new block
            {
                curBlk = nullptr;
                // find a new block
                for (int i = 0; i < totalCnt; i++)
                {
                    curBlk = (Block *)pool + i;
                    if (left[curBlk] >= sizeof(T))
                    {
                        // find a new slot
                        for (int j = 0; j < totalSlot; j++)
                        {
                            if (emptySlot[curBlk][j] == 0)
                            {
                                emptySlot[curBlk][j] = 1;
                                curOffset = j * sizeof(T);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            if (curBlk == nullptr)
            {
                throw runtime_error("No enough space in the disk!");
            }
        }
        catch (runtime_error &e)
        {
            cout << e.what() << endl;
        }
        // assign the address
        T *ret = (T *)((char *)(curBlk) + curOffset);
        left[curBlk] -= sizeof(T);
        blkOf[ret] = curBlk;
        return ret;
    }

    void deallocate(T *node)
    {
        Block *blk = blkOf[node];
        left[blk] += sizeof(node);
        blkOf.erase(node);
        emptySlot[blk][curOffset / sizeof(T)] = 0;
    }

    int countAccessed(vector<T *> node)
    {
        //  a set counter for blocks
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
            curBlk = (Block *)pool + i;
            if (left[curBlk] == sizeof(Block))
            {
                cnt++;
            }
        }
        return cnt;
    }
};
#endif // BPTREE_DISK_HPP
