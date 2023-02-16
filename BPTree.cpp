//
// Created by haoxuanwang on 14/2/23.
//

#include "BPTree.h"


typedef BPTree<int, int, 3> tree;

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