//
//  vulkan_device.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-06.
//

#include "vulkan_device.h"

const char required_device_extension_count = 2;
const char *required_device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_synchronization2"};

enum queue_family_flag_bits{GRAPHICS_FAMILY_BIT, PRESENT_FAMILY_BIT, QUEUE_FAMILY_FLAG_COUNT};

const VkQueueFlagBits queue_family_bits = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT;

void select_physical_device(VkPhysicalDevice *physical_device, VkInstance instance, VkSurfaceKHR surface) {
    *physical_device = VK_NULL_HANDLE;
    
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    
    if(device_count == 0) {
        printf("Failed to find GPU with Vulkan support!\n");
        exit(1);
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    printf("\nAvailable devices:\n");
    for(uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(devices[i], &device_properties);
        
        printf("\t%s\n", device_properties.deviceName);
    }

    for(int i = 0; i < device_count; i++) {
        if(check_device_suitability(devices[i], surface)) {
            *physical_device = devices[i];
            break;
        }
    }

    if(*physical_device == VK_NULL_HANDLE) {
        printf("Failed to find suitable GPU\n");
        exit(1);
    }
}

uint32_t check_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);
    
    queue_family_indices indices = find_queue_families(device);
    
    uint32_t available_extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, NULL);
    
    VkExtensionProperties available_extensions[available_extension_count];
    vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, available_extensions);
    
    const char *available_extension_names[available_extension_count];
    for(int i = 0; i < available_extension_count; i++) {
        available_extension_names[i] = available_extensions[i].extensionName;
    }
    
    printf("\nRequired device extensions:\n");
    if(check_extension_support(available_extension_names, available_extension_count, required_device_extensions, required_device_extension_count)) {
        return 0;
    }
    
    return 1;
}

/*
    Fix automatic queue selection
*/

uint32_t optimal_queue_family(VkPhysicalDevice device, VkQueueFlagBits flag) {
    uint32_t best_index = ~0;
    uint32_t best_fit = ~0;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    for(uint32_t i = 0; i < queue_family_count; i++) {
        if(queue_families[i].queueFlags & flag) {
            printf("%d, %d\n", i, hamming_weight(queue_families[i].queueFlags & (queue_family_bits ^ flag)));
            if(hamming_weight(queue_families[i].queueFlags) < best_fit) {
                best_index = i;
                best_fit = hamming_weight(queue_families[i].queueFlags);
            }
        }
    }

    return best_index;
}

/*Fix -> select dedicated family if possible, otherwise choose available*/
queue_family_indices find_queue_families(VkPhysicalDevice device) {
    /*
        Enumerate queue families & which queues are supported?
    */
    queue_family_indices indices = {
        .graphics_family = 0,
        .transfer_family = 1,
        .compute_family = 2
    };
    
    if(!is_complete(indices)) {
        error(1, "Unsupported Queues");
    }
    return indices;
}

uint32_t is_complete(queue_family_indices indices) {
    return (indices.graphics_family != ~0
          && indices.transfer_family != ~0
          && indices.compute_family != ~0);
}



void create_logical_device(VkDevice *logical_device, VkPhysicalDevice physical_device, device_queues *queues, uint32_t extension_count, const char *extensions[]) {
    binary_tree *queue_tree;
    dynamic_vector *value_vector = vector_alloc(sizeof(uint32_t));

    tree_alloc(&queue_tree);
    
    tree_free(queue_tree);
    vector_free(value_vector);

    
    queue_family_indices indices = find_queue_families(physical_device);
    
    float queue_priority = 1.0f;
    uint32_t queue_count = 3;
    uint32_t unique_indices[queue_count];

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

    unique_indices[0] = indices.graphics_family;
    unique_indices[1] = indices.transfer_family;
    unique_indices[2] = indices.compute_family;

    VkDeviceQueueCreateInfo queue_create_infos[queue_count];

    for(uint32_t i = 0; i < queue_count; i++) {
        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = unique_indices[i],
            .queueCount = 1,
            .pQueuePriorities = &queue_priority
        };
        queue_create_infos[i] = queue_create_info;
    }

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos,
        .queueCreateInfoCount = queue_count,
        .pEnabledFeatures = &device_features,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions
    };

    if(vkCreateDevice(physical_device, &create_info, NULL, logical_device) != VK_SUCCESS) {
        printf("Failed to create logical device!\n");
        exit(1);
    }

    vkGetDeviceQueue(*logical_device, indices.graphics_family, 0, &queues->graphics_queue);
    vkGetDeviceQueue(*logical_device, indices.transfer_family, 0, &queues->transfer_queue);
    vkGetDeviceQueue(*logical_device, indices.compute_family, 0, &queues->compute_queue);
}

