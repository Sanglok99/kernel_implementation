#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/random.h>  // 커널에서 랜덤 수를 생성할 때 사용
#include "animals.h"
#include "calclock.h"

#define list_for_each_non_circular(pos, head) \
	for (pos = (head)->next; pos != NULL; pos = pos->next)

// time counter
long long add_to_list_time = 0;
long long search_list_time = 0;
long long delete_from_list_time = 0;

// node counter
long long add_to_list_count = 0;
long long search_list_count = 0;
long long delete_from_list_count = 0;

extern inline void __list_my_del(struct list_head *prev, struct list_head *next, struct list_head *head);

extern inline void my_list_del(struct list_head *entry, struct list_head *head);

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

#if 0
// 삽입
static inline void my_list_add_tail(struct list_head *entry, struct list_head *head){
        entry->prev = __sync_lock_test_and_set(&head->prev, entry); /*atomic_set()*/
        if(entry->prev == NULL)
                head = entry;
        else
                entry->prev->next = entry;
}
#endif

static inline void my_list_add_tail(struct list_head *new, struct list_head *head)
{
    struct list_head *last = head->prev;

    new->next = NULL;
    new->prev = last;
    last->next = new;
    head->prev = new;
}


/*
void add_to_list(int thread_id, int range_bound[])
{
    	struct timespec localclock[2];
	struct cat *new = kmalloc(sizeof(*new), GFP_KERNEL);
    	if (!new) return;

    	new->var = select_target_index(range_bound);
    	my_INIT_LIST_HEAD(&new->entry);

    	getrawmonotonic(&localclock[0]);
    	spin_lock(&head->list_lock);
    	my_list_add_tail(&new->entry, &head->entry);
    	head->total++;
    	spin_unlock(&head->list_lock);
    	getrawmonotonic(&localclock[1]);

   	calclock(localclock, &add_to_list_time, &add_to_list_count);
}
*/

void add_to_list(int thread_id, int range_bound[])
{
    	struct timespec localclock[2];
    	struct cat *new, *first = NULL;

    	int i;
    	for (i = range_bound[0]; i <= range_bound[1]; i++) {
        	getrawmonotonic(&localclock[0]);

        	/* 동적 메모리 할당 */
        	new = kmalloc(sizeof(*new), GFP_KERNEL);
        	if (!new)
            		continue;

        	new->var = i;  // 고양이 ID 할당

        	if (!first)
            		first = new;

        	/* 리스트 노드 초기화 */
        	my_INIT_LIST_HEAD(&new->entry);

        	spin_lock(&head->list_lock);
        	my_list_add_tail(&new->entry, &head->entry);
        	head->total++;
        	spin_unlock(&head->list_lock);
	
        	getrawmonotonic(&localclock[1]);
        	calclock(localclock, &add_to_list_time, &add_to_list_count);
    	}

    	printk(KERN_INFO "thread #%d: inserted cat #%d-%d to the list, total: %u cats\n", thread_id, first ? first->var : -1, new ? new->var : -1, head->total);
}

int select_target_index(int range_bound[2])
{
        unsigned int rand;
        get_random_bytes(&rand, sizeof(rand));  // rand에 랜덤값 생성

        int low = range_bound[0];
        int high = range_bound[1];

        if (low >= high)
                return low;

        return low + (rand % (high - low + 1));
}

