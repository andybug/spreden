#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

struct list_node {
	void *data;
	struct list_node *next;
	struct list_node *prev;
};

struct list {
	unsigned int length;
	struct list_node *head;
	struct list_node *tail;
};

struct list_iter {
	struct list *list;
	struct list_node *current;
};

extern void list_init(struct list *l);
extern void list_add_front(struct list *l, void *data);
extern void list_add_back(struct list *l, void *data);
extern void list_clear(struct list *l);
extern void list_iter_begin(struct list *l, struct list_iter *iter);
extern bool list_iter_end(struct list_iter *iter);
extern void *list_iter_data(struct list_iter *iter);
extern void list_iter_next(struct list_iter *iter);

#endif
