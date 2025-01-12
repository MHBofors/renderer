

#include "vulkan_command_buffers.h"

void create_primary_command_buffer(VkCommandBuffer *command_buffer, VkDevice logical_device, VkCommandPool command_pool, uint32_t num_buffers) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = num_buffers
    };

    if(vkAllocateCommandBuffers(logical_device, &alloc_info, command_buffer) != VK_SUCCESS) {
        error(1, "Failed to allocate command buffers\n");
    }
}



void create_secondary_command_buffer(VkCommandBuffer *command_buffer, VkDevice logical_device, VkCommandPool command_pool, uint32_t num_buffers) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = num_buffers
    };

    if(vkAllocateCommandBuffers(logical_device, &alloc_info, command_buffer) != VK_SUCCESS) {
        error(1, "Failed to allocate command buffers\n");
    }
}



void begin_command_buffer(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage) {
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = usage,
        .pInheritanceInfo = NULL
    };

    if(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        error(1, "Failed to begin recording command buffer");
    }
}

void end_command_buffer(VkCommandBuffer command_buffer) {
    if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        error(1, "Failed to record command buffer");
    }
}



void submit_command_buffer(VkQueue queue, VkCommandBuffer command_buffers) {

}



void reset_command_pool(VkDevice device, VkCommandPool command_pool) {

}
