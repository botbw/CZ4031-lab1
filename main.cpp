#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <time.h>
#include "BPTree.h"

using namespace std;

#pragma pack(1)

struct _key {
    unsigned int key: 24;

    bool operator<(const _key &b) const {
        return key < b.key;
    }

    friend string to_string(const _key &b) {
        return to_string(b.key);
    }

    friend std::ostream &operator<<(std::ostream &os, const _key &b) {
        os << b.key;
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

// definition of B+ tree
const int N = 3;
using tree = BPTree<_key, _record, N>;
using node = tree::node;

tree *constructTreeFromTsv(string filename) {
    cout << "enter here 0" << "\n";
    tree *trp = new tree();

    ifstream fin(filename);
    string line;
    getline(fin, line);

    unsigned int max_numVotes = 0;
    unsigned int max_tconst = 0;
    cout << "enter here 1" << "\n";
    int cnt = 0;
    while (getline(fin, line)) {
        cnt++;
        istringstream is(line);
        string tconst_str;
        string rating_str;
        unsigned int numVotes_full = 0;
        getline(is, tconst_str, '\t');
        getline(is, rating_str, '\t');
        is >> numVotes_full;

        unsigned int tconst_full = stoi(tconst_str.substr(2, tconst_str.size() - 2));
        unsigned int rating_full = (rating_str[0] - '0') * 10 + (rating_str[2] - '0');

        _key skey = {numVotes_full};
        _record srecord = {tconst_full, rating_full, numVotes_full};
        trp->insert(skey, srecord);

        max_numVotes = max(max_numVotes, numVotes_full);
        max_tconst = max(max_tconst, tconst_full);

    }
    fin.close();
    cout << cnt << endl;
    cout << "max_numVotes = " << max_numVotes << ", max_tconst = " << max_tconst << "\n";
    cout << trp->disk.getAllocatedBlock() << "blks\t" << trp->disk.getAllocatedMem()/1024/1024 << "mb\n";


    return trp;
}

void randomTest() {
    using tree = BPTree<int, int, 3>;

    int n = 50000;
    int range = rand();

    tree tr;
    vector<int> insertedVal; // to store insertion history
    multiset<int> sett; // to simulate the BPTree

    cout << n << " random operations (insert, delete, query)" << endl;

    for (int i = 1; i <= n; i++) {
        int op = rand() % 3;
        if (op == 0) { // insert
            int num = rand() % range;
            tr.insert(num, num);
            insertedVal.push_back(num);
            sett.insert(num);
            assert(tr.selfCheck());
        } else if (op == 1) { // delete
            if (insertedVal.size() == 0) {
                i--;
                continue;
            }
            int id = rand() % ((int) insertedVal.size());
            bool deleted = tr.remove(insertedVal[id]);
            bool _deleted = sett.count(insertedVal[id]);
            if (_deleted) sett.erase(sett.find(insertedVal[id]));
            assert(deleted == _deleted);
            assert(tr.selfCheck());
        } else { // query
            if (insertedVal.size() == 0) {
                i--;
                continue;
            }
            // two random value from insert history
            int id1 = rand() % ((int) insertedVal.size());
            int id2 = rand() % ((int) insertedVal.size());
            int lo = min(insertedVal[id1], insertedVal[id2]);
            int hi = max(insertedVal[id1], insertedVal[id2]);
            auto tmp = tr.query(lo, hi); // [lo, hi]
            vector<int> q1;
            for (auto p: tmp) {
                q1.push_back(*p);
            }
            auto it1 = sett.lower_bound(lo), it2 = sett.upper_bound(hi);
            vector<int> q2(it1, it2);
            assert(q1 == q2);
        }
        assert(tr.size() == sett.size());
    }
    cout << "tree survives after " << n << " operations" << endl;
    cout << "tree height: " << tr.height() << endl;
    cout << "tree structure" << endl;
    tr.levelTraverse();
    cout << "pass" << endl;
    return;
}

void runExperiment() {
    tree *tr = constructTreeFromTsv("../data.tsv");
    cout << "exp1.1: ... " << endl;
    // run1_1();
    // ...
    // ...
}

int main() {
    cout << sizeof(_record) << endl;
    cout << sizeof(_key) << endl;
    srand(time(NULL));
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    randomTest();
//     runExperiment();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    std::cout << "Execution time: " << cpu_time_used << " seconds" << std::endl;
    return 0;

}
