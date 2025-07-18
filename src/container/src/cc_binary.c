#include "../include/cc_binary.h"
#include "../include/cc_common.h"
#include "../include/cc_list.h"
#include <stdarg.h>
#include <stdlib.h>

int cc_binary_insert_left(cc_binary_t *self, void *data)
{
	cc_binary_t *node;
	if (cc_binary_new(&node, self, data))
		return 1;

	if (self->left != NULL)
		self->left->parent = node;

	node->left = self->left;
	self->left = node;
	return 0;
}

int cc_binary_insert_right(cc_binary_t *self, void *data)
{
	cc_binary_t *node;
	if (cc_binary_new(&node, self, data))
		return 1;

	if (self->right != NULL)
		self->right->parent = node;

	node->right = self->right;
	self->right = node;
	return 0;
}

int cc_binary_rotate_left(cc_binary_t **start_slot)
{
	cc_binary_t *start = *start_slot;

	if (start == NULL)
		return 0;
	if (start->right == NULL)
		return 1;

	*start_slot = start->right;
	start->right->parent = start->parent;

	start->parent = start->right;

	if (start->right->left != NULL)
		start->right->left->parent = start;

	start->right = start->right->left;
	(*start_slot)->left = start;

	return 0;
}

int cc_binary_rotate_right(cc_binary_t **start_slot)
{
	cc_binary_t *start = *start_slot;

	if (start == NULL)
		return 0;
	if (start->left == NULL)
		return 1;

	*start_slot = start->left;
	start->left->parent = start->parent;

	start->parent = start->left;

	if (start->left->right != NULL)
		start->left->right->parent = start;

	start->left = start->left->right;
	(*start_slot)->right = start;

	return 0;
}

int cc_binary_new(cc_binary_t **self, cc_binary_t *parent,
		void *data)
{
	cc_binary_t *tmp;
	tmp = cc_alloc(sizeof(*tmp));
	if (tmp == NULL)
		return 1;

	tmp->parent = parent;
	tmp->data = data;
	tmp->left = NULL;
	tmp->right = NULL;

	*self = tmp;
	return 0;
}

int cc_binary_delete(cc_binary_t *self)
{
	if (self == NULL)
		return 0;
	if (cc_binary_delete(self->left))
		return 1;
	if (cc_binary_delete(self->right))
		return 2;

	free(self);
	return 0;
}

int cc_binary_print(cc_binary_t *current, int depth,
		cc_simple_fn_1_t print_fn)
{
	int tmp = 0;

	cc_print_n("\t", depth);
	if (current == NULL) {
		cc_debug_print("<NULL>\n");
		return 0;
	}

	print_fn(current->data);
	cc_debug_print("\n");

	tmp |= cc_binary_print(current->right, depth + 1, print_fn);
	tmp |= cc_binary_print(current->left, depth + 1, print_fn);

	return tmp;
}

static cc_iter_i_t iterator_interface = {
	.next = (cc_iter_next_fn_t)cc_binary_iter_next,
};

static int iter_queue_add(cc_binary_iter_t *self, void *data)
{
	if (data == NULL)
		return 0;
	if (self->direction == CC_TRAVERSE_DEPTH_LEFT ||
			self->direction == CC_TRAVERSE_DEPTH_RIGHT)
		return cc_list_insert_head(self->queue, data);
	else
		return cc_list_insert_tail(self->queue, data);
}

static int iter_queue_add_multi(cc_binary_iter_t *self, int n, ...)
{
	va_list args;
	va_start(args, n);
	while (n-- > 0) {
		if (iter_queue_add(self, va_arg(args, cc_binary_t *)))
			return 1;
	}
	va_end(args);
	return 0;
}

static int iter_queue_add_child(cc_binary_iter_t *self,
		cc_binary_t *node)
{
	int tmp = 0;

	if (node == NULL)
		return 0;

	if (self->direction == CC_TRAVERSE_DEPTH_LEFT ||
			self->direction == CC_TRAVERSE_BREADTH_RIGHT)
		tmp = iter_queue_add_multi(self, 2, node->right, node->left);
	else if (self->direction == CC_TRAVERSE_DEPTH_RIGHT ||
			self->direction == CC_TRAVERSE_BREADTH_LEFT)
		tmp = iter_queue_add_multi(self, 2, node->left, node->right);
	else
		tmp = 2;

	return tmp;
}

int cc_binary_iter_next(cc_binary_iter_t *self, void **item,
		size_t *index)
{
	cc_binary_t *current;

	if (try_reset_double_p(item))
		return 1;
	if (cc_list_remove_head(self->queue, (void **)&current))
		return 2;

	*item = &current->data;

	if (iter_queue_add_child(self, current))
		return 3;

	if (index != NULL)
		*index = self->index;

	self->index++;
	return 0;
}

int cc_binary_iter_new(cc_binary_iter_t **self, cc_binary_t *root,
		enum cc_traverse_direction direction)
{
	cc_binary_iter_t *tmp;

	if (root == NULL)
		goto fail1;

	tmp = cc_alloc(sizeof(*tmp));
	if (tmp == NULL)
		goto fail1;

	if (cc_list_new(&tmp->queue))
		goto fail2;
	if (cc_list_insert_tail(tmp->queue, root))
		goto fail3;

	tmp->iterator = &iterator_interface;
	tmp->index = 0;
	tmp->direction = direction;

	*self = tmp;
	return 0;

fail3:
	cc_list_delete(tmp->queue);
fail2:
	free(tmp);
fail1:
	return 1;
}

int cc_binary_iter_delete(cc_binary_iter_t *self)
{
	int tmp = 0;
	if (cc_list_delete(self->queue))
		tmp = 1;

	free(self);
	return tmp;
}
