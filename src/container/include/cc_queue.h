#ifndef __CC_QUEUE_H
#define __CC_QUEUE_H

typedef int (*cc_queue_enqueue_fn_t)(void *self, void *data);
typedef int (*cc_queue_dequeue_fn_t)(void *self, void **result);
typedef int (*cc_queue_peek_fn_t)(void *self, void **result);

typedef struct cc_queue_i 
{
	cc_queue_enqueue_fn_t enqueue;
	cc_queue_dequeue_fn_t dequeue;
	cc_queue_peek_fn_t peek;
} cc_queue_i_t;

enum 
{
	CC_QUEUE_EMPTY = 0xFD10,
	CC_QUEUE_FULL = 0xFD20,
};

static inline int cc_queue_enqueue(void *self, void *data)
{
	return (*(cc_queue_i_t **)self)->enqueue(self, data);
}

static inline int cc_queue_dequeue(void *self, void **result)
{
	return (*(cc_queue_i_t **)self)->dequeue(self, result);
}

static inline int cc_queue_peek(void *self, void **result)
{
	return (*(cc_queue_i_t **)self)->peek(self, result);
}

#endif
