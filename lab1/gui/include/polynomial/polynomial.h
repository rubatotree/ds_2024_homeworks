#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// If there are two terms ax^n + bx^n, and |a + b| < EPSILON, 
// then a + b can be regarded as 0 and we can merge ax^n + bx^n to be 0.
#define POLYNOMIAL_EPSILON 1e-10

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

// compare == 1 => a is outputed front than b
// compare == 0 => a can be merged with b
// compare == -1 => a is outputed after b
int polynomial_compare(PolynomialNode *a, PolynomialNode *b);

// Compress the polynomial. Example: [x + 2x] ==> [3x]
void polynomial_compress(Polynomial* poly);

// Insert a term to the position and guarantee the list is correctly sorted.
void polynomial_insert(Polynomial* poly, double cof, int deg);

Polynomial polynomial_add(Polynomial a, Polynomial b);

Polynomial polynomial_substract(Polynomial a, Polynomial b);

Polynomial polynomial_derivation(Polynomial a);

Polynomial polynomial_multiply(Polynomial a, Polynomial b);

void polynomial_delete(Polynomial poly);

// Analyze the polynomial in a string.
void polynomial_build_from_string(Polynomial *poly, char *const format_str);

void polynomial_print_raw_list(Polynomial poly);

void polynomial_print(Polynomial poly);

#ifdef __cplusplus
}
#endif