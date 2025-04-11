#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *threads[4];
static int lock = 0;

int thread_fn(void *data) {
    	int id = *(int *)data;
    	while (!kthread_should_stop()) {
        	while (__sync_val_compare_and_swap(&lock, 0, 1) != 0){ // __sync_val_compare_and_swap함수는 lock이 0이면 1로 바꾸고 처음 값인 0을 반환한다. 
			// 반환값이 0이면 -> (0 != 0)은 0, 반환값이 1이면 -> (1 != 0)은 1
			// 따라서 반환값이 0일 때, 즉 처음에 lock이 0이었을 때 이 while 루프가 끝난다.
           		msleep(10);  // spin until lock acquired
		}

        	printk(KERN_INFO "Thread %d acquired the lock (CAS).\n", id);
        	msleep(500);
        	lock = 0;  // release lock
        	msleep(100);
    	}
    	return 0;
}

static int __init compare_and_swap_init(void) {
    	static int ids[4] = {0, 1, 2, 3};
    	int i;
    	for (i = 0; i < 4; i++) {
        	threads[i] = kthread_create(thread_fn, &ids[i], "cas_thread_%d", i);
        	if (!IS_ERR(threads[i])) {
            		wake_up_process(threads[i]);
        	}
    	}
    	return 0;
}

static void __exit compare_and_swap_exit(void) {
	int i;
    	for (i = 0; i < 4; i++) {
        	if (threads[i])
            		kthread_stop(threads[i]);
    	}
   	printk(KERN_INFO "Compare-and-Swap module exited.\n");
}

module_init(compare_and_swap_init);
module_exit(compare_and_swap_exit);
MODULE_LICENSE("GPL");

