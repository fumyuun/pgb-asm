/**
 * pengb-asm.c Copyright (C) 2011-2012 Koray Yanik <fumyuun@gmail.com>
 * Quick & Dirty Gameboy assembler. Written very quick and dirty.
 ***********************************
 * Assembles gameboy ROMs. The assembly language supported is Intel assembly
 * like (parameter order is destination, source), with the difference that it 
 * uses round brackets instead of square brackets for effective adress 
 * statements. Numbers are always interpeted in hexadecimal base, the prefix 0x
 * is allowed but optional. For a list of supported mnemonic instructions, see
 * http://gbdev.gg8.se/wiki/articles/CPU_Instruction_Set
 ***********************************
 * Syntax:
 * The source file must contain valid statements seperated by newlines. 
 * Spaces and tabs are used as in-statement seperators, but only the first one
 * counts (meaning that it doesn't matter if you put in more than one space or
 * tab or anything mixed between in-statement parts, they are all ignored).
 * A valid statement is either a preassembler instruction, a label, a comment or 
 * a mnemonic instruction. 
 * Preassembler instructions:
 * .align n: aligns the next statement to be n bytes after
 * the last. Will be filled up with zero'd bytes.
 * .data n: directly include the byte n at that position in the binary. n can
 * also be a list of numbers seperated by in-statement seperators or a comma.
 * .data "string": will include ascii values as constant bytes in the same way.
 * .include "filename": will include the file specified with filename at this 
 * point before assembling.
 * A comment starts with a # character, and will be ignored. Comments are the 
 * only type that are allowed after an other valid statement on the same line.
 * There are two types of labels, named labels and unnamed labels.
 * Named labels: start with an ascii-character. Are interpeted as normal 
 * assembly labels.
 * Unnamed labels: start with a number. The assembler will attempt to align
 * the next byte to this number as adress.
 * TODO: parse escaped characters in a string.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define IN_BUFLEN	1024
#define MAX_LABELS	1024
#define LABEL_LEN	128
#define MAX_INSTR	5
#define INCL_FLEN	128

// Used for labels
typedef struct
{
	char string[LABEL_LEN];
	unsigned int points_no;
	unsigned int pointsto;
	fpos_t pointsfrom[MAX_LABELS];
	char relative[MAX_LABELS];
	unsigned int refline;		// For undefined errors
	char reffile[INCL_FLEN];
} label_t;

typedef enum 
{
	ERR_NO,
	ERR_ARG,
	ERR_IO,
	ERR_SYNT
	
} error_e;

error_e _err = ERR_NO;
void assemble(char *filename, FILE *input, FILE *output, 
			  label_t labels[], size_t *label_no, unsigned int *out_no);
void parse_file_pass1(char *filename, FILE *input, FILE *output, 
					  label_t labels[], size_t *label_no, unsigned int *out_no);
void parse_instr(char *str, FILE *output, unsigned int *out_no, 
				 unsigned int *line_no, char *filename,
				 label_t labels[], size_t *label_no);
void parse_file_pass2(FILE *output, label_t labels[], size_t *label_no);
unsigned char calc_checksum(FILE *INPUT);

int main(int argc, char **argv)
{
	FILE *input = NULL, *output = NULL;
	label_t *labels = NULL;
	
	if(argc < 3)
	{
		printf("Usage: %s <inputfile> <outputfile>\n", argv[0]);
		_err = ERR_ARG;
		goto exit;
	}
	
	input = fopen(argv[1], "r");
	if(input == NULL)
	{
		printf("Unable to open \'%s\'!\n", argv[1]);
		_err = ERR_IO;
		goto exit;
	}
	
	output = fopen(argv[2], "wb+");
	if(output == NULL)
	{
		printf("Unable to create \'%s\'!\n", argv[2]);
		_err = ERR_IO;
		goto exit;
	}
	
	labels = (label_t*)malloc(sizeof(label_t) * MAX_LABELS);
	size_t label_no = 0;
	
	unsigned int i;
	for(i = 0; i < MAX_LABELS; ++i)
	{
		*(labels[i].string) = 0;
		labels[i].points_no = 0;
		labels[i].pointsto = -1;
		labels[i].refline = -1;
	}
	
	// Amount of bytes written
	unsigned int out_no = 0;
	assemble(argv[1], input, output, labels, &label_no, &out_no);
	
	if(_err != ERR_NO)
		goto exit;
	
	fclose(input);
	fclose(output);
	
	// Calculate checksum
	input = fopen(argv[2], "rb");
	if(input == NULL)
	{
		printf("Unable to open \'%s\'!\n", argv[2]);
		_err = ERR_IO;
		goto exit;
	}
	printf("Assembling completed. Header checksum: 0x%X\n", calc_checksum(input));
	
exit:
	if(labels != NULL)	free(labels);
	if(input != NULL)	fclose(input);
	if(output != NULL)	fclose(output);
	return _err;
}

void assemble(char *filename, FILE *input, FILE *output, label_t labels[], 
			  size_t *label_no, unsigned int *out_no)
{
	// First pass, leaves in labels
	parse_file_pass1(filename, input, output, labels, label_no, out_no);
	if(_err != ERR_NO)
		return;
	
	rewind(output);
	
	// Second pass, fixes labels
	parse_file_pass2(output, labels, label_no);
}

/**
 * Converts a C-string to full upper case.
 */
void strtoupper(char *str)
{
	size_t i;
	for(i = 0; i < strlen(str); ++i)
		str[i] = toupper(str[i]);
}

/**
 * Find a certain label in a range of a list. If not found, adds element after
 * list and returns that element.
 */
label_t *find_label(label_t *begin, size_t *elements, char *string)
{
	label_t *end = begin + *elements;
	while(*(begin->string) != 0 
		  && strcmp(begin->string, string) != 0 
		  && begin++ != end);
	if(*(begin->string) == 0)
	{
		strcpy(begin->string, string);
		*elements += 1;
	}
	return begin;
}

