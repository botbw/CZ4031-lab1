#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include "Block.h"
#include "Disk.hpp"
#include <map>
#include <queue>
#include <vector>
#include <ostream>
#include <cassert>
#include <string>
#include <algorithm>
#include <cstring>


using namespace std;

template<typename _key, typename _record, int N>
class BPTree {
public:
    // keys, childs, cnt, height are on disk
    struct node {
        _key keys[N];
        // childs[i] is _record* or node*
        void *childs[N + 1];
        int cnt, height;

        node() : cnt{0}, height{0} {
#ifdef DEBUG
            memset(childs, 0xcf, sizeof childs);
#endif
            childs[N] = nullptr;
        }

        ~node() {
#ifdef DEBUG
            memset(childs, 0x3f, sizeof(childs));
#endif
        }

        // for debugging
        friend ostream &operator<<(ostream &os, const node &n) {
            os << "(" << n.height << ") ";
            for (int i = 0; i < n.cnt; i++) {
                os << n.keys[i] << " ";
            }
            os << "| ";
            return os;
        }

        friend void print_keys(const node &n) {
            for (int i = 0; i < n.cnt; i++) {
                cout << n.keys[i] << " ";
            }
        }
    };

private:
    int nodeCnt;        // node number of current tree
    int recordCnt;      // record number of current tree
    node *root;         // root of the tree
    Disk<_record> disk; // disk for _record storage

    // only for single node
    node *newNode() {
        nodeCnt++;
        // get from memory (new)
        node *p = new node();
        return p;
    }

    // only for single node
    void deleteNode(node *p) {
        nodeCnt--;
        // free memory (delete)
        delete p;
    }

    // only for single node
    _record *newRecord(const _record &r) {
        recordCnt++;
        // get from disk (disk.allocate)
        return disk.allocate(r);
    }

    // only for single node
    void deleteRecord(_record *p) {
        recordCnt--;
        // free to disk (disk.deallocate)
        disk.deallocate(p);
    }

    // for non leaf node
    // cur->height = max{childs->height} + 1
    void _updateHeight(node *cur) {
        node *ch = (node *) cur->childs[0];
        int mx = ch->height;
        for (int i = 0; i < cur->cnt; i++) {
            ch = (node *) cur->childs[i + 1];
            mx = max(mx, ch->height);
        }
        cur->height = mx + 1;
    }

    // for non leaf
    // there should be a more elegant design, e.g. recursively return left-most leaf in a subtree
    // however, brute-force here won't increase overall complexity
    // for one deletion, the worst case O(logh*logh), average sigma(logh)
    void _updateKey(node *cur, int i) {
        node *p = (node *) cur->childs[i + 1];
        while (p->height != 0)
            p = (node *) p->childs[0];
        cur->keys[i] = p->keys[0];
    }

