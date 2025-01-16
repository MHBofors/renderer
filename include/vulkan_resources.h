//
//  vulkan_resources.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-06.
//

#ifndef vulkan_resources_h
#define vulkan_resources_h

#include <stdio.h>
#include <vulkan/vulkan.h>
#include "vulkan_command_buffers.h"
#include "vulkan_utils.h"

uint32_t select_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);


VkImageView create_image_view(VkImage image, VkDevice logical_device, uint32_t mip_levels, VkFormat image_format, VkImageAspectFlags aspect_flags);
void create_image_view2(VkImageView *image_view, VkImage image, VkDevice logical_device, uint32_t mip_levels, VkFormat image_format);

void create_buffer(VkBuffer *buffer, VkDeviceMemory *buffer_memory, VkDevice logical_device, VkPhysicalDevice physical_device, VkDeviceSize device_size, VkBufferUsageFlagBits buffer_usage, VkMemoryPropertyFlags properties);
void copy_buffer(VkBuffer dest_buffer, VkBuffer source_buffer, VkDevice logical_device, VkCommandBuffer command_buffer, VkQueue queue, VkDeviceSize size);

void create_framebuffer(VkFramebuffer *framebuffer, VkDevice logical_device, VkRenderPass render_pass, uint32_t attachment_count, VkImageView *image_views, VkExtent2D extent);

void create_descriptor_pool(VkDescriptorPool *descriptor_pool, VkDevice logical_device, const VkDescriptorPoolSize *pool_sizes, uint32_t num_pool_size, uint32_t max_sets);
void allocate_descriptor_set(VkDescriptorSet *descriptor_sets, VkDevice logical_device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout *descriptor_set_layouts, uint32_t num_sets);
void update_descriptor_set(VkDevice logical_device, VkDescriptorSet descriptor_set, const VkWriteDescriptorSet *descriptor_write, uint32_t num_writes);
void create_descriptor_set_layout(VkDescriptorSetLayout *descriptor_set_layout, VkDevice logical_device, const VkDescriptorSetLayoutBinding *bindings, uint32_t binding_count);

void create_command_pool(VkCommandPool *command_pool, VkDevice logical_device, uint32_t queue_index);

#endif /* vulkan_resources_h */
