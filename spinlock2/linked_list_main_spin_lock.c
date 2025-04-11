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

        /* initialize animal's list head here */
        INIT_LIST_HEAD(&head->entry);

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

void destroy_list(void)
{
    	struct cat *cur;
    	struct list_head *entry, *tmp;

    	list_for_each_safe(entry, tmp, &head->entry) {
        	cur = list_entry(entry, struct cat, entry);
        	list_del(entry);
        	kfree(cur);
    	}

    	head->total = 0;
}

static void __exit lockfree_module_cleanup(void) {
        // printing the calclock results
        printk(KERN_INFO "Spinlock linked list insert time: %lld ns, count: %lld\n", add_to_list_time, add_to_list_count);
        printk(KERN_INFO "Spinlock linked list search time: %lld ns, count: %lld\n", search_list_time, search_list_count);
        printk(KERN_INFO "Spinlock linked list delete time: %lld ns, count: %lld\n", delete_from_list_time, delete_from_list_count);

        // stoping all threads
        int i;
        for (i = 0; i < NUM_THREADS; i++) {
                if (threads[i])
                        kthread_stop(threads[i]);
        }

        destroy_list();
        printk("After destroyed list: total %d cats\n", head->total);
        kfree(head);

        printk(KERN_INFO "%s: Exiting Spinlock Module!\n", __func__);
}

module_init(lockfree_module_init);
module_exit(lockfree_module_cleanup);

MODULE_LICENSE("GPL");
