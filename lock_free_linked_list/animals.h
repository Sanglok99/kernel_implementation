#ifndef ANIMALS_H
#define ANIMALS_H

#include <linux/list.h>
#include <linux/atomic.h>

#define NUM_THREADS 4

struct animal {
	int total; /* number of entries in the list */
	struct list_head entry; /* head of the list */
	atomic_t removed; /* boolean for marking as deleted */
	struct list_head gc_entry; /* head of the garbage list */
};

struct cat {
	int var; /* id of this cat */
	struct list_head entry; /* list entry to be inserted */
	atomic_t removed; /* boolean for marking as deleted */
	struct list_head gc_entry; /* garbage list entry */
};

// Initialization(slide 11)
static inline void INIT_LF_LIST_HEAD(struct list_head *list)
{
	list->next = NULL;
	list->prev = list;
} 

extern struct animal *head;
extern struct task_struct *threads[NUM_THREADS];

#endif

