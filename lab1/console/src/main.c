#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// If there are two terms ax^n + bx^n, and |a + b| < EPSILON, 
// then a + b can be regarded as 0 and we can merge ax^n + bx^n to be 0.
#define EPSILON 1e-10

#define STRING_MAXLEN 65536
#define REGISTER_NUMBER 65536

// #define DEBUG
#ifdef DEBUG
#define LOG printf
#else
#define LOG //
#endif

typedef enum
{
	OK, FAIL
} Status;

// A term is cof*x^deg.
typedef struct
{
	double cof;
	int deg;
} Term;

// Each node saves the data of a term.
typedef struct PolynomialNode
{
	Term term;
	struct PolynomialNode *next;
} PolynomialNode;

// And a Polynomial is a list of terms. 
// The terms are guarenteed to be sorted in the order set by comare(a, b).
typedef struct Polynomial
{
	PolynomialNode *data_head;
} Polynomial;

Polynomial polynomial_registers[REGISTER_NUMBER];

// View the messages in the console.
int echo = 1;

int invalid_register(int reg)
{
	return reg < 0 || reg > REGISTER_NUMBER;
}

// compare == 1 => a is outputed front than b
// compare == 0 => a can be merged with b
// compare == -1 => a is outputed after b
int compare(PolynomialNode *a, PolynomialNode *b)
{
	if(a->term.deg >  b->term.deg) return 1;
	if(a->term.deg == b->term.deg) return 0;
	if(a->term.deg <  b->term.deg) return -1;
}

// Compress the polynomial. Example: [x + 2x] ==> [3x]
// O(n).
void polynomial_compress(Polynomial* poly)
{
	// Merge the terms with the same degree.
	for(PolynomialNode *p = poly->data_head; p != NULL; p = p->next)
	{
		// *p can be merged with the next node.
		// While there are nodes to be merged, we shouldn't iterate into
		// the next node. So we should use 'while' instead of 'if'.
		while(p->next != NULL && compare(p, p->next) == 0)
		{
			// the node after *p is deleted.
			PolynomialNode* tmp = p->next;
			p->term.cof += p->next->term.cof;
			p->next = p->next->next;
			free(tmp);	
		}
	}

	// After the merge, some terms may have the coefficient become 0.
	// Then we should remove these terms in the list.
	
	// We should first remove the 0s in the head of the list.
	while(poly->data_head != NULL 
			&& fabs(poly->data_head->term.cof) < EPSILON)
	{
		PolynomialNode* tmp = poly->data_head;
		poly->data_head = poly->data_head->next;
		free(tmp);
	}
	for(PolynomialNode *p = poly->data_head; p != NULL; p = p->next)
	{
		while(p->next != NULL && fabs(p->next->term.cof) < EPSILON)
		{
			// The node after *p could be removed.
			PolynomialNode* tmp = p->next;
			p->next = p->next->next;
			free(tmp);	
		}
	}
}

// Insert a term to the position and guarantee the list is correctly sorted.
// O(n).
void polynomial_insert(Polynomial* poly, double cof, int deg)
{
	LOG("# Insert %lf x^%d\n", cof, deg);
	PolynomialNode *node = (PolynomialNode*)malloc(sizeof(PolynomialNode));
	node->term.cof = cof;
	node->term.deg = deg;
	node->next = NULL;

	if(poly->data_head == NULL)			// the list is emply
	{
		poly->data_head = node;
	}
	else if(compare(node, poly->data_head) == 1)	
	{
		// node is in the front of the list
		node->next = poly->data_head;
		poly->data_head = node;
	}
	else
	{
		for(PolynomialNode *p = poly->data_head; p != NULL; p = p->next)
		{
			if(p->next == NULL || compare(node, p->next) == 1)
			{	
				// *node should just follow after *p
				// do insert
				node->next = p->next;
				p->next = node;
				p = node;
				break;
			}
		}
	}

	// We havn't merge the term to the existed term with the same degree.
	// so we should compress the polynomial.
	polynomial_compress(poly);
}

Polynomial polynomial_add(Polynomial a, Polynomial b)
{
	// Simply add each term of a and each term of b together.
	// And merge the terms with the same degree.
	// O(n^2).
	Polynomial c;
	c.data_head = NULL;
	for(PolynomialNode *p = a.data_head; p != NULL; p = p->next)
		polynomial_insert(&c, p->term.cof, p->term.deg);
	for(PolynomialNode *p = b.data_head; p != NULL; p = p->next)
		polynomial_insert(&c, p->term.cof, p->term.deg);
	return c;
}

