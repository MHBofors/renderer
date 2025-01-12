


#ifndef vulkan_command_buffers_h
#define vulkan_command_buffers_h

#include <vulkan/vulkan.h>
#include "vulkan_utils.h"

void create_primary_command_buffer(VkCommandBuffer *command_buffer, VkDevice logical_device, VkCommandPool command_pool, uint32_t num_buffers);
void create_secondary_command_buffer(VkCommandBuffer *command_buffer, VkDevice logical_device, VkCommandPool command_pool, uint32_t num_buffers);

void begin_command_buffer(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage);
void end_command_buffer(VkCommandBuffer command_buffer);

void submit_command_buffer(VkQueue queue, VkCommandBuffer command_buffer);
void reset_command_pool(VkDevice device, VkCommandPool command_pool);
    


#endif