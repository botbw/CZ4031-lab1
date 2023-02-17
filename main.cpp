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
    vector<int> a;
    //16
    for (int i = 0; i < 10000; i++) {
        int num = random() % 10000;
        tr.insert(num, nullptr);
    }
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
    cout << (is_sorted(a.begin(), a.end()) ? "sorted" : "unsorted") << endl;
}


int main() {
    // insert 中pointer有多个internal 指向一个leaf
//    test1();
//    test2();
//    test3();
//    test4();
//    test5();
//    test6();
randomTest();
    return 0;
}
