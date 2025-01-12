//
//  vulkan_debug.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-07.
//

#include "vulkan_debug.h"

#ifndef NDEBUG
    const uint32_t enable_validation_layers = 1;
#else
    const uint32_t enable_validation_layers = 0;
#endif

const uint32_t validation_layer_count = 1;
const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};

const uint32_t debug_extension_count = 1;
const char *debug_extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

void create_validation_layers(VkInstanceCreateInfo *create_info, VkDebugUtilsMessengerCreateInfoEXT *debug_create_info) {
    if(enable_validation_layers) {
        populate_debug_messenger_create_info(debug_create_info);
        
        create_info->enabledLayerCount = validation_layer_count;
        create_info->ppEnabledLayerNames = validation_layers;
        create_info->pNext = (VkDebugUtilsMessengerCreateInfoEXT*) debug_create_info;
    } else {
        create_info->enabledLayerCount = 0;
        create_info->ppEnabledLayerNames = NULL;
        create_info->pNext = NULL;
    }
}

void get_debug_extensions(dynamic_vector *vulkan_extension_config) {
    const uint32_t debug_extension_count = 1;
    const char *debug_extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    for(size_t i = 0; i < debug_extension_count; i++) {
        vector_add(vulkan_extension_config, debug_extensions + i);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
void* p_user_data) {
    printf("\nValidation layer: %s\n", p_callback_data->pMessage);
    return VK_FALSE;
}

VkResult create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, p_create_info, p_allocator, p_debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debug_messenger, p_allocator);
    }
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *p_create_info) {
    p_create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    p_create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    p_create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    p_create_info->pfnUserCallback = debug_callback;
}

uint32_t check_validation_layer_support(void)
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties available_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for(int i = 0; i < validation_layer_count; i++) {
        uint32_t check = 0;

        for(int j = 0; j < layer_count; j++) {
            if(strcmp(available_layers[j].layerName, validation_layers[i])) {
                check = 1;
                break;
            }
        }
        
        if(!check) {
            return 0;
        }
    }
    
    return 1;
}

void create_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT *debug_messenger) {
    if(!enable_validation_layers) {
        return;
    }
    
    VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
    populate_debug_messenger_create_info(&create_info);

    if (create_debug_utils_messenger_EXT(instance, &create_info, NULL, debug_messenger) != VK_SUCCESS) {
        perror("Failed to set up debug messenger!");
    }
}
