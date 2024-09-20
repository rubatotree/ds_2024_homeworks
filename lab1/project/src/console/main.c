#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "polynomial.h"

#define STRING_MAXLEN 65536
#define REGISTER_NUMBER 65536

Polynomial polynomial_registers[REGISTER_NUMBER];
#define REG_LAST polynomial_registers[0]

// View the messages in the console.
int echo = 1, echoformat = 0;

int invalid_register(int reg)
{
	return reg < 0 || reg > REGISTER_NUMBER;
}

// Commands:
// ADD 
// SUB
// MUL
// DERI
// FORMAT
// [polynomial]     // Read and parse a formatted polynomial.
// READ				// Read the polynomial as the Array format.
// There are 65536 registers from 0 to 65535 for you to use.
// SET [reg]		// Read a line of polynomial and store it in the register.
// SAVE [reg]		// Save the answer into the register. 
// CLEAR [reg]		// Set a register to 0.
// COPY [rega] [regb]	// Copy the polynomial in rega into regb.
// PRINT [reg]			// Print the polynomial in the register.
// PRINTF [reg]			// Print the polynomial in the register(Math Expr).
// RADD [rega] [regb] [regc]	// SET regc = rega + regb
// RSUB [rega] [regb] [regc]	// SET regc = rega - regb
// RMUL [rega] [regb] [regc]	// SET regc = rega * regb
// RDERI [rega] [regb]			// SET regb = rega'

