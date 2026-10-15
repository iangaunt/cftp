#include <iostream>

#include "tree.h"

int main() {
    Tree* t = new Tree(25);
    
    t->print_data();
    for (int i = 0; i < 100; i++) {
        t->random_flip();
        t->print_flip();
    }
    t->print_tree();

    return 0;
}