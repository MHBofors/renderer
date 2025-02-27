//
//  vulkan_swapchain.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-28.
//

#include "vulkan_swapchain.h"

void query_surface_present_modes(VkPhysicalDevice device, VkSurfaceKHR surface, dynamic_vector *present_modes) {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    uint32_t format_count = 0;
    uint32_t present_mode_count = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

    VkPresentModeKHR present_mode_array[present_mode_count];

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, present_mode_array);

    for(uint32_t i = 0; i < present_mode_count; i++) {
        vector_add(present_modes, present_mode_array + i);
    }
}

void query_surface_formats(VkPhysicalDevice device, VkSurfaceKHR surface, dynamic_vector *surface_formats) {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    uint32_t format_count = 0;
    uint32_t present_mode_count = 0;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surface_capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

    VkSurfaceFormatKHR formats[format_count];
    VkPresentModeKHR modes[present_mode_count];

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, formats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, modes);

    for(uint32_t i = 0; i < present_mode_count; i++) {
        vector_add(surface_formats, formats + i);
    }

}



VkSurfaceFormatKHR choose_swap_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t surface_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL);

    if(surface_format_count != 0) {
        VkSurfaceFormatKHR surface_formats[surface_format_count];
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats);

        for(uint32_t i = 0; i < surface_format_count; i++) {
            if(surface_formats[i].format == VK_FORMAT_R8G8B8A8_SRGB && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return surface_formats[i];
            }
        }

        return surface_formats[0];
    } else {
        printf("No surface formats found!\n");
        exit(1);
    }
}

VkPresentModeKHR choose_swap_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);

    VkPresentModeKHR present_modes[present_mode_count];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);
    
    for (uint32_t i = 0; i < present_mode_count; i++) {
        if(present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;//Selects how the swap chain displays images to the screen, VK_PRESENT_MODE_FIFO_KHR -> display takes images in front of queue, inserts rendered images to the back
}



void create_swapchain(VkSwapchainKHR *swapchain, VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface, uint32_t image_count, VkExtent2D image_extent) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    VkPresentModeKHR present_mode = choose_swap_present_mode(physical_device, surface);
    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(physical_device, surface);
    
    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,//Specifies what kinds of operations the images will be used for
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = image_extent,
        .imageArrayLayers = 1,
        .presentMode = present_mode,
        .preTransform = capabilities.currentTransform,//Can specify transforms to apply to images in swap chain
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,//Opaque -> alpha channel should not be used to blend with other windows
        .clipped = VK_TRUE,//True -> we don't care about color of obscured pixel
        .oldSwapchain = VK_NULL_HANDLE//If swap chain must be replaced, then reference to old one must be given here
    };
    
    queue_family_indices indices = find_queue_families(physical_device);

    if(!((indices.graphics_family == indices.transfer_family) && (indices.graphics_family == indices.compute_family))) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//Several families can use images without transfer of membership
        create_info.queueFamilyIndexCount = 3;
        create_info.pQueueFamilyIndices = (uint32_t *)&indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//Only one family can use images at a time
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = NULL; // Optional
    }
    
    if (vkCreateSwapchainKHR(device, &create_info, NULL, swapchain) != VK_SUCCESS) {
        printf("Failed to create swap chain!\n");
        exit(1);
    }
}

void get_swapchain_images(VkSwapchainKHR swapchain, VkDevice device, dynamic_vector *swapchain_images) {
    uint32_t image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, vector_reserve(swapchain_images, image_count));
}