    // insert record ptr at leaf node
    //
    // if split happened return the new node ptr
    // return nullptr
    node *_insertAtLeaf(node *cur, const _key &key, const _record &record) {
        // insert at index i
        int i =
                (int) (upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        if (cur->cnt < N) { // leaf is not full
            cur->cnt++;
            for (int j = cur->cnt - 1; j > i; j--) {
                cur->keys[j] = cur->keys[j - 1];
                cur->childs[j] = cur->childs[j - 1];
            }
            cur->keys[i] = key;
            cur->childs[i] = (void *) newRecord(record);
            // no split
            return nullptr;
        }
        node *newCh = newNode();
        void *curLeftSib = cur->childs[N];
        _key tmpKey[N + 1];
        void *tmpChild[N + 2];
        // copy of cur
        memcpy(tmpKey, cur->keys, sizeof(cur->keys));
        memcpy(tmpChild, cur->childs, sizeof(cur->childs));

        // insert new key and record
        for (int j = N; j > i; j--) {
            tmpKey[j] = tmpKey[j - 1];
            tmpChild[j] = tmpChild[j - 1];
        }
        tmpKey[i] = key;
        tmpChild[i] = (void *) newRecord(record);

        // write to cur and newCh
        cur->cnt = (N + 1) / 2;
        for (int j = 0; j < cur->cnt; j++) {
            cur->keys[j] = tmpKey[j];
            cur->childs[j] = tmpChild[j];
        }
        newCh->cnt = (N + 1) - cur->cnt;
        for (int j = 0; j < newCh->cnt; j++) {
            newCh->keys[j] = tmpKey[j + cur->cnt];
            newCh->childs[j] = tmpChild[j + cur->cnt];
        }
        // connect siblings cur -> newCh -> curLeftSib;
        newCh->childs[N] = curLeftSib;
        cur->childs[N] = (void *) newCh;
        return newCh;
    }

    // insert node ptr at internal node
    //
    // if split happened return parent ptr
    // else return nullptr
    node *_insertAtInternal(node *cur, int i, const _key &key, node *ptr) {
        // insert at index i
        if (cur->cnt < N) { // node is not full
            cur->cnt++;
            for (int j = cur->cnt - 1; j > i; j--) {
                cur->keys[j] = cur->keys[j - 1];
                cur->childs[j + 1] = cur->childs[j];
            }
            cur->keys[i] = key;
            cur->childs[i + 1] = ptr;

            _updateHeight(cur);
            return nullptr;
        }
        // node is full, need to split node into cur newCh
        node *newCh = newNode();
        _key tmpKey[N + 1];
        void *tmpChilds[N + 2];
        // copy of cur
        memcpy(tmpKey, cur->keys, sizeof(cur->keys));
        memcpy(tmpChilds, cur->childs, sizeof(cur->childs));

        // insert new key and record
        for (int j = N; j > i; j--) {
            tmpKey[j] = tmpKey[j - 1];
            tmpChilds[j + 1] = tmpChilds[j];
        }
        tmpKey[i] = key;
        tmpChilds[i + 1] = (void *) ptr;

        // took the (N + 1)/2 th key and pointer
        void *ch2Child = tmpChilds[(N + 1) / 2 + 1];

        // write to cur and newCh
        cur->cnt = (N + 1) / 2;
        cur->childs[0] = tmpChilds[0];
        for (int j = 0; j < cur->cnt; j++) {
            cur->keys[j] = tmpKey[j];
            cur->childs[j + 1] = tmpChilds[j + 1];
        }
        newCh->cnt = (N + 1) - cur->cnt - 1;
        newCh->childs[0] = ch2Child;
        for (int j = 0; j < newCh->cnt; j++) {
            newCh->keys[j] = tmpKey[(N + 1) / 2 + 1 + j];
            newCh->childs[j + 1] = tmpChilds[(N + 1) / 2 + 1 + j + 1];
        }

        _updateHeight(cur);
        _updateHeight(newCh);

        return newCh;
    }

    // recursively find insertion position
    // handle new ptr from children if necessary
    node *_insertHelper(node *cur, const _key &key, const _record &record) {
        // non-root leaf
        if (cur->height == 0)
            return _insertAtLeaf(cur, key, record);

        int i =
                (int) (upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        i--;
        node *newCh = _insertHelper((node *) cur->childs[i + 1], key, record);
        // insert to current node if leaf is split
        if (newCh) { // new child is created
            node *p = newCh;
            while (p->height != 0)
                p = (node *) p->childs[0];
            return _insertAtInternal(cur, i + 1, p->keys[0], newCh);
        } else {
            if (i >= 0)
                _updateKey(cur, i);
            return nullptr;
        }
    }

    //        pKeyCur pKeyRSib
    //         |      |
    // lSib -> cur -> rSib
    // return status
    // -1: key not found and no modification
    // 0: cur is fine after deleting the key or borrowing key
    // 1: merged cur into lSib, 2: merged rSib into cur
    int _removeAtLeaf(node *lSib, _key *pKeyCur, node *cur, _key *pKeyRSib, node *rSib, const _key &key) {
        // delete at index i
        int i =
                (int) (std::upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        i--;
        // didn't find key
        if (i == -1 || cur->keys[i] != key)
            return -1;

        // remove the key
        deleteRecord((_record *) cur->childs[i]);
        for (int j = i; j < cur->cnt - 1; j++) {
            cur->keys[j] = cur->keys[j + 1];
            cur->childs[j] = cur->childs[j + 1];
        }
        cur->cnt--;

        // case 1 cur is still fine
        if (cur->cnt >= (N + 1) / 2)
            return 0;

        // case 2 cur borrow from siblings
        // borrow from lSib
        if (lSib && lSib->cnt > (N + 1) / 2) {
            cur->cnt++;
            for (int j = cur->cnt - 1; j > 0; j--) {
                cur->keys[j] = cur->keys[j - 1];
                cur->childs[j] = cur->childs[j - 1];
            }
            cur->keys[0] = lSib->keys[lSib->cnt - 1];
            cur->childs[0] = lSib->childs[lSib->cnt - 1];
            lSib->cnt--;
            *pKeyCur = cur->keys[0];
            return 0;
        }

        // borrow from rSib
        if (rSib && rSib->cnt > (N + 1) / 2) {
            cur->cnt++;
            cur->keys[cur->cnt - 1] = rSib->keys[0];
            cur->childs[cur->cnt - 1] = rSib->childs[0];
            for (int j = 0; j < rSib->cnt - 1; j++) {
                rSib->keys[j] = rSib->keys[j + 1];
                rSib->childs[j] = rSib->childs[j + 1];
            }
            rSib->cnt--;
            *pKeyRSib = rSib->keys[0];
            return 0;
        }

        // case 3 cannot borrow
        // merge cur into lSib
        if (lSib) {
            for (int j = 0; j < cur->cnt; j++) {
                lSib->cnt++;
                lSib->keys[lSib->cnt - 1] = cur->keys[j];
                lSib->childs[lSib->cnt - 1] = cur->childs[j];
            }
            lSib->childs[N] = cur->childs[N];
            deleteNode(cur);
            return 1;
        }

        // merge rSib into cur
        if (rSib) {
            for (int j = 0; j < rSib->cnt; j++) {
                cur->cnt++;
                cur->keys[cur->cnt - 1] = rSib->keys[j];
                cur->childs[cur->cnt - 1] = rSib->childs[j];
            }
            cur->childs[N] = rSib->childs[N];
            deleteNode(rSib);
            return 2;
        }

        // only happens at root
        return 0;
    }

    //        pKeyCur pKeyRSib
    //         |      |
    // lSib -> cur -> rSib
    // return status
    // 0: cur is fine after deleting the key or borrowing key
    // 1: merged cur into lSib, 2: merged rSib into cur
    int _removeAtInternal(node *lSib, _key *pKeyCur, node *cur, _key *pKeyRSib, node *rSib, int i) {
        // remove the key
        for (int j = i; j < cur->cnt - 1; j++) {
            cur->keys[j] = cur->keys[j + 1];
            cur->childs[j + 1] = cur->childs[j + 2];
        }
        cur->cnt--;

        // case 1 node is still fine
        if (cur->cnt >= N / 2)
            return 0;

        // case 2 borrow from siblings

        // zig
        if (lSib && lSib->cnt > N / 2) {
            cur->cnt += 1;
            for (int j = cur->cnt - 1; j > 0; j--) {
                cur->keys[j] = cur->keys[j - 1];
                cur->childs[j + 1] = cur->childs[j];
            }
            cur->childs[1] = cur->childs[0];

            // left most pointer and borrowed key from parent
            cur->childs[0] = lSib->childs[lSib->cnt];
            _updateKey(cur, 0);
            *pKeyCur = lSib->keys[lSib->cnt - 1];

            lSib->cnt--;

            _updateHeight(cur);
            _updateHeight(lSib);
            return 0;
        }

        // zag
        if (rSib && rSib->cnt > N / 2) {
            cur->cnt++;
            cur->keys[cur->cnt - 1] = *pKeyRSib;
            cur->childs[cur->cnt] = rSib->childs[0];
            *pKeyRSib = rSib->keys[0];
            rSib->childs[0] = rSib->childs[1];
            for (int j = 0; j < rSib->cnt - 1; j++) {
                rSib->keys[j] = rSib->keys[j + 1];
                rSib->childs[j + 1] = rSib->childs[j + 2];
            }
            rSib->cnt--;
            _updateHeight(cur);
            _updateHeight(rSib);
            return 0;
        }

        // case 3 cannot borrow

        // merge cur into lSib
        if (lSib) {
            lSib->cnt++;
            lSib->childs[lSib->cnt] = cur->childs[0];
            _updateKey(lSib, lSib->cnt - 1);
            for (int j = 0; j < cur->cnt; j++) {
                lSib->cnt++;
                lSib->keys[lSib->cnt - 1] = cur->keys[j];
                lSib->childs[lSib->cnt] = cur->childs[j + 1];
            }
            deleteNode(cur);
            _updateHeight(lSib);
            return 1;
        }

        // merge rSib into cur
        if (rSib) {
            cur->cnt++;
            cur->keys[cur->cnt - 1] = *pKeyRSib;
            cur->childs[cur->cnt] = rSib->childs[0];
            for (int j = 0; j < rSib->cnt; j++) {
                cur->cnt++;
                cur->keys[cur->cnt - 1] = rSib->keys[j];
                cur->childs[cur->cnt] = rSib->childs[j + 1];
            }
            deleteNode(rSib);
            _updateHeight(cur);
            return 2;
        }

        // only happens at root
        return 0;
    }

    //        pKeyCur pKeyRSib
    //         |      |
    // lSib -> cur -> rSib
    int _removeHelper(node *lSib, _key *pKeyCur, node *cur, _key *pKeyRSib, node *rSib, const _key &key) {
        // leaf node
        if (cur->height == 0)
            return _removeAtLeaf(lSib, pKeyCur, cur, pKeyRSib, rSib, key);
        // internal node
        int i = (int) (std::upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        i--;

        // prepare parameters
        node *_lSib = (i == -1 ? nullptr : (node *) cur->childs[i]);
        _key *_pKeyCur = (i == -1 ? nullptr : &cur->keys[i]);
        node *_cur = (node *) cur->childs[i + 1];
        _key *_pKeyRSib = (i == cur->cnt - 1 ? nullptr : &cur->keys[i + 1]);
        node *_rSib = (i == cur->cnt - 1 ? nullptr : (node *) cur->childs[i + 2]);

        int status = _removeHelper(_lSib, _pKeyCur, _cur, _pKeyRSib, _rSib, key);
        // _cur or _rSib might be freed by _removeHelper

        if (status == -1) { // child no modification
            return -1;
        } else if (status == 0) {               // deletion happens
            if (i >= 0) // update to correct key
                _updateKey(cur, i);
            return 0;
        } else if (status ==
                   1) { // merged _cur -> _lSib and delete _cur, need to delete i-th key and update key accordingly
            return _removeAtInternal(lSib, pKeyCur, cur, pKeyRSib, rSib, i);
        } else if (status ==
                   2) { // merged _rSib -> _cur and delete _rSib, need to delete i+1 -th key update key accordingly
            return _removeAtInternal(lSib, pKeyCur, cur, pKeyRSib, rSib, i + 1);
        }
        // should never reach here
        throw runtime_error("In _removeAtInternal: should never reach here");
    }

    // search lower_bound according to key (the first record_key >= key)
    pair<node *, int> _lower_bound(node *cur, const _key &key, int *accessedCnt) const {
        if (accessedCnt)
            (*accessedCnt)++;
        int i =
                (int) (std::lower_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        if (cur->height == 0) { // leaf node
            if (i == cur->cnt)
                return pair<node *, int>(nullptr, -1);
            return pair<node *, int>(cur, i);
        } else { // non leaf
            if (i == cur->cnt)
                i--;
            // in case there are multiple same keys, find the left-most one
            pair<node *, int> leftSib = _lower_bound((node *) cur->childs[i], key, accessedCnt);
            if (leftSib.first) // found
                return leftSib;
            // search key i
            return _lower_bound((node *) cur->childs[i + 1], key, accessedCnt);
        }
    }

    // dfs deletion
    void _destruct(node *cur) {
        // if leaf, just delete
        if (cur->height == 0) {
            deleteNode(cur);
            return;
        }
        // delete all childs first
        _destruct((node *) cur->childs[0]);
        for (int i = 0; i < cur->cnt; i++)
            _destruct((node *) cur->childs[i + 1]);
        // delete current
        deleteNode(cur);
    }

public:
    BPTree() : nodeCnt{0}, recordCnt{0}, root{newNode()}, disk(DISK_MEM_SIZE) {}

    ~BPTree() {
        _destruct(root);
    }

    int height() const { return root->height; }

    int size() const { return recordCnt; }

    int nodeSize() const { return nodeCnt; }

    node getRootCopy() const {
        return *root;
    }

    pair<node *, int> lower_bound(const _key &key, int *accessedCnt = nullptr) const {
        return _lower_bound(root, key, accessedCnt);
    }

    // if key already exists, insert to upper bound
    void insert(const _key &key, const _record record) {
        node *p = _insertHelper(root, key, record);
        if (p) { // root is split
            node *newRt = newNode();
            newRt->cnt = 1;
            newRt->childs[0] = root;
            newRt->childs[1] = p;
            node *x = (node *) newRt->childs[1];
            while (x->height != 0)
                x = (node *) x->childs[0];
            newRt->keys[0] = x->keys[0];
            _updateHeight(newRt);
            root = newRt;
        }
    }

    bool remove(const _key &key) {
        int status = _removeHelper(nullptr, nullptr, root, nullptr, nullptr, key);
        if (root->height != 0 && root->cnt == 0) { // if root is leaf, do not delete when it is empty
            node *p = root;
            root = (node *) root->childs[0];
            deleteNode(p);
        }
        return status != -1;
    }

    void removeAll(const _key &key) {
        bool success = true;

        while (success) {
            success = remove(key);
        }
    }

    // query single key, return nullptr if not exist
    _record *query(const _key *key, int *accessedCnt = nullptr) {

        pair<node *, int> q = lower_bound(root, key, accessedCnt);
        node *p = q.first;
        int i = q.second;
        // node *p is accessed in the lower_bound function

        if (i == p->cnt || p->keys[i] != key)
            return nullptr;
        return (_record *) p->childs[i];
    }

    // query [lo, hi]
    vector<_record *> query(const _key &lo, const _key &hi, int *accessedCnt = nullptr) {

        assert(lo <= hi);
        pair<node *, int> q = lower_bound(lo, accessedCnt);
        node *p = q.first;
        int i = q.second;
        // node *p is accessed in the lower_bound function

        vector<_record *> ret;

        while (p) {
            for (; i < p->cnt; i++) {
                if (p->keys[i] > hi)
                    return ret;
                ret.push_back((_record *) p->childs[i]);
            }
            p = (node *) p->childs[N];
            if (accessedCnt)
                (*accessedCnt)++;
            i = 0;
        }
        return ret;
    }

    vector<_record *> getAll() const {
        vector<_record *> ret;
        node *p = root;
        while (p->height != 0)
            p = (node *) p->childs[0];
        while (p) {
            int i = 0;
            for (; i < p->cnt; i++) {
                ret.push_back((_record *) p->childs[i]);
            }
            p = (node *) p->childs[N];
        }
        return ret;
    }

    void printRootInfo() const {
        print_keys(*root);
        return;
    }

    void levelTraverse(node *cur) const {
        queue<node *> q;
        q.push(cur);
        int sz = 1;
        while (q.size()) {
            int nxt = 0;
            for (int i = 1; i <= sz; i++) {
                node *frt = q.front();
                q.pop();
                cout << *frt;
                if (frt->height == 0)
                    continue;
                for (int j = 0; j <= frt->cnt; j++) {
                    q.push((node *) frt->childs[j]);
                    nxt++;
                }
            }
            cout << endl;
            sz = nxt;
        }
    }

    void levelTraverse() const {
        levelTraverse(root);
    }

    bool selfCheck(node *cur) const {
        if (cur->cnt > N) // obvious
            return false;
        if (cur->height == 0) {                                              // leaf check
            if (cur != root && cur->cnt < (N + 1) / 2) // cnt should be at least (N+1) / 2;
                return false;
            if (!is_sorted(cur->keys, cur->keys + cur->cnt)) // keys mush be sorted
                return false;
            return true;
        }
        // node check
        if (cur != root && cur->cnt < N / 2) // cnt should be at least N/2
            return false;
        if (!is_sorted(cur->keys, cur->keys + cur->cnt)) // keys must be sorted
            return false;

        //  check subtrees and keys
        if (!selfCheck((node *) cur->childs[0]))
            return false;
        for (int i = 0; i < cur->cnt; i++) {
            // i-th key should be the left-most leaf of i-th subtree
            node *p = (node *) cur->childs[i + 1];
            while (p->height != 0)
                p = (node *) p->childs[0];
            if (cur->keys[i] != p->keys[0])
                return false;

            // i-th key should be >= the right-most leaf of (i-1)-th subtree
            p = (node *) cur->childs[i];
            while (p->height != 0)
                p = (node *) p->childs[p->cnt - 1];
            if (p->keys[p->cnt - 1] > cur->keys[i])
                return false;
            // check childs
            if (!selfCheck(p))
                return false;
        }
        return true;
    }

    // check the structure of BPTree (no simulation, only structure correctness)
    bool selfCheck() const {
        auto tmp = lower_bound(-1);
        int i = tmp.second;
        node *p = tmp.first;
        vector<_key> a;
        while (p) {
            for (; i < p->cnt; i++) {
                a.push_back(p->keys[i]);
            }
            p = (node *) p->childs[N];
            i = 0;
        }
        // leaf must be sorted (the completeness will be checked with multiset simulation in main.cpp)
        return is_sorted(a.begin(), a.end()) && selfCheck(root);
    }

    Disk<_record> *const getDisk() {
        return &disk;
    }

#ifdef DEBUG

    void dfs(node *cur, map<node *, int> &m, int &idx) {
        if (m[cur] == 0)
            m[cur] = ++idx;
        cout << "[" << m[cur] << "]" << *cur;
        if (cur->height == 0)
            return;
        dfs((node *) cur->childs[0], m, idx);
        for (int i = 0; i < cur->cnt; i++)
            dfs((node *) cur->childs[i + 1], m, idx);
    }

    // assign dfs index for each node
    void dfs() {
        static map<node *, int> m;
        static int idx = 0;
        m.clear();
        idx = 0;
        dfs(root, m, idx);
    }

#endif
};

#endif // BPTREE_BPTREE_H