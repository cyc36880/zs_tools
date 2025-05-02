#include "cc_list_queue.h"
#include <assert.h>
#include <stdint.h>

int main(void)
{
	cc_list_queue_t *list;
	uintptr_t tmp;

	assert(!cc_list_queue_new(&list));

	assert(cc_queue_dequeue(list, (void **)&tmp) == CC_QUEUE_EMPTY);

	assert(!cc_queue_enqueue(list, (void *)1));
	assert(!cc_queue_peek(list, (void **)&tmp));
	assert(tmp == 1);

	assert(!cc_queue_enqueue(list, (void *)2));
	assert(!cc_queue_enqueue(list, (void *)3));

	assert(!cc_queue_dequeue(list, (void **)&tmp));
	assert(tmp == 1);
	assert(!cc_queue_dequeue(list, (void **)&tmp));
	assert(tmp == 2);
	assert(!cc_queue_dequeue(list, (void **)&tmp));
	assert(tmp == 3);

	assert(cc_queue_dequeue(list, (void **)&tmp) == CC_QUEUE_EMPTY);

	assert(!cc_list_queue_delete(list));
	return 0;
}
