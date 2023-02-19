#include <iostream>
#include "BPTree.h"

using tree= BPTree<int, int, 3>;
using node = tree::node;

vector<int> getAllFromZero(const tree &tr) {
    vector<int> a;
    // get [0, inf) into a
    auto tmp = tr.lower_bound(-1);
    int i = tmp.second;
    tree::node *p = tmp.first;
    while(p) {
        for(; i < p->cnt; i++) {
            a.push_back(p->keys[i]);
        }
        p = (tree::node*)p->childs[3];
        i = 0;
    }
    return a;
}

void functionalTest() {
    int range = rand();
    int n = 10000;

    tree tr;
    vector<int> b; // to store inserted keys
    multiset<int> s; // to simulate deletions

    cout << "try " << n << " random insertions and then "<< n << " deletions:" << endl;
    for (int i = 1; i <= n; i++) {
        int num = rand() % range;
        tr.insert(num, nullptr);
//        tr.levelTraverse();
        assert(tr.selfCheck());
        b.push_back(num);
        s.insert(num);
    }

    sort(b.begin(), b.end());
    vector<int> a = getAllFromZero(tr);
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

    cout << "pass\n";
}

void randomTest() {
    int n = 10000;
    int range = rand();

    tree tr;
    vector<int> b; // to store inserted keys
    multiset<int> s; // to simulate deletions

    cout << n << " random operations" << endl;

    for(int i = 1; i <= n; i++) {
        int op = rand() % 2;
        if(op == 0) { // insert
            int num = rand() % range;
            tr.insert(num, nullptr);
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

    cout << "pass" << endl;
}


int main() {
    functionalTest();
    randomTest();
    return 0;
}