Polynomial polynomial_substract(Polynomial a, Polynomial b)
{
	// Simply add each term of a and each negative term of b together.
	// And merge the terms with the same degree.
	// O(n^2).
	Polynomial c;
	c.data_head = NULL;
	for(PolynomialNode *p = a.data_head; p != NULL; p = p->next)
		polynomial_insert(&c, p->term.cof, p->term.deg);
	for(PolynomialNode *p = b.data_head; p != NULL; p = p->next)
		polynomial_insert(&c, -p->term.cof, p->term.deg);
	return c;
}

Polynomial polynomial_derivation(Polynomial a)
{
	// Derivate each term in a ((cof * x^deg)' = cof * deg * x^(deg - 1)), and
	// add them together into da.
	// It can be optimized to O(n) because the order of the degrees haven't 
	// changed.
	Polynomial da;
	da.data_head = NULL;
	PolynomialNode *prev = NULL;
	for(PolynomialNode *p = a.data_head; p != NULL; p = p->next)
	{
		if(p->term.deg != 0)
		{
			PolynomialNode *node = (PolynomialNode*)malloc(sizeof(PolynomialNode));
			node->term.cof = p->term.cof * p->term.deg;
			node->term.deg = p->term.deg - 1;
			node->next = NULL;
			if(prev == NULL) da.data_head = node;
			else prev->next = node;
			prev = node;
		}
	}
	return da;
}

Polynomial polynomial_multiply(Polynomial a, Polynomial b)
{
	// Multiply each term in a with each term in b, and add them together.
	// O(n^3).
	Polynomial c;
	c.data_head = NULL;
	for(PolynomialNode *p = a.data_head; p != NULL; p = p->next)
		for(PolynomialNode *q = b.data_head; q != NULL; q = q->next)
			polynomial_insert(&c, p->term.cof * q->term.cof, 
													p->term.deg + q->term.deg);
	return c;
}

void polynomial_delete(Polynomial poly)
{
	PolynomialNode *p = poly.data_head, *tmp = NULL;
	while(p != NULL)
	{
		tmp = p->next;
		free(p);
		p = tmp;
	}
}

