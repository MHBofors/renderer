//
//  renderer.h
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-27.
//
#ifndef vulkan_render_pipeline_h
#define vulkan_render_pipeline_h

#include <stdio.h>
#include "vulkan_resources.h"
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_buffers.h"

typedef struct pipeline_details_t {
    uint32_t stage_count;
    VkPipelineShaderStageCreateInfo *shader_stages;
    VkPipelineVertexInputStateCreateInfo vertex_input;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampler;
    VkPipelineDepthStencilStateCreateInfo depth_stencil;
    VkPipelineColorBlendStateCreateInfo color_blender;
    VkPipelineDynamicStateCreateInfo dynamic_state;
    VkPipelineLayout layout;
} pipeline_details_t;

void create_render_pass(VkRenderPass *render_pass, uint32_t attachment_count, VkAttachmentDescription *attachment_descriptions, VkSubpassDescription *sub_pass);

void create_render_pass_simple(VkRenderPass *render_pass, VkDevice logical_device, VkFormat image_format);

void clear_pipeline_details(pipeline_details_t *pipeline_details);

void set_input_topology(pipeline_details_t *pipeline_details, VkPrimitiveTopology topology);

void set_polygon_mode(pipeline_details_t *pipeline_details, VkPolygonMode polygon_mode);

void set_cull_mode(pipeline_details_t *pipeline_details, VkCullModeFlags cull_mode, VkFrontFace front_face);

void set_multisampler_none(pipeline_details_t *pipeline_details);

void set_color_blending_none(pipeline_details_t *pipeline_details, VkPipelineColorBlendAttachmentState *color_blend_attachment);

void set_blend_attachment_none(VkPipelineColorBlendAttachmentState *color_blend_attachment);

void set_depth_test_none(pipeline_details_t *pipeline_details);

VkPipeline create_graphics_pipeline(VkDevice logical_device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, pipeline_details_t *pipeline_details);

#endif /* vulkan_render_pipeline_h */
