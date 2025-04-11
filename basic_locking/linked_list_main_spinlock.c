#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h> // for kmalloc, kfree

#define NUM_THREADS 4

// time counter
extern long insert_ns_time;
extern long search_ns_time;
extern long delete_ns_time;

// node counter
extern int insert_counter;
extern int search_counter;
extern int delete_counter;

extern int work_fn(void *data);

static struct task_struct *threads[NUM_THREADS];

static int __init list_module_init(void) {
	printk(KERN_INFO "Entering Spinlock Module!\n");

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
                        kfree(id); // thread 생성 실패 시 id 동적 할당한 거 해제해야 함
			threads[i] = NULL;
                } else {
                        wake_up_process(threads[i]);
                }
	}

        return 0;
}

static void __exit list_module_exit(void) {
	printk(KERN_INFO "Spinlock linked list insert time: %ld ns, count: %d\n", insert_ns_time, insert_counter);
	printk(KERN_INFO "Spinlock linked list search time: %ld ns, count: %d\n", search_ns_time, search_counter);
	printk(KERN_INFO "Spinlock linked list delete time: %ld ns, count: %d\n", delete_ns_time, delete_counter);

        int i;
        for (i = 0; i < NUM_THREADS; i++) {
                if (threads[i])
                        kthread_stop(threads[i]);
        }
        printk(KERN_INFO "Exiting Spinlock Module!\n");
}

module_init(list_module_init);
module_exit(list_module_exit);

MODULE_LICENSE("GPL");

