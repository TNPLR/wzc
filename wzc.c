#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
/*  5 */	TK_L_PAR,
		TK_R_PAR,
		TK_EOF,
		TK_ID,
		TK_SEMI,
/* 10 */	TK_AUTO,
		TK_RETURN,
		TK_ASSIGN
};
int token;
int token_len;
union {
	struct local_var *local_id;
	unsigned long int integer;
} token_data;

struct local_var {
	char *name;
	unsigned long int name_len;
	unsigned long int number;
};
Vector g_local_var;

int getnum(void)
{
	return fscanf(source, "%lu", &token_data.integer);
}

int check_keyword(char *str, unsigned long int len)
{
	switch (len) {
	case 4:
		if (!memcmp("auto", str, 4)) {
			token = TK_AUTO;
			return 1;
		}
		break;
	case 6:
		if (!memcmp("return", str, 6)) {
			token = TK_RETURN;
			return 1;
		}
		break;
	default:
		break;
	}
	return 0;
}

void check_local(char *str, unsigned long int len)
{
	Iterator it;
	struct local_var *res;
	token = TK_ID;
	for (it = g_local_var->rbegin(g_local_var); it->freelt(it, g_local_var->rend(g_local_var)); it->next(it)) {
		if (((struct local_var *)it->get(it))->name_len != len) {
			continue;
		}
		if (memcmp(((struct local_var *)it->get(it))->name, str, len)) {
			continue;
		}
		token_data.local_id = (struct local_var *)it->get(it);
		return;
	}
	it->free(it);
	res = malloc(sizeof(struct local_var));
	res->name = malloc(len + 1);
	memcpy(res->name, str, len);
	res->name[len] = 0;
	res->name_len = len;
	res->number = g_local_var->size(g_local_var);
	g_local_var->push_back(g_local_var, res);
	token_data.local_id = res;
}

void get_alpha(void)
{
	unsigned long int i;
	int c;
	char *str = malloc(4096);
	for (i = 0; (c = fgetc(source)); ++i) {
		if (isalpha(c) || c == '_') {
			str[i] = c;
		} else {
			break;
		}
	}
	str[i] = 0;
	REV_SEEK(source);
	if (check_keyword(str, i)) {
		return;
	}
	check_local(str, i);
	free(str);
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
			REV_SEEK(source);
			get_alpha();
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
		case '=':
			token = TK_ASSIGN;
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
		printf("Expected token %d, but found token %d\n", c, token);
		exit(1);
	}
	next();
}

/* EBNF of the WZC Language (simplified C)
 * statement_list:
 * 	statement
 *	statement_list statement
 * statement:
 *	expression_statement
 *	return_statement
 * expression_statement:
 *	expression? ';'
 * return_statement
 *	'return' expression? ';'
 * expression:
 *	assignment
 * assignment:
 *	addicitive
 *	addicitive '=' assignment
 * addictive:
 *	multiplication
 *	addictive '+' multiplication
 *	addictive '-' multiplication
 * multiplication:
 *	unary
 *	multiplication '*' unary
 *	multiplication '/' unary
 * unary
 *	primary
 *	'+' unary
 *	'-' unary
 * primary
 *	identifier
 *	number
 *	'(' expression ')'
 */

ANode *num(void)
{
	ANode *node;
	node = new_anode(AS_NUM, 0);
	node->data.u64 = token_data.integer;
	next();
	return node;
}

ANode *ident(void)
{
	ANode *node;
	node = new_anode(AS_ID, 0);
	node->data.u64 = token_data.local_id->number;
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
		node = expr();
		next();
		break;
	case TK_NUMBER:
		node = num();
		break;
	case TK_ID:
		node = ident();
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
		node = prim();
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
		node = lop;
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
		node = lop;
		break;
	}
	return node;
}
ANode *add(void)
{
	return add_rest(mul());
}

ANode *assign(void)
{
	ANode *node;
	node = add();
	switch (token) {
	case TK_ASSIGN:
		next();
		node = new_anode(AS_ASSIGN, 2, node, assign());
		break;
	default:
		break;
	}
	return node;
}

ANode *expr(void)
{
	return assign();
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
	match_and_pop(TK_RETURN);
	node = new_anode(AS_STAT_RETURN, 1, expr());
	match_and_pop(TK_SEMI);
	return node;

}

ANode *state(void)
{
	ANode *node;
	switch (token) {
	case TK_RETURN:
		node = new_anode(AS_STAT, 1, ret());
		break;
	case TK_SEMI:
	default:
		node = new_anode(AS_STAT, 1, expr_stat());
		break;
	}
	return node;
}

ANode *state_list_rest(ANode *lop)
{
	ANode *node;
	switch (token) {
	case TK_EOF:
		node = new_anode(AS_STAT_LIST1, 1, lop);
		break;
	default:
		node = new_anode(AS_STAT_LIST2, 2, lop, state());
		node = state_list_rest(node);
		break;
	}
	return node;
}

ANode *state_list(void)
{
	return state_list_rest(state());
}

struct lvar {
	unsigned long int number;
	unsigned long int offset;
};

Vector variable;
unsigned long int get_offset(ANode *node)
{
	Iterator it;
	struct lvar *res;
	for (it = variable->rbegin(variable); it->freelt(it, variable->rend(variable)); it->next(it)) {
		res = (struct lvar *)it->get(it);
		if (node->data.u64 == res->number) {
			it->free(it);
			return res->offset;
		}
	}
	puts("Variable not declared");
	exit(1);
}

unsigned long int gen_ref(ANode *node)
{
	unsigned long int offset;
	if (node == (void *)0) {
		return 0;
	}
	switch (node->type) {
	case AS_ID:
		offset = get_offset(node);
		fprintf(dest, "\tleaq -%lu(%%rbp), %%rax\n", offset);
		break;
	default:
		puts("Left value not found");
		exit(1);
	}
	return offset;
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
	case AS_STAT_EXPR:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_STAT_RETURN:
		gen(*node->Node->data(node->Node, 0));
		fprintf(dest, "\tleave\n"
			"\tret\n");
		break;
	case AS_STAT:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_STAT_LIST1:
		gen(*node->Node->data(node->Node, 0));
		break;
	case AS_STAT_LIST2:
		gen(*node->Node->data(node->Node, 0));
		gen(*node->Node->data(node->Node, 1));
		break;
	case AS_ID:
		fprintf(dest, "\tmovq -%lu(%%rbp), %%rax\n", node->data.u64 * 8);
		break;
	case AS_ASSIGN:
		gen(*node->Node->data(node->Node, 1));
		fprintf(dest, "\tpushq %%rax\n");
		gen_ref(*node->Node->data(node->Node, 0));
		fprintf(dest, "\tmovq (%%rsp), (%%rax)\n"
				"\tleaq 8(%%rsp), %%rsp\n");
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
	g_local_var = coonew(Vector);
	variable = coonew(Vector);
	next();
	res = state_list();
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