// Analyze the polynomial in a string.
Status polynomial_build_from_string(Polynomial *poly, char *const format_str)
{
	int length = strlen(format_str);
	char* str = (char*)malloc(sizeof(char) * (length + 1));
	strcpy(str, format_str);

	Polynomial a;
	a.data_head = NULL;

	// Split the string into terms.
	// When we read a term, it is str[p_term_head .. p_term_tail].
	unsigned int p_term_head = 0, p_term_tail = 1;

	// A flag to solve the case that we want to input [x^3]+[x^(-2)]+[-1].
	// If we input x3+x-2-1, it will be read as [x^3]+[x]+[-2]+[-1].
	// So we should add a symbol '^' and input x^3+x^-2-1.

	// When we flagged that there is a '^', the next '+' or '-' won't be 
	// regarded as the sign of the next term, but be regarded as the sign of 
	// the degree.
	int flag_in_degree = 0;
	while(p_term_head < length)
	{
		// Find a '+' or '-' with no '^' ahead or end of the string
		// so the input 5x^3+6x^-2-12x+36 is splited to be
		// [5x^3] + [6x^-2] - [12x] + [36].
		if(p_term_tail >= length
			|| ((str[p_term_tail] == '+' || str[p_term_tail] == '-')
				&& !flag_in_degree))
		{
			if(p_term_tail > length)
				p_term_tail = length;

			// Step 1: Find the signs to split a term
			
			// Save the sign. That position might be set to ' '.
			int sign = 1;
			if(str[p_term_head] == '-')
				sign = -1;

			// Save the next symbol. That position will be set to '\0'.
			// If there is no next term, that position is '\0', we can treat
			// it samely.
			char next_symbol = str[p_term_tail];
			str[p_term_tail] = '\0';

			// find 'x'
			// pos_x == p_term_tail ==> there's no 'x' in the string.
			int pos_x = p_term_head;
			while(str[pos_x] != 'x' && pos_x < p_term_tail)
				pos_x++;
			str[pos_x] = '\0';

			
			// Step 2: Format the string to make it able to read by sscanf.
			
			// remove the chars that makes error of sscanf.  
			for(int i = p_term_head; i < p_term_tail; i++)
			{
				if(!(str[i] >= '0' && str[i] <= '9' || str[i] == '.'
					|| str[i] == '-' || str[i] == '\0'))
				{
					str[i] = ' ';
				}
			}

			// A trick turns "- 5x" to " -5x", with "-5" in it to be read.
			for(int i = p_term_head; i < p_term_tail - 1; i++)
			{
				if(str[i] == '-' && str[i + 1] == ' ')
				{
					str[i] = ' ';
					str[i + 1] = '-';
				}
			}

			// Step 3: Find the cofficient and the degree.

			// For example: ...+5x^2-...
			// ...  +   5    x    ^     2       -    ...
			// ...|head-----\0|pos_x+1------\0(tail)|...
			// Then we can read the cof from the first string
			// and read the index from the second string.
			// ...+x^2-...		If there's no cof, the cof is +1 or -1.
			// ...+5x-...		If there's no deg, the deg is 1.
			// ...+12+...		If pos_x is tail, the deg is 0 (constant).
			double cof = 10000.0;
			int deg = 10000;
			int format_fail_flag = 0;

			if(pos_x == p_term_tail)	// The term is constant.
			{
				LOG("# current term [%s|EMPTY]\n", str + p_term_head);
				deg = 0;
				if(sscanf(str + p_term_head, "%lf", &cof) != 1)
				{
					// When the term is constant but there is no number in it,
					// we regard it as a error and ignore this term.
					format_fail_flag = 1;
				}
			}
			else
			{
				LOG("# current term [%s|%s]\n", 
						str + p_term_head, str + pos_x + 1);
				
				if(sscanf(str + p_term_head, "%lf", &cof) != 1)
				{
					// When we fail to read the cof, the cof is 1 or -1.
					if(sign == -1) 
					{
						cof = -1.0;
						LOG("# No cof, default as -1.\n");
					}
					else
					{
						cof = 1.0;
						LOG("# No cof, default as 1.\n");
					}
				}
				if(sscanf(str + pos_x + 1, "%d", &deg) != 1)
				{
					// When we fail to read the deg but there is a 'x',
					// it means that the degree is 1.
					deg = 1;
					LOG("# No deg, default as 1.\n");
				}
			}

			if(!format_fail_flag)
			{
				polynomial_insert(poly, cof, deg);
			}
			else
			{
				LOG("# No x and no cof, ignore this term.\n");
			}

			// Restore the symbol of the next term which is set to '\0'.
			str[p_term_tail] = next_symbol;

			// Move the head to the position of the symbol of the next term.
			p_term_head = p_term_tail;

			// Cancel the flag to correctly read the next term.
			flag_in_degree = 0;
		}
		else if(str[p_term_tail] == '^')
		{
			// Set up the flag so that the next signed number won't be 
			// regarded as the sign of the next term.
			flag_in_degree = 1;
		}
		else if (str[p_term_tail] >= '0' && str[p_term_tail] <= '9')
		{
			// When we read a number, the flag should be cancelled, to prevent
			// us from failing to read the sign of the next term.
			flag_in_degree = 0;
		}
		p_term_tail++;
	}

	return OK;
}

void polynomial_print_raw_list(Polynomial poly)
{
	printf("cof \t\t| deg\n");
	for(PolynomialNode *p = poly.data_head; p != NULL; p = p->next)
		printf("%lf \t| %d\n", p->term.cof, p->term.deg);
}

void polynomial_print(Polynomial poly)
{
	for(PolynomialNode *p = poly.data_head; p != NULL; p = p->next)
	{
		if(p != poly.data_head && p->term.cof > 0)
		{
			// ... [+ 5x^2] ...
			//      ^
			printf("+");
		}
		if(fabs(p->term.cof + 1) < EPSILON)
		{
			// ... [- x^2] ...
			//       ^
			printf("-");
			if(p->term.deg == 0)
				printf("1");
		}
		else if(fabs(p->term.cof - 1) < EPSILON)
		{
			// ... [+ 1] ...
			//        ^
			if(p->term.deg == 0)
				printf("1");
		}
		else
		{
			// ... [+ 5 x^2] ...
			//        ^
			if(fabs(p->term.cof - (double)(int)(p->term.cof)) < EPSILON)
			{
				printf("%d", (int)(p->term.cof));
			}
			else
			{
				printf("%.2lf", p->term.cof); 
			}
		}
		if(p->term.deg == 1)
		{
			// ... [+5x] ...
			//        ^
			printf("x");
		}
		else if(p->term.deg != 0)
		{
			// ... [+5x ^2] ...
			//           ^
			printf("x^%d", p->term.deg);
		}
	}
	if(poly.data_head == NULL)
	{
		// When the polynomial is Empty
		printf("0");
	}
	printf("\n");
}


