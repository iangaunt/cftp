#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <stack>
#include <string>

#include "tree.h"
#include "xoshiro256.h"

// Random number generation from hardware
std::random_device rd;
xoshiro256ss gen(rd());

void inorder(std::vector<TreeNode*> m_nodes, int size, int i, int* label) {
	if (i >= size) {
		return;
	}

	inorder(m_nodes, size, 2 * i + 1, label);
	m_nodes[i]->m_data = *label;
	(*label)++;
	inorder(m_nodes, size, 2 * i + 2, label);
}
/**
 * Creates a new binary tree of size `n` and populates
 * the nodes with arbitrary numerical data while filling the
 * initial flip node table to store future rotations
 * 
 * @param size The size of the ternary tree
 */
Tree::Tree(int size) {
	this->size = size;

	// Build nodes and set their parent nodes in first pass.
	for (int i = 0; i < size; i++) {
		TreeNode* temp = new TreeNode(i);
		if (i > 0) temp->parent = m_nodes.at((i - 1) / 2);
		m_nodes.push_back(temp);
	}

	this->m_root = m_nodes.at(0);

	// Set left and right children nodes in second pass.
	for (int i = 0; i < size / 2; i++) {
		TreeNode* curr = m_nodes.at(i);
		if (i * 2 + 1 < size) curr->left = m_nodes.at(i * 2 + 1);
		if (i * 2 + 2 < size) curr->right = m_nodes.at(i * 2 + 2);
	}
	
	int inorder_counter = 0;
	inorder(m_nodes, size, 0, &inorder_counter);
	build_flip_list();
}

/** Cleanup function for freeing nodes. */
Tree::~Tree() {
	for (auto n : m_nodes) {
		delete(n);
	}
}

/**
 * Sets the direction of a given flip node.
 * LEFT rotations occur when a node is the right child of its parent.
 * RIGHT rotations occur when a node is the left child of its parent.
 * NONE is used in the case that the node is the root, which cannot be picked
 * from the random flip distribution + is ignored.
 * 
 * @param f The flip node to modify.
 */
void set_dir(FlipNode* f) {
	TreeNode* n = f->node;
	if (n->parent == nullptr) {
		f->dir = Rotation::NONE;
	} else {
		f->dir = n == n->parent->left ? Rotation::RIGHT : Rotation::LEFT;
	}
}

/**
 * Refreshes a given flip by checking to see if
 * the current node is now its parent's left / right
 * child and editing its flip direction in the flip table.
 * 
 * @param n The node to check for its new rotation.
 */
void Tree::refresh_flip(FlipNode* f) {
	if (f == nullptr) return;

	Rotation old_dir = f->dir;
	set_dir(f);
	if (old_dir == f->dir) {
		return;
	}

	// If f is the only node in the left / right tables,
	// remove fully and set to nullptr
	if (f == m_ft->left_head && f == m_ft->left_tail) {
		m_ft->left_head = nullptr;
		m_ft->left_tail = nullptr;
	} else if (f == m_ft->left_head) {
		m_ft->left_head = f->next;
	} else if (f == m_ft->left_tail) {
		m_ft->left_tail = f->prev;
	}

	if (f == m_ft->right_head && f == m_ft->right_tail) {
		m_ft->right_head = nullptr;
		m_ft->right_tail = nullptr;
	} else if (f == m_ft->right_head) {
		m_ft->right_head = f->next;
	} else if (f == m_ft->right_tail) {
		m_ft->right_tail = f->prev;
	}

	// Close list around f
	if (f->prev != nullptr) {
		f->prev->next = f->next;
	}

	if (f->next != nullptr) {
		f->next->prev = f->prev;
	}

	if (m_ft->root == f) {
		m_ft->root = nullptr;
	}

	f->prev = nullptr;
	f->next = nullptr;
	
	if (f->dir == Rotation::NONE) {
		// Set the new root node to the head of the list
		f->next = m_ft->root;
		
		if (m_ft->root) m_ft->root->prev = f;
		m_ft->root = f;
		return;
	}

	// Add node to either the left list / right list
	FlipNode* anchor;
	if (f->dir == Rotation::LEFT) {
		anchor = m_ft->left_tail ? m_ft->left_tail : m_ft->root;
	} else {
		if (m_ft->right_tail) {
			anchor = m_ft->right_tail;
		} else {
			anchor = m_ft->left_tail ? m_ft->left_tail : m_ft->root;
		}
	}

	f->prev = anchor;
	f->next = anchor->next;

	if (anchor->next) {
		anchor->next->prev = f;
	}
	anchor->next = f;

	if (f->dir == Rotation::LEFT) {
		if (!m_ft->left_head) {
			m_ft->left_head = f;
		}

		m_ft->left_tail = f;
	} else {
		if (!m_ft->right_head) {
			m_ft->right_head = f;
		}

		m_ft->right_tail = f;
	}
}

/**
 * Builds the initial flip list from the initially ordered binary tree.
 */