void delete_from_list(int thread_id, int range_bound[])
{
	int start = -1, end = -1;
	struct timespec localclock[2];
	struct list_head *entry;
	struct cat *cur;
	int target_idx = select_target_index(range_bound);

	printk("sl: line: %d, func: %s\n", __LINE__, __func__);
	start = target_idx;
	printk("sl: line: %d, func: %s\n", __LINE__, __func__);
	end = range_bound[1];
	printk("sl: line: %d, func: %s\n", __LINE__, __func__);

	spin_lock(&head->list_lock);
	printk("sl: line: %d, func: %s\n", __LINE__, __func__);

	list_for_each_non_circular(entry, &head->entry) {
		printk("sl: line: %d, func: %s\n", __LINE__, __func__);
		cur = list_entry(entry, struct cat, entry);
		printk("sl: line: %d, func: %s\n", __LINE__, __func__);

		getrawmonotonic(&localclock[0]);
		printk("sl: line: %d, func: %s\n", __LINE__, __func__);

		if (cur->var >= start && cur->var <= end) {
			printk("sl: line: %d, func: %s\n", __LINE__, __func__);
			list_del(&cur->entry);  // 리스트에서 제거
			printk("sl: line: %d, func: %s\n", __LINE__, __func__);
			kfree(cur);             // 메모리 해제
			printk("sl: line: %d, func: %s\n", __LINE__, __func__);
			head->total--;
			printk("sl: line: %d, func: %s\n", __LINE__, __func__);
		}
		printk("sl: line: %d, func: %s\n", __LINE__, __func__);

		getrawmonotonic(&localclock[1]);
		printk("sl: line: %d, func: %s\n", __LINE__, __func__);
		calclock(localclock, &delete_from_list_time, &delete_from_list_count);
		printk("sl: line: %d, func: %s\n", __LINE__, __func__);
	}

	spin_unlock(&head->list_lock);
	printk("sl: line: %d, func: %s\n", __LINE__, __func__);

	printk(KERN_INFO "thread #%d: deleted cat #%d-%d, total: %d cats\n", thread_id, start, end, head->total);
}


/*
// 삭제
void delete_from_list(int thread_id, icalclock(localclock, &add_to_list_time, &add_to_list_count);nt range_bound[])
{
    	struct list_head *entry;
    	struct cat *cur;
    	int target = select_target_index(range_bound);

    	spin_lock(&head->lock);
    	list_for_each_non_circular(entry, &head->entry) {
		getrawmonotonic(&localclock[0]);
        	cur = list_entry(entry, struct cat, entry);
        	if (cur->var == target) {
            		my_list_del(&cur->entry);
            		kfree(cur);
            		head->total--;
            		break;
        	}
		getrawmonotonic(&localclock[1]);
		calclock(localclock, &delete_from_list_time, &delete_from_list_count);
    	}	
    	spin_unlock(&head->lock);
}
*/

/*
// 검색
void search_list(int thread_id, int range_bound[])
{
    	struct list_head *entry;
    	struct cat *cur;
    	int target = select_target_index(range_bound);

    	spin_lock(&head->lock);
    	list_for_each_non_circular(entry, &head->entry) {
		getrawmonotonic(&localclock[0]);
        	cur = list_entry(entry, struct cat, entry);
        	if (cur->var == target) {
            		spin_unlock(&head->lock);
            		return;
        	}
		getrawmonotonic(&localclock[1]);
		calclock(localclock, &search_list_time, &search_list_count);
    	}
    	spin_unlock(&head->lock);
}
*/

int search_list(int thread_id, int range_bound[])
{
    	struct list_head *entry;
   	struct cat *cur;
    	struct timespec localclock[2];
    	int target_idx = select_target_index(range_bound);

    	spin_lock(&head->list_lock);
	
    	list_for_each_non_circular(entry, &head->entry) {
        	cur = list_entry(entry, struct cat, entry);

        	getrawmonotonic(&localclock[0]);

        	if (cur->var == target_idx) {
            		printk(KERN_INFO "Thread #%d found cat #%d!\n", thread_id, cur->var);
			
			getrawmonotonic(&localclock[1]);
                        calclock(localclock, &search_list_time, &search_list_count);

            		spin_unlock(&head->list_lock);
            		return 0;
        	}

        	getrawmonotonic(&localclock[1]);
        	calclock(localclock, &search_list_time, &search_list_count);
    	}

    	spin_unlock(&head->list_lock);
    	return -ENODATA;
}


// Worker function
int work_fn(void *data)
{
        int range_bound[2];
        int err, thread_id = *(int*) data;

        set_iter_range(thread_id, range_bound);
        add_to_list(thread_id, range_bound);
        err = search_list(thread_id, range_bound);
        if (!err)
                delete_from_list(thread_id, range_bound);

        while(!kthread_should_stop()) {
                msleep(500);
        }
        printk(KERN_INFO "thread #%d stopped!\n", thread_id);

        return 0;
}