void input_commands()
{
	while(1)
	{
		if(echo) printf("> ");

		char cmd[STRING_MAXLEN];
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
			polynomial_delete(&polynomial_registers[reg]);

			// Read a line as the polynomial.
			char polynomial_str[STRING_MAXLEN];
			fgets(polynomial_str, STRING_MAXLEN - 1, stdin);

			Polynomial poly;
			poly.data_head = NULL;
			polynomial_build_from_string(&poly, polynomial_str);
			polynomial_registers[reg] = poly;
		}
		else if(strcmp(cmd, "READ") == 0)
		{
			int n;
			if(echo) printf("Please input the number of terms:\n");
			scanf("%d", &n);
			polynomial_delete(&REG_LAST);
			for(int i = 0; i < n; i++)
			{
				double cof;
				int deg;
				scanf("%lf%d", &cof, &deg);
				polynomial_insert(&REG_LAST, cof, deg);
			}
			if(echo) polynomial_print(REG_LAST);
		}
		else if(strcmp(cmd, "SAVE") == 0)
		{
			int reg;
			scanf("%d", &reg);
			if(invalid_register(reg))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}

			polynomial_delete(&polynomial_registers[reg]);
			polynomial_copy(&polynomial_registers[reg], &REG_LAST);

			if(echo)
			{
				printf("Set register %d = ", reg);
				polynomial_print(polynomial_registers[reg]);
			}
		}
		else if(strcmp(cmd, "CLEAR") == 0)
		{
			int reg;
			scanf("%d", &reg);
			if(invalid_register(reg))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}

			polynomial_delete(&polynomial_registers[reg]);

			if(echo)
			{
				printf("Clear register %d\n", reg);
			}
		}
		else if(strcmp(cmd, "COPY") == 0)
		{
			int rega, regb;
			scanf("%d%d", &rega, &regb);
			if(invalid_register(rega) || invalid_register(regb))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}

			polynomial_delete(&polynomial_registers[regb]);
			polynomial_copy(&polynomial_registers[regb],
					&polynomial_registers[rega]);

			if(echo)
			{
				printf("Set register %d to ", regb);
				polynomial_print(polynomial_registers[regb]);
			}
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
			polynomial_print_raw_polynomial(polynomial_registers[reg]);
			polynomial_copy(&REG_LAST, &polynomial_registers[reg]);
		}
		else if(strcmp(cmd, "PRINTF") == 0)
		{
			int reg;
			scanf("%d", &reg);
			if(invalid_register(reg))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			polynomial_print(polynomial_registers[reg]);
			polynomial_copy(&REG_LAST, &polynomial_registers[reg]);
		}
		else if(strcmp(cmd, "FORMAT") == 0)
		{
			polynomial_print(REG_LAST);
		}
		else if(strcmp(cmd, "ADD") == 0)
		{
			char polynomial_str[STRING_MAXLEN];
			Polynomial poly, poly_new;
			scanf("\n");
			fgets(polynomial_str, STRING_MAXLEN - 1, stdin);
			poly.data_head = NULL;
			polynomial_build_from_string(&poly, polynomial_str);

			poly_new = polynomial_add(REG_LAST, poly);
			polynomial_delete(&poly);
			polynomial_delete(&REG_LAST);
			REG_LAST = poly_new;

			if(echo)
			{
				if(echoformat)
					polynomial_print(REG_LAST);
				else
					polynomial_print_raw_polynomial(REG_LAST);
			}
		}
		else if(strcmp(cmd, "SUB") == 0)
		{
			char polynomial_str[STRING_MAXLEN];
			Polynomial poly, poly_new;
			scanf("\n");
			fgets(polynomial_str, STRING_MAXLEN - 1, stdin);
			poly.data_head = NULL;
			polynomial_build_from_string(&poly, polynomial_str);

			poly_new = polynomial_subtract(REG_LAST, poly);
			polynomial_delete(&poly);
			polynomial_delete(&REG_LAST);
			REG_LAST = poly_new;
			if(echo)
			{
				if(echoformat)
					polynomial_print(REG_LAST);
				else
					polynomial_print_raw_polynomial(REG_LAST);
			}
		}
		else if(strcmp(cmd, "MUL") == 0)
		{
			char polynomial_str[STRING_MAXLEN];
			Polynomial poly, poly_new;
			scanf("\n");
			fgets(polynomial_str, STRING_MAXLEN - 1, stdin);
			poly.data_head = NULL;
			polynomial_build_from_string(&poly, polynomial_str);

			poly_new = polynomial_multiply(REG_LAST, poly);
			polynomial_delete(&poly);
			polynomial_delete(&REG_LAST);
			REG_LAST = poly_new;
			if(echo)
			{
				if(echoformat)
					polynomial_print(REG_LAST);
				else
					polynomial_print_raw_polynomial(REG_LAST);
			}
		}
		else if(strcmp(cmd, "DERI") == 0)
		{
			Polynomial poly = polynomial_derivation(REG_LAST);
			polynomial_delete(&REG_LAST);
			REG_LAST = poly;
			if(echo)
			{
				if(echoformat)
					polynomial_print(REG_LAST);
				else
					polynomial_print_raw_polynomial(REG_LAST);
			}
		}
		else if(strcmp(cmd, "RADD") == 0)
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
			polynomial_delete(&polynomial_registers[regc]);
			polynomial_registers[regc] = poly;
			if(&polynomial_registers[regc] == &REG_LAST)
			{
				if(echo) polynomial_print(REG_LAST);
			}
		}
		else if(strcmp(cmd, "RSUB") == 0)
		{
			int rega, regb, regc;
			scanf("%d%d%d", &rega, &regb, &regc);
			if(invalid_register(rega) || invalid_register(regb)
				|| invalid_register(regc))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			Polynomial poly = polynomial_subtract(
					polynomial_registers[rega], polynomial_registers[regb]);
			polynomial_delete(&polynomial_registers[regc]);
			polynomial_registers[regc] = poly;
			if(&polynomial_registers[regc] == &REG_LAST)
			{
				if(echo) polynomial_print(REG_LAST);
			}
		}
		else if(strcmp(cmd, "RMUL") == 0)
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
			polynomial_delete(&polynomial_registers[regc]);
			polynomial_registers[regc] = poly;
			if(&polynomial_registers[regc] == &REG_LAST)
			{
				if(echo) polynomial_print(REG_LAST);
			}
		}
		else if(strcmp(cmd, "RDERI") == 0)
		{
			int rega, regb;
			scanf("%d%d", &rega, &regb);
			if(invalid_register(rega) || invalid_register(regb))
			{
				if(echo) printf("Invalid register number.\n");
				continue;
			}
			Polynomial poly = polynomial_derivation(polynomial_registers[rega]);
			polynomial_delete(&polynomial_registers[regb]);
			polynomial_registers[regb] = poly;
			if(&polynomial_registers[regb] == &REG_LAST)
			{
				if(echo) polynomial_print(REG_LAST);
			}
		}
		else if(strcmp(cmd, "EXIT") == 0)
		{
			break;
		}
		else
		{

			int cmdlen = strlen(cmd);
			fgets(cmd + cmdlen, STRING_MAXLEN - 1 - cmdlen, stdin);
			polynomial_delete(&REG_LAST);
			polynomial_build_from_string(&REG_LAST, cmd);
			if(echo) polynomial_print(REG_LAST);
		}
	}
}

int main(int argc, char* argv[])
{
	for(int i = 0; i < REGISTER_NUMBER; i++)
	{
		polynomial_registers[i].data_head = NULL;
	}
	if(argc >= 2)
	{
		for(int i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "-q") == 0)
			{
				echo = 0;
			}
			else if(strcmp(argv[i], "-f") == 0)
			{
				echoformat = 1;
			}
		}
	}
	input_commands();
	return 0;
}
