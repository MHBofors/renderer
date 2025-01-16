//
//  vulkan_resources.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-11-06.
//

#include "vulkan_resources.h"

uint32_t select_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &device_memory_properties);

    for(uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++) {
        if(type_filter & (1 << i) && device_memory_properties.memoryTypes[i].propertyFlags & properties) {
            return i;
        }
    }

    return ~0;
}

VkImageView create_image_view(VkImage image, VkDevice logical_device, uint32_t mip_levels, VkFormat image_format, VkImageAspectFlags aspect_flags) {
    VkImageView image_view;
    VkImageSubresourceRange subresource_range = {
        .aspectMask = aspect_flags,
        .baseMipLevel = 0,
        .levelCount = mip_levels,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = image_format,
        .subresourceRange = subresource_range
    };

    vkCreateImageView(logical_device, &create_info, NULL, &image_view);
    return image_view;
}

void create_image_view2(VkImageView *image_view, VkImage image, VkDevice logical_device, uint32_t mip_levels, VkFormat image_format) {
    VkImageSubresourceRange subresource_range = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = mip_levels,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = image_format,
        .subresourceRange = subresource_range
    };

    vkCreateImageView(logical_device, &create_info, NULL, image_view);
    return;
}



void create_buffer(VkBuffer *buffer, VkDeviceMemory *buffer_memory, VkDevice logical_device, VkPhysicalDevice physical_device, VkDeviceSize device_size, VkBufferUsageFlagBits buffer_usage, VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = device_size,
        .usage = buffer_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if(vkCreateBuffer(logical_device, &buffer_create_info, NULL, buffer) != VK_SUCCESS) {
        error(1, "Failed to create buffer\n");
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(logical_device, *buffer, &memory_requirements);

    uint32_t memory_type = select_memory_type(physical_device, memory_requirements.memoryTypeBits, properties);
    if(memory_type == ~0) {
        error(1, "Failed to find suitable memory type\n");
    }

    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type
    };

    if(vkAllocateMemory(logical_device, &allocate_info, NULL, buffer_memory) != VK_SUCCESS) {
        error(1, "Failed to allocate buffer memory\n");
    }

    vkBindBufferMemory(logical_device, *buffer, *buffer_memory, 0);
}

void copy_buffer(VkBuffer dest_buffer, VkBuffer source_buffer, VkDevice logical_device, VkCommandBuffer command_buffer, VkQueue queue, VkDeviceSize size) {
    begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkBufferCopy copy_region = {
        .dstOffset = 0,
        .srcOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(command_buffer, source_buffer, dest_buffer, 1, &copy_region);
    
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}



void create_framebuffer(VkFramebuffer *framebuffer, VkDevice logical_device, VkRenderPass render_pass, uint32_t attachment_count, VkImageView *image_views, VkExtent2D extent) {
    VkFramebufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = attachment_count,
        .pAttachments = image_views,
        .width = extent.width,
        .height = extent.height,
        .layers = 1
    };

    if(vkCreateFramebuffer(logical_device, &create_info, NULL, framebuffer) != VK_SUCCESS) {
        error(1, "Failed to create framebuffer\n");
    }
}



void create_descriptor_pool(VkDescriptorPool *descriptor_pool, VkDevice logical_device, const VkDescriptorPoolSize *pool_sizes, uint32_t pool_size_count, uint32_t max_sets) {
    VkDescriptorPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = pool_size_count,
        .pPoolSizes = pool_sizes,
        .maxSets = max_sets
    };

    if(vkCreateDescriptorPool(logical_device, &create_info, NULL, descriptor_pool) != VK_SUCCESS) {
        error(1, "Failed to created descriptor pool\n");
    }
}

void allocate_descriptor_set(VkDescriptorSet *descriptor_sets, VkDevice logical_device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout *descriptor_set_layouts, uint32_t set_count) {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptor_pool,
        .descriptorSetCount = set_count,
        .pSetLayouts = descriptor_set_layouts
    };

    if(vkAllocateDescriptorSets(logical_device, &alloc_info, descriptor_sets) != VK_SUCCESS) {
        error(1, "Failed to allocate descriptor sets\n");
    }
}

void update_descriptor_set(VkDevice logical_device, VkDescriptorSet descriptor_set, const VkWriteDescriptorSet *descriptor_writes, uint32_t num_writes) {
    vkUpdateDescriptorSets(logical_device, num_writes, descriptor_writes, 0, NULL);
}

void create_descriptor_set_layout(VkDescriptorSetLayout *descriptor_set_layout, VkDevice logical_device, const VkDescriptorSetLayoutBinding *bindings, uint32_t binding_count) {
    VkDescriptorSetLayoutCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = binding_count,
        .pBindings = bindings
    };

    if(vkCreateDescriptorSetLayout(logical_device, &create_info, NULL, descriptor_set_layout) != VK_SUCCESS) {
        error(1, "Failed to create descriptor set layout\n");
    }
}



void create_command_pool(VkCommandPool *command_pool, VkDevice logical_device, uint32_t queue_index) {
    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = queue_index,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    };

    if(vkCreateCommandPool(logical_device, &create_info, NULL, command_pool) != VK_SUCCESS) {
        error(1, "Failed to create command pool\n");
    }
}