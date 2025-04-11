#ifndef ANIMALS_H
#define ANIMALS_H

#include <linux/list.h>
#include <linux/spinlock.h>

#define NUM_THREADS 1 // test

struct cat {
    int var;
    struct list_head entry;
};

struct animal {
    int total;
    struct list_head entry;
    spinlock_t list_lock;
};

// Initialization(slide 11)
static inline void my_INIT_LIST_HEAD(struct list_head *list)
{
        list->next = NULL;
        list->prev = list;
}

extern struct animal *head;
extern struct task_struct *threads[NUM_THREADS];

#endif