void Tree::build_flip_list() {
	FlipTable* ft = new FlipTable();
	m_ft = ft;

	for (int i = 0; i < m_nodes.size(); i++) {
		FlipNode* fn = new FlipNode();
		fn->node = m_nodes[i];
		set_dir(fn);
		hash.insert({m_nodes[i], fn});
		
		// Set root
		if (i == 0) {
			m_ft->root = fn;
			continue;
		}

		// Pick left / right flip table pointers
		FlipNode*& head = (fn->dir == Rotation::LEFT ? m_ft->left_head : m_ft->right_head);
		FlipNode*& tail = (fn->dir == Rotation::LEFT ? m_ft->left_tail : m_ft->right_tail);

		// If the lists are empty, then start the 
		// front / end at the same node;
		// otherwise, append to end of flip list
		if (head == nullptr) {
			head = fn;
			tail = fn;
		} else {
			tail->next = fn;
			fn->prev = tail;
			tail = fn;
		}
	}

	// Link the entire list together [root -> left -> right]
	FlipNode* last = m_ft->root;
	if (m_ft->left_head) {
		last->next = m_ft->left_head;
		m_ft->left_head->prev = last;
		last = m_ft->left_tail;
	}
	if (m_ft->right_head) {
		last->next = m_ft->right_head;
		m_ft->right_head->prev = last;
	}
}

/**
 * Performs a random flip on the tree and returns the direction
 * of rotation of the flip. Updates the flip table for all
 * rotated nodes.
 */
Rotation Tree::random_flip() {
	if (m_nodes.size() < 2) return Rotation::NONE;
	
	// Selects a random node from the tree.
	std::uniform_int_distribution<int> distr(1, m_nodes.size() - 1);
	int selected_index = distr(gen);

	FlipNode* selected_fnode = m_ft->root;
	for (int i = 0; i < selected_index; i++) {
		selected_fnode = selected_fnode->next;
	}
	
	TreeNode* toFlip = selected_fnode->node;
	TreeNode* parent = toFlip->parent;

	Rotation rot = selected_fnode->dir;
	TreeNode* moved = nullptr;

	// Rotates the node to the left or right
	// (left nodes rotate right, right nodes rotate left)
	if (toFlip == parent->left) {
		moved = toFlip->right;
		parent->left = moved;
		toFlip->right = parent;
	} else {
		moved = toFlip->left;
		parent->right = moved;
		toFlip->left = parent;
	}

	if (moved != nullptr) {
		moved->parent = parent;
	}

	// Adjust the grandparent pointers to 
	// point to their new children (the child
	// turned parent in the flip)
	TreeNode* grandparent = parent->parent;
	toFlip->parent = grandparent;
	if (grandparent != nullptr) {
		if (parent == grandparent->left) {
			grandparent->left = toFlip;
		} else {
			grandparent->right = toFlip;
		}
	}
	parent->parent = toFlip;

	// If the flip changed the root, point to the new root
	if (m_root == parent) {
		m_root = toFlip;
	}

	// Store flip nodes location in memory as they are rearranged
	FlipNode* f_toFlip = hash[toFlip];
	FlipNode* f_parent = hash[parent];

	// Refresh flip state in the flip table
	refresh_flip(f_toFlip);
	refresh_flip(f_parent);

	if (moved != nullptr) {
		FlipNode* f_moved = hash[moved];
		refresh_flip(f_moved);
	}

	return rot;
}

/**
 * Performs a random walk on the binary tree, e.g. performing
 * random flips for length steps.
 * 
 * @param length The length of the walk
 */
void Tree::random_walk(int length) {
	for (int i = 0; i < length; i++) {
		random_flip();
	}
}

/** Recursive tree print statement. */
void print_r(const std::string& prefix, const TreeNode* node, bool isLeft) {
    if (node != nullptr) {
        std::cout << prefix;
        std::cout << (isLeft ? "├──" : "└──" );

        std::cout << node->m_data;
		std::cout << " " << 
			(isLeft ? "(L)" : (node->parent == nullptr ? "(N)" : "(R)")) << std::endl;
        print_r(prefix + (isLeft ? "│   " : "    "), node->left, true);
        print_r(prefix + (isLeft ? "│   " : "    "), node->right, false);
    }
}

/** Prints out the tree. */
void Tree::print_tree() {
	printf("Tree (n = %d):\n", size);
	print_r("", m_root, false);
}

/** Prints out the flip table. */
void Tree::print_flip() {
	printf("Tree (n = %d) Flip Table:\n", size);
	FlipNode* f = m_ft->root;
	while (f != NULL) { 
		printf("%d: %s\n", 
			f->node->m_data,
			f->dir == Rotation::LEFT ? "LEFT" : (f->dir == Rotation::NONE ? "NONE" : "RIGHT")
		);
		f = f->next;
	}
}

/** Prints both the tree and the flip table. */
void Tree::print_data() {
	print_tree();
	printf("\n");
	
	print_flip();
	printf("\n");
}