#ifndef BPTREE_DISK_HPP
#define BPTREE_DISK_HPP

#include "Block.h"
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using namespace std;

template<typename T>
class Disk {
private:
    const int poolSize;
    const int numBlock;
    const int numTPerBlk;
    // total rawDisk size in our simulated disk
    char *rawDisk;
    // available T*
    queue<T *> unallocated;
    // allocated T*
    unordered_set<T *> allocated;
    // from node to block
    unordered_map<T *, Block *> blkOf;
    // the index of first available block
    int blkId;
    // assume cache is one block
    Block cache;

public:
    Disk(int _poolSize) : poolSize{_poolSize}, numBlock{poolSize / (int) sizeof(Block)},
                          numTPerBlk{sizeof(Block) / sizeof(T)}, rawDisk{new char[poolSize]}, blkId{0} {
        assert(poolSize >= sizeof(Block));
        assert(sizeof(Block) >= sizeof(T));
    }

    ~Disk() {
        delete[] rawDisk;
    }

    T *allocate(const T &val) {
        if (unallocated.empty()) { // no available space, try to lazily get blocks
            if (blkId == numBlock)
                throw runtime_error("No enough space in the disk!"); // no unallocated mem and no more block to use
            // if there are blocks left
            Block *blkStart = (Block *) rawDisk;
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

    vector<T*> getAllocated() const {
        vector<T*> res(allocated.begin(), allocated.end());
        return res;
    }

    pair<int,vector<T>> accessRecordFromDisk(const vector<T *> &pTs) {
        //  a set counter for blocks
        unordered_map<Block *, vector<int>> diskMapping;
        for (auto pT: pTs) {
            Block *pBlk = blkOf[pT];
            T* pTSt = (T*) pBlk;
            int slotId = (int) (pT - pTSt);
            diskMapping[pBlk].push_back(slotId);
        }
        vector<T> res;
        for(auto p:diskMapping) {
            Block *blk = p.first;
            // load block to cache
            memcpy(&cache, blk, sizeof(Block));
            auto v = p.second;
            for(auto slotId: v) {
                T *pT = (T*)blk + slotId;
                res.push_back(*pT);
            }
        }
        return pair<int,vector<T>>((int) diskMapping.size(), res);
    }

    int getAllocatedBlock() {
        unordered_set<Block *> cntBlock;
        for (auto pT: allocated)
            cntBlock.insert(blkOf[pT]);
        return cntBlock.size();
    }

    // in bytes
    int getAllocatedMem() {
        return allocated.size() * sizeof(T);
    }

    int getNumRecordsPerBlock() {
        return sizeof(Block) / sizeof(T);
    }
};

#endif // BPTREE_DISK_HPP
