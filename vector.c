#include "vector.h"
#include "iterator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Assert(x) do { if (!(x)) { perror(#x); exit(1); } } while (0)


static unsigned long int
capacity(Vector const self)
{
	return self->p_capacity;
}

static unsigned long int
size(Vector const self)
{
	return self->p_size;
}

static void **
data(Vector const self, unsigned long int index)
{
	return self->arr + index;
}

static void **
front(Vector const self)
{
	return self->arr;
}

static void **
back(Vector const self)
{
	return self->arr + self->p_size - 1;
}

static void
reserve(Vector const self, unsigned long int size)
{
	if (size > self->p_capacity) {
		Assert((self->arr = (void **)realloc(self->arr, size * sizeof(void *))) != (void *)0);
		self->p_capacity = size;
	}
}

static void
push_back(Vector const self, void *const data)
{
	if (self->p_size == self->p_capacity) {
		reserve(self, (self->p_capacity + 1) * 2);
	}
	self->arr[self->p_size++] = data;
}

static void
pop_back(Vector const self)
{
	if (self->p_size > 0) {
		--self->p_size;
	}
}

struct it_data {
	void **arr;
	unsigned long int index;
};

static void *
get(Iterator const it)
{
	return ((struct it_data *)it->data)->arr[((struct it_data *)it->data)->index];
}

void
set(Iterator const it, void *const data)
{
	((struct it_data *)it->data)->arr[((struct it_data *)it->data)->index] = data;
}

static void *
rget(Iterator const it)
{
	return *(((struct it_data *)it->data)->arr - ((struct it_data *)it->data)->index);
}

static void
rset(Iterator const it, void *const data)
{
	*(((struct it_data *)it->data)->arr - ((struct it_data *)it->data)->index) = data;
}

static void
next(Iterator const it)
{
	++((struct it_data *)it->data)->index;
}

static int
lt(Iterator const it, Iterator const that)
{
	return ((struct it_data *)it->data)->index < ((struct it_data *)that->data)->index;
}

static int
freelt(Iterator const it, Iterator const that)
{
	int ret = ((struct it_data *)it->data)->index < ((struct it_data *)that->data)->index;
	that->free(that);
	return ret;
}

static void
it_free(Iterator const it)
{
	free(it->data);
	free(it);
}

static Iterator begin(Vector const self)
{
	Iterator res = (Iterator)malloc(sizeof(struct iterator));
	Assert(res != (void *)0);

	res->data = (void *)malloc(sizeof(struct it_data));
	Assert(res->data != (void *)0);

	((struct it_data *)res->data)->arr = self->arr;
	((struct it_data *)res->data)->index = 0;

	res->get = get;
	res->set = set;
	res->next = next;
	res->lt = lt;
	res->freelt = freelt;
	res->free = it_free;
	return res;
}

static Iterator it_end(Vector const self)
{
	Iterator res = (Iterator)malloc(sizeof(struct iterator));
	Assert(res != (void *)0);

	res->data = (void *)malloc(sizeof(struct it_data));
	Assert(res->data != (void *)0);

	((struct it_data *)res->data)->arr = self->arr;
	((struct it_data *)res->data)->index = self->p_size;

	res->get = get;
	res->set = set;
	res->next = next;
	res->lt = lt;
	res->freelt = freelt;
	res->free = it_free;
	return res;
}

static Iterator rbegin(Vector const self)
{
	Iterator res = (Iterator)malloc(sizeof(struct iterator));
	Assert(res != (void *)0);

	res->data = (void *)malloc(sizeof(struct it_data));
	Assert(res->data != (void *)0);

	((struct it_data *)res->data)->arr = self->arr + self->p_size - 1;
	((struct it_data *)res->data)->index = 0;

	res->get = rget;
	res->set = rset;
	res->next = next;
	res->lt = lt;
	res->free = it_free;
	return res;
}

static Iterator rend(Vector const self)
{
	Iterator res = (Iterator)malloc(sizeof(struct iterator));
	Assert(res != (void *)0);

	res->data = (void *)malloc(sizeof(struct it_data));
	Assert(res->data != (void *)0);

	((struct it_data *)res->data)->arr = self->arr + self->p_size - 1;
	((struct it_data *)res->data)->index = self->p_size;

	res->get = rget;
	res->set = rset;
	res->next = next;
	res->lt = lt;
	res->free = it_free;
	return res;
}

static void vector_free(Vector const self)
{
	free(self->arr);
	free(self);
}

Vector
init_Vector(void)
{
	Vector res = (Vector)malloc(sizeof(struct vector));
	Assert(res != (void *)0);

	res->arr = (void **)malloc(sizeof(void *));
	Assert(res->arr != (void *)0);

	res->p_capacity = 1;
	res->capacity = capacity;

	res->p_size = 0;
	res->size = size;

	res->data = data;
	res->front = front;
	res->back = back;

	res->reserve = reserve;
	res->push_back = push_back;
	res->pop_back = pop_back;

	res->begin = begin;
	res->end = it_end;
	res->rbegin = rbegin;
	res->rend = rend;

	res->free = vector_free;
	return res;
}
