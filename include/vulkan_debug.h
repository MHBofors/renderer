//
//  vulkan_debug.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-07.
//

#ifndef vulkan_debug_h
#define vulkan_debug_h

#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "vulkan_utils.h"

void create_validation_layers(VkInstanceCreateInfo *create_info, VkDebugUtilsMessengerCreateInfoEXT *debug_create_info);

void get_debug_extensions(dynamic_vector *vulkan_extension_config);

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, 
const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data);

void destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator);

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);

uint32_t check_validation_layer_support(void);

void create_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger);


#endif /* vulkan_debug_h */
