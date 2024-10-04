#include "functional.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

void for_each(void (*func)(void *), array_t list)
{
	// Initialize a pointer to the first element in the array
	void *ptr = list.data;
	int i;
	// Iterate through each element in the array and apply the function to it
	for (i = 0; i < list.len; i++) {
		func(ptr);
		ptr += list.elem_size;
		// Advance the pointer to the next element
	}
}

array_t map(void (*func)(void *, void *),
			int new_list_elem_size,
			void (*new_list_destructor)(void *),
			array_t list)
{	// Allocate memory for the new resulting array
	void *new_data = malloc(list.len * new_list_elem_size);
	void *src_ptr = list.data;
	void *dest_ptr = new_data;
	int i;
	// Iterate through each element in the original array and
	// apply the function to it, storing the result in the new array
	for (i = 0; i < list.len; i++) {
		func(dest_ptr, src_ptr);
		// Advance the pointer to the next element in the original array
		src_ptr += list.elem_size;
		// Advance the pointer to the next element in the new array
		dest_ptr += new_list_elem_size;
	}
	// Free the memory allocated for the original array
	if (list.destructor) {
		src_ptr = list.data;
		for (i = 0; i < list.len; i++) {
			list.destructor(src_ptr);
			src_ptr += list.elem_size;
		}
	}
	// Construct and return the resulting array
	free(list.data);
	array_t new_array;
	new_array.data = new_data;
	new_array.elem_size = new_list_elem_size;
	new_array.len = list.len;
	new_array.destructor = new_list_destructor;
	return new_array;
}

array_t filter(boolean(*func)(void *), array_t list)
{
	// Allocate memory for the resulting array
	void *f_data = malloc(list.len * list.elem_size);
	void *src_ptr = list.data;
	void *dest_ptr = f_data;
	int f_len = 0, i;
	// Iterate through each element in the original array and
	// copy only those elements that pass the specified test
	for (i = 0; i < list.len; i++) {
		if (func(src_ptr)) {
			memcpy(dest_ptr, src_ptr, list.elem_size);
			dest_ptr += list.elem_size;
			f_len++;
		}
		src_ptr += list.elem_size;
	}
	free(list.data);
	// Construct and return the resulting array
	array_t f_list = {f_data, list.elem_size, f_len, list.destructor};
	return f_list;
}

void *reduce(void (*func)(void *, void *), void *acc, array_t list)
{
	// Initialize a pointer to the first element in the array
	void *ptr = list.data;
	int i;
	// Iterate through each element in the array and apply a function to it
	for (i = 0; i < list.len; i++) {
		func(acc, ptr);
		// Advance the pointer to the next element
		ptr += list.elem_size;
	}
	return acc;
}

void for_each_multiple(void(*func)(void **), int varg_c, ...)
{
	va_list args;
	va_start(args, varg_c);
	int i, j, min_len = INT_MAX;
	// Determine the minimum length among the arrays
	for (i = 0; i < varg_c; i++) {
		array_t array = va_arg(args, array_t);
		if (array.len < min_len)
			min_len = array.len;
	}
	// Iterate through each element index up to the minimum length
	for (i = 0; i < min_len; i++) {
		va_start(args, varg_c);
		void *elements[varg_c];
		// Populate the elements array with pointers to
		// corresponding elements in each array
		for (j = 0; j < varg_c; j++) {
			array_t array = va_arg(args, array_t);
			elements[j] = array.data + (i * array.elem_size);
		}
		// Apply the function to the set of elements
		func(elements);
	}
	va_end(args);
}

array_t map_multiple(void (*func)(void *, void **),
					 int new_list_elem_size,
					 void (*new_list_destructor)(void *),
					 int varg_c, ...)
{
	int i, j, min_len = INT_MAX;
	va_list args;
	va_start(args, varg_c);
	array_t *arrays = (array_t *)malloc(varg_c * sizeof(array_t));
	// Collect information about the arrays and determine the minimum length
	for (i = 0; i < varg_c; i++)
		arrays[i] = va_arg(args, array_t);
	for (i = 0; i < varg_c; i++) {
		if (arrays[i].len < min_len)
			min_len = arrays[i].len;
	}
	va_end(args);
	void *new_data = malloc(min_len * new_list_elem_size);
	// Apply the function to each set of elements and construct the new array
	for (i = 0; i < min_len; i++) {
		void *elements[varg_c];
		for (j = 0; j < varg_c; j++)
			elements[j] = arrays[j].data + (i * arrays[j].elem_size);
		func(new_data + (i * new_list_elem_size), elements);
		// Clean up memory for each array's elements
		for (j = 0; j < varg_c; j++) {
			if (arrays[j].destructor)
				arrays[j].destructor(arrays[j].data + i * arrays[j].elem_size);
		}
	}
	// Free memory allocated for the arrays
	for (j = 0; j < varg_c; j++)
		free(arrays[j].data);
	free(arrays);
	// Construct and return the resulting array
	array_t new_array;
	new_array.data = new_data;
	new_array.elem_size = new_list_elem_size;
	new_array.len = min_len;
	new_array.destructor = new_list_destructor;
	return new_array;
}

void *reduce_multiple(void(*func)(void *, void **), void *acc, int varg_c, ...)
{
	int i, j, min_len = INT_MAX;
	va_list args;
	va_start(args, varg_c);
	array_t arrays[varg_c];
	// Collect information about the arrays and determine the minimum length
	for (i = 0; i < varg_c; i++)
		arrays[i] = va_arg(args, array_t);
	va_end(args);
	for (i = 0; i < varg_c; i++) {
		if (arrays[i].len < min_len)
			min_len = arrays[i].len;
	}
	// Apply the reduction function to each set of elements
	for (i = 0; i < min_len; i++) {
		void *elements[varg_c];
		for (j = 0; j < varg_c; j++)
			elements[j] = arrays[j].data + (i * arrays[j].elem_size);
		func(acc, elements);
	}
	return acc;
}
