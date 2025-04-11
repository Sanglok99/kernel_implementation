#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *threads[4];
static int lock = 0;

int thread_fn(void *data) {
    int id = *(int *)data;
    while (!kthread_should_stop()) {
        int ret = __sync_lock_test_and_set(&lock, 1);  // __sync_lock_test_and_set 함수는 lock에 1을 무조건 할당하고, 이전 lock 값을 반환한다.
	// 반환값이 0: 이전의 lock을 아무도 점유하고 있지 않았다는 뜻으로, 해당 프로세스(스레드)가 lock을 획득했음을 의미
	// 반환값이 1: 이전의 lock이 이미 누군가 점유하고 있었다는 뜻으로, 해당 프로세스(스레드)가 lock 획득에 실패했음을 의미
        if (ret == 0) { // 락 획득 성공
            printk(KERN_INFO "Thread %d acquired the lock.\n", id);
            msleep(500);
            lock = 0;
        }
        msleep(100);
    }
    return 0;
}

static int __init test_and_set_init(void) {
    	static int ids[4] = {0, 1, 2, 3};
	int i;
    	for (i = 0; i < 4; i++) {
        	threads[i] = kthread_create(thread_fn, &ids[i], "tas_thread_%d", i);
        	if (!IS_ERR(threads[i])) {
            		wake_up_process(threads[i]);
        	}
    	}
    	return 0;
}

static void __exit test_and_set_exit(void) {
    	int i;
	for (i = 0; i < 4; i++) {
        	if (threads[i])
            		kthread_stop(threads[i]);
    	}
    	printk(KERN_INFO "Test-and-Set module exited.\n");
}

module_init(test_and_set_init);
module_exit(test_and_set_exit);
MODULE_LICENSE("GPL");

