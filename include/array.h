//
//  array_t.h
//  Dynamicarray_t
//
//  Created by Markus Höglin on 2023-09-28.
//

#ifndef array_h
#define array_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define array_INIT_SIZE 8
#define WORD_SIZE 8

typedef struct array array_t;

array_t *array_alloc(size_t size);
void array_free(array_t *array);

uint32_t array_count(array_t *array);
uint32_t array_size(array_t *array);
uint32_t array_element_size(array_t *array);

static inline int array_resize(array_t *array, uint32_t count);
void *array_reserve(array_t *array, uint32_t count);
int array_set_size(array_t *array, uint32_t count);

int array_add(array_t *array, void *element_new);
int array_insert(array_t *array, void *element_new, uint32_t index);
int array_append(array_t *array, void *data, uint32_t count);
void array_remove(array_t *array, uint32_t index);

void array_set(array_t *array, void *element, uint32_t index);
void *array_get_element(array_t *array, uint32_t index);

void *array_get_data(array_t *array);



#endif /* array_t_h */
