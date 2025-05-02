#include "cc_list.h"
#include "cc_iter.h"
#include <stdlib.h>

cc_iter_i_t cc_list_iter_interface = {
	.next = (cc_iter_next_fn_t)cc_list_iter_next,
};

/*****************************************************************************
 * List Cursor Functions
 ****************************************************************************/
int cc_list_cursor_relative_next(cc_list_cursor_t *self, int offset,
		cc_list_node_t **result)
{
	cc_list_node_t *node;
	/*
	 * The `last` element can be the root node, so it's
	 * `n != &self->list->root` here
	 */
	for (node = self->current; node != &self->list->root && offset > 0;
			offset--)
		node = node->next;

	if (offset > 0)
		return CC_LIST_CURSOR_MOVE_OUT_OF_RANGE;

	*result = node;
	return 0;
}

int cc_list_cursor_relative_prev(cc_list_cursor_t *self, int offset,
		cc_list_node_t **result)
{
	cc_list_node_t *node;
	/*
	 * The `first` element can NOT be the root node, so it's
	 * `n->prev != &self->list->root` here
	 */
	for (node = self->current;
			node->prev != &self->list->root && offset > 0;
			offset--)
		node = node->prev;

	if (offset > 0)
		return CC_LIST_CURSOR_MOVE_OUT_OF_RANGE;

	*result = node;
	return 0;
}

/* This function can only return 0 or CC_LIST_CURSOR_MOVE_OUT_OF_RANGE */
int cc_list_cursor_relative_pos(cc_list_cursor_t *self, int offset,
		cc_list_node_t **result)
{
	if (offset >= 0)
		return cc_list_cursor_relative_next(self, offset, result);
	else
		return cc_list_cursor_relative_prev(self, -offset, result);
}

int cc_list_cursor_insert_before(cc_list_cursor_t *self, int offset,
		void *data)
{
	cc_list_node_t *node;

	if (cc_list_cursor_relative_pos(self, offset, &node))
		return CC_LIST_CURSOR_MOVE_OUT_OF_RANGE;
	if (cc_list_node_insert_before(node, data))
		return 1;

	self->list->root.size++;
	return 0;
}

int cc_list_cursor_remove(cc_list_cursor_t *self, int offset, int count)
{
	cc_list_node_t *node1, *node2;
	int i;

	if (offset <= 0 && offset + count > 0)
		return CC_LIST_CURSOR_REMOVING_CURRENT;
	if (cc_list_cursor_relative_pos(self, offset, &node1))
		return CC_LIST_CURSOR_MOVE_OUT_OF_RANGE;
	if (cc_list_cursor_relative_pos(self, offset + count, &node2))
		return CC_LIST_CURSOR_MOVE_OUT_OF_RANGE;

	/* fix the chain before removing nodes */
	node1->prev->next = node2;
	node2->prev = node1->prev;

	for (i = 0; node1 != node2; i++) {
		if (cc_list_node_delete_and_next(&node1, self->remove_fn))
			return 1;
	}
	self->list->root.size -= i;
	return 0;
}

/* CAUTION:
 * Please make sure that `result` can hold `count` numbers of pointers.
 */
int cc_list_cursor_get(cc_list_cursor_t *self, int offset, int count,
		void **result)
{
	cc_list_node_t *node;
	int i;

	if (cc_list_cursor_relative_pos(self, offset, &node))
		return CC_LIST_CURSOR_MOVE_OUT_OF_RANGE;

	for (i = 0; i < count && node != &self->list->root;
			i++, node = node->next)
		result[i] = node->data;

	if (i < count)
		return CC_LIST_CURSOR_GET_OUT_OF_RANGE;

	return 0;
}

int cc_list_cursor_move(cc_list_cursor_t *self, int offset)
{
	cc_list_node_t *new_pos;

	if (cc_list_cursor_relative_pos(self, offset, &new_pos))
		return CC_LIST_CURSOR_MOVE_OUT_OF_RANGE;

	self->current = new_pos;
	return 0;
}

