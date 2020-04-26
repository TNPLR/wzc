#ifndef VECTOR_H_
#define VECTOR_H_
#include "coo.h"
#include "iterator.h"
struct vector {
	void **arr;

	unsigned long int p_capacity;
	unsigned long int p_size;
	unsigned long int (*capacity)(struct vector *const);
	unsigned long int (*size)(struct vector *const);

	void **(*data)(struct vector *const, unsigned long int index);
	void **(*front)(struct vector *const);
	void **(*back)(struct vector *const);

	void (*reserve)(struct vector *const, unsigned long int size);
	void (*push_back)(struct vector *const, void *const data);
	void (*pop_back)(struct vector *const);

	Iterator (*begin)(struct vector *const);
	Iterator (*end)(struct vector *const);
	Iterator (*rbegin)(struct vector *const);
	Iterator (*rend)(struct vector *const);

	void (*free)(struct vector *const);
};
typedef struct vector * Vector;
Vector init_Vector(void);
#endif /* VECTOR_H_ */
