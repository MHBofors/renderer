//
//  array_t.c
//  Dynamicdata
//
//  Created by Markus Höglin on 2023-09-28.
//

#include "array.h"

struct array {
    size_t element_size;
    uint32_t element_count;
    uint32_t array_size;
    void *data;
};

//typedef enum return_type{unsuccessful_execution = -1, successful_execution = 0} return_type;

array_t *array_alloc(size_t size) {
    array_t *array_new = malloc(sizeof(array_t));
    if(array_new != NULL) {
        array_new->array_size = array_INIT_SIZE;
        array_new->element_count = 0;
        array_new->element_size = size;
        array_new->data = malloc(array_INIT_SIZE * size);

        if(array_new->data != NULL) {
            return array_new;
        } else {
            free(array_new);
            return NULL;
        }
    } else {
        return NULL;
    }
}

void array_free(array_t *array) {
    free(array->data);
    free(array);
}



uint32_t array_count(array_t *array) {
    return array->element_count;
}

uint32_t array_size(array_t *array) {
    return array->array_size;
}

uint32_t array_element_size(array_t *array) {
    return array->element_size;
}



static inline int array_resize(array_t *array, uint32_t count) {
    char *data_new = realloc(array->data, count * array->element_size);
    if(data_new != NULL) {
        array->data = data_new;
        array->array_size = count;
        return 0;
    } else {
        return -1;
    }
}



int array_add(array_t *array, void *element_new) {
    if(array->array_size == array->element_count) {
        int return_val = array_resize(array, 2 * array->element_count);
        if(return_val != 0) {
            return return_val;
        }
    }
    
    array_set(array, element_new, array->element_count++);
    return 0;
}

int array_insert(array_t *array, void *element_new, uint32_t index) {
    if(index < 0 || array->element_count < index) {
        return -1;
    } else if(array->array_size == array->element_count) {
        int return_val = array_resize(array, 2 * array->element_count);
        if(return_val != 0) {
            return return_val;
        }
    }
    
    memmove(array_get_element(array, index + 1), array_get_element(array, index), (array->element_count - index)*array_element_size(array));
    array->element_count++;
    array_set(array, element_new, index);
    return 0;
}

void array_remove(array_t *array, uint32_t index) {
    if(index < 0 || array->element_count <= index) {
        return;
    }

    memmove(array_get_element(array, index), array_get_element(array, index + 1), (array->element_count - (index + 1))*array_element_size(array));
    array->element_count--;
}

void array_set(array_t *array, void *element, uint32_t index) {
    void *p_index = array_get_element(array, index);
    if(p_index == NULL) {
        return;
    }
    else {
        memcpy(p_index, element, array->element_size);
    }
}



void *array_get_element(array_t *array, uint32_t index) {
    if(index < 0 || array->element_count < index) {
        return NULL;
    }
    else {
        return (char *)array->data + array->element_size*index;
    }
}

void *array_get_data(array_t *array) {
    return array->data;
}

int array_append(array_t *array, void *data, uint32_t count) {
    if(array->array_size < array->element_count + count) {
        uint32_t new_size = array->array_size;
        while(new_size <= array->element_count + count) {
            new_size <<= 1;
        }

        if(array_resize(array, new_size) != 0) {
            return -1;
        }
    }

    memcpy(array_get_element(array, count), data, array->element_size * count);
    return 0;
}



void *array_reserve(array_t *array, uint32_t count) {
    uint32_t new_size = array->array_size;
    while(new_size <= array->element_count + count) {
        new_size <<= 1;
    }
    if(array_resize(array, new_size) == -1) {
        return NULL;
    } else {
        uint32_t old_count = array->element_count;
        array->element_count += count;
        return array_get_element(array, old_count);
    }
}

int array_set_size(array_t *array, uint32_t count) {
    if(count <= array->array_size) {
        array->element_count = count;
        return 0;
    } else {
        uint32_t new_size = array->array_size;
        while(new_size <= count) {
            new_size <<= 1;
        }
        if(array_resize(array, new_size) == -1) {
            return -1;
        } else {
            array->element_count = count;
            return 0;
        }
    }
}

/*
int array_add_data(array_t *array, void *data, uint32_t data_count) {
    if(array->array_size < array->element_count + data_count) {
        if(array_reserve(array, array->element_count + data_count) == -1) {
            return -1;
        }
    }
    for(uint32_t i = 0; i < data_count; i++) {
        array_add(array, (char *)data + i*array->element_size);
    }
    return 0;
}

int array_reserve(array_t *array, uint32_t count) {
    uint32_t new_count = 1;
    while(new_count <= count) {
        new_count <<= 1;
    }
    
    return array_resize(array, new_count);
}
*/
