#include <iostream>
#include "BPTree.h"

using tree= BPTree<int, int, 3>;
using node = tree::node;

void test1() {  // insertion in tut
    tree tr;
    int arr[11];

    for (int i = 1; i <= 10; i++) {
        arr[i] = i * 10;
        cout << "inserting: " << arr[i] << endl;
        tr.insert(arr[i], arr + i);
        tr.levelTraverse();
        cout << endl << endl << endl;
    }

    cout << tr.height() << endl;
    cout << tr.size() << endl;
}

void test2() {  // insertion in lec
    tree tr;
    int arr[12] = {1, 4, 7, 10, 17, 21, 31, 25, 19, 20, 28, 42};
    for (int i = 0; i < 12; i++) {
        cout << "inserting: " << arr[i] << endl;
        tr.insert(arr[i], arr + i);
        tr.levelTraverse();
        cout << endl << endl << endl;
    }
    cout << tr.height() << endl;
    cout << tr.size() << endl;
}

void test3() {  // delete 5 in lec
    tree tr;
    int arr[] = {1, 4, 7, 10, 17, 21, 31, 25, 19, 20, 5};
    for (int i = 0; i < 11; i++) {
        tr.insert(arr[i], arr + i);
    }
    tr.levelTraverse();
    cout << "deleting: " << 5 << endl;
    tr.remove(5);
    tr.levelTraverse();
    cout << endl << endl << endl;
}

void test4() {  // delete 17 in lec
    tree tr;
    int arr[] = {1, 4, 7, 10, 17, 21, 31, 25, 19, 20, 5, 16};
    for (int i = 0; i < 12; i++) {
        tr.insert(arr[i], arr + i);
    }
    tr.levelTraverse();
    cout << "deleting: " << 17 << endl;
    tr.remove(17);
    tr.levelTraverse();
    cout << endl << endl << endl;
}

void test5() {  // delete 4 in lec
    tree tr;
    int arr[] = {1, 4, 7, 10, 17, 21, 31, 25, 19, 20};
    for (int i = 0; i < 10; i++) {
        tr.insert(arr[i], arr + i);
    }
    tr.levelTraverse();
    cout << "deleting: " << 4 << endl;
    tr.remove(4);
    tr.levelTraverse();
    cout << endl << endl << endl;
}

void test6() {  // delete 4 in lec
    tree tr;
    int arr[] = {1, 4, 7, 10, 17, 21, 31, 25, 19, 20};
    for (int i = 0; i < 10; i++) {
        tr.insert(arr[i], arr + i);
    }
    tr.remove(17);
    tr.remove(19);
    tr.levelTraverse();
    cout << "deleting: " << 4 << endl;
    tr.remove(4);
    tr.levelTraverse();
    cout << endl << endl << endl;
}

void randomTest() {
    tree tr;
    vector<int> a, b;
    multiset<int> s;
    //16
//    srand(time(NULL));
    int n = 500;
    for (int i = 1; i <= n; i++) {
        int num = rand() % 10000;
//        num = 1;
        tr.insert(num, nullptr);
        b.push_back(num);
        s.insert(num);

    }
    sort(b.begin(), b.end());
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

    assert(is_sorted(a.begin(), a.end()));
    assert(a == b);

    cout << "after " << n << " insertions the leaves are " << (is_sorted(a.begin(), a.end()) ? "sorted" : "unsorted") << " height:" << tr.height() << endl;
    tr.levelTraverse();
//    return;
    for (int i = 1; i <= n; i++) {
        int id = rand() % ((int) a.size());
        bool deleted = tr.remove(a[id]);
        bool _deleted = s.contains(a[id]);
        if(deleted != _deleted) {
            cout << a[id] << endl;
            tr.levelTraverse();
            return;
        }
        if(_deleted == true) s.erase(s.find(a[id]));
    }
    cout << "pass\n";
}

void deleteTest() {
    tree tr;
    int n = 15;
    for (int i = 1; i <= n; i++) {
        tr.insert(1, nullptr);
    }
    tr.levelTraverse();
    cout << endl << endl;

    for (int i = 1; i <= n; i++) {
        tr.remove(1);
        cout << i << ": " << endl;
        tr.levelTraverse();
        cout << endl;
    }
}


int main() {
    // insert 中pointer有多个internal 指向一个leaf
//    test1();
//    test2();
//    test3();
//    test4();
//    test5();
//    test6();
//    insertTest();
//deleteTest();
randomTest();
    return 0;
}
