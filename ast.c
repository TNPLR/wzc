#include "ast.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#define Assert(x) do { if (!(x)) { perror(#x); exit(1); } } while (0);

ANode *new_anode(enum ast_type t, unsigned long int c, ...)
{
	va_list args;
	ANode *res;
	unsigned long int i;
	va_start(args, c);
	res = (ANode *)calloc(1, sizeof(ANode));
	Assert(res != (void *)0);

	res->type = t;
	res->Node = coonew(Vector);

	for (i = 0; i < c; ++i) {
		res->Node->push_back(res->Node, va_arg(args, ANode *));
	}
	return res;
}

void as_free_subtree(ANode *node)
{
	Iterator it;
	if (node == (void *)0) {
		return;
	}
	for (it = node->Node->begin(node->Node); it->freelt(it, node->Node->end(node->Node)); it->next(it)) {
		as_free_subtree(it->get(it));
	}
	it->free(it);
	node->Node->free(node->Node);
	free(node);
}