/**
 * Parses a file for the first pass. The first pass assembles instructions
 * to bytecode, ignores comments, imports binary data, and stores label source
 * bytepositions. Leaves labels in instructions intact (parsed in second pass).
 * Generates a .o file.
 */
void parse_file_pass1(char *filename, FILE *input, FILE *output, 
					  label_t labels[], size_t *label_no,
					  unsigned int *out_no)
{
	char in_buf[IN_BUFLEN];
	unsigned int line_no = 0;
	size_t str_pos;
	
	while(fgets(in_buf, IN_BUFLEN, input) != NULL)
	{
		if(_err != 0)
			break;
		
		line_no++;
		str_pos = 0;
		while(in_buf[str_pos] == ' ' || in_buf[str_pos] == '\t')
			++str_pos;
		
		size_t str_len = strlen(in_buf+str_pos);
		
		// Comment, ignore rest of string
		if(in_buf[str_pos] == '#')
			continue;
		// Empty line
		if(in_buf[str_pos] == '\n')
			continue;
		// .include file
		if(strstr(in_buf+str_pos, ".include") == in_buf+str_pos)
		{
			str_pos += 5;
			char inc_filename[INCL_FLEN];
			char *p1 = strchr(in_buf+str_pos, '\"');
			char *p2 = strrchr(in_buf+str_pos, '\"');
			if(p1 == NULL || p2 == NULL)
			{
				printf("%s:%u: Syntax error: \" expected near %s!\n", filename, line_no, in_buf + str_pos);
				_err = ERR_SYNT;
				return;
			}
			*p2 = 0;
			strncpy(inc_filename, p1+1, INCL_FLEN-1);
			FILE *inc_input = fopen(inc_filename, "r");
			if(inc_input == NULL)
			{
				printf("%s:%u: Unable to open included file \'%s\'!\n", filename, line_no, inc_filename);
				_err = ERR_SYNT;
				return;
			}
			parse_file_pass1(inc_filename, inc_input, output, labels, label_no, out_no);
			
			fclose(inc_input);
			continue;
		}
		
		// .data segment, parse rest as block of data seperated by ','
		if(strstr(in_buf+str_pos, ".data") == in_buf+str_pos)
		{
			str_pos += 5;
			while(in_buf[str_pos] == ' ' || in_buf[str_pos] == '\t')
				++str_pos;
			
			if(in_buf[str_pos] == '#')	// ignore comments
				continue;
			
			if(in_buf[str_pos] == '\"')	// string
			{
				++str_pos;
				while(in_buf[str_pos] != '\"' && in_buf[str_pos])
				{
					fputc(in_buf[str_pos], output);
					++str_pos;
					*out_no += 1;
				}
				continue;
			}
			else if(isdigit(in_buf[str_pos]))	// constant block
			{			
				char *pch = strtok(in_buf+str_pos, ", \n\t");
				while(pch != NULL)
				{
					if(*pch == '#')	// ignore comments
						break;
					
					long i = strtol(pch, NULL, 16);
					fputc((int)i, output);
					*out_no += 1;
					pch = strtok(NULL, ", \n\t");
				}
				continue;
			}
		}
		// .align n: fill with n zeros.
		if(strstr(in_buf+str_pos, ".align") == in_buf+str_pos)
		{
			str_pos += 6;
			while(in_buf[str_pos] == ' ' || in_buf[str_pos] == '\t')
				++str_pos;
			if(isdigit(in_buf[str_pos]))
			{
				long i = strtol(in_buf + str_pos, NULL, 16);
				*out_no += i;
				while(i-- > 0)
					fputc(0x00, output);
				continue;
			}
			else
			{
				printf("%s:%u: Syntax error, number constant expected near %s", filename, line_no, in_buf + str_pos);
				return;
			}
		}
		// Labels
		size_t llen = strcspn(in_buf+str_pos, ":");
		if(llen != str_len)
		{
			// label that starts with a digit must be a forced byte alignment.
			if(isdigit(in_buf[str_pos]))
			{
				unsigned int bytepos = strtol(in_buf+str_pos, NULL, 16);
				if(bytepos < *out_no)
				{
					printf("%s:%u: Cannot align to byte adress 0x%X, assembled binary size is already 0x%X!\n", filename, line_no, bytepos, *out_no);
					_err = ERR_SYNT;
					return;
				}
				while(*out_no != bytepos)
				{
					fputc(0x00, output);
					*out_no += 1;
				}
				continue;
			}
			
			// otherwise treat as normal label.
			char buf[LABEL_LEN];
			strncpy(buf, in_buf+str_pos, llen);
			buf[llen] = 0;
			strtoupper(buf);
			
			label_t *l = find_label(labels, label_no, buf);
			l->pointsto = *out_no;
			continue;
		}
		
		parse_instr(in_buf+str_pos, output, out_no, &line_no, filename,
					labels, label_no);
	}
}

/**
 * Parse file for a second pass. Changes all labels in their labelpositions.
 */
void parse_file_pass2(FILE *output, label_t labels[], size_t *label_no)
{
	size_t i;
	for(i = 0; i < *label_no; ++i)
	{
		if(labels[i].pointsto == (unsigned int)-1)
		{
			printf("%s:%d: Undefined label \'%s\' referenced!\n", labels[i].reffile, labels[i].refline, labels[i].string);
			_err = ERR_SYNT;
			return;
		}
		size_t j;
		for(j = 0; j < labels[i].points_no; ++j)
		{
			fsetpos(output, &labels[i].pointsfrom[j]);
			if(labels[i].relative[j])
			{
				char rel = labels[i].pointsto - ftell(output) - 1;
				fputc(rel, output);
			}
			else
			{
				fputc(labels[i].pointsto & 0xFF, output);
				fputc((labels[i].pointsto >> 8) & 0xFF, output);
			}
		}
	}
}

/**
 * Calculate checksum of a binary.
 */
