#include "functional.h"
#include "tasks.h"
#include "tests.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LENGTH 1000

// Reverse elements from old array to new array
void my_reverse(void *new, void *old)
{
	array_t *aux = (array_t *)new;
	void *new_data = malloc((aux->len + 1) * aux->elem_size);
	// Copy the original data and append it to the end of the new array
	memcpy(new_data, old, aux->elem_size);
	memcpy(new_data + aux->elem_size, aux->data, aux->len * aux->elem_size);
	free(aux->data);
	aux->data = new_data;
	aux->len++;
}

// Create a new reversed array from the original list
array_t reverse(array_t list)
{
	array_t new;
	new.len = 0;
	new.destructor = list.destructor;
	new.elem_size = list.elem_size;
	new.data = malloc(list.len * list.elem_size);
	// Copy the elements of the original array to the new array
	memcpy(new.data, list.data, list.len * list.elem_size);
	// Reverse the new array using my_reverse function
	reduce(my_reverse, &new, list);
	return new;
}

// Free memory used by number_t structure
void f_nr(void *number_ptr)
{
	number_t *number = (number_t *)number_ptr;
	if (number->string)
		free(number->string);
}

// Create number_t structure from integer and fractional parts
void c_nr(void *number_ptr, void **parts)
{
	int *integer = (int *)parts[0];
	int *fractional = (int *)parts[1];
	number_t *number = (number_t *)number_ptr;
	// Create a string representation of the number
	number->integer_part = *integer;
	number->fractional_part = *fractional;
	number->string = malloc(MAX_LENGTH * sizeof(char));
	snprintf(number->string, MAX_LENGTH, "%d.%d", *integer, *fractional);
}

// Create an array of number_t structures from integer and fractional parts
array_t create_number_array(array_t integer_part, array_t fractional_part)
{
	array_t intgr = integer_part;
	array_t fract = fractional_part;
	array_t numbers;
	// Map the integer and fractional parts into number_t structures
	numbers = map_multiple(c_nr, sizeof(number_t), f_nr, 2, intgr, fract);
	return numbers;
}

// Check if a student has passed based on grade
boolean check_passed(void *student_ptr)
{
	student_t *student = (student_t *)student_ptr;
	return student->grade >= 5.0;
}

// Get student's name and duplicate it
void get_name(void *name_ptr, void *student_ptr)
{
	student_t *student = (student_t *)student_ptr;
	*((char **)name_ptr) = strdup(student->name);
}

// Get names of students who passed
array_t get_passing_students_names(array_t list)
{
	// Filter passing students from the list
	array_t passing_students = filter(check_passed, list);
	// Map student names to an array of strings
	array_t names = map(get_name, sizeof(char **), NULL, passing_students);
	return names;
}

// Add element value to sum
void add_elem(void *sum_elem, void *elem)
{
	*(int *)sum_elem += *(int *)elem;
}

// Sum elements of a list and store in sum_elem
void add_to_sum_elem(void *sum_elem, void *list)
{
	*(int *)sum_elem = 0;  // Initialize sum to zero
	reduce(add_elem, sum_elem, *(array_t *)list);  // Sum elements
}

// Compare two sums and store result in bool_elem
void compare_sum(void *bool_elem, void **elems)
{
	*(boolean *)bool_elem = *(int *)elems[0] >= *(int *)elems[1];
}

// Check which lists have a sum greater than elements in int_list
array_t check_bigger_sum(array_t list_list, array_t int_list)
{
	return map_multiple(compare_sum, sizeof(boolean), NULL, 2,
			map(add_to_sum_elem, sizeof(int), NULL, list_list), int_list);
}

// Placeholder for getting even indexed strings
array_t get_even_indexed_strings(array_t list)
{
	(void)list;  // Avoid unused variable warning
	array_t new_list = {0, 0, 0, 0};  // Initialize empty array
	return new_list;
}

// Free memory allocated for list data
void simple_list_destructor(void *elem)
{
	free(((array_t *)elem)->data);
}

// Initialize lists based on input length
void initialise_lists(void *n, void *elem)
{
	array_t *list = (array_t *)elem;
	list->destructor = NULL;  // No destructor
	list->elem_size = sizeof(int);
	list->len = *(int *)n;  // Set list length
	*(array_t *)elem = *list;  // Update original element
}

// Create sequential indexes starting from provided value
void create_indexes(void *index, void *elem)
{
	*(int *)elem = *(int *)index;  // Set element to current index
	(*(int *)index)++;  // Increment index for next element
}

// Copy current index to list element
void copy_index(void *list_elem, void *index)
{
	*(int *)list_elem = *(int *)index;
}

// Create a matrix using lists of integers
void create_matrix(void *start_index, void **elem)
{
	array_t *list_ptr = (array_t *)elem[0];
	array_t inxs;  // Array to hold indexes
	inxs.elem_size = sizeof(int);
	inxs.len = list_ptr->len;  // Set length from list
	inxs.destructor = NULL;
	inxs.data = calloc(inxs.len, inxs.elem_size);  // Allocate memory
	int index = *(int *)start_index;  // Get starting index
	reduce(create_indexes, &index, inxs);  // Fill in indexes
	*(array_t *)(elem[0]) = map(copy_index, sizeof(int), NULL, inxs);  // Copy
	(*(int *)start_index)++;  // Increment starting index
}

// Generate a square matrix of arrays
array_t generate_square_matrix(int n)
{
	array_t list_lists;  // List of arrays
	list_lists.elem_size = sizeof(array_t);
	list_lists.len = n;  // Set number of lists
	list_lists.destructor = simple_list_destructor;
	list_lists.data = calloc(n, list_lists.elem_size);  // Allocate memory
	reduce(initialise_lists, &n, list_lists);  // Initialize lists
	int initial_index = 1;  // Starting index for matrix
	reduce_multiple(create_matrix, &initial_index, 1, list_lists);  // Create
	return list_lists;  // Return the created matrix
}
