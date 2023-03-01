#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <time.h>
#include "BPTree.h"

using namespace std;

const int MAXN = 16;

#pragma pack(1) // might not work on x86

struct _key {
    unsigned int key: 24;

    // functional comparator, for BPTree
    bool operator<(const _key &b) const {
        return key < b.key;
    }

    // for assertion and other purposes
    bool operator<=(const _key &b) const {
        return key <= b.key;
    }

    bool operator>(const _key &b) const {
        return key > b.key;
    }

    bool operator>=(const _key &b) const {
        return key >= b.key;
    }

    bool operator==(const _key &b) const {
        return key == b.key;
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

#pragma pack(1) // might not work on x86
struct _record {
    unsigned int tConst: 24;
    unsigned int rating: 8;
    unsigned int numVotes: 24;
};
#pragma pack(0)

void printLinebreak() {
    cout << "---------------------------------------------------------------------";
    cout << endl;
}

template<int n>
void correctnessTest() {
    using tree = BPTree<int, int, n>;

    // selfCheck take a lot of time, OPERATIONS cannot be too large
    const int OPERATIONS = 10000;
    int range = rand();

    tree tr;
    vector<int> insertedVal; // to store insertion history
    multiset<int> sett; // to simulate the BPTree

    cout << OPERATIONS << " random operations (insert, delete, query) on BPTree<int, int, " << n << ">" << endl;

    for (int i = 1; i <= OPERATIONS; i++) {
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
            int shiftRange = range / 10;
            int offset = (rand() % shiftRange) - (shiftRange / 2);
            int lo = min(insertedVal[id1] + offset, insertedVal[id2] + offset);
            int hi = max(insertedVal[id1] + offset, insertedVal[id2] + offset);
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
    cout << "pass" << endl;
    printLinebreak();
    correctnessTest<n - 1>();
}

template<>
void correctnessTest<1>() {
    return;
}

template<int n>
void insertionEfficiencyTest(int &bestN, int &bestClk) {
    using tree = BPTree<int, int, n>;

    // around the number of data.tsv
    const int OPERATIONS = 1000000;
    int range = rand();

    tree tr;

    cout << OPERATIONS << " random insertions on BPTree<int, int, " << n << ">" << endl;

    clock_t st = clock();

    for (int i = 1; i <= OPERATIONS; i++) {
        int num = rand() % range;
        clock_t st = clock();
        tr.insert(num, num);
    }

    clock_t duration = clock() - st;

    if (duration < bestClk) {
        bestClk = duration;
        bestN = n;
    }

    cout << "runtime for " << n << " : " << (double) duration / CLOCKS_PER_SEC << " s" << endl;
    printLinebreak();

    insertionEfficiencyTest<n - 1>(bestN, bestClk);
}

template<>
void insertionEfficiencyTest<1>(int &bestN, int &bestClk) {
    return;
}

void findBestN() {
    int bestN = 0, bestClk = INT32_MAX;
    insertionEfficiencyTest<MAXN>(bestN, bestClk);
    cout << "best n is " << bestN << " with runtime " << (double) bestClk / CLOCKS_PER_SEC << " s" << endl;
}

const int N = 15;
using tree = BPTree<_key, _record, N>;

tree *constructTreeFromTsv(string filename) {
    // definition of B+ tree
    using tree = BPTree<_key, _record, N>;
    using node = tree::node;
    cout << "Start processing the tsv..." << "\n";
    tree *trp = new tree();

    ifstream fin(filename);
    if (!fin) {
        throw runtime_error("data.tsv not found");
    }
    string line;
    getline(fin, line);

    unsigned int max_numVotes = 0;
    unsigned int max_tconst = 0;

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
    cout << "max of numVotes = " << max_numVotes << ", max of tconst = " << max_tconst << "\n";

    return trp;
}


void experiment1(tree *tr) {
    cout << "Start Emperiment 1: " << "\n";

    cout << "1.1 number of records: " << tr->size() << "\n";
    cout << "1.2 size of a record: " << sizeof(_record) << " bytes\n";

    tr->getDisk();

    cout << "1.3 number of records stored in a block: " << sizeof(Block) / sizeof(_record) << "\n";
    cout << "1.4 number of blocks to storing data: " << tr->getDisk()->getAllocatedBlock() << "\n";

    cout << "Completed Experiment 1. " << "\n\n";
}

void experiment2(tree *tr) {
    cout << "Start Emperiment 2: " << "\n";

    cout << "2.1 parameter N: " << N << "\n";
    cout << "2.2 number of nodes: " << tr->nodeSize() << "\n";

    cout << "2.3 number of levels: " << tr->height() << "\n";
    cout << "2.4 keys of the root node: ";
    tr->printRootInfo();
    cout << "\n";

    cout << "Completed Experiment 2. " << "\n\n";
}

void experiment3(tree *tr) {
    cout << "Start Emperiment 3: " << "\n";

    clock_t start, end;
    start = clock();

    int accessedCnt = 0;

    vector<_record *> records = tr->query(_key{500}, _key{500}, &accessedCnt);

    end = clock();

    cout << "number of records that numVotes = 500: " << records.size() << "\n";

    cout << "3.1. number of accessed tree nodes: " << accessedCnt << "\n";
    cout << "3.2. number of accessed data blocks: " << tr->getDisk()->getAccessedBlock(records) << "\n";

    int sum = 0;
    for (int i = 0; i < records.size(); i++) {
        sum += records[i]->rating;
    }
    double avg = double(sum) / 10.0 / records.size();

    cout << "3.3. average value of averageRating: " << avg << "\n";

    cout << "3.4. running time of retrieval process: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << " ms \n";

    cout << "3.5.1 number of data blocks accessed in linear scan: " << "TODO" << "\n";
    cout << "3.5.2 running time of linear scan: " << "TODO" << "\n";

    cout << "Completed Experiment 3. " << "\n\n";
}

void experiment4(tree *tr) {
    cout << "Start Experiment 4: " << "\n";

    clock_t start, end;
    double cpu_time_used;
    start = clock();

    int accessedCnt = 0;

    vector<_record *> records = tr->query(_key{30000}, _key{40000}, &accessedCnt);

    end = clock();
    cpu_time_used = (double) (end - start); //in clocks

    cout << "number of records that numVotes in [30000, 40000]: " << records.size() << "\n";

    cout << "4.1. number of accessed tree nodes: " << accessedCnt << "\n";
    cout << "4.2. number of accessed data blocks: " << tr->getDisk()->getAccessedBlock(records) << "\n";

    int sum = 0;
    for (int i = 0; i < records.size(); i++) {
        sum += records[i]->rating;
    }
    double avg = double(sum) / 10.0 / records.size();

    cout << "4.3. average value of averageRating: " << avg << "\n";

    cout << "4.4. running time of retrieval process: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << " ms \n";

    cout << "4.5.1 number of data blocks accessed in linear scan: " << "TODO" << "\n";
    cout << "4.5.2 running time of linear scan: " << "TODO" << "\n";

    cout << "Completed Experiment 4. " << "\n\n";
}

void experiment5(tree *tr) {
    cout << "Start Experiment 5: " << "\n";

    clock_t start, end;
    double cpu_time_used;
    start = clock();

    tr -> removeAll(_key{1000});

    end = clock();
    cpu_time_used = (double) (end - start); //in clocks

    cout << "5.1. updated number of tree nodes: " << tr -> nodeSize() << "\n";
    cout << "5.2. updated tree height: " << tr -> height() << "\n";

    cout << "5.3 keys of the root node: ";
    tr->printRootInfo();
    cout << "\n";

    cout << "5.4. running time of deletion process: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << " ms \n";

    cout << "5.5.1 number of data blocks accessed in linear scan: " << "TODO" << "\n";
    cout << "5.5.2 running time of linear scan: " << "TODO" << "\n";

    cout << "Completed Experiment 5. " << "\n\n";
}


void runExperiment() {
    tree *tr = constructTreeFromTsv("data.tsv");
    printLinebreak();
    experiment1(tr);
    printLinebreak();
    experiment2(tr);
    printLinebreak();
    experiment3(tr);
    printLinebreak();
    experiment4(tr);
    printLinebreak();
    experiment5(tr);
}

int main() {
    srand(43); // for consistent output
    //correctnessTest<MAXN>(); // check correctness from n = 15 to n = 2
    //findBestN(); // simulate 1,000,000 insertions, and get best n (it might output different optimal n each time, but we will choose one in our following experiment)
    cout << sizeof(BPTree<_key, _record, MAXN>::node) << endl;
    cout << "experiment starts: " << endl;
    cout << "size of struct key: " << sizeof(_key) << "\n";
    cout << "size of struct record: " << sizeof(_record) << "\n";
    cout << "Set parameter N = " << N << ", so the size of tree node is " << sizeof(tree::node) << " bytes\n";
    clock_t start = clock();
    runExperiment();
    clock_t duration = clock() - start;
    cout << "Execution time: " << double (duration) / CLOCKS_PER_SEC << " seconds" << endl;
    return 0;

}
