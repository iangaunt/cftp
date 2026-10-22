#include <iostream>

#include "tree.h"

int main() {
    Tree* t = new Tree(25);

    t->random_walk(2500);
    t->print_data();

    return 0;
}