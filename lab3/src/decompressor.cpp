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
};
std::vector<WordToken> word_table;

std::deque<char> bit_stream;
int read_bit()
{
	if(bit_stream.empty())
	{
		unsigned char ch;
		if(fread(&ch, sizeof(unsigned char), 1, file_input) != 1) return -1;
		for(int i = 7; i >= 0; i--)
			bit_stream.push_back((ch >> i) & 1);
	}
	int bit = bit_stream.front();
	bit_stream.pop_front();
	return bit;
}
int read_tree()
{
	int root_visit = 0;
	root = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
	root->type = NODE;
	root->parent = NULL;
	root->node.lchild = NULL;
	root->node.rchild = NULL;
	HuffmanTreeNode* p = root;
	do
	{
		if(p->parent == NULL) root_visit++;
		int bit = read_bit();
		if(bit == -1) return 1;
		else if(bit == 0)
		{
			auto ch = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
			ch->type = UNKNOWN;
			ch->parent = p;
			ch->node.lchild = NULL;
			ch->node.rchild = NULL;
			if(p->node.lchild == NULL) p->node.lchild = ch;
			else if(p->node.rchild == NULL) p->node.rchild = ch;
			else return 1;
			p = ch;
		}
		else if(bit == 1)
		{
			if(p->node.lchild == NULL && p->node.rchild == NULL)
			{
				p->type = LEAF;
				p->leaf.word_index = word_table.size();
				word_table.push_back({{'\0'}, 0});
			}
			else if(p->node.lchild != NULL && p->node.rchild != NULL)
			{
				p->type = NODE;
			}
			else return 1;
			p = p->parent;
		}
	} while (!(p->parent == NULL && root_visit == 2));
	bit_stream.clear();
	return 0;
}
std::vector<int> code;
void output_table(HuffmanTreeNode* p = root)
{
	if(p == NULL) return;
	if(p->type == LEAF)
	{
		printf("[%d] %s \t-> ", p->leaf.word_index, word_table[p->leaf.word_index].word);
		for(int i = 0; i < code.size(); i++)
			std::cout << code[i];
		printf("\n");
		return;
	}
	code.push_back(0);
	output_table(p->node.lchild);
	code.pop_back();
	code.push_back(1);
	output_table(p->node.rchild);
	code.pop_back();
}
int read_table()
{
	for(int i = 0; i < word_table.size(); i++)
	{
		unsigned char ch;
		if(fread(&ch, sizeof(unsigned char), 1, file_input) != 1)
			return 1;
		word_table[i].length = ch;
		if(fread(word_table[i].word, sizeof(unsigned char), word_table[i].length, file_input) != 1) return -1;
	}
	// output_table();
	return 0;
}
int decompress()
{
	int bit;
	HuffmanTreeNode* p = root;
	unsigned long long current_pos = ftell(file_input);
	fseek(file_input, 0, SEEK_END);
	unsigned long long file_size = ftell(file_input);
	fseek(file_input, -1, SEEK_END);
	unsigned char padding;
	if(fread(&padding, sizeof(unsigned char), 1, file_input) != 1) return 1;
	unsigned long long total_bits = (file_size - current_pos - 1ull) * 8ull - (unsigned long long)padding;
	fseek(file_input, current_pos, SEEK_SET);

	for(unsigned long long i = 0; i < total_bits; i++)
	{
		bit = read_bit();
		if(p == NULL) return 1;

		if(bit == 0) p = p->node.lchild;
		else if(bit == 1) p = p->node.rchild;

		if(p == NULL) return 1;
		if(p->type == LEAF)
		{
			fwrite(word_table[p->leaf.word_index].word, 
					sizeof(unsigned char), 
					word_table[p->leaf.word_index].length, file_output);
			p = root;
		}
	}
	return 0;
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

int process()
{
	if(read_tree()) return 1;
	std::cout << "Successfully read the tree." << std::endl;
	if(read_table()) return 1;
	std::cout << "Successfully read the table." << std::endl;
	if(decompress()) return 1;
	std::cout << "Successfully decompressed the file." << std::endl;
	delete_huffman_tree(root);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("Format error.\nExpect: ./decompressor <input_file> <output_file>\n");
		return 0;
	}
	file_input = fopen(argv[1], "rb");
	file_output = fopen(argv[2], "wb");
	if(file_input == NULL || file_output == NULL)
	{
		printf("Failed to open files.");
		return 0;
	}
	int status = process();
	if(status) printf("Failed to decompress.\n");
	fclose(file_input);
	fclose(file_output);
	return status;
}
