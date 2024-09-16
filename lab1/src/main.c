#include <stdio.h>

typedef struct PolynomialNode
{
	double cof;
	int deg;
	struct PolynomialNode *next;
} PolynomialNode;

typedef struct Polynomial
{
	PolynomialNode *data;
	int length;
} Polynomial;

Polynomial polynomial_add(Polynomial a, Polynomial b)
{
	Polynomial c;
	return c;
}

Polynomial polynomial_substract(Polynomial a, Polynomial b)
{
	Polynomial c;
	return c;
}

Polynomial polynomial_derivation(Polynomial a)
{
	Polynomial da;
	return da;
}

Polynomial polynomial_multiply(Polynomial a, Polynomial b)
{
	Polynomial c;
	return c;
}

Polynomial polynomial_read()
{
	Polynomial a;
	return a;
}

void polynomial_print(Polynomial a)
{
}

int main()
{
	return 0;
}
