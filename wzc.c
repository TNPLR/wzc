#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define REV_SEEK(x) fseek(x, -1, SEEK_CUR)

FILE *source;
FILE *dest;

/* token */
enum {
		NUMBER = 0,
		MUL,
		DIV,
		ADD,
		MINUS,
/*  5  */	L_PAR,
		R_PAR,
		END,
		ID,
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
			token = NUMBER;
			return;
		}
		if (isalpha(c) || c == '_') {
			token = ID;
			REV_SEEK(source);
			return;
		}
		token_len = 1;
		switch (c) {
		case '+':
			token = ADD;
			return;
		case '-':
			token = MINUS;
			return;
		case '*':
			token = MUL;
			return;
		case '/':
			token = DIV;
			return;
		case '(':
			token = L_PAR;
			return;
		case ')':
			token = R_PAR;
			return;
		case -1:
			token = END;
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
	case L_PAR:
		next();
		expr();
		next();
		break;
	case NUMBER:
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
	case ADD:
		next();
		unary();
		break;
	case MINUS:
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
	case MUL:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		unary();
		fprintf(dest, "\tmovq %%rax, %%rbx\n"
			"\tpopq %%rax\n"
			"\timulq %%rbx\n");
		mul_rest();
		break;
	case DIV:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		unary();
		fprintf(dest, "\tmovq %%rax, %%rbx\n"
			"\tpopq %%rax\n"
			"\tcqo\n"
			"\tidivq %%rbx\n");
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
	case ADD:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		mul();
		fprintf(dest, "\tpopq %%rbx\n"
				"\taddq %%rbx, %%rax\n");
		add_rest();
		break;
	case MINUS:
		next();
		fprintf(dest, "\tpushq %%rax\n");
		mul();
		fprintf(dest, "\tpopq %%rbx\n"
				"\tsubq %%rbx, %%rax\n");
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
