#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/init.h> // included for __init and __exit macros

static struct task_struct *threads[4];
static int counter = 0;

int thread_fn(void *data) {
    	int id = *(int *)data;
    	while (!kthread_should_stop()) {
        	int val = __sync_fetch_and_add(&counter, 1);  
        	printk(KERN_INFO "Thread %d incremented counter to %d\n", id, val + 1);
        	msleep(500); // 너무 빠르게 반복되지 않도록
    	}
    	return 0;
}

static int __init atomic_module_init(void) {
    	static int ids[4] = {0, 1, 2, 3};
    	int i;
    	for (i = 0; i < 4; i++) {
        	threads[i] = kthread_create(thread_fn, &ids[i], "atomic_thread_%d", i);
        	if (!IS_ERR(threads[i])) {
            		wake_up_process(threads[i]);
        	}
    	}
    	return 0;
}

static void __exit atomic_module_exit(void) {
	int i;
    	for (i = 0; i < 4; i++) {
        	if (threads[i])
           		kthread_stop(threads[i]);
    	}
    	printk(KERN_INFO "Atomic module exited.\n");
}

module_init(atomic_module_init);
module_exit(atomic_module_exit);
MODULE_LICENSE("GPL");
