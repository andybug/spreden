#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "spreden.h"

void list_init(struct list *l)
{
	assert(l != NULL);

	l->length = 0;
	l->head = NULL;
	l->tail = NULL;
}

void list_add(struct list *l, void *data)
{
	struct list_node *node;

	assert(l != NULL);

	node = malloc(sizeof(struct list_node));
	if (!node) {
		perror("list_add");
		exit(EXIT_FAILURE);
	}

	node->data = data;
	node->next = NULL;
	node->prev = l->tail;

	if (!l->head)
		l->head = node;

	if (l->tail)
		l->tail->next = node;

	l->tail = node;
	l->length++;
}

void list_clear(struct list *l)
{
	struct list_node *node, *next;
	unsigned int removed = 0;

	assert(l != NULL);

	node = l->head;
	while (node) {
		next = node->next;
		free(node);
		node = next;
		removed++;
	}

	assert(removed == l->length);
	l->length = 0;
	l->head = NULL;
	l->tail = NULL;
}

void list_iter_begin(struct list *l, struct list_iter *iter)
{
	assert(l != NULL);
	assert(iter != NULL);

	iter->list = l;
	iter->current = l->head;
}

bool list_iter_end(struct list_iter *iter)
{
	assert(iter != NULL);

	return (iter->current == NULL);
}

void *list_iter_data(struct list_iter *iter)
{
	assert(iter != NULL);

	if (iter->current)
		return iter->current->data;

	return NULL;
}

void list_iter_next(struct list_iter *iter)
{
	assert(iter != NULL);

	if (iter->current)
		iter->current = iter->current->next;
}
