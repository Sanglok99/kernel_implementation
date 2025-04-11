#include <linux/kernel.h>  // All modules
#include <linux/module.h>
#include <linux/init.h>    // For macros
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/slab.h>   //kmalloc()
#include <linux/wait.h>   //For wait queues
#include <linux/delay.h>
#include <linux/sched/signal.h> // for signal_pending()

struct task_struct *waiters[8];
struct task_struct *waker[2];
struct wait_queue_head wait_queue;
int wait_queue_flag_1 = 0; // condition flag for waiters type 1
int wait_queue_flag_2 = 0; // condition flag for waiters type 2
int indexs[8] = {0,1,2,3,4,5,6,7}; 
int waker_types[2] = {1,2};

int waiter_type_1_fn(void *data) {
	int index = *(int*) data;
	DEFINE_WAIT(wait); // Create a wait queue entry
	add_wait_queue(&wait_queue, &wait); // Add itself to the wait queue
	pr_info("Waiter %d is added to the wait queue\n", index); 
	while(wait_queue_flag_1 != 1) { // condition: the event we waiting for
		prepare_to_wait(&wait_queue, &wait, TASK_INTERRUPTIBLE); 
		/* Change the state of task to
		 * TASK_INTERRUPTIBLE or
		 * TASK_UNINTERRUPTIBLE */
		if (signal_pending(current)) { /*handle signal*/
			finish_wait(&wait_queue, &wait);
            		pr_info("Waiter %d is interrupted by signal\n", index);
            		return -ERESTARTSYS;  // 또는 다른 에러 코드
		}
		schedule();
	}
	finish_wait(&wait_queue, &wait);
	/* Condition meets, waiter finishes
	 * waiting, changes the state
	 * to TASK_RUNNING, removes from wq */

	pr_info("Condition meets, so waiter %d finishes waiting\n", index);
	return 0;
}

// 내 코드
int waiter_type_2_fn(void *data) {
	// your codes should work with wait_queue_flag_2
	int index = *(int*) data;
        DEFINE_WAIT(wait); // Create a wait queue entry
        add_wait_queue(&wait_queue, &wait); // Add itself to the wait queue
        pr_info("Waiter %d is added to the wait queue\n", index); 
        while(wait_queue_flag_2 != 1) { // condition: the event we waiting for
                prepare_to_wait(&wait_queue, &wait, TASK_INTERRUPTIBLE); 
                /* Change the state of task to
                 * TASK_INTERRUPTIBLE or
                 * TASK_UNINTERRUPTIBLE */
                if (signal_pending(current)) { /*handle signal*/
                        finish_wait(&wait_queue, &wait);
                        pr_info("Waiter %d is interrupted by signal\n", index);
                        return -ERESTARTSYS;  // 또는 다른 에러 코드
                }
                schedule();
        }   
        finish_wait(&wait_queue, &wait);
        /* Condition meets, waiter finishes
         * waiting, changes the state
         * to TASK_RUNNING, removes from wq */

        pr_info("Condition meets, so waiter %d finishes waiting\n", index);
        return 0;
}

int waker_fn(void *data) {
	int flag = *(int*) data;
	if (flag == 1) {
		wait_queue_flag_1 = 1;
		printk("Current wait_queue_flag is %d", wait_queue_flag_1);
	}

	if (flag == 2){
		wait_queue_flag_2 = 1;
		printk("Current wait_queue_flag is %d", wait_queue_flag_2);
	}

	wake_up_interruptible(&wait_queue);
	return 0;
}

int __init wait_module_init(void) {
	printk("Initialize a wait queue!\n");
	// Initialize wait queue
	init_waitqueue_head(&wait_queue);
	// Create waiting threads
	int i;
	for (i = 0; i < 4; i++) {
		waiters[i] = kthread_run(waiter_type_1_fn, &indexs[i], "waiting threads");
	}
       	for (i = 4; i < 8; i++) {
		waiters[i] = kthread_run(waiter_type_2_fn, &indexs[i], "waiting threads");
	}
	return 0;
}

void __exit wait_module_exit(void) {
	if (waitqueue_active(&wait_queue)){
		int i;
		for (i = 0; i < 2; i++) {
	    		waker[i] = kthread_run(waker_fn, &waker_types[i], "waker thread");	
		}
	}
	printk("Exiting the wait queue!\n");
}

module_init(wait_module_init);
module_exit(wait_module_exit);

MODULE_LICENSE("GPL");