unsigned char calc_checksum(FILE *input)
{
	char c;
	unsigned char checksum = 0;
	unsigned int i = 0;
	while(i < 0x0134)
	{
		c = fgetc(input);
		++i;
	}
	while(i < 0x014D)
	{
		c = fgetc(input);
		checksum = checksum - c - 1;
		++i;
	}
	
	return checksum;
}

// terrible macro's incoming, read at own risk :)

#define match0(x)	(strcmp(instr[0],x) == 0)
#define match1(x)	(strcmp(instr[1],x) == 0)
#define match2(x)	(strcmp(instr[2],x) == 0)
#define match3(x)	(strcmp(instr[3],x) == 0)
#define match4(x)	(strcmp(instr[4],x) == 0)
#define matchd1		(isdigit(*instr[1]))
#define matchd2		(isdigit(*instr[2]))
#define matchdx(x)	(isdigit(*x))
// pointers have () brackets
#define matchp1		((instr[1][0] == '(') \
					&& (instr[1][strlen(instr[1])-1] == ')'))
#define matchp2		((instr[2][0] == '(') \
					&& (instr[2][strlen(instr[2])-1] == ')'))
#define write(x)	{fputc(x, output); (*out_no)++;}
// decimal short
#define writeds1	{write((int)strtol(instr[1], NULL, 16));}
#define writeds2	{write((int)strtol(instr[2], NULL, 16));}
// decimal long
#define writedl1	{int i = strtol(instr[1], NULL, 16);\
					 write((int)(i & 0xFF)); write((int)((i >> 8) & 0xFF));}
#define writedl2	{int i = strtol(instr[2], NULL, 16);\
					 write((int)(i & 0xFF)); write((int)((i >> 8) & 0xFF));}
#define writedlx(x)	{int i = strtol(x, NULL, 16);\
					write((int)(i & 0xFF)); write((int)((i >> 8) & 0xFF));}
#define writedsx(x)	{int i = strtol(x, NULL, 16);\
					write((int)(i & 0xFF));}
// label

#define writellx(x)	\
{	label_t *l = find_label(labels, label_no, x);	\
	fgetpos(output, &l->pointsfrom[l->points_no]);	\
	l->relative[l->points_no++] = 0;				\
	if(l->refline == (unsigned int)-1)				\
	{												\
		l->refline = *line_no;						\
		strncpy(l->reffile, filename, INCL_FLEN-1);	\
	}												\
	write(0); write(0);	}
#define writell1	{writellx(instr[1]);}
#define writell2	{writellx(instr[2]);}

#define writelsx(x)	\
{	label_t *l = find_label(labels, label_no, x);	\
	fgetpos(output, &l->pointsfrom[l->points_no]);	\
	l->relative[l->points_no++] = 1;				\
	if(l->refline == (unsigned int)-1)				\
	{												\
		l->refline = *line_no;						\
		strncpy(l->reffile, filename, INCL_FLEN-1);	\
	}												\
	write(0);}
#define writels1	{writelsx(instr[1]);}
#define writels2	{writelsx(instr[2]);}

/**
 * Parse an instruction line. Leaves labels in the code.
 */
