#ifndef material_h
#define material_h

#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include "vulkan_descriptors.h"
#include "graphics_matrices.h"
#include "vulkan_shader.h"
#include "vulkan_render_pipeline.h"

typedef struct material_pipeline_t {
    VkPipeline pipeline;
    VkPipelineLayout layout;
} material_pipeline_t;

typedef struct material_t {
    material_pipeline_t *material_pipeline;
    VkDescriptorSet descriptor;
} material_t;

typedef struct push_data_t {
    transformation_t model;
    float t;
} push_data_t;

typedef struct scene_data_t {
    transformation_t view;
    transformation_t projection;
} scene_data_t;

typedef struct render_object_t {
    uint32_t index_count;
    uint32_t first_index;
    VkBuffer index_buffer;
    VkBuffer *vertex_buffer;

    push_data_t push_constant;

    material_t *material_instance;
} render_object_t;

typedef struct {
    struct {
        float x, y, z;
    } position;
    struct {
        float r, g, b, alpha;
    } color;
    struct {
        float u, v;
    } texture_coordinates;
} vertex_t;

material_pipeline_t build_textured_mesh_pipeline(VkDevice logical_device, VkRenderPass render_pass, VkDescriptorSetLayout *scene_layout, VkDescriptorSetLayout *material_layout, VkExtent2D extent);

VkSampler create_linear_sampler(VkDevice logical_device);
void create_compute_pipeline_layout(VkPipelineLayout *pipeline_layout, VkDevice logical_device, VkDescriptorSetLayout layout);
void create_compute_pipeline(VkPipeline *compute_pipeline, VkPipelineLayout pipeline_layout, VkDevice logical_device, const char *file_name);

#endif