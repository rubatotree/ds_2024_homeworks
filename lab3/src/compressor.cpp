#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>
#include <iomanip>

const unsigned int max_word_number = 65536;
const unsigned int max_word_length = 255;
const unsigned int max_code_length = 256;

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
	unsigned char code[max_code_length];
	unsigned char code_length;
	int freq;
};
std::vector<WordToken> word_table;
std::deque<char> bit_stream;

bool is_letter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
unsigned int add_word()
{
	word_table.push_back({{'\0'}, 0, {'\0'}, 0, 0});
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
void write_uchar(unsigned char n)
{
	fwrite(&n, sizeof(unsigned char), 1, file_output);
}
void write_chars(unsigned char* p, unsigned int n)
{
	fwrite(p, sizeof(unsigned char), n, file_output);
}
void write_code(int word_index)
{
	for(int i = 0; i < word_table[word_index].code_length; i++)
		bit_stream.push_back((word_table[word_index].code[i / 8] >> (7 - i % 8)) & 1);
	write_try();
}
void code_pushback(int word_index, int code_bit)
{
	unsigned char mask = (code_bit & 1) << (7 - word_table[word_index].code_length % 8);
	word_table[word_index].code[word_table[word_index].code_length / 8] &= ~mask;
	word_table[word_index].code[word_table[word_index].code_length / 8] |= mask;
	word_table[word_index].code_length++;
}
void set_code(int word_index, char* code, int code_length)
{
	word_table[word_index].code_length = 0;
	for(int i = 0; i < code_length; i++)
		code_pushback(word_index, code[i]);
	word_table[word_index].code_length = code_length;
}
void print_code(int word_index)
{
	for(int i = 0; i < word_table[word_index].code_length; i++)
		std::cout << ((word_table[word_index].code[i / 8] >> (7 - i % 8)) & 1);
}
void print_word_token_info(int word_index)
{
	std::cout << "[" << word_index << "] " << word_table[word_index].word << "\t" << "-> "; 
	print_code(word_index);
	std::cout << "\t freq: " << word_table[word_index].freq << std::endl;
}

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

int make_huffman_tree()
{
	auto cmp_huffman_node = [](HuffmanTreeNode* a, HuffmanTreeNode* b) 
						-> bool { return a->weight > b->weight; };
	std::priority_queue<HuffmanTreeNode*, 
		std::vector<HuffmanTreeNode*>, 
		decltype(cmp_huffman_node)> nodes_queue(cmp_huffman_node);

	// TODO (Optional 2): Use the n + ceil(lgn) - 2 algorithm
	// Use a Segment tree to find the two smallest elements.
	for(int i = 0; i < word_table.size(); i++)
	{
		if(word_table[i].freq > 0)
		{
			auto node = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
			node->type = LEAF;
			node->weight = word_table[i].freq;
			node->parent = NULL;
			node->leaf.word_index = i;
			nodes_queue.push(node);
		}
	}
	while(nodes_queue.size() > 1)
	{
		auto node = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
		node->type = NODE;
		node->node.lchild = nodes_queue.top(); nodes_queue.pop();
		node->node.rchild = nodes_queue.top(); nodes_queue.pop();
		node->weight = node->node.lchild->weight + node->node.rchild->weight;
		node->node.lchild->parent = node;
		node->node.rchild->parent = node;
		nodes_queue.push(node);
	}
	root = nodes_queue.top(); nodes_queue.pop();
	return 0;
}

char code[max_code_length];
std::vector<char> traverse_path_code;
std::vector<int> traverse_order;
void traverse_huffman_tree(HuffmanTreeNode* p, int dep)
{
	if(p == NULL) return;
	if(p->type == LEAF)
	{
		set_code(p->leaf.word_index, code, dep);
		traverse_path_code.push_back(1);
		traverse_order.push_back(p->leaf.word_index);
		return;
	}
	code[dep] = 0;
	traverse_path_code.push_back(0);
	traverse_huffman_tree(p->node.lchild, dep + 1);
	code[dep] = 1;
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

int make_huffman_codes()
{
	make_huffman_tree();
	traverse_huffman_tree(root, 0);
	delete_huffman_tree(root);
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
		bit_stream.push_back(traverse_path_code[i]);
		write_try();
	}
	// tag of word
	char word_tag = (word_table.size() > 256);
	bit_stream.push_back(word_tag);
	write_clear_stream();
	for(int i = 0; i < traverse_order.size(); i++)
	{
		int word_index = traverse_order[i];
		if(word_tag) write_uchar(word_table[word_index].length);
		write_chars(word_table[word_index].word, word_table[word_index].length);
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
	if(make_huffman_codes()) return 1;
	std::cout << "Successfully made the Huffman codes." << std::endl;
	if(output_code_table()) return 1;
	std::cout << "Successfully output the code table." << std::endl;
	if(compress()) return 1;
	std::cout << "Successfully compressed the file." << std::endl;
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		std::cout << "Format error.\nExpect: ./compressor <input_file> <output_file>" << std::endl;
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
