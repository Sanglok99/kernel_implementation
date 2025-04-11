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

// time counter
long long add_to_list_time = 0;
long long search_list_time = 0;
long long delete_from_list_time = 0;

// node counter
long long add_to_list_count = 0;
long long search_list_count = 0;
long long delete_from_list_count = 0;

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

static inline void lf_list_add_tail(struct list_head *entry, struct list_head *head)
{
	entry->prev = __sync_lock_test_and_set(&head->prev, entry); /*atomic_set()*/
	if(entry->prev == NULL)
		head = entry;
	else
		entry->prev->next = entry;
} 

// 삽입 함수
void add_to_list(int thread_id, int range_bound[])
{
	struct timespec localclock[2];
	struct cat *new, *first = NULL;
	
	int i;
	for (i = range_bound[0]; i < range_bound[1] + 1; i++) {
		getrawmonotonic(&localclock[0]);

		/* initialize new cat here */
		new = kmalloc(sizeof(*new), GFP_KERNEL);
		new->var = i; // allocate cat id

		if (!first)
			first = new;

		/* initialize cat's list head and gc_list head here */
		INIT_LF_LIST_HEAD(&new->entry);
		INIT_LF_LIST_HEAD(&new->gc_entry);
		atomic_set(&new->removed, 0);

		/* add new cat into animal's list */
		lf_list_add_tail(&new->entry, &head->entry);
		__sync_fetch_and_add(&head->total, 1);

		getrawmonotonic(&localclock[1]);
		calclock(localclock, &add_to_list_time, &add_to_list_count);
	}
	printk(KERN_INFO "thread #%d: inserted cat #%d-%d to the list, total: %u cats\n", thread_id, first->var, new->var, head->total);
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

// 검색 함수
int search_list(int thread_id, int range_bound[])
{
        struct list_head *entry, *iter = &head->entry;
        /* This will point on the actual data structures during the iteration */
        struct cat *cur;
        struct timespec localclock[2];
        int target_idx = select_target_index(range_bound);

        /* iterate over the list, skip removed entry, search cat with target index */
        while((entry = iter) != NULL){ // 현재의 iter 위치를 entry에 백업 & iter가 NULL에 도달했는지 조건 확인
                getrawmonotonic(&localclock[0]);
                if(__sync_val_compare_and_swap(&iter, entry, iter->next) != entry) // CAS(Compare-And-Swap)를 통해 race condition 없이 안전하게 포인터를 전진시키는 Lock-Free 방식
                        // Prototype: T __sync_val_compare_and_swap (T* __p, U __compVal, V __exchVal, ...);
                        // __compVal과 __p가 가리키는 변수의 값을 비교한다
                        // 일치하면 __eachVal값이 __p가 가리키는 변수에 저장됨
                        // return 값: __p가 가리키는 변수의 원래 값
                        // *** T --> struct list_head *
			// 즉, *(&iter) 와 entry의 비교. *(&iter) --> iter 이므로 iter와 entry가 같은지 비교하는 것
			// iter와 entry가 같을 시: iter = iter->next(포인터 전진)
                        continue;

                if(entry == &head->entry)
                	// if (!entry->prev->next) // skip for head entry (struct animal)
                	// 조건문이 취약하다고 판단되어 변경함
                        continue;

                cur = list_entry(entry, struct cat, entry);
                // `entry`는 `list_head` 포인터이고, 이것이 `struct cat` 내부에 박혀 있으므로
                // 이를 바탕으로 `cur` 포인터를 얻음
                // `container_of()` 매크로 역할을 함

                if(atomic_read(&cur->removed))
                        continue;

                /* action for this searched entry
                for example, we can make if and break statements */
                if (cur->var == target_idx) {
                        printk(KERN_INFO "Thread #%d found cat #%d!\n", thread_id, cur->var);

                        getrawmonotonic(&localclock[1]);
                        calclock(localclock, &search_list_time, &search_list_count);

                        return 0;
                }
                getrawmonotonic(&localclock[1]);
                calclock(localclock, &search_list_time, &search_list_count);
        }
        return -ENODATA;
}

# if 0 // 당장은 필요 없는 함수라 주석 처리
static void add_to_garbage_list(int thread_id, void *data)
{
	struct cat *target = (struct cat *) data;
	/* set cat as deleted */
	/* add to garbage list */
	/* decrease total cats in struct animal */
}
#endif

// 삭제 함수
void delete_from_list(int thread_id, int range_bound[])
{
	int start = -1, end = -1;
	struct timespec localclock[2];
	struct list_head *entry, *iter = &head->entry;
	/* This will point on the actual data structures during the iteration */
	struct cat *cur;
	int target_idx = select_target_index(range_bound);

	// setting up delete target range
	start = target_idx;
	end = range_bound[1];

	/* iterate over the list, skip removed entry */
	while((entry = iter) != NULL){
		getrawmonotonic(&localclock[0]);

                if(__sync_val_compare_and_swap(&iter, entry, iter->next) != entry)
                        continue;
                if (!entry->prev->next) /* skip for head entry (struct animal) */
                        continue;
                cur = list_entry(entry, struct cat, entry);
                if(atomic_read(&cur->removed))
                        continue;

                /* add cat into garbage list if its id is within target range */
                if (cur->var <= end && cur->var >= start) {
			atomic_set(&cur->removed, 1);
			lf_list_add_tail(&cur->gc_entry, &head->gc_entry);
			__sync_fetch_and_sub(&head->total, 1);
                }
		getrawmonotonic(&localclock[1]);
        	calclock(localclock, &delete_from_list_time, &delete_from_list_count);
        }
	printk(KERN_INFO "thread #%d: marked cat #%d-%d as deleted, total: %d cats\n", thread_id, start, end, head->total);
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
