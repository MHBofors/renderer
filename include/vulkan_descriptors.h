#ifndef vulkan_descriptors_h
#define vulkan_descriptors_h


#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "vulkan_utils.h"
#include "vulkan_resources.h"

typedef struct descriptor_manager_t descriptor_manager_t;

typedef struct descriptor_layout_builder_t {
    uint32_t array_size;
    uint32_t binding_count;
    VkDescriptorSetLayoutBinding *bindings;
} descriptor_layout_builder_t;

typedef struct descriptor_allocator_t {
    uint32_t array_size;
    uint32_t pool_count;
    uint32_t free_count;
    VkDescriptorPool *free_pools;
    VkDescriptorPool *full_pools;

    uint32_t max_sets;

    uint32_t type_count;
    VkDescriptorType *types;
    uint32_t *type_ratios;
} descriptor_allocator_t;

typedef struct descriptor_writer_t {
    uint32_t write_array_size;
    uint32_t write_count;
    VkWriteDescriptorSet *writes;
} descriptor_writer_t;

descriptor_layout_builder_t initialise_layout_builder();
void free_layout_builder(descriptor_layout_builder_t *layout_builder);

void add_binding(descriptor_layout_builder_t *layout_builder, uint32_t binding, VkDescriptorType type, VkShaderStageFlags shader_stage);
void clear_bindings(descriptor_layout_builder_t *layout_builder);

VkDescriptorSetLayout build_layout(descriptor_layout_builder_t *layout_builder, VkDevice logical_device);



descriptor_allocator_t initialise_allocator(uint32_t type_count, VkDescriptorType *types, uint32_t *type_ratios);
void free_allocator(descriptor_allocator_t *allocator);

void create_pool(descriptor_allocator_t *allocator);
void clear_pools(descriptor_allocator_t *allocator);

VkDescriptorSet allocate_set(descriptor_allocator_t *allocator, VkDescriptorSetLayout layout);



descriptor_writer_t initialise_writer();
void free_writer(descriptor_writer_t *descriptor_writer);

void write_image(descriptor_writer_t *descriptor_writer, uint32_t binding, VkDescriptorType type, VkImageView image_view, VkImageLayout layout);
void write_sampler(descriptor_writer_t *descriptor_writer, uint32_t binding, VkSampler sampler);
void write_buffer(descriptor_writer_t *descriptor_writer, uint32_t binding, VkDescriptorType type, VkBuffer buffer, size_t size, size_t offset);
void update_set(descriptor_writer_t *descriptor_writer, VkDevice logical_device, VkDescriptorSet set);
void clear_writes(descriptor_writer_t *descriptor_writer);

#endif /* vulkan_descriptors_h */