int cc_list_cursor_at_end(cc_list_cursor_t *self)
{
	return self->current == &self->list->root;
}

void cc_list_cursor_reset(cc_list_cursor_t *self)
{
	self->current = self->list->root.next;
}

int cc_list_cursor_init(cc_list_cursor_t *self, cc_list_t *list,
		cc_delete_fn_t remove_fn)
{
	self->remove_fn = remove_fn;
	self->list = list;
	self->current = &list->root;
	return 0;
}

int cc_list_cursor_new(cc_list_cursor_t **self, cc_list_t *list,
		cc_delete_fn_t remove_fn)
{
	cc_list_cursor_t *tmp;

	tmp = malloc(sizeof(*tmp));
	if (tmp == NULL)
		goto fail1;
	if (cc_list_cursor_init(tmp, list, remove_fn))
		goto fail2;

	*self = tmp;
	return 0;
fail2:
	free(tmp);
fail1:
	return 1;
}

int cc_list_cursor_delete(cc_list_cursor_t *self)
{
	free(self);
	return 0;
}

/*****************************************************************************
 * List Iterator Functions
 ****************************************************************************/
static inline void cc_list_iter_step(cc_list_iter_t *self)
{
	if (self->direction == 0)
		self->cursor = self->cursor->next;
	else
		self->cursor = self->cursor->prev;
}

int cc_list_iter_next(cc_list_iter_t *self, void **item, size_t *index)
{
	if (item == NULL)
		return 1;
	if (self->cursor == &self->list->root)
		return 2;

	*item = &self->cursor->data;

	if (index != NULL)
		*index = self->index;

	self->index++;
	cc_list_iter_step(self);
	return 0;
}

int cc_list_iter_init(cc_list_iter_t *self, cc_list_t *list,
		int direction)
{
	if (list == NULL)
		return 1;

	self->iterator = &cc_list_iter_interface;
	self->list = list;
	self->index = 0;
	self->direction = direction;
	self->cursor = &self->list->root;

	cc_list_iter_step(self);
	return 0;
}

/*****************************************************************************
 * List Node Functions
 ****************************************************************************/
int cc_list_node_insert_before(cc_list_node_t *self, void *data)
{
	cc_list_node_t *node;

	node = malloc(sizeof(*node));
	if (node == NULL)
		return 1;

	node->data = data;
	node->prev = self->prev;
	node->next = self;
	self->prev->next = node;
	self->prev = node;
	return 0;
}

int cc_list_node_insert_after(cc_list_node_t *self, void *data)
{
	cc_list_node_t *node;

	node = malloc(sizeof(*node));
	if (node == NULL)
		return 1;

	node->data = data;
	node->next = self->next;
	node->prev = self;
	self->next->prev = node;
	self->next = node;
	return 0;
}

int cc_list_node_remove_before(cc_list_node_t *self, void **result)
{
	cc_list_node_t *node;

	/* You have to provide `result`, or the `self->next->data` may leak. */
	if (result == NULL)
		return 1;

	*result = self->prev->data;
	node = self->prev;
	self->prev->prev->next = self;
	self->prev = self->prev->prev;

	free(node);
	return 0;
}

int cc_list_node_remove_after(cc_list_node_t *self, void **result)
{
	cc_list_node_t *node;

	/* You have to provide `result`, or the `self->next->data` may leak. */
	if (result == NULL)
		return 1;

	*result = self->next->data;
	node = self->next;
	self->next->next->prev = self;
	self->next = self->next->next;

	free(node);
	return 0;
}

int cc_list_node_delete_and_next(cc_list_node_t **pcurrent,
		cc_delete_fn_t remove_fn)
{
	cc_list_node_t *current, *next;
	current = *pcurrent;
	next = current->next;
	if (remove_fn != NULL) {
		if (remove_fn(current->data))
			return 1;
	}

	free(current);

	*pcurrent = next;
	return 0;
}

