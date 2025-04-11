#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h> // for kmalloc, kfree

#include "animals.h"

struct animal *head; // Global variable
struct task_struct *threads[NUM_THREADS];

// time counter
extern long long add_to_list_time;
extern long long search_list_time;
extern long long delete_from_list_time;

// node counter
extern long long add_to_list_count;
extern long long search_list_count;
extern long long delete_from_list_count;


extern int work_fn(void *data);

static int __init lockfree_module_init(void) {
	printk(KERN_INFO "%s: Entering Lock-free Module!\n", __func__);

	/* initialize struct animal here */
	// 동물 컨테이너인 head를 커널 힙에 동적 할당
    	head = kmalloc(sizeof(struct animal), GFP_KERNEL);
    	if (!head) {
        	pr_err("Failed to allocate memory for animal head.\n");
        	return -ENOMEM;
    	}

    	// 고양이 수 초기화
    	head->total = 0;

	/* initialize animal's list head and gc_list head here */
	// 동물 리스트 (entry)와 GC 리스트 (gc_entry) 초기화
    	INIT_LF_LIST_HEAD(&head->entry);
    	INIT_LF_LIST_HEAD(&head->gc_entry);

	// starting each thread
	int i;
	for (i = 0; i < NUM_THREADS; i++) {
		int *id = kmalloc(sizeof(int), GFP_KERNEL);
		if(!id) {
			pr_err("Failed to allocate memory for thread id %d\n", i);
			continue;
		}

		*id = i;
		threads[i] = kthread_create(work_fn, id, "list_thread_%d", i);
		if (IS_ERR(threads[i])) {
                        pr_err("Failed to create thread %d\n",i);
                        kfree(id); // releasing dynamically allocated memory in case of thread creation failure
			threads[i] = NULL;
                } else {
                        wake_up_process(threads[i]);
                }
	}

        return 0;
}

static inline void
__gc_list_del(struct list_head *prev, struct list_head *next, struct list_head *head)
{
	if (next)
		next->prev = prev;
	else /* tail entry */
		head->prev = prev;

	prev->next = next;
}

static inline void
gc_list_del(struct list_head *entry, struct list_head *head)
{
	__gc_list_del(entry->prev, entry->next, head);
}

int empty_garbage_list(void)
{
	struct cat *cur;
	struct list_head *entry, *iter = &head->gc_entry;
	unsigned int counter = 0;

	/* iterate garbage list,
	remove each entry from every list and free it */
	while ((entry = iter) != NULL) { 
		iter = iter->next; 
		if (!entry->prev->next) /* skip for head entry (struct animal) */
			continue; 
		cur = list_entry(entry, struct cat, gc_entry);
		if (!atomic_read(&cur->removed))
			continue;

		/* action for this searched entry */
		gc_list_del(&cur->entry, &head->entry); /* delete from animal list */
		gc_list_del(&cur->gc_entry, &head->gc_entry); /* delete from garbage collection list */
		kfree(cur); 
		counter++;
	}

	printk(KERN_INFO "%s: freed %u cats\n", __func__, counter);
	
	return 0;
}

void destroy_list(void)
{
	struct cat *cur;
	struct list_head *entry, *iter = &head->entry;
	
	/* iterate the list,
	remove each entry from every list and free it */
	while ((entry = iter) != NULL) { // 다음 노드로 이동 전에 백업
        	iter = iter->next;

        	// head 구조체(컨테이너)는 삭제 대상이 아님
        	if (!entry->prev->next)
            		continue;

        	cur = list_entry(entry, struct cat, entry);

        	// animal list에서 제거
        	gc_list_del(&cur->entry, &head->entry);

        	// GC 리스트에도 들어 있을 수 있으므로 같이 제거
        	gc_list_del(&cur->gc_entry, &head->gc_entry);

        	// 메모리 해제
        	kfree(cur);
	}

    	// 리스트 비운 후 총 노드 수도 초기화
    	head->total = 0;
}

static void __exit lockfree_module_cleanup(void) {
	// printing the calclock results
	printk(KERN_INFO "Lock-free linked list insert time: %lld ns, count: %lld\n", add_to_list_time, add_to_list_count);
	printk(KERN_INFO "Lock-free linked list search time: %lld ns, count: %lld\n", search_list_time, search_list_count);
	printk(KERN_INFO "Lock-free linked list delete time: %lld ns, count: %lld\n", delete_from_list_time, delete_from_list_count);

	// stoping all threads
        int i;
        for (i = 0; i < NUM_THREADS; i++) {
                if (threads[i])
                        kthread_stop(threads[i]);
        }

        empty_garbage_list();
	printk("After gc: total %d cats\n", head->total);
	destroy_list();
	printk("After destroyed list: total %d cats\n", head->total);
	kfree(head);

	printk(KERN_INFO "%s: Exiting Lock-free Module!\n", __func__);
}

module_init(lockfree_module_init);
module_exit(lockfree_module_cleanup);

MODULE_LICENSE("GPL");

