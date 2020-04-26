#include "ast.h"

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
		TK_SEMI
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
		case ';':
			token = TK_SEMI;
			return;
		case -1:
			token = TK_EOF;
			return;
		}
	}
}

void match_and_pop(int c)
{
	if (token != c) {
		printf("Expected %c, but found %c\n", c, token);
		exit(1);
	}
	next();
}

ANode *num(void)
{
	ANode *node;
	node = new_anode(AS_NUM, 0);
	node->data.u64 = token_data.integer;
	next();
	return node;
}

ANode *expr(void);
ANode *prim(void)
{
	ANode *node;
	switch (token) {
	case TK_L_PAR:
		next();
		node = new_anode(AS_NOP_PRIM, 1, expr());
		next();
		break;
	case TK_NUMBER:
		node = new_anode(AS_NOP_PRIM, 1, num());
		break;
	default:
		puts("Primary expression error");
		exit(1);
	}
	return node;
}

ANode *unary(void)
{
	ANode *node;
	switch (token) {
	case TK_ADD:
		next();
		node = new_anode(AS_POS, 1, unary());
		break;
	case TK_MINUS:
		next();
		node = new_anode(AS_NEG, 1, unary());
		break;
	default:
		node = new_anode(AS_NOP_UNARY, 1, prim());
		break;
	}
	return node;
}

ANode *mul_rest(ANode *lop)
{
	ANode *node;
	switch (token) {
	case TK_MUL:
		next();
		node = new_anode(AS_MUL, 2, lop, unary());
		node = mul_rest(node);
		break;
	case TK_DIV:
		next();
		node = new_anode(AS_DIV, 2, lop, unary());
		node = mul_rest(node);
		break;
	default:
		node = new_anode(AS_NOP_MUL, 1, lop);
		break;
	}
	return node;
}
ANode *mul(void)
{
	return mul_rest(unary());
}
ANode *add_rest(ANode *lop)
{
	ANode *node;
	switch (token) {
	case TK_ADD:
		next();
		node = new_anode(AS_ADD, 2, lop, mul());
		node = add_rest(node);
		break;
	case TK_MINUS:
		next();
		node = new_anode(AS_SUB, 2, lop, mul());
		node = add_rest(node);
		break;
	default:
		node = new_anode(AS_NOP_ADD, 1, lop);
		break;
	}
	return node;
}
ANode *add(void)
{
	return add_rest(mul());
}
ANode *expr(void)
{
	ANode *node;
	node = new_anode(AS_NOP_EXPR, 1, add());
	return node;
}
ANode *expr_stat(void)
{
	ANode *node;
	node = new_anode(AS_STAT_EXPR, 1, expr());
	match_and_pop(TK_SEMI);
	return node;
}
ANode *ret(void)
{
	ANode *node;
	node = new_anode(AS_STAT_RETURN, 1, expr_stat());
	return node;

}

void gen(ANode *node)
{
	if (node == (void *)0) {
		return;
	}
	switch (node->type) {
	case AS_NUM:
		fprintf(dest, "\tmovq $%lu, %%rax\n" , node->data.u64);
		break;
	case AS_ADD:
		gen(*node->Node->data(node->Node, 1));
		fprintf(dest, "\tpushq %%rax\n");
		gen(*node->Node->data(node->Node, 0));
		fprintf(dest, "\taddq (%%rsp), %%rax\n");
		fprintf(dest, "\tleaq 8(%%rsp), %%rsp\n");
		break;
	case AS_SUB:
		gen(*node->Node->data(node->Node, 1));
		fprintf(dest, "\tpushq %%rax\n");
		gen(*node->Node->data(node->Node, 0));
		fprintf(dest, "\tsubq (%%rsp), %%rax\n");
		fprintf(dest, "\tleaq 8(%%rsp), %%rsp\n");
		break;
	case AS_MUL:
		gen(*node->Node->data(node->Node, 1));
		fprintf(dest, "\tpushq %%rax\n");
		gen(*node->Node->data(node->Node, 0));
		fprintf(dest, "\timulq (%%rsp), %%rax\n");
		fprintf(dest, "\tleaq 8(%%rsp), %%rsp\n");
		break;
	case AS_DIV:
		gen(*node->Node->data(node->Node, 1));
		fprintf(dest, "\tpushq %%rax\n");
		gen(*node->Node->data(node->Node, 0));
		fprintf(dest, "\tcqo\n");
		fprintf(dest, "\tidivq (%%rsp)\n");
		fprintf(dest, "\tleaq +8(%%rsp), %%rsp\n");
		break;
	case AS_NEG:
		gen(*node->Node->data(node->Node, 0));
		fprintf(dest, "\tnegq %%rax\n");
		break;
	case AS_POS:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_NOP_PRIM:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_NOP_UNARY:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_NOP_MUL:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_NOP_ADD:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_NOP_EXPR:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_STAT_EXPR:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_STAT_RETURN:
		gen(*node->Node->data(node->Node, 0));
		fprintf(dest, "\tpopq %%rbp\n"
			"\tret\n");
		break;
	default:
		printf("Unknown Ast Node\n");
		exit(1);
	}
	return;
}
void cc(void)
{
	ANode *res;
	next();
	res = ret();
	fprintf(dest, ".text\n"
		".globl main\n"
		"main:\n"
		"\tpushq %%rbp\n"
		"\tmovq %%rsp, %%rbp\n");
	gen(res);
	as_free_subtree(res);
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
