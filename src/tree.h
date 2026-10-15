#ifndef TREE_HPP
#define TREE_HPP

#include <map>
#include <vector>

struct TreeNode {
	int m_data;

	TreeNode* parent;
	TreeNode* left;
	TreeNode* right;

	TreeNode() : m_data(0), parent(nullptr), left(nullptr), right(nullptr) {}
	explicit TreeNode(int data) : m_data(data), parent(nullptr), left(nullptr), right(nullptr) {}
};

enum class Rotation { LEFT, RIGHT, NONE };
struct FlipNode {
	TreeNode* node;
	FlipNode* prev;
	FlipNode* next;

	Rotation dir = Rotation::NONE;
};

struct FlipTable {
	FlipNode* root;
	
	FlipNode* left_head = nullptr;
	FlipNode* left_tail = nullptr;
	FlipNode* right_head = nullptr;
	FlipNode* right_tail = nullptr;
};

class Tree {
	public:
		explicit Tree(int size);
		~Tree();

		int size; 
		TreeNode* m_root;

		std::vector<TreeNode*> m_nodes;
		FlipTable* m_ft;
		std::map<TreeNode*, FlipNode*> hash;

		Rotation random_flip();
		void random_walk(int length);

		void refresh_flip(FlipNode* f);
		void build_flip_list();

		void print_tree();
		void print_flip();
		void print_data();
};

#endif