#include <iostream>
#include "BPTree.h"

typedef BPTree<int, int, 3> tree;

void test1() {
    tree tr;
    int arr[11];

    for (int i = 1; i <= 10; i++) {
        arr[i] = i * 10;
        tr.insert(arr[i], arr + i);
        tr.levelTraverse();
        cout << endl << endl << endl;
    }
}

void test2() {
    tree tr;
    int arr[12] = {1, 4, 7, 10, 17, 21, 31, 25, 19, 20, 28, 42};
    for (int i = 0; i < 12; i++) {
        tr.insert(arr[i], arr + i);
        tr.levelTraverse();
        cout << endl << endl << endl;
    }
}

int main() {
    test1();
    test2();
    return 0;
}
