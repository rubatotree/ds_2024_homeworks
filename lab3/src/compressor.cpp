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

// Optional 1: Use a Trie to calculate the freq of words.
const int letter_n = 52;
struct TrieNode
{
	unsigned char val;
	int freq, word_index;
	TrieNode *ch[letter_n], *parent;
	TrieNode(unsigned char val = '\0'): val(val)
	{
		freq = 0;
		parent = nullptr;
		word_index = -1;
		for(int i = 0; i < letter_n; i++)
			ch[i] = nullptr;
	}
};

// Optional 2: Use a Segment tree to find two min values in an array.
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
int letter_to_trie_code(char c) 
{
	if(c >= 'A' && c <= 'Z') return c - 'A';
	else if(c >= 'a' && c <= 'z') return 26 + c - 'a';
	return -1;
}
char trie_code_to_letter(int code)
{
	if(0 <= code && code <= 25) return code + 'A';
	else if(26 <= code && code <= 51) return code - 26 + 'a';
	return '\0';
}
int ceil_log_2(int x)
{
	int i = 0;
	while(((x - 1) >> i) > 0) i++;
	return i;
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

int read_char()
{
	unsigned char ch;
	if(!fread(&ch, sizeof(unsigned char), 1, file_input))
		return -1;
	return (int)ch;
}

int word_threshold = -1;
TrieNode* word_trie_root;
std::deque<unsigned char> char_stream;
int read_word()
{
	if(word_threshold < 0) return read_char();
	if(char_stream.size() > 0)
	{
		int ch = (int)char_stream.front();
		char_stream.pop_front();
		return ch;
	}
	else
	{
		int ch = 0;
		TrieNode* p = word_trie_root;
		int dep = 0;
		do
		{
			ch = read_char();
			if(ch == -1)
			{
				return -1;
			}
			char_stream.push_back((unsigned char)ch);
			if(!is_letter(ch)) 
			{
				if(p != nullptr && p->freq > 0 && p->word_index != -1)
				{
					char_stream.clear();
					char_stream.push_back(ch);
					return p->word_index;
				}
				else return read_word();
			}
			else
			{
				if(p != nullptr) 
				{
					p = p->ch[letter_to_trie_code(ch)];
					dep++;
					if(dep > max_word_length)
						return read_word();
				}		
			}
		} 
		while(dep <= max_word_length);
		return -1;
	}
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
void print_tree_code(HuffmanTreeNode* p)
{
	if(p->parent == nullptr) return;
	print_tree_code(p->parent);
	char code = (p == p->parent->node.rchild);
	printf("%d", code);
}
void write_code(int word_index)
{
	write_tree_code(word_table[word_index].node);
}
void print_code(int word_index)
{
	print_tree_code(word_table[word_index].node);
}
void print_word_token_info(int word_index)
{
	std::cout << "[" << word_index << "] " 
		<< word_table[word_index].word << "\t" << "-> "; 
	print_code(word_index);
	std::cout << "\t freq: " << word_table[word_index].freq << std::endl;
}

// Processing functions
std::vector<int> freq_table;
unsigned char cur_word[max_word_length];

void traverse_word_trie(TrieNode *p)
{
	if(p == NULL) return;
	for(int i = 0; i < letter_n; i++)
		traverse_word_trie(p->ch[i]);
	if(p != word_trie_root) freq_table.push_back(p->freq);
}
void polish_word_trie(TrieNode *p, int dep)
{
	if(p == NULL) return;
	int ch_n = 0;
	for(int i = 0; i < letter_n; i++)
	{
		if(p->ch[i] != nullptr)
		{
			cur_word[dep] = trie_code_to_letter(i);
			polish_word_trie(p->ch[i], dep + 1);
			cur_word[dep] = '\0';
			if(p->ch[i] != nullptr) ch_n++;
		}
	}
	if(ch_n == 0 && (word_threshold < 0 || p->freq <= word_threshold))
	{
		p->parent->ch[letter_to_trie_code(p->val)] = nullptr;
		delete p;
		return;
	}
	if(word_threshold >= 0 && p->freq > word_threshold)
	{
		unsigned int idx = add_word();
		cur_word[dep] = '\0';
		for(int i = 0; i < dep; i++)
			word_table[idx].word[i] = cur_word[i];
		word_table[idx].word[dep] = 0;
		word_table[idx].length = dep;
		word_table[idx].freq = p->freq;
		p->word_index = idx;
	}
}
int scan_words()
{
	printf("Begin scanning words\n");
	fseek(file_input, 0, SEEK_SET);
	word_trie_root = new TrieNode();
	TrieNode *p = word_trie_root;

	int ch_int;
	int dep = 0;
	while((ch_int = read_char()) != -1)
	{
		unsigned char ch = (unsigned char) ch_int;
		int idx = letter_to_trie_code(ch);
		if(idx == -1)
		{
			if(p != word_trie_root) p->freq++;
			p = word_trie_root;
			dep = 0;
		}
		else
		{
			if(dep <= max_word_length)
			{
				if(p->ch[idx] == nullptr)
				{
					p->ch[idx] = new TrieNode(ch);
					p->ch[idx]->parent = p;
				}
				p = p->ch[idx];
				dep++;
			}	
		}
	}
	printf("Successfully build trie\n");
	traverse_word_trie(word_trie_root);
	printf("Successfully traversed trie\n");
	if(freq_table.size() > max_word_number - 256)
	{
		std::sort(freq_table.begin(), freq_table.end(), std::greater<int>());
		word_threshold = freq_table[max_word_number - 256];
	}
	polish_word_trie(word_trie_root, 0);
	printf("Successfully polished trie\n");
	return 0;
}

int scan_file()
{
	for(int i = 0; i < 256; i++)
		add_word((unsigned char)i);

	fseek(file_input, 0, SEEK_SET);
	int word_index;
	while((word_index = read_char()) != -1)
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
	if(word_threshold >= 0) scan_words();
	return 0;
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
		// printf("Leaf ");
		// printf("%d\n", p->leaf.word_index);
		// printf("1\n");
		free(p);
		return;
	}
	// printf("0l\n");
	// printf("%lld %lld %lld\n", p, p->node.lchild, p->node.rchild);
	delete_huffman_tree(p->node.lchild);
	// printf("0r\n");
	delete_huffman_tree(p->node.rchild);
	// printf("1\n");
	free(p);
}

void delete_trie(TrieNode* p)
{
	if(p == nullptr) return;
	for(int i = 0; i < letter_n; i++)
		delete_trie(p->ch[i]);
	delete p;
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
	std::cout << "Successfully freed the huffman tree." << std::endl;
	if(word_threshold >= 0)
	{
		delete_trie(word_trie_root);
		std::cout << "Successfully freed the trie." << std::endl;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		std::cout << "Format error.\nExpect:"
			" ./compressor <input_file> <output_file>" << std::endl;
		return 0;
	}
	else if(argc != 3 && argc != 5)
	{
		if(strcmp(argv[3], "-w") != 0)
		{
			std::cout << "Format error.\nExpect:"
				" ./compressor <input_file> <output_file> -w <word_freq>"
			   	<< std::endl;
			return 0;
		}
	}
	else if(argc == 5)
	{
		word_threshold = atoi(argv[4]);
		if(word_threshold < 0)
		{
			std::cout << "Word freq should be positive." << std::endl;
			return 0;
		}
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
