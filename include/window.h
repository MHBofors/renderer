//
//  window.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-27.
//

#ifndef window_h
#define window_h

#include <stdio.h>
#include "dynamic_vector.h"
#include "vulkan_utils.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef GLFWwindow *window_t;

void create_surface(VkSurfaceKHR *surface, VkInstance instance, window_t window);
void get_framebuffer_size(window_t window, int *width, int *height);
void get_window_extensions(uint32_t *extension_count, const char *extensions[]);
VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR *capabilities, window_t window);

void initialise_window(window_t *window);
void window_update();
void window_wait_events();
int window_should_close(window_t window);
void terminate_window(window_t window);

#endif /* window_h */
