#include "../include/cc_array.h"
#include <stdlib.h>
#include <string.h>

cc_iter_i_t cc_array_iter_interface = {
	.next = (cc_iter_next_fn_t)cc_array_iter_next,
};

static inline void cc_array_get_(cc_array_t *self, size_t index,
		void *result)
{
	memcpy(result, self->data + index * self->elem_size, self->elem_size);
}

static inline void cc_array_get_ref_(cc_array_t *self, size_t index,
		void **ref)
{
	*ref = self->data + index * self->elem_size;
}

static inline void cc_array_set_(cc_array_t *self, size_t index,
		void *value)
{
	memcpy(self->data + index * self->elem_size, value, self->elem_size);
}

void cc_array_swap(cc_array_t *self, size_t i, size_t j)
{
	unsigned char tmp[self->elem_size];
	cc_array_get_(self, i, tmp);
	memcpy(self->data + i * self->elem_size,
			self->data + j * self->elem_size, self->elem_size);
	cc_array_set_(self, j, tmp);
}

int cc_array_cmp(cc_array_t *self, cc_cmp_fn_t cmp, size_t i, size_t j)
{
	return cmp(self->data + i * self->elem_size,
			self->data + j * self->elem_size);
}

int cc_array_is_valid_index(cc_array_t *self, size_t index)
{
	return index < self->elem_nums;
}

void cc_array_get_unsafe(cc_array_t *self, size_t index, void *result)
{
	cc_array_get_(self, index, result);
}

void cc_array_get_ref_unsafe(cc_array_t *self, size_t index, void **ref)
{
	cc_array_get_ref_(self, index, ref);
}

int cc_array_get(cc_array_t *self, size_t index, void *result)
{
	if (result == NULL)
		return 1;
	if (index >= self->elem_nums)
		return CC_ARRAY_OUT_OF_RANGE;

	cc_array_get_(self, index, result);
	return 0;
}

int cc_array_get_ref(cc_array_t *self, size_t index, void **ref)
{
	if (try_reset_double_p(ref))
		return 1;
	if (index >= self->elem_nums)
		return CC_ARRAY_OUT_OF_RANGE;

	cc_array_get_ref_(self, index, ref);
	return 0;
}

void cc_array_set_unsafe(cc_array_t *self, size_t index, void *value)
{
	cc_array_set_(self, index, value);
}

int cc_array_set(cc_array_t *self, size_t index, void *value)
{
	if (index >= self->elem_nums)
		return CC_ARRAY_OUT_OF_RANGE;

	cc_array_set_(self, index, value);
	return 0;
}

int cc_array_reverse(cc_array_t *self, size_t start, size_t end)
{
	size_t middle, i;

	if (start == end)
		return 1;
	if (start >= self->elem_nums)
		return CC_ARRAY_OUT_OF_RANGE;
	if (end > self->elem_nums)
		end = self->elem_nums;

	middle = (end - start) / 2;
	for (i = 0; i < middle; i++)
		cc_array_swap(self, start + i, end - 1 - i);

	return 0;
}

int cc_array_init(cc_array_t *self, unsigned char *data, size_t elem_nums,
		size_t elem_size)
{
	self->elem_nums = elem_nums;
	self->elem_size = elem_size;
	self->data = data;
	return 0;
}

int cc_array_new(cc_array_t **self, size_t elem_nums, size_t elem_size)
{
	cc_array_t *tmp;
	unsigned char *data;

	tmp = cc_alloc(sizeof(*tmp));
	if (tmp == NULL)
		goto fail1;

	data = cc_alloc(elem_nums * elem_size);
	if (data == NULL)
		goto fail2;
	if (cc_array_init(tmp, data, elem_nums, elem_size))
		goto fail3;

	*self = tmp;
	return 0;

fail3:
	free(data);
fail2:
	free(tmp);
fail1:
	return 1;
}

int cc_array_delete_keep_data(cc_array_t *self)
{
	free(self);
	return 0;
}

int cc_array_delete(cc_array_t *self)
{
	free(self->data);
	free(self);
	return 0;
}

int cc_array_iter_next(cc_array_iter_t*self, void **item, size_t *index)
{
	if (try_reset_double_p(item))
		return 1;
	if (cc_array_get_ref(self->data, self->cursor, item))
		return 2;

	if (index != NULL)
		*index = self->cursor;

	self->cursor++;
	return 0;
}

int cc_array_iter_init(cc_array_iter_t*self, cc_array_t *data)
{
	if (data == NULL)
		return 1;

	self->iterator = &cc_array_iter_interface;
	self->data = data;
	self->cursor = 0;
	return 0;
}
