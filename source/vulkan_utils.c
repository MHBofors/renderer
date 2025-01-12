//
//  vulkan_utils.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-27.
//

#include "vulkan_utils.h"
#include "vulkan_debug.h"

#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define MAX(a,b) ((a) < (b)) ? (b) : (a)


/*
    01010101010101010101010101010101
    00110011001100110011001100110011
    00001111000011110000111100001111
    00000000111111110000000011111111
    00000000000000001111111111111111
*/

const uint32_t m1 = 0x55555555;
const uint32_t m2 = 0x33333333;
const uint32_t m4 = 0x0F0F0F0F;
const uint32_t m8 = 0x00FF00FF;
const uint32_t m16 = 0x0000FFFF;

uint32_t check_extension_support(const char **p_available, uint32_t available_count, const char **p_required, uint32_t required_count) {
    uint32_t unsupported_extension_count = 0;
    
    for(int i = 0; i < required_count; i++)
    {
        int check = 0;
        for(int j = 0; j < available_count; j++)
        {
            if(strcmp(p_required[i], p_available[j]))
            {
                check = 1;
                break;
            }
        }
        
        if(check)
        {
            printf("\t%s - supported\n", p_required[i]);
        }
        else
        {
            unsupported_extension_count++;
            printf("\t%s - not supported\n", p_required[i]);
        }
    }
    return unsupported_extension_count;
}

uint32_t hamming_weight(uint32_t n) {
    n = (n & m1) + ((n >> 1) & m1);
    n = (n & m2) + ((n >> 2) & m2);
    n = (n & m4) + ((n >> 4) & m4);
    n = (n & m8) + ((n >> 8) & m8);
    n = (n & m16) + ((n >> 16) & m16);
    return n;
}

uint32_t hamming_weight_alt(uint32_t n) {
    uint32_t weight;
    for(weight = 0; n; weight++) {
        n &= n - 1;
    }
    return n;
}

void binary_string(char string[32], uint32_t n) {
    for(uint32_t i = 0; i < 32; i++) {
        string[i] = (1 << (31 - i)) & n ? '1' : '0';
    }
}

uint32_t bound(uint32_t n, uint32_t a, uint32_t b) {
    /*
        uint32_t bounded = (a < n) ? ((n < b) ? n : b) : a;
    */
    return MAX(MIN(n, b), a);
}

void error(uint32_t error_num, const char *error_message) {
    fprintf(stderr, "%s\n", error_message);
    exit(error_num);
}