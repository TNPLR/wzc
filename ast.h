#ifndef AST_H_
#define AST_H_
#include "vector.h"
#include <stdarg.h>

enum ast_type {
	AS_NUM = 0,
	AS_ADD,
	AS_SUB,
	AS_MUL,
	AS_DIV,
	AS_NEG,
	AS_POS,
	AS_STAT_EXPR,
	AS_STAT_RETURN,
	AS_STAT,
	AS_STAT_LIST1,
	AS_STAT_LIST2,
	AS_ID,
	AS_ASSIGN,
	AS_DECLARATION,
	AS_DECLARATION_LIST1,
	AS_DECLARATION_LIST2,
	AS_STAT_COMPOUND,
	AS_FUNCTION_DEFI
};

typedef struct ast_node ANode;

struct ast_node {
	enum ast_type type;
	Vector Node;
	union {
		unsigned long int u64;
		void *other;
	} data;
};
ANode *new_anode(enum ast_type, unsigned long int son_count, ...);
void as_free_subtree(ANode *);
#endif /* AST_H_ */
