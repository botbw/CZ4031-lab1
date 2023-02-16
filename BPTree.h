#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include "bits/stdc++.h"

using namespace std;

template <typename _key, typename _record, int N>
class BPTree {
private:
    // keys, child, cnt, height are on disk
    struct node {
        _key keys[N];
        // child[i] is _record* or node*
        void *child[N + 1];
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
    // global cnt for node
    static int globalNodeCnt;

    node *root;
    // node number of current tree
    int nodeCnt;
    // recordNumber
    int recordCnt;

    // only for single node
    node *newNode() {
        nodeCnt++;
        return newNodeGlobal();
    }

    // only for single node
    void deleteNode(node *p) {
        nodeCnt--;
        deleteNodeGlobal(p);
    }

    pair<node *, int> _searchNode(node *cur, const _key &key) const {
        int i =
                (int)(lower_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        // leaf node
        if (cur->height == 0) return pair<node *, int>(cur, i);
        // non leaf
        if (i >= cur->cnt || key < cur->keys[i]) i--;
        return _searchNode((node *)cur->child[i + 1], key);
    }

    // for non leaf node
    void _updateHeight(node *cur) {
        node *ch = (node *)cur->child[0];
        int mx = ch->height;
        for (int i = 0; i < cur->cnt; i++) {
            ch = (node *)cur->child[i + 1];
            mx = max(mx, ch->height);
        }
        cur->height = mx + 1;
    }

    node *_insertAtLeaf(node *ch1, const _key &key, _record *record) {
        // insert at index i
        int i =
                (int)(upper_bound(ch1->keys, ch1->keys + ch1->cnt, key) - ch1->keys);
        if (ch1->cnt < N) {  // leaf is not full
            ch1->cnt++;
            for (int j = ch1->cnt - 1; j > i; j--) {
                ch1->keys[j] = ch1->keys[j - 1];
                ch1->child[j] = ch1->child[j - 1];
            }
            ch1->keys[i] = key;
            ch1->child[i] = (void *)record;
            // no split
            return nullptr;
        }
        node *ch2 = newNode();
        void *ch3 = ch1->child[N];
        _key tmpKey[N + 1];
        void *tmpChild[N + 2];
        // copy of ch1
        memcpy(tmpKey, ch1->keys, sizeof(ch1->keys));
        memcpy(tmpChild, ch1->child, sizeof(ch1->child));

        // insert new key and record
        for (int j = N; j > i; j--) {
            tmpKey[j] = tmpKey[j - 1];
            tmpChild[j] = tmpChild[j - 1];
        }
        tmpKey[i] = key;
        tmpChild[i] = (void *)record;

        // write to ch1 and ch2
        ch1->cnt = (N + 1) / 2;
        for (int j = 0; j < ch1->cnt; j++) {
            ch1->keys[j] = tmpKey[j];
            ch1->child[j] = tmpChild[j];
        }
        ch2->cnt = (N + 1) - ch1->cnt;
        for (int j = 0; j < ch2->cnt; j++) {
            ch2->keys[j] = tmpKey[j + ch1->cnt];
            ch2->child[j] = tmpChild[j + ch1->cnt];
        }
        // connect siblings ch1 -> ch2 -> ch3;
        ch2->child[N] = ch3;
        ch1->child[N] = (void *)ch2;
        return ch2;
    }

    node *_insertAtInternal(node *ch1, int i, const _key &key, node *ptr) {
        // insert at index i
        if (ch1->cnt < N) {  // node is not full
            ch1->cnt++;
            for (int j = ch1->cnt - 1; j > i; j--) {
                ch1->keys[j] = ch1->keys[j - 1];
                ch1->child[j + 1] = ch1->child[j];
            }
            ch1->keys[i] = key;
            ch1->child[i + 1] = ptr;
            return nullptr;
        }
        // node is full, need to split node into ch1 ch2
        node *ch2 = newNode();
        _key tmpKey[N + 1];
        void *tmpChild[N + 2];
        // copy of ch1
        memcpy(tmpKey, ch1->keys, sizeof(ch1->keys));
        memcpy(tmpChild, ch1->child, sizeof(ch1->child));

        // insert new key and record
        for (int j = N; j > i; j--) {
            tmpKey[j] = tmpKey[j - 1];
            tmpChild[j + 1] = tmpChild[j];
        }
        tmpKey[i] = key;
        tmpChild[i + 1] = (void *)ptr;

        // took the (N + 1)/2 th key and pointer
        _key parentKey = tmpKey[(N + 1) / 2];
        void *ch2Child = tmpChild[(N + 1) / 2 + 1];
        // write to ch1 and ch2
        ch1->cnt = (N + 1) / 2;
        for (int j = 0; j < ch1->cnt; j++) {
            ch1->keys[j] = tmpKey[j];
            ch1->child[j + 1] = tmpChild[j + 1];
        }
        ch2->cnt = (N + 1) - ch1->cnt - 1;
        for (int j = 0; j < ch2->cnt; j++) {
            ch2->keys[j] = tmpKey[j + ch1->cnt + 1];
            ch2->child[j + 1] = tmpChild[j + ch1->cnt + 2];
        }

        // parent node
        node *parent = newNode();
        ch2->child[0] = ch2Child;
        parent->cnt = 1;
        parent->keys[0] = parentKey;
        parent->child[0] = ch1;
        parent->child[1] = ch2;
        _updateHeight(ch1);
        _updateHeight(ch2);
        _updateHeight(parent);
        return parent;
    }

    // helper
    node *_insertHelper(node *cur, const _key &key, _record *record) {
        // non-root leaf
        if (cur->height == 0) return _insertAtLeaf(cur, key, record);
        int i =
                (int)(upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        i--;
        node *p = _insertHelper((node *)cur->child[i + 1], key, record);
        if (!p) return nullptr;  // no need split further
        // insert to current node
        node *ret = _insertAtInternal(cur, i + 1, p->keys[0], p);
        return ret;
    }

    // ch1 -> ch2 -> ch3
    // ch1 and ch3 might be null
    // -1: key not found ,no modification
    // 0: deleted, no modification
    // 1: borrow from ch1 2: borrow from ch3
    // 3: merge ch2 into ch1 4: merge ch3 into ch2
    int _removeAtLeaf(node *ch1, node *ch2, node *ch3, const _key &key) {
        // delete at index i
        int i =
                (int)(lower_bound(ch2->keys, ch2->keys + ch2->cnt, key) - ch2->keys);

        // didn't find key
        if (i == ch2->cnt || ch2->keys[i] != key) return -1;

        // remove the key
        for (int j = i; j < ch2->cnt - 1; j++) {
            ch2->keys[j] = ch2->keys[j + 1];
            ch2->child[j] = ch2->child[j + 1];
        }
        ch2->cnt--;

        // case 1 node is still fine
        if (ch2->cnt >= (N + 1) / 2) return 0;

        // case 2 borrow from sibings
        // borrow from ch1
        if (ch1 && ch1->cnt > (N + 1) / 2) {
            ch2->cnt++;
            for (int j = ch2->cnt - 1; j > 0; j--) {
                ch2->keys[j] = ch2->keys[j - 1];
                ch2->child[j] = ch2->child[j - 1];
            }
            ch2->keys[0] = ch1->keys[ch1->cnt - 1];
            ch2->child[0] = ch1->child[ch1->cnt - 1];
            ch1->cnt--;
            return 1;
        }

        // borrow from ch3
        if (ch3 && ch3->cnt > (N + 1) / 2) {
            ch2->cnt++;
            ch2->keys[ch2->cnt - 1] = ch3->keys[0];
            ch2->child[ch2->cnt - 1] = ch3->child[0];
            for (int j = 0; j < ch3->cnt - 1; j++) {
                ch3->keys[j] = ch3->keys[j + 1];
                ch3->child[j] = ch3->child[j + 1];
            }
            ch3->cnt--;
            return 2;
        }

        // case 3 cannot borrow
        // merge into ch1
        if (ch1) {
            for (int j = 0; j < ch2->cnt; j++) {
                ch1->cnt++;
                ch1->keys[ch1->cnt - 1] = ch2->keys[j];
                ch1->child[ch1->cnt - 1] = ch2->child[j];
            }
            ch1->child[N] = ch3;
            deleteNode(ch2);
            return 3;
        }

        // merge with ch3
        if (ch3) {
            for (int j = 0; j < ch3->cnt; j++) {
                ch2->cnt++;
                ch2->keys[ch2->cnt - 1] = ch3->keys[j];
                ch2->child[ch2->cnt - 1] = ch3->child[j];
            }
            ch2->child[N] = ch3->child[N];
            deleteNode(ch3);
            return 4;
        }

        // only happens at root
        return 0;
    }

    //        key1   key2
    //         |      |
    // ch1 -> ch2 -> ch3
    // ch1 and ch3 might be null
    // 0: no need to modify further
    // 1: borrow from ch1 2: borrow from ch3
    // 3: merge ch2 into ch1 4: merge ch3 into ch2
    int _removeAtInternal(node *ch1, _key &key1, node *ch2, _key &key2,
                          node *ch3, int i) {
        // remove the key
        for (int j = i; j < ch2->cnt - 1; j++) {
            ch2->keys[j] = ch2->keys[j + 1];
            ch2->child[j + 1] = ch2->child[j + 2];
        }
        ch2->cnt--;

        // case 1 node is still fine
        if (ch2->cnt >= N / 2) return 0;

        // case 2 borrow from sibings

        // borrow from ch1 (zig)
        if (ch1 && ch1->cnt > N / 2) {
            ch2->cnt += 1;
            for (int j = ch2->cnt - 1; j > 0; j--) {
                ch2->keys[j] = ch2->keys[j - 1];
                ch2->child[j + 1] = ch2->child[j];
            }
            ch2->child[1] = ch2->child[0];

            // left most pointer and borrowed key from parent
            ch2->keys[0] = key1;
            ch2->child[0] = ch1->child[ch1->cnt];
            key1 = ch1->keys[ch1->cnt - 1];

            ch1->cnt--;
            return 0;
        }

        // borrow from ch3 (zag)
        if (ch3 && ch3->cnt > N / 2) {
            ch2->cnt++;
            ch2->keys[ch2->cnt - 1] = key2;
            ch2->child[ch2->cnt] = ch3->child[0];
            key2 = ch3->keys[0];
            ch3->child[0] = ch3->child[1];
            for (int j = 0; j < ch3->cnt - 1; j++) {
                ch3->keys[j] = ch3->keys[j + 1];
                ch3->child[j + 1] = ch3->child[j + 2];
            }
            ch3->cnt--;
            return 0;
        }

        // case 3 cannot borrow

        // merge ch2 into ch1
        if (ch1) {
            ch1->cnt++;
            ch1->keys[ch1->cnt - 1] = key1;
            ch1->child[ch1->cnt] = ch2->child[0];
            for (int j = 0; j < ch2->cnt; j++) {
                ch1->cnt++;
                ch1->keys[ch1->cnt - 1] = ch2->keys[j];
                ch1->child[ch1->cnt] = ch2->child[j + 1];
            }
            deleteNode(ch2);
            return 3;
        }

        // merge ch3 into ch2
        if (ch3) {
            ch2->cnt++;
            ch2->keys[ch2->cnt - 1] = key2;
            ch2->child[ch2->cnt] = ch3->child[0];
            for (int j = 0; j < ch3->cnt; j++) {
                ch2->cnt++;
                ch2->keys[ch2->cnt - 1] = ch3->keys[j];
                ch2->child[ch2->cnt] = ch3->child[j + 1];
            }
            deleteNode(ch3);
            return 4;
        }

        // only happens at root
        // TODO

        return 0;
    }

    // 0: no modification
    // 1: borrow from ch1 2: borrow from ch3
    // 3: merge ch2 into ch1 4: merge ch3 into ch2
    int _removeHelper(node *ch1, _key &key1, node *ch2, _key &key2, node *ch3,
                      const _key &key) {
        // leaf node
        if (ch2->height == 0) {
            int status = _removeAtLeaf(ch1, ch2, ch3, key);
            return status;
        }
        int i =
                (int)(lower_bound(ch2->keys, ch2->keys + ch2->cnt, key) - ch2->keys);
        if (i == ch2->cnt || key < ch2->keys[i]) i--;
        _key dumpKey = _key();
        node *_ch1 = (i == -1 ? nullptr : (node *)ch2->child[i]);
        _key &_key1 = (i == -1 ? dumpKey : ch2->keys[i]);
        node *_ch2 = (node *)ch2->child[i + 1];
        _key &_key2 = (i == ch2->cnt - 1 ? dumpKey : ch2->keys[i + 1] );
        node *_ch3 = (i == ch2->cnt - 1 ? nullptr : (node *)ch2->child[i + 2]);
        int status = _removeHelper(_ch1, _key1, _ch2, _key2, _ch3, key);
        if (status == 0 || status == -1)
            return status;
        else if (status == 1) {
            ch2->keys[i] = _ch2->keys[0];
            return 0;
        } else if (status == 2) {
            ch2->keys[i + 1] = _ch3->keys[0];
            return 0;
        } else if (status == 3) {
            return _removeAtInternal(ch1, key1, ch2, key2, ch3, i);
        } else if (status == 4) {
            return _removeAtInternal(ch1, key1, ch2, key2, ch3, i + 1);
        }
        // should never reach here
        throw runtime_error("In _removeAtInternal: should never reach here");
    }

public:
    static int globalSize() { return globalNodeCnt; }
    static node *newNodeGlobal() {
        // TODO: use disk pool
        node *p = new node();

        p->cnt = 0;
        p->height = 0;
        memset(p->child, 0, sizeof 0);
        return p;
    }
    // only for single node
    static void deleteNodeGlobal(node *p) {
        // TODO: use disk pool
        globalNodeCnt--;
        memset(p->child, 0, sizeof(p->child));
        delete p;
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
                cout << (*frt);
                if (frt->height == 0) continue;
                for (int j = 0; j <= frt->cnt; j++) {
                    q.push((node *)frt->child[j]);
                    nxt++;
                }
            }
            cout << endl;
            sz = nxt;
        }
    }

