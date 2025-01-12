//
//  vulkan_swapchain.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-28.
//

#ifndef vulkan_swapchain_h
#define vulkan_swapchain_h

#include <stdio.h>
#include <vulkan/vulkan.h>

#include "vulkan_device.h"
#include "vulkan_utils.h"

VkPresentModeKHR choose_swap_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
VkSurfaceFormatKHR choose_swap_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

void create_swapchain(VkSwapchainKHR *swapchain, VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface, uint32_t image_count, VkExtent2D image_extent);
void get_swapchain_images(VkSwapchainKHR swapchain, VkDevice device, dynamic_vector *swapchain_images);

#endif