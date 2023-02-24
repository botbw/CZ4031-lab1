#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <cstdlib>
#include <cassert>

#include "BPTree.h"

#pragma pack(1)
struct _key {
    unsigned int key: 24;
    bool operator<(const _key &b) const {
        return key < b.key;
    }
    string to_string(const _key &b) const {
        int key_int  = b.key;
        stringstream temp;
        temp<<key_int;
        return temp.str();
    }
    friend std::ostream& operator << ( std::ostream& os, const _key& b ) {
        int key_int  = b.key;
        os << key_int;
        return os;
    }
};
#pragma pack(0)

#pragma pack(1)
struct _record {
    unsigned int tConst: 24;
    unsigned int rating: 8;
    unsigned int numVotes: 24;
};
#pragma pack(0)

// num of children
const int N = 10;

using tree= BPTree<_key, _record, N>;

using node = tree::node;


tree* constructTreeFromTsv(string filename){
    cout << "enter here 0" << "\n";
    tree* trp = new tree();

    ifstream fin(filename);
    string line;
    getline(fin, line);
    
    unsigned int max_numVotes = 0;
    unsigned int max_tconst = 0;
    cout << "enter here 1" << "\n";
    while (getline(fin, line)) {
        istringstream is(line);
        string tconst_str;
        string rating_str;
        unsigned int numVotes_full = 0;
        getline( is, tconst_str, '\t' );
        getline( is, rating_str, '\t' );
        is >> numVotes_full; 

        unsigned int tconst_full = stoi(tconst_str.substr(2, tconst_str.size()-2));
        unsigned int rating_full = (rating_str[0] - '0')*10 + (rating_str[2] - '0');

        struct _key skey = {numVotes_full};
        struct _record srecord = {tconst_full, rating_full, numVotes_full};
        trp->insert(skey, srecord);

        max_numVotes = max(max_numVotes, numVotes_full);
        max_tconst = max(max_tconst, tconst_full);

    }
    fin.close();

    cout << "max_numVotes = " << max_numVotes << ", max_tconst = " << max_tconst << "\n";

    return trp;
}

/*
vector<int> getAllFromZero(const tree &tr) {
    vector<int> a;
    // get [0, inf) into a
    auto tmp = tr.lower_bound(-1);
    int i = tmp.second;
    node *p = tmp.first;
    while (p) {
        for (; i < p->cnt; i++) {
            a.push_back(p->keys[i]);
        }
        p = (node *) p->childs[N];
        i = 0;
    }
    return a;
}

void functionalTest() {
    int range = rand();
    int n = 10000;

    tree tr;
    multiset<int> s; // to simulate tree

    cout << "try " << n << " random insertions and then " << n << " deletions:" << endl;
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
        if (deleted != _deleted) cout << a[id] << endl;
        assert(deleted == _deleted);
        if (_deleted) s.erase(s.find(a[id]));
    }
    cout << "tree survives after " << n << " deletions" << endl;
    cout << "tree height: " << tr.height() << endl;
    cout << "tree structure" << endl;
    tr.levelTraverse();
    cout << "pass\n";
}

void randomTest() {
    int n = 10000;
    int range = rand();

    tree tr;
    vector<int> b; // to store inserted keys
    multiset<int> s; // to simulate deletions

    cout << n << " random operations" << endl;

    for (int i = 1; i <= n; i++) {
        int op = rand() % 2;
        if (op == 0) { // insert
            int num = rand() % range;
            tr.insert(num, num);
            b.push_back(num);
            s.insert(num);
        } else {
            if (b.size() == 0) continue;
            int id = rand() % ((int) b.size());
            bool deleted = tr.remove(b[id]);
            bool _deleted = s.contains(b[id]);

            assert(deleted == _deleted);

            if (_deleted) s.erase(s.find(b[id]));
        }
        assert(tr.selfCheck());
    }
    cout << "tree survives after " << n << " operations" << endl;
    cout << "tree height: " << tr.height() << endl;
    cout << "tree structure" << endl;
    tr.levelTraverse();
    cout << "pass" << endl;
}
*/

int main() {
    //srand(time(NULL));
    //functionalTest();
    //randomTest();
    cout << "start \n";
    tree* trp = constructTreeFromTsv("data.tsv");
    cout << "tree height: " << trp->height() << endl;
    cout << "tree structure" << endl;
    trp->levelTraverse();
    cout << "pass" << endl;
    return 0;
    
}
