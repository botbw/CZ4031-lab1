#include <iostream>
#include "BPTree.h"

// num of children
const int N = 10;

using tree= BPTree<int, int, N>;
using node = tree::node;

vector<int> getAllFromZero(const tree &tr) {
    vector<int> a;
    // get [0, inf) into a
    auto tmp = tr.lower_bound(-1);
    int i = tmp.second;
    node *p = tmp.first;
    while(p) {
        for(; i < p->cnt; i++) {
            a.push_back(p->keys[i]);
        }
        p = (node*)p->childs[N];
        i = 0;
    }
    return a;
}

void functionalTest() {
    int range = rand();
    int n = 100000;

    tree tr;
    multiset<int> s; // to simulate tree

    cout << "try " << n << " random insertions and then "<< n << " deletions:" << endl;
    for (int i = 1; i <= n; i++) {
        int num = rand() % range;
        tr.insert(num, num);
//        tr.levelTraverse();
        assert(tr.selfCheck());
        s.insert(num);
    }

    vector<int> a = getAllFromZero(tr);
    vector<int> b(s.begin(), s.end());
    assert(a == b);

    cout << "tree survives after " << n << " insertions" << endl;
    cout << "tree height: " << tr.height() << endl;

    for (int i = 1; i <= n; i++) {
        int id = rand() % ((int) a.size());
        bool deleted = tr.remove(a[id]);
        assert(tr.selfCheck());
        bool _deleted = s.contains(a[id]);
        if(deleted != _deleted) cout << a[id] << endl;
        assert(deleted == _deleted);
        if(_deleted == true) s.erase(s.find(a[id]));
    }
    cout << "tree survives after " << n << " deletions" << endl;
    cout << "tree height: " << tr.height() << endl;
    cout << "tree structure" << endl;
    tr.levelTraverse();
    cout << "pass\n";
}

void randomTest() {
    int n = 100000;
    int range = rand();

    tree tr;
    vector<int> b; // to store inserted keys
    multiset<int> s; // to simulate deletions

    cout << n << " random operations" << endl;

    for(int i = 1; i <= n; i++) {
        int op = rand() % 2;
        if(op == 0) { // insert
            int num = rand() % range;
            tr.insert(num, num);
            b.push_back(num);
            s.insert(num);
        } else {
            if(b.size() == 0) continue;
            int id = rand() % ((int) b.size());
            bool deleted = tr.remove(b[id]);
            bool _deleted = s.contains(b[id]);

            assert(deleted == _deleted);

            if(_deleted) s.erase(s.find(b[id]));
        }
        assert(tr.selfCheck());
    }
    cout << "tree survives after " << n << " operations" << endl;
    cout << "tree height: " << tr.height() << endl;
    cout << "tree structure" << endl;
    tr.levelTraverse();
    cout << "pass" << endl;
}


int main() {
    srand(time(NULL));
    functionalTest();
    randomTest();
    return 0;
}
