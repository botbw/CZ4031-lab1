#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include "BPTree.h"

using namespace std;

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
const int N = 3;

using tree = BPTree<int, int, N>;

using node = tree::node;

//
//tree* constructTreeFromTsv(string filename){
//    cout << "enter here 0" << "\n";
//    tree* trp = new tree();
//
//    ifstream fin(filename);
//    string line;
//    getline(fin, line);
//
//    unsigned int max_numVotes = 0;
//    unsigned int max_tconst = 0;
//    cout << "enter here 1" << "\n";
//    while (getline(fin, line)) {
//        istringstream is(line);
//        string tconst_str;
//        string rating_str;
//        unsigned int numVotes_full = 0;
//        getline( is, tconst_str, '\t' );
//        getline( is, rating_str, '\t' );
//        is >> numVotes_full;
//
//        unsigned int tconst_full = stoi(tconst_str.substr(2, tconst_str.size()-2));
//        unsigned int rating_full = (rating_str[0] - '0')*10 + (rating_str[2] - '0');
//
//        _key skey = {numVotes_full};
//        _record srecord = {tconst_full, rating_full, numVotes_full};
//        trp->insert(skey, srecord);
//
//        max_numVotes = max(max_numVotes, numVotes_full);
//        max_tconst = max(max_tconst, tconst_full);
//
//    }
//    fin.close();
//
//    cout << "max_numVotes = " << max_numVotes << ", max_tconst = " << max_tconst << "\n";
//
//    return trp;
//}

void randomTest() {
    int n = 50000;
    int range = rand();

    tree tr;
    vector<int> b; // to store inserted keys
    multiset<int> s; // to simulate the BPTree

    cout << n << " random operations (insert, delete, query)" << endl;

    for (int i = 1; i <= n; i++) {
        int op = rand() % 3;
        if (op == 0) { // insert
            int num = rand() % range;
            tr.insert(num, num);
            b.push_back(num);
            s.insert(num);
            assert(tr.selfCheck());
        } else if(op == 1) { // delete
            if (b.size() == 0) continue;
            int id = rand() % ((int) b.size());
            bool deleted = tr.remove(b[id]);
            bool _deleted = s.contains(b[id]);
            if (_deleted) s.erase(s.find(b[id]));
            assert(deleted == _deleted);
            assert(tr.selfCheck());
        } else { // query
            if (b.size() == 0) continue;
            int id1 = rand() % ((int) b.size());
            int id2 = rand() % ((int) b.size());
            int lo = min(b[id1], b[id2]);
            int hi = max(b[id1], b[id2]);
            auto tmp = tr.query(lo, hi); // [lo, hi]
            vector<int> q1;
            for(auto p : tmp) {
                q1.push_back(*p);
            }
            auto it1 = s.lower_bound(lo), it2 = s.upper_bound(hi);
            vector<int> q2(it1, it2);
            assert(q1 == q2);
        }

    }
    cout << "tree survives after " << n << " operations" << endl;
    cout << "tree height: " << tr.height() << endl;
    cout << "tree structure" << endl;
    tr.levelTraverse();
    cout << "pass" << endl;
}

int main() {
    //srand(time(NULL));

    randomTest();
    BPTree<string, string, 3> tr;
    tr.insert("nihao", "wobuhao");
    tr.insert("haha", "heihei");
    tr.insert("aldsjfldsafj", "hadsfsdafeihei");
    tr.insert("1213fdsa", "2134");
    tr.insert("d123", "adsfn");

    tr.levelTraverse();

    return 0;
    
}
