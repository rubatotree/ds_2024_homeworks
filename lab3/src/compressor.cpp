#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>
#include <iomanip>
#include <tuple>
#include <functional>

const unsigned int max_word_number = 32768;
const unsigned int max_word_length = 255;
const unsigned int max_code_length = 32768;

FILE *file_input, *file_output;

enum HuffmanTreeNodeType { LEAF, NODE, UNKNOWN };
struct HuffmanTreeNode
{
	HuffmanTreeNodeType type;
	int weight;
	HuffmanTreeNode *parent;
	union
	{
		struct{ int word_index; } leaf;
		struct{ HuffmanTreeNode *lchild, *rchild; } node;
	};
};
HuffmanTreeNode* root;

struct WordToken
{
	unsigned char word[max_word_length];
	unsigned char length;
	int freq;
	HuffmanTreeNode* node;
};
std::vector<WordToken> word_table;
std::deque<char> bit_stream;

// Optional 2: Use a segment tree to find two min values in an array.
template <class T>
struct SegmentTreeNode
{
	int arg;
	SegmentTreeNode *parent, *lchild, *rchild;
};
template <class T>
SegmentTreeNode<T>* build_seg_tree(SegmentTreeNode<T> *p, 
		std::vector<T> &arr, int l, int r, 
		std::function<bool(T&, T&)> cmp)
{
	if(r == l + 1)
	{
		p->arg = l;
		return p;
	}
	p->lchild = new SegmentTreeNode<T>();
	p->rchild = new SegmentTreeNode<T>();
	p->lchild->parent = p->rchild->parent = p;
	int mid = (l + r + 1) >> 1;
	auto lmin = build_seg_tree(p->lchild, arr, l, mid, cmp);
	auto rmin = build_seg_tree(p->rchild, arr, mid, r, cmp);
	bool cmp_res = cmp(arr[p->lchild->arg], arr[p->rchild->arg]);
	p->arg = cmp_res ? p->lchild->arg : p->rchild->arg;
	return cmp_res ? lmin : rmin;
}
template <class T>
void del_seg_tree(SegmentTreeNode<T> *p)
{
	if(p == NULL) return;
	del_seg_tree(p->lchild);
	del_seg_tree(p->rchild);
	delete p;
}
template <class T>
std::pair<int, int> find_two_mins(
		std::vector<T> &arr, std::function<bool(T&, T&)> cmp)
{
	int n = arr.size();
	if(n <= 1) return { 0, 0 };
	auto seg_root = new SegmentTreeNode<T>();
	seg_root->parent = nullptr;
	auto min_ptr = build_seg_tree<T>(seg_root, arr, 0, n, cmp);
	int min1 = seg_root->arg, min2 = -1, arg;
	auto p = min_ptr;
	if(p->parent == nullptr) return { min1, min1 };
	while(p->parent != nullptr)
	{
		if(p == p->parent->lchild) arg = p->parent->rchild->arg;
		else arg = p->parent->lchild->arg;
		p = p->parent;
		if(min2 == -1) min2 = arg;
		else
		{
			bool cmp_res = cmp(arr[arg], arr[min2]);
			if(cmp_res) min2 = arg;
		}
	}
	del_seg_tree(seg_root);
	return { min1, min2 };
};

// Wordtable & Bitstream processing functions
bool is_letter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
unsigned int add_word()
{
	word_table.push_back({{'\0'}, 0, 0, nullptr});
	return word_table.size() - 1;
}
unsigned int add_word(unsigned char c)
{
	unsigned short idx = add_word();
	word_table[idx].word[0] = c;
	word_table[idx].length = 1;
	return idx;
}
unsigned int add_word(unsigned char* p_word)
{
	unsigned short idx = add_word();
	short i;
	for(i = 0; !is_letter(p_word[i]); i++)
		word_table[idx].word[i] = p_word[i];
	word_table[idx].length = i;
	return idx;
}

