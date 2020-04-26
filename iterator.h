#ifndef ITERATOR_H_
#define ITERATOR_H_
typedef struct iterator {
	void *data;
	void *(*get)(struct iterator *const);
	void (*set)(struct iterator *const, void *const data);
	void (*next)(struct iterator *const);
	int (*lt)(struct iterator *const self, struct iterator *const that);
	int (*freelt)(struct iterator *const self, struct iterator *const that);
	void (*free)(struct iterator *const);
} *Iterator;
#endif /* ITERATOR_H_ */
