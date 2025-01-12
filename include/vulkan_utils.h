//
//  vulkan_utils.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-27.
//

#ifndef vulkan_utils_h
#define vulkan_utils_h

#include <stdio.h>
#include <stdlib.h>
#include "dynamic_vector.h"
#include "tree.h"

uint32_t check_extension_support(const char **p_available, uint32_t available_count, const char **p_required, uint32_t required_count);

void binary_string(char string[32], uint32_t n);

uint32_t hamming_weight(uint32_t n);

uint32_t bound(uint32_t n, uint32_t a, uint32_t b);

void error(uint32_t error_num, const char *error_message);

#endif /* vulkan_utils_h */