int read_word()
{
	// TODO(Optional 1): change the unit of word to word
	// Actually read a char there
	unsigned char ch;
	if(!fread(&ch, sizeof(unsigned char), 1, file_input))
		return -1;
	return (int)ch;
}
void write_try()
{
	while(bit_stream.size() >= 8)
	{
		unsigned char ch = 0;
		for(int i = 0; i < 8; i++)
		{
			ch = ch << 1;
			ch += bit_stream.front();
			bit_stream.pop_front();
		}
		fwrite(&ch, sizeof(unsigned char), 1, file_output);
	}
}
unsigned char write_clear_stream()
{
	int size = bit_stream.size();
	if(size % 8 > 0)
	{
		for(int i = 0; i < 8 - size % 8; i++)
			bit_stream.push_back(0);
	}
	write_try();
	if(size == 0) return 0;
	return 8 - size % 8;
}
void write_bit(char bit)
{
	bit_stream.push_back(bit);
	write_try();
}
void write_uchar(unsigned char n)
{
	fwrite(&n, sizeof(unsigned char), 1, file_output);
}
void write_chars(unsigned char* p, unsigned int n)
{
	fwrite(p, sizeof(unsigned char), n, file_output);
}
void write_tree_code(HuffmanTreeNode* p, 
		std::function<void(char)> writer = write_bit)
{
	if(p->parent == nullptr) return;
	write_tree_code(p->parent);
	char code = (p == p->parent->node.rchild);
	writer(code);
}
void write_code(int word_index)
{
	write_tree_code(word_table[word_index].node);
}
void print_code(int word_index)
{
	write_tree_code(word_table[word_index].node, 
			[](char bit) -> void { std::cout << (int)bit; });
}
void print_word_token_info(int word_index)
{
	std::cout << "[" << word_index << "] " 
		<< word_table[word_index].word << "\t" << "-> "; 
	print_code(word_index);
	std::cout << "\t freq: " << word_table[word_index].freq << std::endl;
}

// Processing functions
int scan_words()
{
	// TODO (Optional 1)
	fseek(file_input, 0, SEEK_SET);
	return 1;
}

int scan_file()
{
	for(int i = 0; i < 256; i++)
		add_word((unsigned char)i);
	scan_words();

	fseek(file_input, 0, SEEK_SET);
	int word_index;
	while((word_index = read_word()) != -1)
		word_table[word_index].freq++;

	int available_word_number = 0;
	for(int i = 0; i < word_table.size(); i++)
	{
		if(word_table[i].freq > 0)
		{
			available_word_number++;
		}
	}
	if(available_word_number <= 1)
	{
		unsigned char p = 0;
		while(available_word_number <= 1)
		{
			while(word_table[p].freq > 0)
			{
				p++;
			}
			word_table[p].freq = 1;
			p++;
			available_word_number++;
		}
	}
	return 0;
}

int ceil_log_2(int x)
{
	int i = 0;
	while(((x - 1) >> i) > 0) i++;
	return i;
}

std::vector<char> traverse_path_code;
std::vector<int> traverse_order;
void traverse_huffman_tree(HuffmanTreeNode* p, int dep)
{
	if(p == NULL) return;
	if(p->type == LEAF)
	{
		// set_code(p->leaf.word_index, code, dep);
		traverse_path_code.push_back(1);
		traverse_order.push_back(p->leaf.word_index);
		return;
	}
	traverse_path_code.push_back(0);
	traverse_huffman_tree(p->node.lchild, dep + 1);
	traverse_path_code.push_back(0);
	traverse_huffman_tree(p->node.rchild, dep + 1);
	if(p != root) traverse_path_code.push_back(1);
}

void delete_huffman_tree(HuffmanTreeNode* p)
{
	if(p == NULL) return;
	if(p->type == LEAF)
	{
		free(p);
		return;
	}
	delete_huffman_tree(p->node.lchild);
	delete_huffman_tree(p->node.rchild);
	free(p);
}

