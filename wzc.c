#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define REV_SEEK(x) fseek(x, -1, SEEK_CUR)

FILE *source;
FILE *dest;

/* token */
enum {
		TK_NUMBER = 0,
		TK_MUL,
		TK_DIV,
		TK_ADD,
		TK_MINUS,
/*  5  */	TK_L_PAR,
		TK_R_PAR,
		TK_EOF,
		TK_ID,
};
int token;
int token_len;
union {
	unsigned long int integer;
} token_data;

int getnum(void)
{
	return fscanf(source, "%lu", &token_data.integer);
}

void next(void)
{
	while (1) {
		int c;
		c = fgetc(source);
		if (isdigit(c)) {
			REV_SEEK(source);
			token_len = getnum();
			token = TK_NUMBER;
			return;
		}
		if (isalpha(c) || c == '_') {
			token = TK_ID;
			REV_SEEK(source);
			return;
		}
		token_len = 1;
		switch (c) {
		case '+':
			token = TK_ADD;
			return;
		case '-':
			token = TK_MINUS;
			return;
		case '*':
			token = TK_MUL;
			return;
		case '/':
			token = TK_DIV;
			return;
		case '(':
			token = TK_L_PAR;
			return;
		case ')':
			token = TK_R_PAR;
			return;
		case -1:
			token = TK_EOF;
			return;
		}
	}
}

void expr(void);
void num(void)
{
	fprintf(dest, "\tmovq $%lu, %%rax\n" , token_data.integer);
	next();
}

void prim(void)
{
	switch (token) {
	case TK_L_PAR:
		next();
		expr();
		next();
		break;
	case TK_NUMBER:
		num();
		break;
	default:
		puts("Primary expression error");
		exit(1);
	}
}

void unary(void)
{
	switch (token) {
	case TK_ADD:
		next();
		unary();
		break;
	case TK_MINUS:
		next();
		unary();
		fprintf(dest, "\tnegq %%rax\n");
		break;
	default:
		prim();
		break;
	}
}

void mul_rest(void)
{
	switch (token) {
	case TK_MUL:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		unary();
		fprintf(dest, "\tpopq %%rdi\n"
			"\timulq %%rdi, %%rax\n");
		mul_rest();
		break;
	case TK_DIV:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		unary();
		fprintf(dest, "\tmovq %%rax, %%rdi\n"
			"\tpopq %%rax\n"
			"\tcqo\n"
			"\tidivq %%rdi\n");
		mul_rest();
		break;
	default:
		break;
	}
}
void mul(void)
{
	unary();
	mul_rest();
}
void add_rest(void)
{
	switch (token) {
	case TK_ADD:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		mul();
		fprintf(dest, "\tpopq %%rdi\n"
				"\taddq %%rdi, %%rax\n");
		add_rest();
		break;
	case TK_MINUS:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		mul();
		fprintf(dest, "\tmovq %%rax, %%rdi\n"
				"\tpopq %%rax\n"
				"\tsubq %%rdi, %%rax\n");
		add_rest();
		break;
	default:
		break;
	}
}
void add(void)
{
	mul();
	add_rest();
}
void expr(void)
{
	add();
}
void expr_stat(void)
{
	expr();
}
void ret(void)
{
	expr_stat();
	fprintf(dest, "\tpopq %%rbp\n"
		"\tret\n");

}
void cc(void)
{
	fprintf(dest, ".text\n"
		".globl main\n"
		"main:\n"
		"\tpushq %%rbp\n"
		"\tmovq %%rsp, %%rbp\n");
	next();
	ret();
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		puts("ERROR: No input file.");
		return 1;
	}
	source = fopen(argv[1], "r");
	if (source == (void *)0) {
		perror("ERROR: ");
		return 1;
	}

	if (argc == 3) {
		dest = fopen(argv[2], "w");
	} else {
		dest = fopen("out.s", "w");
	}
	if (dest == (void *)0) {
		perror("ERROR: ");
		return 1;
	}

	cc();
	fclose(source);
	return 0;
}