/*****************************************************************************
 * List Functions
 ****************************************************************************/
int cc_list_to_cc_array(cc_list_t *self, cc_array_t **result)
{
	cc_list_iter_t iter;
	cc_array_t *arr;
	void **tmp;
	size_t i;

	if (cc_array_new(&arr, self->root.size, sizeof(void *)))
		goto fail1;
	if (cc_list_iter_init(&iter, self, 0))
		goto fail2;
	while (!cc_iter_next(&iter, &tmp, &i)) {
		if (cc_array_set(arr, i, tmp))
			goto fail2;
	}

	*result = arr;
	return 0;
fail2:
	cc_array_delete(arr);
fail1:
	return 1;
}

static inline void cc_list_reset(cc_list_t *self)
{
	self->root.prev = &self->root;
	self->root.next = &self->root;
	self->root.size = 0;
}

/* CAUTION:
 * You may need to delete `right` list after this concatenation.
 */
int cc_list_concat(cc_list_t *left, cc_list_t *right)
{
	/* `left` can not be NULL since it holds the result. */
	if (left == NULL)
		return 1;
	if (right == NULL)
		return 0;

	left->root.prev->next = right->root.next;
	right->root.next->prev = left->root.prev;

	left->root.prev = right->root.prev;
	right->root.prev->next = &left->root;

	left->root.size += right->root.size;

	cc_list_reset(right);
	return 0;
}

int cc_list_insert_head(cc_list_t *self, void *data)
{
	if (cc_list_node_insert_after(&self->root, data))
		return 1;

	self->root.size++;
	return 0;
}

int cc_list_insert_tail(cc_list_t *self, void *data)
{
	if (cc_list_node_insert_before(&self->root, data))
		return 1;

	self->root.size++;
	return 0;
}

int cc_list_remove_head(cc_list_t *self, void **data)
{
	if (cc_list_is_empty(self))
		return CC_LIST_EMPTY;
	if (cc_list_node_remove_after(&self->root, data))
		return 2;

	self->root.size--;
	return 0;
}

int cc_list_remove_tail(cc_list_t *self, void **data)
{
	if (cc_list_is_empty(self))
		return CC_LIST_EMPTY;
	if (cc_list_node_remove_before(&self->root, data))
		return 2;

	self->root.size--;
	return 0;
}

int cc_list_get_head(cc_list_t *self, void **data)
{
	if (cc_list_is_empty(self))
		return CC_LIST_EMPTY;

	*data = self->root.next->data;
	return 0;
}

int cc_list_get_tail(cc_list_t *self, void **data)
{
	if (cc_list_is_empty(self))
		return CC_LIST_EMPTY;

	*data = self->root.prev->data;
	return 0;
}

int cc_list_is_empty(cc_list_t *self)
{
	return self->root.next == &self->root;
}

size_t cc_list_size(cc_list_t *self) { return self->root.size; }

int cc_list_new(cc_list_t **self)
{
	cc_list_t *tmp;
	tmp = malloc(sizeof(*tmp));
	if (tmp == NULL)
		return 1;

	cc_list_reset(tmp);
	*self = tmp;
	return 0;
}

int cc_list_delete(cc_list_t *self)
{
	cc_list_node_t *node;

	node = self->root.next;
	while (node != &self->root) {
		if (cc_list_node_delete_and_next(&node, NULL))
			return 1;
	}

	free(self);
	return 0;
}

int cc_list_print(cc_list_t *self, int direction)
{
	cc_list_iter_t iter;
	size_t *tmp;
	size_t index;

	cc_debug_print("list content: \n");
	if (cc_list_iter_init(&iter, self, direction))
		return 1;
	while (!cc_iter_next(&iter, &tmp, &index))
		cc_debug_print("(%d)%llu, ", index, *tmp);

	cc_debug_print("\n\n");
	return 0;
}