void output_tree_info()
{
	std::cout << "Tree:" << std::endl;
	unsigned char ch = 0;
	for(int i = 0; i < traverse_path_code.size(); i++)
	{
		std::cout << (int)traverse_path_code[i];
		if(i % 8 == 7)
		{
			std::cout << (int)ch << " ";
		}
	}
	std::cout << std::endl;
	for(int i = 0; i < ((traverse_path_code.size() - 1) / 8 + 1) * 8; i++)
	{
		ch <<= 1;
		ch += i < traverse_path_code.size() ? traverse_path_code[i] : 0;
		if(i % 8 == 3 || i % 8 == 7)
		{
			std::cout << std::hex << (int)ch << std::dec;
			ch = 0;
		}
		if(i % 8 == 7) std::cout << " ";
	}
	std::cout << std::endl;
	for(int i = 0; i < word_table.size(); i++)
	{
		if(word_table[i].freq > 0)
			print_word_token_info(i);
	}
}

int make_huffman_tree()
{
	int compn = 0;
	std::vector<HuffmanTreeNode*> huffman_nodes;

	for(int i = 0; i < word_table.size(); i++)
	{
		if(word_table[i].freq > 0)
		{
			auto node = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
			node->type = LEAF;
			node->weight = word_table[i].freq;
			node->parent = NULL;
			node->leaf.word_index = i;
			word_table[i].node = node;
			huffman_nodes.push_back(node);
		}
	}
	while(huffman_nodes.size() > 1)
	{
		int n = huffman_nodes.size(), max_compn = n + ceil_log_2(n) - 2;
		compn = 0;
		auto two_mins = find_two_mins<HuffmanTreeNode*>(huffman_nodes, 
					[&compn](HuffmanTreeNode* &a, HuffmanTreeNode* &b) 
						-> bool { compn++; return a->weight < b->weight; });
		// printf("Compared %d times, n+ceil(lgn)-2=%d\n", compn, max_compn);
		if(compn > max_compn) return 2;	// Don't satisfy optional 2: Abort

		auto node = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
		node->type = NODE;
		node->node.lchild = huffman_nodes[two_mins.first];
		node->node.rchild = huffman_nodes[two_mins.second];
		node->weight = node->node.lchild->weight + node->node.rchild->weight;
		node->node.lchild->parent = node;
		node->node.rchild->parent = node;

		huffman_nodes[two_mins.first]  = huffman_nodes[n - 1];
		huffman_nodes[two_mins.second] = huffman_nodes[n - 2];
		huffman_nodes.pop_back();
		huffman_nodes.pop_back();
		huffman_nodes.push_back(node);
	}
	root = huffman_nodes[0];
	huffman_nodes.pop_back();

	traverse_huffman_tree(root, 0);
	// output_tree_info();
	return 0;
}


int output_code_table()
{
	bit_stream.clear();
	// the first of path should be 0 and last should be 1
	// so we can save two bits to do something
	for(int i = 1; i < traverse_path_code.size(); i++)
	{
		write_bit(traverse_path_code[i]);
	}
	// tag of word
	char word_tag = (word_table.size() > 256);
	write_bit(word_tag);
	write_clear_stream();
	for(int i = 0; i < traverse_order.size(); i++)
	{
		int ind = traverse_order[i];
		if(word_tag) write_uchar(word_table[ind].length);
		write_chars(word_table[ind].word, word_table[ind].length);
	}
	return 0;
}

int compress()
{
	fseek(file_input, 0, SEEK_SET);
	int word_index;
	while((word_index = read_word()) != -1)
	{
		write_code(word_index);
	}
	unsigned char deleted_bits = write_clear_stream();
	write_uchar(deleted_bits);
	return 0;
}

int process()
{
	if(scan_file()) return 1;
	std::cout << "Successfully scanned the file." << std::endl;
	if(make_huffman_tree()) return 1;
	std::cout << "Successfully made the Huffman tree." << std::endl;
	if(output_code_table()) return 1;
	std::cout << "Successfully output the code table." << std::endl;
	if(compress()) return 1;
	std::cout << "Successfully compressed the file." << std::endl;
	delete_huffman_tree(root);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		std::cout << "Format error.\nExpect:"
			" ./compressor <input_file> <output_file>" << std::endl;
		return 0;
	}
	file_input = fopen(argv[1], "rb");
	file_output = fopen(argv[2], "wb");
	if(file_input == NULL || file_output == NULL)
	{
		std::cout << "Failed to open files.";
		return 0;
	}
	int status = process();
	if(status) printf("Failed to compress.\n");
	fclose(file_input);
	fclose(file_output);
	return status;
}
