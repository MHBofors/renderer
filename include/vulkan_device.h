//
//  vulkan_device.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-06.
//

#ifndef vulkan_device_h
#define vulkan_device_h

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "vulkan_utils.h"

typedef struct {
    uint32_t graphics_family;
    uint32_t transfer_family;
    uint32_t compute_family;
} queue_family_indices;

typedef struct {
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    VkQueue compute_queue;
} device_queues;

void select_physical_device(VkPhysicalDevice *physical_device, VkInstance instance, VkSurfaceKHR surface);

uint32_t check_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface);

uint32_t optimal_queue_family(VkPhysicalDevice device, VkQueueFlagBits flag);

queue_family_indices find_queue_families(VkPhysicalDevice device);

uint32_t is_complete(queue_family_indices indices);

void create_logical_device(VkDevice *logical_device, VkPhysicalDevice physical_device, device_queues *queues, uint32_t extension_count, const char *extensions[]);

#endif /* vulkan_device_h */
