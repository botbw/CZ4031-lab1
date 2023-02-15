#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include <bits/stdc++.h>

using namespace std;
template <typename _key, typename _record, int N>
class BPTree {
private:
    // keys, child, cnt, height are on disk
    struct node {
        node() : cnt{0}, height{0} { memset(child, 0, sizeof(child)); }

        _key keys[N];
        void *child[N + 1];
        size_t cnt, height;

        // for debugging
        friend ostream &operator<<(ostream &os, const node &n) {
            os << "(" << n.height << ") ";
            for (size_t i = 0; i < n.cnt; i++) {
                os << n.keys[i] << " ";
            }
            os << "| ";
            return os;
        }
    };

    static size_t globalNodeCnt;

    node *root;
    size_t nodeCnt;

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

    pair<node *, size_t> _searchNode(node *cur, const _key &key) const {
        size_t i =
                (size_t)(lower_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        // leaf node
        if (cur->height == 0) return pair<node *, size_t>(cur, i);
        // non leaf
        if (i == 0 && key < cur->keys[0])
            return _searchNode((node *)cur->child[0], key);
        else
            return _searchNode((node *)cur->child[i + 1], key);
    }

    // for non leaf node
    void _updateHeight(node *cur) {
        node *ch = (node *)cur->child[0];
        size_t mx = ch->height;
        for (size_t i = 0; i < cur->cnt; i++) {
            ch = (node *)cur->child[i + 1];
            mx = max(mx, ch->height);
        }
        cur->height = mx + 1;
    }

    node *_insertAtLeaf(node *ch1, const _key &key, _record *record) {
        // insert at index i
        size_t i =
                (size_t)(upper_bound(ch1->keys, ch1->keys + ch1->cnt, key) - ch1->keys);
        if (ch1->cnt < N) {  // leaf is not full
            ch1->cnt++;
            for (size_t j = ch1->cnt - 1; j > i; j--) {
                ch1->keys[j] = ch1->keys[j - 1];
                ch1->child[j] = ch1->child[j - 1];
            }
            ch1->keys[i] = key;
            ch1->child[i] = (void *)record;
            // no split
            return nullptr;
        } else {
            node *ch2 = newNode();
            void *ch3 = ch1->child[N];
            _key tmpKey[N + 1];
            void *tmpChild[N + 2];
            // copy of ch1
            memcpy(tmpKey, ch1->keys, sizeof(ch1->keys));
            memcpy(tmpChild, ch1->child, sizeof(ch1->child));

            // insert new key and record
            for (size_t j = N; j > i; j--) {
                tmpKey[j] = tmpKey[j - 1];
                tmpChild[j] = tmpChild[j - 1];
            }
            tmpKey[i] = key;
            tmpChild[i] = (void *)record;

            // write to ch1 and ch2
            ch1->cnt = (N + 1) / 2;
            for (size_t j = 0; j < ch1->cnt; j++) {
                ch1->keys[j] = tmpKey[j];
                ch1->child[j] = tmpChild[j];
            }
            ch2->cnt = (N + 1) - ch1->cnt;
            for (size_t j = 0; j < ch2->cnt; j++) {
                ch2->keys[j] = tmpKey[j + ch1->cnt];
                ch2->child[j] = tmpChild[j + ch1->cnt];
            }
            // connect siblings ch1 -> ch2 -> ch3;
            ch2->child[N] = ch3;
            ch1->child[N] = (void *)ch2;
            return ch2;
        }
    }

    node *_insertAtInternal(node *ch1, const _key &key, node *ptr) {
        // insert at index i
        size_t i =
                (size_t)(upper_bound(ch1->keys, ch1->keys + ch1->cnt, key) - ch1->keys);
        if (ch1->cnt < N) {  // node is not full
            ch1->cnt++;
            for (size_t j = ch1->cnt - 1; j > i; j--) {
                ch1->keys[j] = ch1->keys[j - 1];
                ch1->child[j + 1] = ch1->child[j];
            }
            ch1->keys[i] = key;
            ch1->child[i + 1] = ptr;
            return nullptr;
        } else {  // node is full, need to split node into ch1 ch2
            node *ch2 = newNode();
            _key tmpKey[N + 1];
            void *tmpChild[N + 2];
            // copy of ch1
            memcpy(tmpKey, ch1->keys, sizeof(ch1->keys));
            memcpy(tmpChild, ch1->child, sizeof(ch1->child));

            // insert new key and record
            for (size_t j = N; j > i; j--) {
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
            for (size_t j = 0; j < ch1->cnt; j++) {
                ch1->keys[j] = tmpKey[j];
                ch1->child[j + 1] = tmpChild[j + 1];
            }
            ch2->cnt = (N + 1) - ch1->cnt - 1;
            for (size_t j = 0; j < ch2->cnt; j++) {
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
    }

    // helper
    node *_insertHelper(node *cur, const _key &key, _record *record) {
        // non-root leaf
        if (cur->height == 0) return _insertAtLeaf(cur, key, record);
        size_t i =
                (size_t)(upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        if (i > 0) i--;
        node *p;
        if (i == 0 && key < cur->keys[0])
            p = _insertHelper((node *)cur->child[0], key, record);
        else
            p = _insertHelper((node *)cur->child[i + 1], key, record);
        if (!p) return nullptr;  // no need split further
        // insert to current node
        node *ret = _insertAtInternal(cur, p->keys[0], p);
        return ret;
    }

//    void _deleteAtLeaf(node *ch1, const _key &key) {
//        // delete at index i
//        size_t i =
//                (size_t)(lower_bound(ch1->keys, ch1->keys + ch1->cnt, key) - ch1->keys);
//    }

public:
    static size_t globalSize() { return globalNodeCnt; }
    static node *newNodeGlobal() {
        // TODO: use disk pool
        return new node();
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
        size_t sz = 1;
        while (q.size()) {
            size_t nxt = 0;
            for (size_t i = 1; i <= sz; i++) {
                node *frt = q.front();
                q.pop();
                cout << (*frt);
                if (frt->height == 0) continue;
                for (size_t j = 0; j <= frt->cnt; j++) {
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

    size_t height() const { return root->height; }

    size_t size() const { return nodeCnt; }

    // if key already exists, insert to upper bound
    void insert(const _key &key, _record *record) {
        size_t h = root->height;
        node *p = _insertHelper(root, key, record);
        if (p) {         // if split happened
            if (h == 0) {  // the root was leaf, need to create new root
                node *newRt = newNode();
                newRt->cnt = 1;
                newRt->keys[0] = p->keys[0];
                newRt->child[0] = root;
                newRt->child[1] = p;
                _updateHeight(newRt);
                root = newRt;
            } else {  // the root was internal node, parent is new root
                root = p;
            }
        }
    }

//    void _remove(const _key &key) {
//
//    }

    // query single key, nullptr if doesn't exist
    _record *query(const _key *key) const {
        pair<node *, size_t> q = _searchNode(root, key);
        node *p = q.first;
        size_t i = q.second;
        if (i >= p->cnt || p->keys[i] != key) return nullptr;
        return (_record *)p->child[i];
    }

    // query [lo, hi)
    vector<_record *> query(const _key &lo, const _key &hi) const {
        assert(lo < hi);
        pair<node *, size_t> q = _searchNode(root, lo);
        node *p = q.first;
        size_t i = q.second;

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
size_t BPTree<_key, _record, N>::globalNodeCnt = 0;

typedef BPTree<int, int, 3> tree;


#endif //BPTREE_BPTREE_H
