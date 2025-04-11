#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/mutex.h>

// time counter
long insert_ns_time = 0;
long search_ns_time = 0;
long delete_ns_time = 0;

// node counter
int insert_counter = 0;
int search_counter = 0;
int delete_counter = 0;

// global mutex lock
static DEFINE_MUTEX(my_lock);

// 리스트 노드 구조체
struct my_node {
    int data;
    struct list_head entry;
};

// 글로벌 연결 리스트
static LIST_HEAD(my_list);

// 작업 범위 설정 함수
void set_iter_range(int thread_id, int range[2]) {
    	int total = 1000000;
    	int chunk = total / 4;
    	range[0] = thread_id * chunk;
    	range[1] = (thread_id + 1) * chunk - 1;
}

// 삽입 함수
void *add_to_list(int thread_id, int range[2])
{
    	struct timespec start, end;
	struct my_node *first = NULL;

	getnstimeofday(&start);
    	mutex_lock(&my_lock);

	int i;
    	for (i = range[0]; i <= range[1]; i++) {
        	struct my_node *new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
        	new->data = i;
        	INIT_LIST_HEAD(&new->entry);
        	list_add_tail(&new->entry, &my_list);

        	if (i == range[0])
            		first = new;

		__sync_fetch_and_add(&insert_counter, 1); // node insert counter 증가
    	}
    	mutex_unlock(&my_lock);
	getnstimeofday(&end);

	long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
	__sync_fetch_and_add(&insert_ns_time, elapsed_ns); // time insert counter 증가

    	printk(KERN_INFO "Thread #%d range: %d ~ %d \n", thread_id, range[0], range[1]);
    	return first;
}

// 검색 함수
int search_list(int thread_id, void *data, int range[2])
{
    	struct timespec start, end;
    	struct my_node *pos;

    	getnstimeofday(&start);
    	mutex_lock(&my_lock);

    	list_for_each_entry(pos, &my_list, entry) {
        	if (pos->data >= range[0] && pos->data <= range[1]) {
            		// printk(KERN_INFO "Thread #%d confirmed data %d is in range\n", thread_id, pos->data);
			// 필요없는 출력
			__sync_fetch_and_add(&search_counter, 1); // node search counter 증가
        	}
    	}

    	mutex_unlock(&my_lock);
    	getnstimeofday(&end);

    	long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
	__sync_fetch_and_add(&search_ns_time, elapsed_ns); // time insert counter 증가

    	printk(KERN_INFO "Thread #%d searched range: %d ~ %d \n", thread_id, range[0], range[1]);
    	return 0;
}

// 삭제 함수
int delete_from_list(int thread_id, int range[2])
{
    	struct my_node *pos, *tmp;
    	struct timespec start, end;

    	getnstimeofday(&start);
    	mutex_lock(&my_lock);

    	list_for_each_entry_safe(pos, tmp, &my_list, entry) {
        	if (pos->data >= range[0] && pos->data <= range[1]) {
            		list_del(&pos->entry);
            		kfree(pos);
			__sync_fetch_and_add(&delete_counter, 1); // node delete counter 증가
        	}
    	}

    	mutex_unlock(&my_lock);
    	getnstimeofday(&end);

    	long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
	__sync_fetch_and_add(&delete_ns_time, elapsed_ns); // time insert counter 증가
    	printk(KERN_INFO "Thread #%d deleted range: %d ~ %d \n", thread_id, range[0], range[1]);
    	return 0;
}

// Worker function
int work_fn(void *data)
{
        int range_bound[2];
        int thread_id = *(int*) data;

        set_iter_range(thread_id, range_bound);
        void *ret = add_to_list(thread_id, range_bound);
        search_list(thread_id, ret, range_bound);
        delete_from_list(thread_id, range_bound);

        while(!kthread_should_stop()) {
                msleep(500);
        }
        printk(KERN_INFO "thread #%d stopped!\n", thread_id);

	kfree(data); // 메모리 해제
        return 0;
}

MODULE_LICENSE("GPL");

