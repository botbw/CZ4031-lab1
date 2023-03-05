#ifndef BPTREE_DISK_HPP
#define BPTREE_DISK_HPP

#include "Block.h"
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cstring>

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
    queue<pair<int, int>> unallocated;

    // allocated T*
    vector<vector<bool>> bitmap;

    // the index of first available block
    int blkId;
    // the index of the next block to scan
    int nextScanBlkId;
    // simulate cache (size 1)
    Block cache;

    /*
     * bool bitmap[numBlock][numTPerBlk]
     *                  i       j
     *                  (i, j)
     *
     *
     *     unallocated -> T*
     *     T* -> (i, j) -> bitmap[i][j] = true
     *
     *
     *     deallocate(T *)
     *
     *     T* -> (i, j) -> bitmap[i][j] = false;
     *
     *
     *     r* -> blkOf(r*) -> available slots
     *     blkId
     *     bitmap[blkId]
     *
     *
     *
     */

public:
    Disk(int _poolSize) : poolSize{_poolSize}, numBlock{poolSize / (int) sizeof(Block)},
                          numTPerBlk{sizeof(Block) / sizeof(T)}, rawDisk{new char[poolSize]},
                          bitmap(numBlock, vector<bool>(numTPerBlk)), blkId{0}, nextScanBlkId{0} {
        assert(poolSize >= sizeof(Block));
        assert(sizeof(Block) >= sizeof(T));
    }

    Disk(const Disk &b) : poolSize{b.poolSize}, numBlock{b.numBlock},
                          numTPerBlk{b.numTPerBlk}, rawDisk{new char[poolSize]},
                          unallocated{b.unallocated}, bitmap{b.bitmap}, blkId{b.blkId},
                          nextScanBlkId{0} {
        memcpy(this->rawDisk, b.rawDisk, sizeof(char) * poolSize);
    }

    Disk &operator=(const Disk &b) = delete;

    ~Disk() {
        delete[] rawDisk;
    }

    // map from node* to block*
    inline Block *blkOf(T *r) {
        char *pr = (char *) r;
        int id = int(pr - rawDisk) / sizeof(Block);
        Block *st = (Block *) rawDisk;
        return st + id;
    }

    inline pair<int, int> getBitMapId(T *pT) {
        Block *pBlk = blkOf(pT);
        int i = pBlk - (Block *) rawDisk;
        int j = pT - (T *) pBlk;
        return {i, j};
    }

    inline T *getTAddFromId(int i, int j) {
        Block *blkSt = (Block *) rawDisk + i;
        T *tSt = (T *) blkSt + j;
        return tSt;
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
                unallocated.push(getBitMapId(curT));
            }
            blkId++;
        }

        auto tmp = unallocated.front();
        int i = tmp.first, j = tmp.second;

        T *ret = getTAddFromId(i, j);
        unallocated.pop();

        new(ret) T(val);

        assert(bitmap[i][j] == false);
        bitmap[i][j] = true;

        return ret;
    }

    void deallocate(T *p) {
        auto tmp = getBitMapId(p);
        int i = tmp.first, j = tmp.second;
        assert(bitmap[i][j] == true);
        bitmap[i][j] = false;
        unallocated.push(tmp);
    }

    // blk = blkOf(T*)
    vector<T> getAllFromBlock(Block *blk) {
        // read into cache
        memcpy(&cache, blk, sizeof(cache));
        T *pT = (T *) blk;
        auto tmp = getBitMapId(pT);
        int i = tmp.first;

        vector<T> res;

        T *tSt = (T *) cache;

        for (int j = 0; j < numTPerBlk; j++) {
            if (bitmap[i][j]) res.push_back(*(tSt + j));
        }
        return res;
    }


    int getAllocatedBlock() {
        int cnt = 0;
        for (int i = 0; i < blkId; i++) {
            bool allocated = false;
            for (int j = 0; j < numTPerBlk; j++) {
                if (bitmap[i][j]) {
                    allocated = true;
                    break;
                }
            }
            if (allocated) cnt++;
        }
        return cnt;
    }

    void innitializeScan() {
        nextScanBlkId = 0;
    }

    vector<T> linearScanNextBlk() {
        vector<T> ret;
        bool allocated = false;

        Block *blkStart = (Block *) rawDisk;
        Block *curBlk = blkStart + nextScanBlkId;

        do {
            if (nextScanBlkId == blkId) return vector<T>();

            curBlk = blkStart + nextScanBlkId;
            int i = nextScanBlkId;

            for (int j = 0; j < numTPerBlk; j++) {
                if (bitmap[i][j]) {
                    allocated = true;
                    break;
                }
            }

            nextScanBlkId++;
        } while (!allocated);

        return getAllFromBlock(curBlk);
    }

    //vjs are indexes of the T in the just return vector<T>
    void deleteFromLastScanedBlkForLinearScan(unordered_set<int> vjs) {
        int i = nextScanBlkId - 1;
        int cnt_vj = -1;
        vector<int> js;

        //convert vj to j in bitmap[i][j]
        for (int j = 0; j < numTPerBlk; j++) {
            if (bitmap[i][j]) {
                cnt_vj++;

                if (vjs.count(cnt_vj) > 0) js.push_back(j);
            }
        }

        for (auto j: js) {
            T *pT = getTAddFromId(i, j);
            deallocate(pT);
        }
    }
};

#endif // BPTREE_DISK_HPP