// Commands:
// There are 65536 registers from 0 to 65535 for you to use.
// SET [reg]		// Read a line of polynomial and store it in the register.
// PRINT [reg]		// Print the polynomial in the register.
// ADD [rega] [regb] [regc]		// SET regc = rega + regb
// SUB [rega] [regb] [regc]		// SET regc = rega - regb
// MUL [rega] [regb] [regc]		// SET regc = rega * regb
// DERI [rega] [regb]			// SET regb = rega'

void input_commands()
{
	while(1)
	{
		char cmd[STRING_MAXLEN];
		if(echo) printf("> ");
		if(scanf("%s", cmd) == EOF)
		{
			break;
		}
		int len = strlen(cmd);
		for(int i = 0; i < len; i++)
			if(cmd[i] >= 'a' && cmd[i] <= 'z')
				cmd[i] += 'A' - 'a';
		if(strcmp(cmd, "SET") == 0)
		{
			int reg;
			scanf("%d", &reg);
			if(invalid_register(reg))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			if(echo)
				printf("Please input the polynomial in register %d:\n", reg);
			scanf("\n");

			// Delete the existed polynomial.
			polynomial_delete(polynomial_registers[reg]);

			// Read a line as the polynomial.
			char polynomial_str[STRING_MAXLEN];
			fgets(polynomial_str, STRING_MAXLEN - 1, stdin);

			Polynomial poly;
			poly.data_head = NULL;
			Status status = polynomial_build_from_string(&poly, polynomial_str);
			if(status != OK)
			{
				if(echo) printf("Failed to read the polynomial.");
				continue;
			}
			polynomial_registers[reg] = poly;
		}
		else if(strcmp(cmd, "PRINT") == 0)
		{
			int reg;
			scanf("%d", &reg);
			if(invalid_register(reg))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			polynomial_print(polynomial_registers[reg]);
		}
		else if(strcmp(cmd, "ADD") == 0)
		{
			int rega, regb, regc;
			scanf("%d%d%d", &rega, &regb, &regc);
			if(invalid_register(rega) || invalid_register(regb)
				|| invalid_register(regc))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			Polynomial poly = polynomial_add(
					polynomial_registers[rega], polynomial_registers[regb]);
			polynomial_delete(polynomial_registers[regc]);
			polynomial_registers[regc] = poly;
		}
		else if(strcmp(cmd, "SUB") == 0)
		{
			int rega, regb, regc;
			scanf("%d%d%d", &rega, &regb, &regc);
			if(invalid_register(rega) || invalid_register(regb)
				|| invalid_register(regc))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			Polynomial poly = polynomial_substract(
					polynomial_registers[rega], polynomial_registers[regb]);
			polynomial_delete(polynomial_registers[regc]);
			polynomial_registers[regc] = poly;
		}
		else if(strcmp(cmd, "MUL") == 0)
		{
			int rega, regb, regc;
			scanf("%d%d%d", &rega, &regb, &regc);
			if(invalid_register(rega) || invalid_register(regb)
				|| invalid_register(regc))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			Polynomial poly = polynomial_multiply(
					polynomial_registers[rega], polynomial_registers[regb]);
			polynomial_delete(polynomial_registers[regc]);
			polynomial_registers[regc] = poly;
		}
		else if(strcmp(cmd, "DERI") == 0)
		{
			int rega, regb;
			scanf("%d%d", &rega, &regb);
			if(invalid_register(rega) || invalid_register(regb))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			Polynomial poly = polynomial_derivation(polynomial_registers[rega]);
			polynomial_delete(polynomial_registers[regb]);
			polynomial_registers[regb] = poly;
		}
		else if(strcmp(cmd, "EXIT") == 0)
		{
			break;
		}
		else
		{
			if(echo) printf("Invalid command.\n");
		}
	}
}

int main(int argc, char* argv[])
{
	if(argc >= 2)
	{
		for(int i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "-q") == 0)
			{
				echo = 0;
			}
		}
	}
	input_commands();
	return 0;
}