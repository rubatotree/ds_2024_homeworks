#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *file_input, *file_output;
unsigned long long freq[256];
unsigned long long file_len;
unsigned char *file_str;

int compress()
{
	for(int i = 0; i < file_len; i++)
		file_str[i]++;
}

int read_input()
{
	fseek(file_input, 0, SEEK_END);
	file_len = (unsigned long long)ftell(file_input);
	fseek(file_input, 0, SEEK_SET);
	file_str = (unsigned char *)malloc((file_len + 1) * sizeof(char));
	if(!fread(file_str, sizeof(char), file_len, file_input))
	{
		printf("Fail to read input file.\n");
		return 1;
	}
	memset(freq, 0, sizeof(freq));
	for(int i = 0; i < file_len; i++)
	{
		freq[(unsigned char)file_str[i]]++;
	}
	return 0;
}

int process()
{
	if(read_input()) return 1;
	if(compress()) return 1;
	fwrite(file_str, sizeof(char), file_len, file_output);
	free(file_str);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("Format error.\nExpect: ./compressor <input_file> <output_file>\n");
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
	fclose(file_input);
	fclose(file_output);
	return status;
}
