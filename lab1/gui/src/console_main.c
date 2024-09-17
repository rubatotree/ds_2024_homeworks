#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "polynomial.h"

#define STRING_MAXLEN 65536
#define REGISTER_NUMBER 65536

Polynomial polynomial_registers[REGISTER_NUMBER];

// View the messages in the console.
int echo = 1;

int invalid_register(int reg)
{
	return reg < 0 || reg > REGISTER_NUMBER;
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
			polynomial_build_from_string(&poly, polynomial_str);
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
