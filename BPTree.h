#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include "bits/stdc++.h"

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

public:
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

    // insert record ptr at leaf node
    //
    // if split happened return the new node ptr
    // return nullptr
    node *_insertAtLeaf(node *ch1, const _key &key, _record *record) {
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
            ch1->childs[i] = (void *) record;
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
        tmpChild[i] = (void *) record;

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
    void _insertAtInternal(node *&ch1, int i, const _key &key, node *ptr) {
        // insert at index i
        if (ch1->cnt < N) {  // node is not full
            ch1->cnt++;
            for (int j = ch1->cnt - 1; j > i; j--) {
                ch1->keys[j] = ch1->keys[j - 1];
                ch1->childs[j + 1] = ch1->childs[j];
            }
            ch1->keys[i] = key;
            ch1->childs[i + 1] = ptr;
            return;
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
        _key parentKey = tmpKey[(N + 1) / 2];
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
            ch2->childs[j + 1] = tmpChilds[j + (N + 1) / 2 + 1 + j + 1];
        }

        // parent node
        node *parent = newNode();
        parent->cnt = 1;
        parent->keys[0] = parentKey;
        parent->childs[0] = ch1;
        parent->childs[1] = ch2;
        _updateHeight(ch1);
        _updateHeight(ch2);
        _updateHeight(parent);
        ch1 = parent;
    }

    // recursively find insertion position
    // handle new ptr from children if necessary
    node *_insertHelper(node* &cur, const _key &key, _record *record) {
        // non-root leaf
        if (cur->height == 0) return _insertAtLeaf(cur, key, record);
        int i =
                (int) (upper_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        i--;
        node *value = (node*) (cur->childs[i + 1]);
        node *p = _insertHelper(value, key, record);
        cur->childs[i + 1] = value;
        if (!p) return nullptr;  // no need split further
        // insert to current node
        _insertAtInternal(cur, i + 1, p->keys[0], p);
        return nullptr;
    }

    // ch1 -> ch2 -> ch3
    // return status
    // -1: key not found and no modification
    // 0: ch2 is fine after deleting the key
    // 1: merge ch2 into ch1, 2: merge ch3 into ch2
    int _removeAtLeaf(node *ch1, _key *pKey1, node *ch2, _key *pKey2, node *ch3, const _key &key) {
        // delete at index i
        int i =
                (int) (std::lower_bound(ch2->keys, ch2->keys + ch2->cnt, key) - ch2->keys);

        // didn't find key
        if (i == ch2->cnt || ch2->keys[i] != key) return -1;

        // remove the key
        for (int j = i; j < ch2->cnt - 1; j++) {
            ch2->keys[j] = ch2->keys[j + 1];
            ch2->childs[j] = ch2->childs[j + 1];
        }
        ch2->cnt--;

        // case 1 ch2 is still fine
        if (ch2->cnt >= (N + 1) / 2) return 0;

        // case 2 ch2 borrow from siblings
        // borrow from ch1
        if (ch1 && ch1->cnt > (N + 1) / 2) {
            ch2->cnt++;
            for (int j = ch2->cnt - 1; j > 0; j--) {
                ch2->keys[j] = ch2->keys[j - 1];
                ch2->childs[j] = ch2->childs[j - 1];
            }
            ch2->keys[0] = ch1->keys[ch1->cnt - 1];
            ch2->childs[0] = ch1->childs[ch1->cnt - 1];
            ch1->cnt--;
            *pKey1 = ch2->keys[0];
            return 0;
        }

        // borrow from ch3
        if (ch3 && ch3->cnt > (N + 1) / 2) {
            ch2->cnt++;
            ch2->keys[ch2->cnt - 1] = ch3->keys[0];
            ch2->childs[ch2->cnt - 1] = ch3->childs[0];
            for (int j = 0; j < ch3->cnt - 1; j++) {
                ch3->keys[j] = ch3->keys[j + 1];
                ch3->childs[j] = ch3->childs[j + 1];
            }
            ch3->cnt--;
            *pKey2 = ch3->keys[0];
            return 0;
        }

        // case 3 cannot borrow
        // merge ch2 into ch1
        if (ch1) {
            for (int j = 0; j < ch2->cnt; j++) {
                ch1->cnt++;
                ch1->keys[ch1->cnt - 1] = ch2->keys[j];
                ch1->childs[ch1->cnt - 1] = ch2->childs[j];
            }
            ch1->childs[N] = ch3;
            deleteNode(ch2);
            return 1;
        }

        // merge ch2 into ch3
        if (ch3) {
            for (int j = 0; j < ch3->cnt; j++) {
                ch2->cnt++;
                ch2->keys[ch2->cnt - 1] = ch3->keys[j];
                ch2->childs[ch2->cnt - 1] = ch3->childs[j];
            }
            ch2->childs[N] = ch3->childs[N];
            deleteNode(ch3);
            return 2;
        }

        // only happens at root
        return 0;
    }

    //        key1   key2
    //         |      |
    // ch1 -> ch2 -> ch3
    // return status
    // 0: ch2 is fine after deleting the key
    // 1: merge ch2 into ch1, 2: merge ch3 into ch2
    int _removeAtInternal(node *ch1, _key *pKey1, node *ch2, _key *pKey2,
                          node *ch3, int i) {
        // remove the key
        for (int j = i; j < ch2->cnt - 1; j++) {
            ch2->keys[j] = ch2->keys[j + 1];
            ch2->childs[j + 1] = ch2->childs[j + 2];
        }
        ch2->cnt--;

        // case 1 node is still fine
        if (ch2->cnt >= N / 2) return 0;

        // case 2 borrow from siblings

        // zig
        if (ch1 && ch1->cnt > N / 2) {
            ch2->cnt += 1;
            for (int j = ch2->cnt - 1; j > 0; j--) {
                ch2->keys[j] = ch2->keys[j - 1];
                ch2->childs[j + 1] = ch2->childs[j];
            }
            ch2->childs[1] = ch2->childs[0];

            // left most pointer and borrowed key from parent
            ch2->keys[0] = *pKey1;
            ch2->childs[0] = ch1->childs[ch1->cnt];
            *pKey1 = ch1->keys[ch1->cnt - 1];

            ch1->cnt--;
            return 0;
        }

        // zag
        if (ch3 && ch3->cnt > N / 2) {
            ch2->cnt++;
            ch2->keys[ch2->cnt - 1] = *pKey2;
            ch2->childs[ch2->cnt] = ch3->childs[0];
            *pKey2 = ch3->keys[0];
            ch3->childs[0] = ch3->childs[1];
            for (int j = 0; j < ch3->cnt - 1; j++) {
                ch3->keys[j] = ch3->keys[j + 1];
                ch3->childs[j + 1] = ch3->childs[j + 2];
            }
            ch3->cnt--;
            return 0;
        }

        // case 3 cannot borrow

        // merge ch2 into ch1
        if (ch1) {
            ch1->cnt++;
            ch1->keys[ch1->cnt - 1] = *pKey1;
            ch1->childs[ch1->cnt] = ch2->childs[0];
            for (int j = 0; j < ch2->cnt; j++) {
                ch1->cnt++;
                ch1->keys[ch1->cnt - 1] = ch2->keys[j];
                ch1->childs[ch1->cnt] = ch2->childs[j + 1];
            }
            deleteNode(ch2);
            return 1;
        }

        // merge ch3 into ch2
        if (ch3) {
            ch2->cnt++;
            ch2->keys[ch2->cnt - 1] = *pKey2;
            ch2->childs[ch2->cnt] = ch3->childs[0];
            for (int j = 0; j < ch3->cnt; j++) {
                ch2->cnt++;
                ch2->keys[ch2->cnt - 1] = ch3->keys[j];
                ch2->childs[ch2->cnt] = ch3->childs[j + 1];
            }
            deleteNode(ch3);
            return 2;
        }

        // only happens at root
        return 0;
    }

    int _removeHelper(node *ch1, _key *pKey1, node *ch2, _key *pKey2, node *ch3,
                      const _key &key) {
        // leaf node
        if (ch2->height == 0) return _removeAtLeaf(ch1, pKey1, ch2, pKey2, ch3, key);
        // internal node
        int i = (int) (std::lower_bound(ch2->keys, ch2->keys + ch2->cnt, key) - ch2->keys);
        if (i == ch2->cnt || key < ch2->keys[i]) i--;
        node *_ch1 = (i == -1 ? nullptr : (node *) ch2->childs[i]);
        _key *_pKey1 = (i == -1 ? nullptr : &ch2->keys[i]);
        node *_ch2 = (node *) ch2->childs[i + 1];
        _key *_pKey2 = (i == ch2->cnt - 1 ? nullptr : &ch2->keys[i + 1]);
        node *_ch3 = (i == ch2->cnt - 1 ? nullptr : (node *) ch2->childs[i + 2]);
        int status = _removeHelper(_ch1, _pKey1, _ch2, _pKey2, _ch3, key);
        if (status == 0 || status == -1)
            return status;
        else if (status == 1) // merged ch2 -> ch1, delete key at i
            return _removeAtInternal(ch1, pKey1, ch2, pKey2, ch3, i);
        else if (status == 2) // merged ch3 -> ch2, delete key at i + 1
            return _removeAtInternal(ch1, pKey1, ch2, pKey2, ch3, i + 1);
        // should never reach here
        throw runtime_error("In _removeAtInternal: should never reach here");
    }

    void _destruct(node *cur) {
        if (cur->height == 0) {
            deleteNode(cur);
            return;
        }
//        _destruct((node *) cur->childs[0]);
        for (int i = 0; i < cur->cnt; i++)
            _destruct((node *) cur->childs[i + 1]);
        deleteNode(cur);
    }

    // search lower_bound according to key (the first record_key >= key)
    pair<node *, int> _lower_bound(node *cur, const _key &key) const {
        int i =
                (int) (std::lower_bound(cur->keys, cur->keys + cur->cnt, key) - cur->keys);
        // leaf node
        if (cur->height == 0) return pair<node *, int>(cur, i);
        // non leaf
        if (i >= cur->cnt || key < cur->keys[i]) i--;
        return _lower_bound((node *) cur->childs[i + 1], key);
    }

public:
    static int globalNodeSize() { return globalNodeCnt; }

    static node *newNodeGlobal() {
        // TODO: use disk pool
        node *p = new node();

        p->cnt = 0;
        p->height = 0;
        memset(p->childs, 0, sizeof 0);
        return p;
    }

    // only for single node
    static void deleteNodeGlobal(node *p) {
        // TODO: use disk pool
        globalNodeCnt--;
        memset(p->childs, 0x3f, sizeof(p->childs));
        delete p;
    }

    // for debugging
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

    BPTree() : root{newNode()}, nodeCnt{0}, recordCnt{0} {}

    ~BPTree() {
        _destruct(root);
    }

    int height() const { return root->height; }

    int size() const { return recordCnt; }

    int nodeSize() const { return nodeCnt; }

    pair<node*, int> lower_bound(const _key &key) const {
        return _lower_bound(root, key);
    }

    // if key already exists, insert to upper bound
    void insert(const _key &key, _record *record) {
        recordCnt++;
        if (root->height == 0) {  // root is leaf
            node *p = _insertAtLeaf(root, key, record);
            if (p) {  // root is splited
                node *newRt = newNode();
                newRt->cnt = 1;
                newRt->keys[0] = p->keys[0];
                newRt->childs[0] = root;
                newRt->childs[1] = p;
                _updateHeight(newRt);
                root = newRt;
            }
        } else {
            node *p = _insertHelper(root, key, record);  // root is split
            if (p) root = p;
        }
    }

    bool remove(const _key &key) {
        if (root->height == 0) {
            int status = _removeAtLeaf(nullptr, nullptr, root, nullptr, nullptr, key);
            if (status != -1) recordCnt--;
            return status != -1;
        } else {
            int status = _removeHelper(nullptr, nullptr, root, nullptr, nullptr, key);
            if (root->cnt == 0) {
                node *p = root;
                root = (node *) root->childs[0];
                deleteNode(p);
            }
            if(status != -1) recordCnt--;
            return status != -1;
        }
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
};

template<typename _key, typename _record, int N>
int BPTree<_key, _record, N>::globalNodeCnt = 0;

#endif //BPTREE_BPTREE_H
