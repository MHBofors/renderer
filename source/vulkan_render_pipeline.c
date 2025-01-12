//
//  vulkan_render_pipeline.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-27.
//

#include "vulkan_render_pipeline.h"

#define DEFAULT_LINEWIDTH (1.0f)

void create_render_pass_simple(VkRenderPass *render_pass, VkDevice logical_device, VkFormat image_format) {
    VkAttachmentDescription color_attachment = {
        .format = image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,//Specifies what to do with the data in the attachment before and after rendering
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    
    VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref
    };
    
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,//Specifies the indices of the dependency in the dependent subpass
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,//Specifies the operations to wait for, and in which stage they occur
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,//Specifies the operations waiting for the previous operations, and in which stage they occur
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    
    VkRenderPassCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };
    
    if (vkCreateRenderPass(logical_device, &create_info, NULL, render_pass) != VK_SUCCESS) {
        printf("Failed to create render pass\n");
        exit(1);
    }
}

void clear_pipeline_details(pipeline_details_t *pipeline_details) {
    *pipeline_details = (pipeline_details_t){
        .stage_count     = 0,
        .shader_stages  = NULL,
        .vertex_input   = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO},
        .input_assembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO},
        .viewport       = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO},
        .rasterizer     = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO},
        .multisampler   = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO},
        .depth_stencil  = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO},
        .color_blender  = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO},
        .dynamic_state  = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO},
        .layout         = {0}
    };
}

void set_input_topology(pipeline_details_t *pipeline_details, VkPrimitiveTopology topology) {
    pipeline_details->input_assembly.topology = topology;
    pipeline_details->input_assembly.primitiveRestartEnable = VK_FALSE;
}

void set_polygon_mode(pipeline_details_t *pipeline_details, VkPolygonMode polygon_mode) {
    pipeline_details->rasterizer.polygonMode = polygon_mode;
    pipeline_details->rasterizer.lineWidth = DEFAULT_LINEWIDTH;
}

void set_cull_mode(pipeline_details_t *pipeline_details, VkCullModeFlags cull_mode, VkFrontFace front_face) {
    pipeline_details->rasterizer.cullMode = cull_mode;
    pipeline_details->rasterizer.frontFace = front_face;
}

void set_multisampler_none(pipeline_details_t *pipeline_details) {
    pipeline_details->multisampler.alphaToCoverageEnable = VK_FALSE;
    pipeline_details->multisampler.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_details->multisampler.minSampleShading = 1.0f;
    pipeline_details->multisampler.pSampleMask = NULL;
    pipeline_details->multisampler.alphaToCoverageEnable = VK_FALSE;
    pipeline_details->multisampler.alphaToOneEnable = VK_FALSE;
}

void set_color_blending_none(pipeline_details_t *pipeline_details, VkPipelineColorBlendAttachmentState *color_blend_attachment) {
    pipeline_details->color_blender.pNext = NULL;
    pipeline_details->color_blender.logicOpEnable = VK_FALSE;
    pipeline_details->color_blender.logicOp = VK_LOGIC_OP_COPY;
    pipeline_details->color_blender.attachmentCount = 1;
    pipeline_details->color_blender.pAttachments = color_blend_attachment;
}

void set_blend_attachment_none(VkPipelineColorBlendAttachmentState *color_blend_attachment) {
    *color_blend_attachment = (VkPipelineColorBlendAttachmentState){
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE
    };
}

void set_depth_test_none(pipeline_details_t *pipeline_details) {
    pipeline_details->depth_stencil.depthTestEnable = VK_FALSE;
    pipeline_details->depth_stencil.depthWriteEnable = VK_FALSE;
    pipeline_details->depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    pipeline_details->depth_stencil.depthBoundsTestEnable = VK_FALSE;
    pipeline_details->depth_stencil.stencilTestEnable = VK_FALSE;
    pipeline_details->depth_stencil.front = (VkStencilOpState){};
    pipeline_details->depth_stencil.back = (VkStencilOpState){};
    pipeline_details->depth_stencil.minDepthBounds = 0.0f;
    pipeline_details->depth_stencil.maxDepthBounds = 1.0f;
}

VkPipeline create_graphics_pipeline(VkDevice logical_device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, pipeline_details_t *pipeline_details) {
    VkPipeline graphics_pipeline;
    
    VkGraphicsPipelineCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = pipeline_details->stage_count,
        .pStages = pipeline_details->shader_stages,
        .pVertexInputState = &pipeline_details->vertex_input,
        .pInputAssemblyState = &pipeline_details->input_assembly,
        .pViewportState = &pipeline_details->viewport,
        .pRasterizationState = &pipeline_details->rasterizer,
        .pMultisampleState = &pipeline_details->multisampler,
        .pDepthStencilState = &pipeline_details->depth_stencil,
        .pColorBlendState = &pipeline_details->color_blender,
        .pDynamicState = &pipeline_details->dynamic_state,
        .layout = pipeline_layout,
        .renderPass = render_pass,
        .subpass = 0,
        /* Used if pipeline is recreated */
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if(vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &create_info, NULL, &graphics_pipeline) != VK_SUCCESS) {
        error(1, "Failed to create graphics pipeline\n");
    }

    return graphics_pipeline;
}

