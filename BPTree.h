#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include "bits/stdc++.h"

#define DEBUG

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

        // for debugging
        friend ostream &operator<<(ostream &os, const node &n) {
            os << "(" << n.height << ") ";
            for (int i = 0; i < n.cnt; i++) {
                os << n.keys[i] << " ";
            }
            os << "| ";
            return os;
        }

        // for debugging
        friend string to_string(const node &n) {
            string ret;
            ret += "(" + to_string(n.height) + ")";
            for (int i = 0; i < n.cnt; i++) {
                ret += to_string(n.keys[i]);
                ret += " ";
            }
            ret += "| ";
            return ret;
        }
    };

private:
    node *root;
    // node number of current tree
    int nodeCnt;
    // recordNumber
    int recordCnt;

    // only for single node
    node *newNode() {
        nodeCnt++;
        // TODO
        node *p = new node();
#ifdef DEBUG
        p->cnt = 0;
        p->height = 0;
        p->childs[N] = 0;
        memset(p->keys, 0, sizeof p->keys);
        memset(p->childs, 0, sizeof p->childs);
#endif
        return p;
    }

    // only for single node
    void deleteNode(node *p) {
        nodeCnt--;
#ifdef DEBUG
        memset(p->keys, 0x3f, sizeof p->keys);
        memset(p->childs, 0x3f, sizeof(p->childs));
#endif
        // TODO
        delete p;
    }

    // only for single node
    _record *newRecord(const _record &r) {
        recordCnt++;
        // TODO, need copy constructor of record
        return new _record(r);
    }

    // only for single node
    void deleteRecord(_record *p) {
        recordCnt--;
        // TODO
        delete p;
    }

    // for non leaf node
    void _updateHeight(node *cur) {
        node *ch = (node *) cur->childs[0];
        int mx = ch->height;
        for (int i = 0; i < cur->cnt; i++) {
            ch = (node *) cur->childs[i + 1];
            mx = max(mx, ch->height);
        }
        cur->height = mx + 1;
    }

    // for non leaf node
    void _updateKey(node *cur, int i) {
        node *p = (node *) cur->childs[i + 1];
        while (p->height != 0) p = (node *) p->childs[0];
        cur->keys[i] = p->keys[0];
    }

    // insert record ptr at leaf node
    //
    // if split happened return the new node ptr
    // return nullptr
    node *_insertAtLeaf(node *ch1, const _key &key, const _record &record) {
        // insert at index i
        int i =
                (int) (upper_bound(ch1->keys, ch1->keys + ch1->cnt, key) - ch1->keys);
        if (ch1->cnt < N) {  // leaf is not full
            ch1->cnt++;
            for (int j = ch1->cnt - 1; j > i; j--) {
                ch1->keys[j] = ch1->keys[j - 1];
                ch1->childs[j] = ch1->childs[j - 1];
            }
            ch1->keys[i] = key;
            ch1->childs[i] = (void *) newRecord(record);
            // no split
            return nullptr;
        }
        node *ch2 = newNode();
        void *ch3 = ch1->childs[N];
        _key tmpKey[N + 1];
        void *tmpChild[N + 2];
        // copy of ch1
        memcpy(tmpKey, ch1->keys, sizeof(ch1->keys));
        memcpy(tmpChild, ch1->childs, sizeof(ch1->childs));

        // insert new key and record
        for (int j = N; j > i; j--) {
            tmpKey[j] = tmpKey[j - 1];
            tmpChild[j] = tmpChild[j - 1];
        }
        tmpKey[i] = key;
        tmpChild[i] = (void *) newRecord(record);

        // write to ch1 and ch2
        ch1->cnt = (N + 1) / 2;
        for (int j = 0; j < ch1->cnt; j++) {
            ch1->keys[j] = tmpKey[j];
            ch1->childs[j] = tmpChild[j];
        }
        ch2->cnt = (N + 1) - ch1->cnt;
        for (int j = 0; j < ch2->cnt; j++) {
            ch2->keys[j] = tmpKey[j + ch1->cnt];
            ch2->childs[j] = tmpChild[j + ch1->cnt];
        }
        // connect siblings ch1 -> ch2 -> ch3;
        ch2->childs[N] = ch3;
        ch1->childs[N] = (void *) ch2;
        return ch2;
    }

    // insert node ptr at internal node
    //
    // if split happened return parent ptr
    // else return nullptr
    node *_insertAtInternal(node *ch1, int i, const _key &key, node *ptr) {
        // insert at index i
        if (ch1->cnt < N) {  // node is not full
            ch1->cnt++;
            for (int j = ch1->cnt - 1; j > i; j--) {
                ch1->keys[j] = ch1->keys[j - 1];
                ch1->childs[j + 1] = ch1->childs[j];
            }
            ch1->keys[i] = key;
            ch1->childs[i + 1] = ptr;

            _updateHeight(ch1);
            return nullptr;
        }
        // node is full, need to split node into ch1 ch2
        node *ch2 = newNode();
        _key tmpKey[N + 1];
        void *tmpChilds[N + 2];
        // copy of ch1
        memcpy(tmpKey, ch1->keys, sizeof(ch1->keys));
        memcpy(tmpChilds, ch1->childs, sizeof(ch1->childs));

        // insert new key and record
        for (int j = N; j > i; j--) {
            tmpKey[j] = tmpKey[j - 1];
            tmpChilds[j + 1] = tmpChilds[j];
        }
        tmpKey[i] = key;
        tmpChilds[i + 1] = (void *) ptr;

        // took the (N + 1)/2 th key and pointer
        void *ch2Child = tmpChilds[(N + 1) / 2 + 1];

        // write to ch1 and ch2
        ch1->cnt = (N + 1) / 2;
        ch1->childs[0] = tmpChilds[0];
        for (int j = 0; j < ch1->cnt; j++) {
            ch1->keys[j] = tmpKey[j];
            ch1->childs[j + 1] = tmpChilds[j + 1];
        }
        ch2->cnt = (N + 1) - ch1->cnt - 1;
        ch2->childs[0] = ch2Child;
        for (int j = 0; j < ch2->cnt; j++) {
            ch2->keys[j] = tmpKey[(N + 1) / 2 + 1 + j];
            ch2->childs[j + 1] = tmpChilds[(N + 1) / 2 + 1 + j + 1];
        }

        _updateHeight(ch1);
        _updateHeight(ch2);

        return ch2;
    }

    // recursively find insertion position
    // handle new ptr from children if necessary
    node *_insertHelper(node *cur, const _key &key, const _record &record) {
        // non-root leaf
        if (cur->height == 0) return _insertAtLeaf(cur, key, record);

        int i =
                (int) (upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        i--;
        node *ch = (node *) cur->childs[i + 1];
        node *newCh = _insertHelper(ch, key, record);
        // insert to current node if leaf is split
        if (newCh) { // new child is created
            node *p = newCh;
            while (p->height != 0) p = (node *) p->childs[0];
            return _insertAtInternal(cur, i + 1, p->keys[0], newCh);
        } else {
            if (i >= 0) _updateKey(cur, i);
            return nullptr;
        }
    }


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
        if (i == -1 || cur->keys[i] != key) return -1;

        // remove the key
        deleteRecord((_record *) cur->childs[i]);
        for (int j = i; j < cur->cnt - 1; j++) {
            cur->keys[j] = cur->keys[j + 1];
            cur->childs[j] = cur->childs[j + 1];
        }
        cur->cnt--;

        // case 1 cur is still fine
        if (cur->cnt >= (N + 1) / 2) return 0;

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
            lSib->childs[N] = rSib;
            deleteNode(cur);
            return 1;
        }

        // merge cur into rSib
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
        if (cur->cnt >= N / 2) return 0;

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
        if (cur->height == 0) return _removeAtLeaf(lSib, pKeyCur, cur, pKeyRSib, rSib, key);
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
        } else if (status == 0) { // child might modify keys
            if (i >= 0) _updateKey(cur, i);
            return 0;
        } else if (status == 1) { // merged _cur -> _lSib and delete _cur, need to delete i-th key and update key accordingly
            return _removeAtInternal(lSib, pKeyCur, cur, pKeyRSib, rSib, i);
        } else if (status == 2) { // merged _rSib -> _cur and delete _rSib, need to delete i+1 -th key update key accordingly
            return _removeAtInternal(lSib, pKeyCur, cur, pKeyRSib, rSib, i + 1);
        }
        // should never reach here
        throw runtime_error("In _removeAtInternal: should never reach here");
    }

    void _destruct(node *cur) {
        if (cur->height == 0) {
            deleteNode(cur);
            return;
        }
        _destruct((node *) cur->childs[0]);
        for (int i = 0; i < cur->cnt; i++)
            _destruct((node *) cur->childs[i + 1]);
        deleteNode(cur);
    }

    // search lower_bound according to key (the first record_key >= key)
    pair<node *, int> _lower_bound(node *cur, const _key &key) const {
        assert(false); // might be buggy, 需要详细讨论下重复key情况
        int i =
                (int) (std::lower_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        // leaf node
        if (cur->height == 0) return pair<node *, int>(cur, i);
        // non leaf
        if (i >= cur->cnt || key < cur->keys[i]) i--;
        return _lower_bound((node *) cur->childs[i + 1], key);
    }

public:
    BPTree() : root{newNode()}, nodeCnt{0}, recordCnt{0} {}

    ~BPTree() {
        _destruct(root);
    }

    int height() const { return root->height; }

    int size() const { return recordCnt; }

    int nodeSize() const { return nodeCnt; }

    pair<node *, int> lower_bound(const _key &key) const {
        return _lower_bound(root, key);
    }

    // if key already exists, insert to upper bound
    void insert(const _key &key, const _record &record) {
        recordCnt++;
        node *p = _insertHelper(root, key, record);
        if (p) { // root is split
            node *newRt = newNode();
            newRt->cnt = 1;
            newRt->childs[0] = root;
            newRt->childs[1] = p;
            node *x = (node *) newRt->childs[1];
            while (x->height != 0) x = (node *) x->childs[0];
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
        if (status != -1) recordCnt--;
        return status != -1;
    }

    // query single key, return nullptr if not exist
    _record *query(const _key *key) const {
        pair<node *, int> q = lower_bound(root, key);
        node *p = q.first;
        int i = q.second;
        if (i == p->cnt || p->keys[i] != key) return nullptr;
        return (_record *) p->childs[i];
    }

    // query [lo, hi)
    vector<_record *> query(const _key &lo, const _key &hi) const {
        assert(lo < hi);
        pair<node *, int> q = lower_bound(root, lo);
        node *p = q.first;
        int i = q.second;

        vector<_record *> ret;

        while (p && p->keys[i] < hi) {
            for (; i < p->cnt; i++) {
                ret.push_back((_record *) p->childs[i]);
            }
            p = (node *) p->childs[N];
            i = 0;
        }
        return ret;
    }

#ifdef DEBUG

    void dfs(node *cur, map<node *, int> &m, int &idx) {
        if (m[cur] == 0) m[cur] = ++idx;
        cout << "[" << m[cur] << "]" << *cur;
        if (cur->height == 0) return;
        dfs((node *) cur->childs[0], m, idx);
        for (int i = 0; i < cur->cnt; i++) dfs((node *) cur->childs[i + 1], m, idx);
    }

    void dfs() {
        static map<node *, int> m;
        static int idx = 0;
        m.clear();
        idx = 0;
        dfs(root, m, idx);
    }


    bool selfCheck(node *cur) {
        if (cur->height == 0) { // leaf check
            if (cur != root && cur->cnt < (N + 1) / 2) return false; // cnt check
            // siblings check will be done by checking order
            return true;
        }
        // node check
        if (cur != root && cur->cnt < N / 2) return false;
        if (!selfCheck((node *) cur->childs[0])) return false;
        for (int i = 0; i < cur->cnt; i++) {
            node *p = (node *) cur->childs[i + 1];
            while (p->height != 0) p = (node *) p->childs[0];
            if (cur->keys[i] != p->keys[0]) return false;
            // check childs
            if (!selfCheck(p)) return false;
        }
        return true;
    }

    bool selfCheck() {
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
        return is_sorted(a.begin(), a.end()) && selfCheck(root);
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
                if (frt->height == 0) continue;
                for (int j = 0; j <= frt->cnt; j++) {
                    q.push((node *) frt->childs[j]);
                    nxt++;
                }
            }
            cout << endl;
            sz = nxt;
        }
    }

    // for debugging
    void levelTraverse() const {
        queue<node *> q;
        q.push(root);
        int sz = 1;
        while (q.size()) {
            int nxt = 0;
            for (int i = 1; i <= sz; i++) {
                node *frt = q.front();
                q.pop();
                cout << *frt;
                if (frt->height == 0) continue;
                for (int j = 0; j <= frt->cnt; j++) {
                    q.push((node *) frt->childs[j]);
                    nxt++;
                }
            }
            cout << endl;
            sz = nxt;
        }
    }

#endif

};

#endif //BPTREE_BPTREE_H