void parse_instr(char *str, FILE *output, unsigned int *out_no, 
				 unsigned int *line_no, char *filename,
				 label_t labels[], size_t *label_no)
{
	strtoupper(str);
	
	char *instr[MAX_INSTR];
	unsigned int instr_n = 0;
	char *pch;
	pch = strtok(str, ", \n\t");
	while(pch != NULL)
	{
		if(*pch == '#')	// Ignore comments, yet again
			break;
		instr[instr_n++] = pch;
		pch = strtok(NULL, ", \n\t");
	}
	switch(instr_n)
	{
		case 1:
			if(match0("CCF")){	write(0x3F);				break;}
			if(match0("CPL")){	write(0x2F);				break;}
			if(match0("DAA")){	write(0x27);				break;}
			if(match0("DI")){	write(0xF3);				break;}
			if(match0("EI")){	write(0xFB);				break;}
			if(match0("HALT")){	write(0x76);				break;}
			if(match0("NOP")){	write(0x00);				break;}
			if(match0("RET")){	write(0xC9);				break;}
			if(match0("RETI")){	write(0xD9);				break;}
			if(match0("RLA")){	write(0x17);				break;}
			if(match0("RLCA")){	write(0x07);				break;}
			if(match0("RRA")){	write(0x1F);				break;}
			if(match0("RRCA")){	write(0x0F);				break;}
			if(match0("SCF")){	write(0x37);				break;}
			if(match0("STOP")){	write(0x10); write(0x00);	break;}
			printf("%s:%u: error: syntax error near \'%s\'\n", 
				   filename, *line_no, instr[0]);
			_err = ERR_SYNT;
			break;
		case 2:
			if(match0("ADD"))// ADD n
			{
				if(match1("A")){	write(0x87);				break;}
				if(match1("B")){	write(0x80);				break;}
				if(match1("C")){	write(0x81);				break;}
				if(match1("D")){	write(0x82);				break;}
				if(match1("E")){	write(0x83);				break;}
				if(match1("H")){	write(0x84);				break;}
				if(match1("L")){	write(0x85);				break;}
				if(match1("(HL)")){	write(0x86);				break;}
				if(matchd1){		write(0xC6);	writeds1;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("ADC"))// ADC n
			{
				if(match1("A")){	write(0x8F);				break;}
				if(match1("B")){	write(0x88);				break;}
				if(match1("C")){	write(0x89);				break;}
				if(match1("D")){	write(0x8A);				break;}
				if(match1("E")){	write(0x8B);				break;}
				if(match1("H")){	write(0x8C);				break;}
				if(match1("L")){	write(0x8D);				break;}
				if(match1("(HL)")){	write(0x8E);				break;}
				if(matchd1){		write(0xCE);	writeds1;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("AND"))// AND n
			{
				if(match1("A")){	write(0xA7);				break;}
				if(match1("B")){	write(0xA0);				break;}
				if(match1("C")){	write(0xA1);				break;}
				if(match1("D")){	write(0xA2);				break;}
				if(match1("E")){	write(0xA3);				break;}
				if(match1("H")){	write(0xA4);				break;}
				if(match1("L")){	write(0xA5);				break;}
				if(match1("(HL)")){	write(0xA6);				break;}
				if(matchd1){		write(0xE6);	writeds1;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("CALL"))// CALL nn
			{
				write(0xCD);
				if(matchd1){	writedl1;	break;}	// nn
				else{			writell1;	break;} // label
			}
			if(match0("CP"))// CP n
			{
				if(match1("A")){	write(0xBF);				break;}
				if(match1("B")){	write(0xB8);				break;}
				if(match1("C")){	write(0xB9);				break;}
				if(match1("D")){	write(0xBA);				break;}
				if(match1("E")){	write(0xBB);				break;}
				if(match1("H")){	write(0xBC);				break;}
				if(match1("L")){	write(0xBD);				break;}
				if(match1("(HL)")){	write(0xBE);				break;}
				if(matchd1){		write(0xFE);	writeds1;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("DEC"))// DEC n, DEC nn
			{
				if(match1("A")){	write(0x3D);				break;}
				if(match1("B")){	write(0x05);				break;}
				if(match1("C")){	write(0x0D);				break;}
				if(match1("D")){	write(0x15);				break;}
				if(match1("E")){	write(0x1D);				break;}
				if(match1("H")){	write(0x25);				break;}
				if(match1("L")){	write(0x2D);				break;}
				if(match1("(HL)")){	write(0x35);				break;}
				if(match1("BC")){	write(0x0B);				break;}
				if(match1("DE")){	write(0x1B);				break;}
				if(match1("HL")){	write(0x2B);				break;}
				if(match1("SP")){	write(0x3B);				break;}
				printf("%s:%u: error: register, register-pair or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("INC"))// INC n, INC nn
			{
				if(match1("A")){	write(0x3C);				break;}
				if(match1("B")){	write(0x04);				break;}
				if(match1("C")){	write(0x0C);				break;}
				if(match1("D")){	write(0x14);				break;}
				if(match1("E")){	write(0x1C);				break;}
				if(match1("H")){	write(0x24);				break;}
				if(match1("L")){	write(0x24);				break;}
				if(match1("(HL)")){	write(0x34);				break;}
				if(match1("BC")){	write(0x03);				break;}
				if(match1("DE")){	write(0x13);				break;}
				if(match1("HL")){	write(0x23);				break;}
				if(match1("SP")){	write(0x33);				break;}
				printf("%s:%u: error: register, register-pair or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("JP"))// JP (HL), JP nn
			{
				if(match1("(HL)")){	write(0xE9);				break;}
				if(matchd1){		write(0xC3);	writedl1;	break;}	// long
				else{				write(0xC3);	writell1;	break;} // label
				
			}
			if(match0("JR"))// JR nn
			{
				if(matchd1){		write(0x18);	writeds1;	break;}	// short
				else{				write(0x18);	writels1;	break;} // label
			}
			if(match0("OR"))// OR n
			{
				if(match1("A")){	write(0xB7);				break;}
				if(match1("B")){	write(0xB0);				break;}
				if(match1("C")){	write(0xB1);				break;}
				if(match1("D")){	write(0xB2);				break;}
				if(match1("E")){	write(0xB3);				break;}
				if(match1("H")){	write(0xB4);				break;}
				if(match1("L")){	write(0xB5);				break;}
				if(match1("(HL)")){	write(0xB6);				break;}
				if(matchd1){		write(0xF6);	writeds1;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("POP"))// POP nn
			{
				if(match1("AF")){	write(0xF1);				break;}
				if(match1("BC")){	write(0xC1);				break;}
				if(match1("DE")){	write(0xD1);				break;}
				if(match1("HL")){	write(0xE1);				break;}
				printf("%s:%u: error: register-pair expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("PUSH"))// PUSH nn
			{
				if(match1("AF")){	write(0xF5);				break;}
				if(match1("BC")){	write(0xC5);				break;}
				if(match1("DE")){	write(0xD5);				break;}
				if(match1("HL")){	write(0xE5);				break;}
				printf("%s:%u: error: register-pair expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("RET"))// RET cc
			{
				if(match1("NZ")){	write(0xC0);				break;}
				if(match1("Z")){	write(0xC8);				break;}
				if(match1("NC")){	write(0xD0);				break;}
				if(match1("C")){	write(0xD8);				break;}
				printf("%s:%u: error: condition expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("RLC"))// RLC n
			{
				if(match1("A")){	write(0xCB); write(0x07);	break;}
				if(match1("B")){	write(0xCB); write(0x00);	break;}
				if(match1("C")){	write(0xCB); write(0x01);	break;}
				if(match1("D")){	write(0xCB); write(0x02);	break;}
				if(match1("E")){	write(0xCB); write(0x03);	break;}
				if(match1("H")){	write(0xCB); write(0x04);	break;}
				if(match1("L")){	write(0xCB); write(0x05);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x06);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("RL"))// RL n
			{
				if(match1("A")){	write(0xCB); write(0x17);	break;}
				if(match1("B")){	write(0xCB); write(0x10);	break;}
				if(match1("C")){	write(0xCB); write(0x11);	break;}
				if(match1("D")){	write(0xCB); write(0x12);	break;}
				if(match1("E")){	write(0xCB); write(0x13);	break;}
				if(match1("H")){	write(0xCB); write(0x14);	break;}
				if(match1("L")){	write(0xCB); write(0x15);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x16);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("RRC"))// RRC n
			{
				if(match1("A")){	write(0xCB); write(0x0F);	break;}
				if(match1("B")){	write(0xCB); write(0x08);	break;}
				if(match1("C")){	write(0xCB); write(0x09);	break;}
				if(match1("D")){	write(0xCB); write(0x0A);	break;}
				if(match1("E")){	write(0xCB); write(0x0B);	break;}
				if(match1("H")){	write(0xCB); write(0x0C);	break;}
				if(match1("L")){	write(0xCB); write(0x0D);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x0E);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("RR"))// RR n
			{
				if(match1("A")){	write(0xCB); write(0x1F);	break;}
				if(match1("B")){	write(0xCB); write(0x18);	break;}
				if(match1("C")){	write(0xCB); write(0x19);	break;}
				if(match1("D")){	write(0xCB); write(0x1A);	break;}
				if(match1("E")){	write(0xCB); write(0x1B);	break;}
				if(match1("H")){	write(0xCB); write(0x1C);	break;}
				if(match1("L")){	write(0xCB); write(0x1D);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x1E);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("RST"))// RST n
			{
				if(matchd1)
				{
					int i = strtol(instr[1], NULL, 16);
					switch(i)
					{
						case 0x00:	write(0xC7);	break;
						case 0x08:	write(0xCF);	break;
						case 0x10:	write(0xD7);	break;
						case 0x18:	write(0xDF);	break;
						case 0x20:	write(0xE7);	break;
						case 0x28:	write(0xEF);	break;
						case 0x30:	write(0xF7);	break;
						case 0x38:	write(0xFF);	break;
						default:	printf("%s:%u: error: valid restart address expected near \'%s\'\n", filename, *line_no, instr[1]);
									_err = ERR_SYNT;
									break;
					}
					break;
				}
				printf("%s:%u: error: valid restart address expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SBC"))// SBC n
			{
				if(match1("A")){	write(0x9F);				break;}
				if(match1("B")){	write(0x98);				break;}
				if(match1("C")){	write(0x99);				break;}
				if(match1("D")){	write(0x9A);				break;}
				if(match1("E")){	write(0x9B);				break;}
				if(match1("H")){	write(0x9C);				break;}
				if(match1("L")){	write(0x9D);				break;}
				if(match1("(HL)")){	write(0x9E);				break;}
				if(matchd1){		write(0xDE);	writeds1;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SLA"))// SLA n
			{
				if(match1("A")){	write(0xCB); write(0x27);	break;}
				if(match1("B")){	write(0xCB); write(0x20);	break;}
				if(match1("C")){	write(0xCB); write(0x21);	break;}
				if(match1("D")){	write(0xCB); write(0x22);	break;}
				if(match1("E")){	write(0xCB); write(0x23);	break;}
				if(match1("H")){	write(0xCB); write(0x24);	break;}
				if(match1("L")){	write(0xCB); write(0x25);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x26);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SRA"))// SRA n
			{
				if(match1("A")){	write(0xCB); write(0x2F);	break;}
				if(match1("B")){	write(0xCB); write(0x28);	break;}
				if(match1("C")){	write(0xCB); write(0x29);	break;}
				if(match1("D")){	write(0xCB); write(0x2A);	break;}
				if(match1("E")){	write(0xCB); write(0x2B);	break;}
				if(match1("H")){	write(0xCB); write(0x2C);	break;}
				if(match1("L")){	write(0xCB); write(0x2D);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x2E);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SRL"))// SRL n
			{
				if(match1("A")){	write(0xCB); write(0x3F);	break;}
				if(match1("B")){	write(0xCB); write(0x38);	break;}
				if(match1("C")){	write(0xCB); write(0x39);	break;}
				if(match1("D")){	write(0xCB); write(0x3A);	break;}
				if(match1("E")){	write(0xCB); write(0x3B);	break;}
				if(match1("H")){	write(0xCB); write(0x3C);	break;}
				if(match1("L")){	write(0xCB); write(0x3D);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x3E);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SUB"))// SUB n
			{
				if(match1("A")){	write(0x97);				break;}
				if(match1("B")){	write(0x90);				break;}
				if(match1("C")){	write(0x91);				break;}
				if(match1("D")){	write(0x92);				break;}
				if(match1("E")){	write(0x93);				break;}
				if(match1("H")){	write(0x94);				break;}
				if(match1("L")){	write(0x95);				break;}
				if(match1("(HL)")){	write(0x96);				break;}
				if(matchd1){		write(0xD6);	writeds1;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SWAP"))// SWAP n
			{
				if(match1("A")){	write(0xCB); write(0x37);	break;}
				if(match1("B")){	write(0xCB); write(0x30);	break;}
				if(match1("C")){	write(0xCB); write(0x31);	break;}
				if(match1("D")){	write(0xCB); write(0x32);	break;}
				if(match1("E")){	write(0xCB); write(0x33);	break;}
				if(match1("H")){	write(0xCB); write(0x34);	break;}
				if(match1("L")){	write(0xCB); write(0x35);	break;}
				if(match1("(HL)")){	write(0xCB); write(0x36);	break;}
				printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("XOR"))// XOR n
			{
				if(match1("A")){	write(0xAF);				break;}
				if(match1("B")){	write(0xA8);				break;}
				if(match1("C")){	write(0xA9);				break;}
				if(match1("D")){	write(0xAA);				break;}
				if(match1("E")){	write(0xAB);				break;}
				if(match1("H")){	write(0xAC);				break;}
				if(match1("L")){	write(0xAD);				break;}
				if(match1("(HL)")){	write(0xAE);				break;}
				if(matchd1){		write(0xEE); writeds1;		break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			printf("%s:%u: error: syntax error near \'%s\'\n", 
				   filename, *line_no, instr[0]);
			_err = ERR_SYNT;
			break;
		case 3:
			if(match0("ADC"))// ADC A,n
			{
				if(!match1("A"))
				{
					printf("%s:%u: error: A expected near \'%s\'\n", filename, *line_no, instr[1]);
					_err = ERR_SYNT;
					break;
				}
				if(match2("A")){	write(0x8F);				break;}
				if(match2("B")){	write(0x88);				break;}
				if(match2("C")){	write(0x89);				break;}
				if(match2("D")){	write(0x8A);				break;}
				if(match2("E")){	write(0x8B);				break;}
				if(match2("H")){	write(0x8C);				break;}
				if(match2("L")){	write(0x8D);				break;}
				if(match2("(HL)")){	write(0x8E);				break;}
				if(matchd2){		write(0xCE); writeds2;		break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[2]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("ADD"))// ADD A,n; ADD HL,n; ADD SP,n
			{
				if(match1("A"))
				{
					if(match2("A")){	write(0x87);				break;}
					if(match2("B")){	write(0x80);				break;}
					if(match2("C")){	write(0x81);				break;}
					if(match2("D")){	write(0x82);				break;}
					if(match2("E")){	write(0x83);				break;}
					if(match2("H")){	write(0x84);				break;}
					if(match2("L")){	write(0x85);				break;}
					if(match2("(HL)")){	write(0x86);				break;}
					if(matchd2){		write(0xC6); writeds2;		break;}	// Digit
					printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[2]);
					_err = ERR_SYNT;
					break;
				}
				if(match1("HL"))
				{
					if(match2("BC")){	write(0x09);				break;}
					if(match2("DE")){	write(0x19);				break;}
					if(match2("HL")){	write(0x29);				break;}
					if(match2("SP")){	write(0x39);				break;}
					printf("%s:%u: error: register-pair expected near \'%s\'\n", filename, *line_no, instr[2]);
					_err = ERR_SYNT;
					break;
				}
				if(match1("SP"))
				{
					if(matchd2){		write(0xE8); writeds2;	break;}
					printf("%s:%u: error: byte constant expected near \'%s\'\n", filename, *line_no, instr[2]);
					_err = ERR_SYNT;
					break;
				}
				printf("%s:%u: error: A, HL or SP expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("BIT"))// BIT b,r
			{
				if(matchd1)
				{
					unsigned int i = strtol(instr[1], NULL, 16);
					if(i > 7)
					{
						printf("%s:%u: error: value between 0 and 7 expected near \'%s\'\n", filename, *line_no, instr[1]);
						_err = ERR_SYNT;
						break;
					}
					if(match2("A")){	write(0xCB); write(0x47);	break;}
					if(match2("B")){	write(0xCB); write(0x40);	break;}
					if(match2("C")){	write(0xCB); write(0x41);	break;}
					if(match2("D")){	write(0xCB); write(0x42);	break;}
					if(match2("E")){	write(0xCB); write(0x43);	break;}
					if(match2("H")){	write(0xCB); write(0x44);	break;}
					if(match2("L")){	write(0xCB); write(0x45);	break;}
					if(match2("(HL)")){	write(0xCB); write(0x46);	break;}
					printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[2]);
					_err = ERR_SYNT;
					break;
				}
				printf("%s:%u: error: byte constant expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("CALL"))// CALL cc,nn
			{
				if(match1("NZ")){		write(0xC4);}
				else if(match1("Z")){	write(0xCC);}
				else if(match1("NC")){	write(0xD4);}
				else if(match1("C")){	write(0xDC);}
				else
				{
					printf("%s:%u: error: condition expected near \'%s\'\n", filename, *line_no, instr[1]);
					_err = ERR_SYNT;
					break;
				}
				
				if(matchd2){	writedl2;	break;}	// decimal long
				else{			writell2;	break;}	// label
			}
			if(match0("JP"))// JP cc,nn
			{
				if(match1("NZ")){		write(0xC2);}
				else if(match1("Z")){	write(0xCA);}
				else if(match1("NC")){	write(0xD2);}
				else if(match1("C")){	write(0xDA);}
				else
				{
					printf("%s:%u: error: condition expected near \'%s\'\n", filename, *line_no, instr[1]);
					_err = ERR_SYNT;
					break;
				}
				
				if(matchd2){	writedl2;	break;}	// decimal long
				else{			writell2;	break;}	// label
			}
			if(match0("JR"))// JR cc,n
			{
				{
					if(match1("NZ")){		write(0x20);}
					else if(match1("Z")){	write(0x28);}
					else if(match1("NC")){	write(0x30);}
					else if(match1("C")){	write(0x38);}
					else
					{
						printf("%s:%u: error: condition expected near \'%s\'\n", filename, *line_no, instr[1]);
						_err = ERR_SYNT;
						break;
					}
					
					if(matchd2){	writeds2;	break;}	// decimal short
					else {			writels2;	break;}	// short label
				}
			}
			if(match0("LD") && match1("(C)") && match2("A"))// LD (C),A
			{
				write(0xE2);
				break;
			}
			if(match0("LD") && match1("A") && match2("(C)"))// LD A,(C)
			{
				write(0xF2);
				break;
			}
			if(   (match0("LD") && match1("(HL+)") && match2("A")) // LD (HL+),A
			   || (match0("LD") && match1("(HLI)") && match2("A")) // LD (HLI),A
			   || (match0("LDI") && match1("(HL)") && match2("A")))// LDI (HL),A
			{
				write(0x22);
				break;
			}
			if(   (match0("LD") && match1("(HL-)") && match2("A")) // LD (HL-),A
			   || (match0("LD") && match1("(HLD)") && match2("A")) // LD (HLD),A
			   || (match0("LDD") && match1("(HL)") && match2("A")))// LDD (HL),A
			{
				write(0x32);
				break;
			}
			if(   (match0("LD") && match1("A") && match2("(HL+)")) // LD A,(HL+)
			   || (match0("LD") && match1("A") && match2("(HLI)")) // LD A,(HLI)
			   || (match0("LDI") && match1("A") && match2("(HL)")))// LDI A,(HL)
			{
				write(0x2A);
				break;
			}
			if(   (match0("LD") && match1("A") && match2("(HL-)")) // LD A,(HL-)
			   || (match0("LD") && match1("A") && match2("(HLD)")) // LD A,(HLD)
			   || (match0("LDD") && match1("A") && match2("(HL)")))// LDD A,(HL)
			{
				write(0x3A);
				break;
			}
			if(match0("LD") && matchp1 && match2("SP"))// LD (nn),SP
			{
				write(0x08);
				if(matchdx(instr[1]+1)){writedlx(instr[1]+1);}
				//else{					writellx(instr[1]+1);}
				printf("%s:%u: error: constant pointer expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
				
			}
			
			if(match0("LD"))// LD r1,r2
			{
				if(match1("A"))
				{ 
					if(match2("A")){	write(0x7F);	break;}
					if(match2("B")){	write(0x78);	break;}
					if(match2("C")){	write(0x79);	break;}
					if(match2("D")){	write(0x7A);	break;}
					if(match2("E")){	write(0x7B);	break;}
					if(match2("H")){	write(0x7C);	break;}
					if(match2("L")){	write(0x7D);	break;}
					if(match2("(BC)")){	write(0x0A);	break;}
					if(match2("(DE)")){	write(0x1A);	break;}
					if(match2("(HL)")){	write(0x7E);	break;}
					if(matchp2)
					{
						char *foo = strstr(instr[2], "+");
						if(foo != NULL)	// LD A,(0xFF00+n)
						{
							++foo;	// skip +
							write(0xF0);
							writedsx(foo);
							break;
						}
						else			// LD A,(nn)
						{
							write(0xFA);
							writedlx(instr[2] + 1);
							break;
						}
					}
					if(matchd2)
					{
						// LD A,n
						write(0x3E);
						writeds2;
						break;
					}
					write(0x3E);
					writels2;
					break;
				}
				if(match1("B"))
				{
					if(match2("A")){	write(0x47);	break;}
					if(match2("B")){	write(0x40);	break;}
					if(match2("C")){	write(0x41);	break;}
					if(match2("D")){	write(0x42);	break;}
					if(match2("E")){	write(0x43);	break;}
					if(match2("H")){	write(0x44);	break;}
					if(match2("L")){	write(0x45);	break;}
					if(match2("(HL)")){	write(0x46);	break;}
					if(matchd2){		write(0x06);	writeds2;	break;}
					write(0x06);
					writels2;
					break;
				}
				if(match1("C"))
				{
					if(match2("A")){	write(0x4F);	break;}
					if(match2("B")){	write(0x48);	break;}
					if(match2("C")){	write(0x49);	break;}
					if(match2("D")){	write(0x4A);	break;}
					if(match2("E")){	write(0x4B);	break;}
					if(match2("H")){	write(0x4C);	break;}
					if(match2("L")){	write(0x4D);	break;}
					if(match2("(HL)")){	write(0x4E);	break;}
					if(matchd2){		write(0x0E);	writeds2;	break;}
					write(0x0E);
					writels2;
					break;
				}
				if(match1("D"))
				{
					if(match2("A")){	write(0x57);	break;}
					if(match2("B")){	write(0x50);	break;}
					if(match2("C")){	write(0x51);	break;}
					if(match2("D")){	write(0x52);	break;}
					if(match2("E")){	write(0x53);	break;}
					if(match2("H")){	write(0x54);	break;}
					if(match2("L")){	write(0x55);	break;}
					if(match2("(HL)")){	write(0x56);	break;}
					if(matchd2){		write(0x16);	writeds2;	break;}
					write(0x16);
					writels2;
					break;
				}
				if(match1("E"))
				{
					if(match2("A")){	write(0x5F);	break;}
					if(match2("B")){	write(0x58);	break;}
					if(match2("C")){	write(0x59);	break;}
					if(match2("D")){	write(0x5A);	break;}
					if(match2("E")){	write(0x5B);	break;}
					if(match2("H")){	write(0x5C);	break;}
					if(match2("L")){	write(0x5D);	break;}
					if(match2("(HL)")){	write(0x5E);	break;}
					if(matchd2){		write(0x1E);	writeds2;	break;}
					write(0x1E);
					writels2;
					break;
				}
				if(match1("H"))
				{
					if(match2("A")){	write(0x67);	break;}
					if(match2("B")){	write(0x60);	break;}
					if(match2("C")){	write(0x61);	break;}
					if(match2("D")){	write(0x62);	break;}
					if(match2("E")){	write(0x63);	break;}
					if(match2("H")){	write(0x64);	break;}
					if(match2("L")){	write(0x65);	break;}
					if(match2("(HL)")){	write(0x66);	break;}
					if(matchd2){		write(0x26);	writeds2;	break;}
					write(0x26);
					writels2;
					break;
				}
				if(match1("L"))
				{
					if(match2("A")){	write(0x6F);	break;}
					if(match2("B")){	write(0x68);	break;}
					if(match2("C")){	write(0x69);	break;}
					if(match2("D")){	write(0x6A);	break;}
					if(match2("E")){	write(0x6B);	break;}
					if(match2("H")){	write(0x6C);	break;}
					if(match2("L")){	write(0x6D);	break;}
					if(match2("(HL)")){	write(0x6E);	break;}
					if(matchd2){		write(0x2E);	writeds2;	break;}
					write(0x2E);
					writels2;
					break;
				}
				if(match1("(HL)"))
				{
					if(match2("A")){	write(0x77);	break;}
					if(match2("B")){	write(0x70);	break;}
					if(match2("C")){	write(0x71);	break;}
					if(match2("D")){	write(0x72);	break;}
					if(match2("E")){	write(0x73);	break;}
					if(match2("H")){	write(0x74);	break;}
					if(match2("L")){	write(0x75);	break;}
					if(matchd2){		write(0x36);	writeds2;	break;}
					write(0x36);
					writels2;
					break;
				}
				if(match1("(BC)") && match2("A")){write(0x02);	break;}
				if(match1("(DE)") && match2("A")){write(0x12);	break;}
				if(matchp1 && match2("A"))
				{
					char *foo = strstr(instr[1], "+");
					if(foo != NULL)	// LD (0xFF00+n),A
					{
						++foo;	// skip +
						write(0xE0);
						writedsx(foo);
						break;
					}
					else			// LD (nn),A
					{
						write(0xEA);
						if(matchdx(instr[1] + 1)){	writedlx(instr[1] + 1);}
					//	else{						writelx(instr[1] + 1);}
						
						break;
					}
				}
				
				// LD n,nn
				if(match1("BC")){if(matchd2){	write(0x01);writedl2;	break;}
								else{			write(0x01);writell2;	break;}}
				if(match1("DE")){if(matchd2){	write(0x11);writedl2;	break;}
								else{			write(0x11);writell2;	break;}}
				if(match1("HL")){if(matchd2){	write(0x21);writedl2;	break;}
								else{			write(0x21);writell2;	break;}}
				if(match1("SP")){if(matchd2){	write(0x31);writedl2;	break;}
								else{			write(0x31);writell2;	break;}}
				printf("%s:%u: error: syntax error near \'%s\'\n", filename, *line_no, instr[0]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("LDH"))// LDH (n),A; LDH A,(n)
			{
				if(matchp1 && match2("A"))
				{
					write(0xE0);
					writedsx(instr[1]+1);
					break;
				}
				if(match1("A") && matchp2)
				{
					write(0xF0);
					writedsx(instr[2]+1);
					break;
				}
				printf("%s:%u: error: A or constant pointer expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("LD") && match1("SP") && match2("HL"))// LD SP,HL
			{
				write(0xF9);
				break;
			}
			if(match0("RES"))// RES b,r
			{
				if(matchd1)
				{
					unsigned int i = strtol(instr[1], NULL, 16);
					if(i > 7)
					{
						printf("%s:%u: error: value between 0 and 7 expected near \'%s\'\n", filename, *line_no, instr[1]);
						_err = ERR_SYNT;
						break;
					}
					if(match2("A")){	write(0xCB); write(0x87);	break;}
					if(match2("B")){	write(0xCB); write(0x80);	break;}
					if(match2("C")){	write(0xCB); write(0x81);	break;}
					if(match2("D")){	write(0xCB); write(0x82);	break;}
					if(match2("E")){	write(0xCB); write(0x83);	break;}
					if(match2("H")){	write(0xCB); write(0x84);	break;}
					if(match2("L")){	write(0xCB); write(0x85);	break;}
					if(match2("(HL)")){	write(0xCB); write(0x86);	break;}
					printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[2]);
					_err = ERR_SYNT;
					break;
				}
				printf("%s:%u: error: byte constant expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SBC") && match1("A"))// SBC A,n
			{
				if(match2("A")){	write(0x9F);				break;}
				if(match2("B")){	write(0x98);				break;}
				if(match2("C")){	write(0x99);				break;}
				if(match2("D")){	write(0x9A);				break;}
				if(match2("E")){	write(0x9B);				break;}
				if(match2("H")){	write(0x9C);				break;}
				if(match2("L")){	write(0x9D);				break;}
				if(match2("(HL)")){	write(0x9E);				break;}
				if(matchd2){		write(0xDE);	writeds2;	break;}	// Digit
				printf("%s:%u: error: register, (HL) or constant byte expected near \'%s\'\n", filename, *line_no, instr[2]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("SET"))// SET b,r
			{
				if(matchd1)
				{
					unsigned int i = strtol(instr[1], NULL, 16);
					if(i > 7)
					{
						printf("%s:%u: error: value between 0 and 7 expected near \'%s\'\n", filename, *line_no, instr[1]);
						_err = ERR_SYNT;
						break;
					}
					if(match2("A")){	write(0xCB); write(0xC7);	break;}
					if(match2("B")){	write(0xCB); write(0xC0);	break;}
					if(match2("C")){	write(0xCB); write(0xC1);	break;}
					if(match2("D")){	write(0xCB); write(0xC2);	break;}
					if(match2("E")){	write(0xCB); write(0xC3);	break;}
					if(match2("H")){	write(0xCB); write(0xC4);	break;}
					if(match2("L")){	write(0xCB); write(0xC5);	break;}
					if(match2("(HL)")){	write(0xCB); write(0xC6);	break;}
					printf("%s:%u: error: register or (HL) expected near \'%s\'\n", filename, *line_no, instr[2]);
					_err = ERR_SYNT;
					break;
				}
				printf("%s:%u: error: byte constant expected near \'%s\'\n", filename, *line_no, instr[1]);
				_err = ERR_SYNT;
				break;
			}
			if(match0("LD") && match1("HL") 
			   && (strstr(instr[2], "SP+") == instr[2]))// LD HL,SP+n
			{
				char *foo = instr[2];
				foo += 3;	// skip SP+
				write(0xF8);
				writedsx(foo);
				break;	
			}
			if(match0("LDHL") && match1("SP"))// LDHL SP,n
			{
				write(0xF8);
				writeds2;
				break;
			}
			printf("%s:%u: error: syntax error near \'%s\'\n", 
				   filename, *line_no, instr[0]);
			_err = ERR_SYNT;
			break;
		case 4:
			if((match0("LD") && match1("HL") && match2("SP+"))// LD HL, SP+ n
			|| (match0("LD") && match1("HL") && match2("SP")))// LD HL, SP +n
			{
				char *foo = instr[3];
				if(*foo == '+')
					++foo;
				write(0xF8);
				writedsx(foo);
				break;	
			}
			printf("%s:%u: error: syntax error near \'%s\'\n", 
				   filename, *line_no, instr[0]);
			_err = ERR_SYNT;
			break;
		case 5:
			if((match0("LD") && match1("HL") && match2("SP")) && match3("+"))
			{	// LD HL, SP + n
				char *foo = instr[4];
				if(*foo == '+')
					++foo;
				write(0xF8);
				writedsx(foo);
				break;	
			}
			printf("%s:%u: error: syntax error near \'%s\'\n", 
				   filename, *line_no, instr[0]);
			_err = ERR_SYNT;
			break;
			
		default: 
			printf("%s:%u: error: syntax error near \'%s\'\n", 
				   filename, *line_no, instr[0]);
			_err = ERR_SYNT;
			break;
	}
}