    BPTree() : root{newNode()}, nodeCnt{0} {}

    ~BPTree() {
        // TODO
    }

    int height() const { return root->height; }

    int size() const { return nodeCnt; }

    // if key already exists, insert to upper bound
    void insert(const _key &key, _record *record) {
        node *p;
        if (root->height == 0) {  // root is leaf
            p = _insertAtLeaf(root, key, record);
            if (p) {  // root is splited
                node *newRt = newNode();
                newRt->cnt = 1;
                newRt->keys[0] = p->keys[0];
                newRt->child[0] = root;
                newRt->child[1] = p;
                _updateHeight(newRt);
                root = newRt;
            }
            return;
        }
        p = _insertHelper(root, key, record);  // root is splited
        if (p) root = p;
    }

    void remove(const _key &key) {
        if (root->height == 0) {
            _removeAtLeaf(nullptr, root, nullptr, key);
            return;
        }
        _key dump = _key();
        int status = _removeHelper(nullptr, dump, root, dump, nullptr, key);
        if (root->cnt == 0) {
            node *p = root;
            root = (node *)root->child[0];
            deleteNode(p);
        }
    }

    // query single key, nullptr if doesn't exist
    _record *query(const _key *key) const {
        pair<node *, int> q = _searchNode(root, key);
        node *p = q.first;
        int i = q.second;
        if (i == p->cnt || p->keys[i] != key) return nullptr;
        return (_record *)p->child[i];
    }

    // query [lo, hi)
    vector<_record *> query(const _key &lo, const _key &hi) const {
        assert(lo < hi);
        pair<node *, int> q = _searchNode(root, lo);
        node *p = q.first;
        int i = q.second;

        vector<_record *> ret;

        while (p && p->keys[i] < hi) {
            for (; i < p->cnt; i++) {
                ret.push_back((_record *)p->child[i]);
            }
            p = (node *)p->child[N];
            i = 0;
        }
        return ret;
    }
};

template <typename _key, typename _record, int N>
int BPTree<_key, _record, N>::globalNodeCnt = 0;

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();

#endif //BPTREE_BPTREE_H
