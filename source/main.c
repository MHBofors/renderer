//
//  main.c
//  vulkanRenderer
//
//  Created by Markus Höglin on 2023-10-04.
//

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include "renderer.h"
#include "window.h"
#include "graphics_matrices.h"
#include <unistd.h>



extern const uint32_t frames_in_flight;
extern const uint32_t enable_validation_layers;
extern const uint32_t HEIGHT;
extern const uint32_t WIDTH;

uint32_t rot_group[3][8] = {{2, 3, 6, 7, 0, 1, 4, 5}, {4, 0, 6, 2, 5, 1, 7, 3}, {1, 3, 0, 2, 5, 7, 4, 6}};

uint32_t octahedral_rotation(uint32_t m, uint32_t axis) {
    return rot_group[axis % 3][m % 8];
}

typedef struct compute_push_constants_t {
    float a, b, c, d;
    union {
        complex float z;
        float C[2];
    };
    float t;
    uint32_t padding;
} compute_push_constants_t;

typedef struct fractal_data_t {
    VkPipeline pipeline;
    VkPipelineLayout layout;

    VkImageMemoryBarrier *begin_barriers, *end_barriers;

    VkDescriptorSetLayout descriptor_layout;
    VkDescriptorSet *descriptors;

    uint32_t texture_width, texture_height;
    image_t *fractal_images;
    VkImageView *fractal_image_views;

    compute_push_constants_t push_data;
} fractal_data_t;

typedef struct mesh_t {
    uint32_t vertex_count;
    vertex_t *vertices;
    uint32_t index_count;
    uint16_t *indices;
} mesh_t;

void create_compute_pipeline_layout(VkPipelineLayout *pipeline_layout, VkDevice logical_device, VkDescriptorSetLayout layout) {
    VkPushConstantRange push_constant_range = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(compute_push_constants_t)
    };

    VkPipelineLayoutCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range
    };

    if(vkCreatePipelineLayout(logical_device, &create_info, NULL, pipeline_layout) != VK_SUCCESS) {
        error(1, "Failed to create compute pipeline layout");
    }
}

void create_compute_pipeline(VkPipeline *compute_pipeline, VkPipelineLayout pipeline_layout, VkDevice logical_device, const char *file_name) {
    VkShaderModule compute_shader;
    load_shader_module(&compute_shader, logical_device, file_name);
    VkPipelineShaderStageCreateInfo shader_stage_create_info = create_shader_stage(compute_shader, VK_SHADER_STAGE_COMPUTE_BIT);

    VkComputePipelineCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .layout = pipeline_layout,
        .stage = shader_stage_create_info,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if(vkCreateComputePipelines(logical_device, VK_NULL_HANDLE, 1, &create_info, NULL, compute_pipeline) != VK_SUCCESS) {
        error(1, "Failed to create compute pipeline");
    }

    vkDestroyShaderModule(logical_device, compute_shader, NULL);
}

mesh_t create_cube_mesh() {
    uint32_t vertex_count = 8;
    uint32_t index_count = 36;
    vertex_t *vertices = malloc(vertex_count*sizeof(vertex_t));
    uint16_t *indices = malloc(index_count*sizeof(uint16_t));

    for(uint32_t i = 0; i < vertex_count; i++) {
        vertex_t vertex = {
            .position = {
                .x = (float)(1 & i)*1.f,
                .y = (float)(1 & i>>1)*1.f,
                .z = (float)(1 & i>>2)*1.f
            },
            .color = {
                .r = 1.f,
                .g = 1.f,
                .b = 1.f,
                .alpha = 0.f
            },
            .texture_coordinates = {
                .u = (float)(1 & i>>1)*1.f,
                .v = (float)((1 & i) ^ i>>2)*1.f
            }
        };

        vertices[i] = vertex;
    }

    for(uint32_t i = 0; i < 8; i++) {
        vertices[i].position.x -= 0.5;
        vertices[i].position.y -= 0.5;
        vertices[i].position.z -= 0.5;
    }

    for(uint32_t i = 0; i < 6; i++) {
        uint32_t j = i % 3;
        uint32_t k = i % 2;

        uint32_t i00 = 0b111*k, i01 = octahedral_rotation(i00, j), i11 = octahedral_rotation(i01, j), i10 = octahedral_rotation(i11, j);
        indices[i + 0] = i00;
        indices[i + 1] = i01;
        indices[i + 2] = i10;
        //indices[i + 3] = i11;
        //indices[i + 4] = i10;
        //indices[i + 5] = i01;
    }

    indices[0] = 0;
    indices[1] = 2;
    indices[2] = 1;
    indices[3] = 1;
    indices[4] = 2;
    indices[5] = 3;

    indices[6] = 0;
    indices[7] = 4;
    indices[8] = 2;
    indices[9] = 2;
    indices[10] = 4;
    indices[11] = 6;

    indices[12] = 0;
    indices[13] = 1;
    indices[14] = 4;
    indices[15] = 1;
    indices[16] = 5;
    indices[17] = 4;

    indices[18] = 7;
    indices[19] = 5;
    indices[20] = 3;
    indices[21] = 5;
    indices[22] = 1;
    indices[23] = 3;

    indices[24] = 7;
    indices[25] = 6;
    indices[26] = 5;
    indices[27] = 5;
    indices[28] = 6;
    indices[29] = 4;

    indices[30] = 7;
    indices[31] = 3;
    indices[32] = 6;
    indices[33] = 2;
    indices[34] = 6;
    indices[35] = 3;

    mesh_t mesh = {
        .index_count = 36,
        .indices = indices,
        .vertex_count = 8,
        .vertices = vertices
    };

    return mesh;
}

fractal_data_t initialise_fractal_data(renderer_t *renderer) {
    uint32_t frames_in_flight = renderer->frame_count;

    image_t *fractal_images = malloc(frames_in_flight*sizeof(image_t));
    VkImageView *fractal_image_views = malloc(frames_in_flight*sizeof(VkImage));

    VkImageMemoryBarrier *begin_barriers = malloc(frames_in_flight*sizeof(VkImageMemoryBarrier));
    VkImageMemoryBarrier *end_barriers = malloc(frames_in_flight*sizeof(VkImageMemoryBarrier));

    uint32_t texture_width = 512, texture_height = 512;
    for(uint32_t i = 0; i < frames_in_flight; i++) {
        fractal_images[i] = create_image(renderer, texture_width, texture_height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
        fractal_image_views[i] = create_image_view(fractal_images[i].image, renderer->logical_device, 1, VK_FORMAT_R32G32B32A32_SFLOAT);
        
        begin_barriers[i] = (VkImageMemoryBarrier){
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .image = fractal_images[i].image,
            .subresourceRange = (VkImageSubresourceRange){
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .baseMipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .pNext = NULL
        };

        end_barriers[i] = (VkImageMemoryBarrier){
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .image = fractal_images[i].image,
            .subresourceRange = (VkImageSubresourceRange){
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .baseMipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .pNext = NULL
        };
    }

    VkDescriptorPool descriptor_pool = renderer->global_pool;

    descriptor_layout_builder_t layout_builder = initialise_layout_builder();
    descriptor_writer_t writer = initialise_writer();

    add_binding(&layout_builder, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
    VkDescriptorSetLayout fractal_layout = build_layout(&layout_builder, renderer->logical_device);
    free_layout_builder(&layout_builder);

    VkDescriptorSet *fractal_sets = malloc(frames_in_flight*sizeof(VkDescriptorSet));
    for(uint32_t i = 0; i < frames_in_flight; i++) {
        allocate_descriptor_set(&fractal_sets[i], renderer->logical_device, descriptor_pool, &fractal_layout, 1);

        write_image(&writer, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, fractal_image_views[i], VK_IMAGE_LAYOUT_GENERAL);
        update_set(&writer, renderer->logical_device, fractal_sets[i]);
        clear_writes(&writer);
    }
    free_writer(&writer);

    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    
    create_compute_pipeline_layout(&pipeline_layout, renderer->logical_device, fractal_layout);
    create_compute_pipeline(&pipeline, pipeline_layout, renderer->logical_device, "shaders/compute.spv");

    fractal_data_t fractal_data = {
        .pipeline = pipeline,
        .layout = pipeline_layout,
        .begin_barriers = begin_barriers,
        .end_barriers = end_barriers,
        .texture_width = texture_width,
        .texture_height = texture_height,
        .fractal_images = fractal_images,
        .fractal_image_views = fractal_image_views,
        .descriptor_layout = fractal_layout,
        .descriptors = fractal_sets,
    };

    return fractal_data;
}

void update_fractal(fractal_data_t *fractal_data, VkCommandBuffer command_buffer, compute_push_constants_t push, uint32_t frame_index) {
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &fractal_data->begin_barriers[frame_index]);
    
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, fractal_data->pipeline);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, fractal_data->layout, 0, 1, &fractal_data->descriptors[frame_index], 0, NULL);
    vkCmdPushConstants(command_buffer, fractal_data->layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(compute_push_constants_t), &push);
    vkCmdDispatch(command_buffer, fractal_data->texture_width/32 + (fractal_data->texture_width % 32 != 0), fractal_data->texture_height/32 + (fractal_data->texture_height % 32 != 0), 1);
    
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &fractal_data->end_barriers[frame_index]);
}

void update_scene(host_buffer_t scene_buffer, double t) {

}

void destroy_fractal_data(fractal_data_t *fractal_data, VkDevice logical_device) {
    for(uint32_t i = 0; i < frames_in_flight; i++) {
        destroy_image(&fractal_data->fractal_images[i], logical_device);
        vkDestroyImageView(logical_device, fractal_data->fractal_image_views[i], NULL);
    }

    vkDestroyPipelineLayout(logical_device, fractal_data->layout, NULL);
    vkDestroyPipeline(logical_device, fractal_data->pipeline, NULL);
    vkDestroyDescriptorSetLayout(logical_device, fractal_data->descriptor_layout, NULL);

    free(fractal_data->begin_barriers);
    free(fractal_data->end_barriers);
    free(fractal_data->descriptors);
    free(fractal_data->fractal_images);
    free(fractal_data->fractal_image_views);
}

void run_fractal(engine_t *engine) {
    uint32_t frame_index = 0;
    uint32_t frames_in_flight = engine->renderer.frame_count;

    renderer_t *renderer = &engine->renderer;

    VkDescriptorSet global_sets[frames_in_flight];
    VkDescriptorSet material_sets[frames_in_flight];

    descriptor_layout_builder_t layout_builder = initialise_layout_builder();
    add_binding(&layout_builder, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayout scene_layout = build_layout(&layout_builder, renderer->logical_device);
    clear_bindings(&layout_builder);
    add_binding(&layout_builder, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);
    add_binding(&layout_builder, 1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayout material_layout = build_layout(&layout_builder, renderer->logical_device);
    free_layout_builder(&layout_builder);

    material_pipeline_t textured_pipeline = build_textured_mesh_pipeline(renderer->logical_device, renderer->render_pass, &scene_layout, &material_layout, renderer->extent);
    material_t fractal_material = {
        .descriptor = material_sets[0],
        .material_pipeline = &textured_pipeline
    };

    VkDescriptorPool descriptor_pool;

    VkDescriptorPoolSize image_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 64
    };

    VkDescriptorPoolSize texture_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 64
    };

    VkDescriptorPoolSize sampler_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = frames_in_flight
    };

    VkDescriptorPoolSize buffer_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 64
    };

    VkDescriptorPoolSize pool_sizes[4] = {image_pool_size, texture_pool_size, sampler_pool_size, buffer_pool_size};

    create_descriptor_pool(&descriptor_pool, renderer->logical_device, pool_sizes, 4, 256);
    create_descriptor_pool(&renderer->global_pool, renderer->logical_device, pool_sizes, 4, 256);

    for(uint32_t i = 0; i < frames_in_flight; i++) {
        allocate_descriptor_set(&global_sets[i], renderer->logical_device, descriptor_pool, &scene_layout, 1);
        allocate_descriptor_set(&material_sets[i], renderer->logical_device, descriptor_pool, &material_layout, 1);
    }
    

    vertex_t vertices[4];
    uint16_t indices[6];
    
    fractal_data_t fractal_data = initialise_fractal_data(renderer);
    mesh_t cube = create_cube_mesh();

    VkCommandBuffer init_command_buffer[2];
    create_primary_command_buffer(init_command_buffer, renderer->logical_device, renderer->command_pool, 2);

    buffer_t vertex_buffer = create_vertex_buffer(renderer, cube.vertex_count, sizeof(vertex_t), cube.vertices, renderer->queues.graphics_queue, init_command_buffer[0]);
    buffer_t index_buffer = create_index_buffer(renderer, cube.index_count, cube.indices, renderer->queues.graphics_queue, init_command_buffer[1]);
    
    free(cube.indices);
    free(cube.vertices);

    host_buffer_t scene_buffer[renderer->frame_count];

    vector3_t eye = {1.5f, 1.f, 1.5f};
    vector3_t object = {0.f, 0.f, 0.f};
    vector3_t up = {1.f, 0.f, 1.f};

    float aspect_ratio = (float)renderer->extent.height/(float)renderer->extent.width;
    scene_data_t scene_data = {
        .view = camera_matrix(eye, object, up),
        .projection = perspective_matrix(M_PI_2, aspect_ratio, 0.01f, 100.0f),
    };

    for(uint32_t i = 0; i < renderer->frame_count; i++) {
        scene_buffer[i] = create_host_buffer(renderer, sizeof(scene_data_t), renderer->queues.graphics_queue);
        memcpy(scene_buffer[i].mapped_memory, &scene_data, sizeof(scene_data_t));
    }

    VkSampler sampler = create_linear_sampler(renderer->logical_device);
    descriptor_writer_t writer = initialise_writer();
    for(uint32_t i = 0; i < frames_in_flight; i++) {
        VkImageView image_view = fractal_data.fractal_image_views[i];

        write_image(&writer, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        write_sampler(&writer, 1, sampler);
        update_set(&writer, renderer->logical_device, material_sets[i]);
        clear_writes(&writer);

        write_buffer(&writer, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, scene_buffer[i].buffer, sizeof(scene_data_t), 0);
        update_set(&writer, renderer->logical_device, global_sets[i]);
        clear_writes(&writer);
    }
    
    free_writer(&writer);
    /*
    To do:
    Create & write global descriptor sets
    Create mesh
        -vertex buffer
        -index buffer
        -uniform buffers
        -push data
    Create render loop
        -begin frame
        -compute pass
            -layout undef->general memory barrier
            -compute shader
            -layout general->sample optimal memory barrier
            -set correct image for material
        -graphics pass
        -end frame
    Cleaner functions
    */
    
    render_object_t mesh = {
        .first_index = 0,
        .index_count = 36,
        .index_buffer = index_buffer.buffer,
        .vertex_buffer = &vertex_buffer.buffer,
        .push_constant = {
            .model = identity_matrix()
        },
        .material_instance = &fractal_material
    };

    double t = 0, d_t;
    clock_t time_start = clock();

    printf("Initialisation complete\n");
    frame_t *current_frame;
    while(!window_should_close(engine->window)) {
        window_update();

        current_frame = &renderer->frames[frame_index];
        uint32_t image_index = begin_frame(engine, frame_index);

        d_t = (double)(clock() - time_start)/CLOCKS_PER_SEC - t;
        t += d_t;
        double theta = 0.25*t;
        compute_push_constants_t push = {
            .a = -1.f,
            .b =  1.f,
            .c = -1.f,
            .d =  1.f,
            .z = 1.025f*(CMPLXF((cos(theta) - cos(2.00*theta)*0.5)*0.5, (sin(theta) - sin(2.00*theta)*0.5)*0.5)),
            .t = t,
            .padding = 0
        };

        fractal_material.descriptor = material_sets[frame_index];
        update_fractal(&fractal_data, current_frame->command_buffer, push, frame_index);

        float s = 0.25f*t;
        vector3_t axis = {cos(s)-sin(s), sin(s)-cos(s), cos(s)};
        VkClearValue clear_color = {{{0.0625f*(cos(t)-sin(t)), 0.25*0.0625f*(sin(t)-cos(t)), 0.0625f*(cos(t))}}};
        VkRenderPassBeginInfo render_pass_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderer->render_pass,
            .framebuffer = renderer->framebuffers[image_index],
            .renderArea = {
                .offset = {0, 0},
                .extent = renderer->extent
            },
            .clearValueCount = 1,
            .pClearValues = &clear_color,
            .pNext = NULL
        };

        vkCmdBeginRenderPass(current_frame->command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = renderer->extent.width,
            .height = renderer->extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = renderer->extent
        };
        vkCmdSetViewport(current_frame->command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(current_frame->command_buffer, 0, 1, &scissor);

        mesh.push_constant.model = rotation_matrix(axis, 2.5f*t);

        draw_mesh(current_frame, &mesh, global_sets[frame_index]);

        vkCmdEndRenderPass(current_frame->command_buffer);

        end_frame(engine, frame_index, image_index);
        frame_index = (frame_index + 1) % frames_in_flight;
    }

    vkDeviceWaitIdle(renderer->logical_device);

    destroy_buffer(&vertex_buffer, renderer->logical_device);
    destroy_buffer(&index_buffer, renderer->logical_device);
    for(uint32_t i = 0; i < renderer->frame_count; i++) {
        destroy_host_buffer(&scene_buffer[i], renderer->logical_device);
    }
   
    vkDestroyPipeline(renderer->logical_device, textured_pipeline.pipeline, NULL);
    vkDestroyPipelineLayout(renderer->logical_device, textured_pipeline.layout, NULL);
    vkDestroyDescriptorSetLayout(renderer->logical_device, scene_layout, NULL);
    vkDestroyDescriptorSetLayout(renderer->logical_device, material_layout, NULL);
    vkDestroyDescriptorPool(renderer->logical_device, renderer->global_pool, NULL);
    vkDestroyDescriptorPool(renderer->logical_device, descriptor_pool, NULL);
    vkDestroySampler(renderer->logical_device, sampler, NULL);
    destroy_fractal_data(&fractal_data, renderer->logical_device);
}

/*
int main(int argc, const char * argv[]) {
    engine_t entropy_engine;
    initialise_engine(&entropy_engine);
    terminate_engine(&entropy_engine);

    float phi = (1.0 + sqrt(5.0))*0.5;
    float phi_1 = (sqrt(5.0) - 1)*0.5;

    window_t window;
    vulkan_context_t context;
    device_context_t device;
    swap_resources_t swap_resources;
    render_pipeline_t render_pipeline;
    
    initialise_window(&window);
    setup_context(&context, window);
    setup_device_context(&device, &context);
    setup_swap_resources(&swap_resources, &context, &device, window);
    create_render_pass_simple(&render_pipeline.render_pass, device.logical_device, swap_resources.image_format);
    

    for(uint32_t i = 0; i < swap_resources.image_count; i++) {
        create_framebuffer(swap_resources.framebuffers + i, device.logical_device, render_pipeline.render_pass, 1, swap_resources.image_views + i, swap_resources.extent);
    }

    host_buffer_t uniform_buffers[frames_in_flight];

    image_t fractal_images[frames_in_flight];
    VkImageView fractal_image_views[frames_in_flight];
    VkSampler sampler;

    create_linear_sampler(&sampler, device.logical_device);

    VkImageMemoryBarrier compute_pipeline_barriers[frames_in_flight][2];

    VkPipelineLayout compute_pipeline_layout;
    VkPipeline compute_pipeline;

    VkDescriptorPool descriptor_pool;

    VkDescriptorPoolSize image_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = frames_in_flight
    };

    VkDescriptorPoolSize texture_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = frames_in_flight
    };

    VkDescriptorPoolSize sampler_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = frames_in_flight
    };

    VkDescriptorPoolSize buffer_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = frames_in_flight
    };

    VkDescriptorPoolSize pool_sizes[4] = {image_pool_size, texture_pool_size, sampler_pool_size, buffer_pool_size};

    create_descriptor_pool(&descriptor_pool, device.logical_device, pool_sizes, 4, 256);

    VkDescriptorSetLayoutBinding image_binding = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .binding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    };

    VkDescriptorSetLayoutBinding buffer_binding = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .binding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    };

    VkDescriptorSetLayoutBinding texture_binding = {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .binding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    };

    VkDescriptorSetLayoutBinding sampler_binding = {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .binding = 2,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER
    };

    VkDescriptorSetLayoutBinding bindings[] = {buffer_binding, texture_binding, sampler_binding};

    VkDescriptorSetLayout image_layout;
    VkDescriptorSetLayout descriptor_layout;
    create_descriptor_set_layout(&image_layout, device.logical_device, &image_binding, 1);
    create_descriptor_set_layout(&descriptor_layout, device.logical_device, &bindings, 3);

    setup_graphics_pipeline_layout(&render_pipeline.pipeline_layout, device.logical_device, descriptor_layout);
    setup_graphics_pipeline(&render_pipeline.graphics_pipeline, render_pipeline.pipeline_layout, render_pipeline.render_pass, device, swap_resources);

    VkDescriptorSet compute_descriptor_sets[frames_in_flight];
    VkDescriptorSetLayout layouts[frames_in_flight];
    for(uint32_t i = 0; i < frames_in_flight; i++) {
        layouts[i] = image_layout;
    }
    
    
    create_compute_pipeline_layout(&compute_pipeline_layout, device.logical_device, image_layout);
    create_compute_pipeline(&compute_pipeline, compute_pipeline_layout, device.logical_device, "shaders/compute.spv");

    uint32_t texture_width = 1024;
    uint32_t texture_height = 1024;

    for(uint32_t i = 0; i < frames_in_flight; i++) {
        create_image(&fractal_images[i].image, &fractal_images[i].memory, device.logical_device, device.physical_device, texture_width, texture_height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
        create_image_view(fractal_image_views + i, fractal_images[i].image, device.logical_device, 1, VK_FORMAT_R32G32B32A32_SFLOAT);
        
        create_buffer(&uniform_buffers[i].buffer, &uniform_buffers[i].memory, device.logical_device, device.physical_device, (VkDeviceSize)3*sizeof(float[4][4]), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(device.logical_device, uniform_buffers[i].memory, 0, (VkDeviceSize)3*sizeof(float[4][4]), 0, &uniform_buffers[i].mapped_memory);

        compute_pipeline_barriers[i][0] = (VkImageMemoryBarrier){
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .image = fractal_images[i].image,
            .subresourceRange = (VkImageSubresourceRange){
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .baseMipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .pNext = NULL
        };

        compute_pipeline_barriers[i][1] = (VkImageMemoryBarrier){
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .image = fractal_images[i].image,
            .subresourceRange = (VkImageSubresourceRange){
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .baseMipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .pNext = NULL
        };

        VkImageView image_views[1] = {fractal_image_views[i]};
    }

    allocate_descriptor_set(compute_descriptor_sets, device.logical_device, descriptor_pool, layouts, frames_in_flight);
    
    for(uint32_t i = 0; i < frames_in_flight; i++) {
        VkDescriptorImageInfo image_info = {
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .imageView = fractal_image_views[i]
        };

        VkWriteDescriptorSet image_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .dstBinding = 0,
            .dstSet = compute_descriptor_sets[i],
            .descriptorCount = 1,
            .pImageInfo = &image_info
        };

        update_descriptor_set(device.logical_device, compute_descriptor_sets[i], &image_write, 1);
    }

    for(uint32_t i = 0; i < frames_in_flight; i++) {
        layouts[i] = descriptor_layout;
    }

    VkDescriptorSet graphics_descriptor_sets[frames_in_flight];
    allocate_descriptor_set(graphics_descriptor_sets, device.logical_device, descriptor_pool, layouts, frames_in_flight);

    for(uint32_t i = 0; i < frames_in_flight; i++) {
        VkDescriptorBufferInfo buffer_info = {
            .buffer = uniform_buffers[i].buffer,
            .offset = 0,
            .range = 3*sizeof(float[4][4])
        };

        VkWriteDescriptorSet buffer_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .dstBinding = 0,
            .dstSet = graphics_descriptor_sets[i],
            .descriptorCount = 1,
            .pBufferInfo = &buffer_info
        };

        VkDescriptorImageInfo image_info = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = fractal_image_views[i]
        };

        VkWriteDescriptorSet image_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .dstBinding = 1,
            .dstSet = graphics_descriptor_sets[i],
            .descriptorCount = 1,
            .pImageInfo = &image_info
        };

        VkDescriptorImageInfo sampler_info = {
            .sampler = sampler
        };

        VkWriteDescriptorSet sampler_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .dstBinding = 2,
            .dstSet = graphics_descriptor_sets[i],
            .descriptorCount = 1,
            .pImageInfo = &sampler_info
        };

        VkWriteDescriptorSet writes[] = {buffer_write, image_write, sampler_write};
        update_descriptor_set(device.logical_device, graphics_descriptor_sets[i], writes, 3);
    }

    queue_family_indices indices = find_queue_families(device.physical_device);
    VkCommandPool command_pool;
    create_command_pool(&command_pool, device.logical_device, indices.graphics_family);
    frame_t *frames = create_frames(device.logical_device, command_pool, frames_in_flight);

    buffer_t vertex_buffer;
    buffer_t index_buffer;
    dynamic_vector *vertex_vector = vector_alloc(sizeof(vertex_t));
    dynamic_vector *index_vector = vector_alloc(sizeof(uint16_t));

    vertex_t vertex;

    vertex = (vertex_t){
        .position = {
            .x =-1.0,
            .y =-1.0,
            .z = 0.0
        },
        .color = {
            .r = 1.0f,
            .g = 0,
            .b = 0,
            .alpha = 1.0f
        },
        .texture_coordinates = {
            .u = 0,
            .v = 0
        }
    };
    vector_add(vertex_vector, &vertex);

    vertex = (vertex_t){
        .position = {
            .x = 1.0,
            .y =-1.0,
            .z = 0.0
        },
        .color = {
            .r = 1.0f,
            .g = 0,
            .b = 0,
            .alpha = 1.0f
        },
        .texture_coordinates = {
            .u = 1,
            .v = 0
        }
    };
    vector_add(vertex_vector, &vertex);

    vertex = (vertex_t){
        .position = {
            .x = 1.0,
            .y = 1.0,
            .z = 0.0
        },
        .color = {
            .r = 1.0f,
            .g = 0,
            .b = 0,
            .alpha = 1.0f
        },
        .texture_coordinates = {
            .u = 1,
            .v = 1
        }
    };
    vector_add(vertex_vector, &vertex);

    vertex = (vertex_t){
        .position = {
            .x =-1.0,
            .y = 1.0,
            .z = 0.0
        },
        .color = {
            .r = 1.0f,
            .g = 0,
            .b = 0,
            .alpha = 1.0f
        },
        .texture_coordinates = {
            .u = 0,
            .v = 1
        }
    };
    vector_add(vertex_vector, &vertex);

    uint16_t triangle[6] = {0, 1, 2, 3};
    vector_add(index_vector, triangle + 0);
    vector_add(index_vector, triangle + 1);
    vector_add(index_vector, triangle + 3);
    vector_add(index_vector, triangle + 2);
    vector_add(index_vector, triangle + 3);
    vector_add(index_vector, triangle + 1);

    create_vertex_buffer(&vertex_buffer, vertex_vector, device.logical_device, device.physical_device, device.queues.graphics_queue, command_pool);
    create_index_buffer(&index_buffer, index_vector, device.logical_device, device.physical_device, device.queues.graphics_queue, command_pool);


    uint32_t image_index;
    uint32_t frame_index = 0;

    clock_t time_start = clock();
    float t_0 = 0;
    float t = 0;
    float s = 0;
    frame_t *frame;
    VkCommandBuffer command_buffer;

    transformation_t matrices[3] = {0};
    vector3_t u = {0, 0, 1}, v = {0, 0, 0}, w = {0, 1, 0}, r = {1, 1, 0};
    while(!window_should_close(window)) {
        update_window();
        
        t_0 = t;
        t = (float)(clock() - time_start)/CLOCKS_PER_SEC;

        frame = frames + frame_index;
        command_buffer = frame->command_buffer;

        VkResult result;

        image_index = begin_frame(frame, &result, device.logical_device, swap_resources.swapchain);
        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swap_resources(&swap_resources, &context, &device, &render_pipeline, window);
        } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            error(1, "Failed to acquire swap chain image!");
        }


        matrices[0] = rotation_matrix(r, t);
        matrices[1] = camera_matrix(u, v, w);
        float aspect_ratio = (float)swap_resources.extent.height/(float)swap_resources.extent.width;
        matrices[2] = perspective_matrix(M_PI_2, aspect_ratio, 0.1f, 10.0f);

        memcpy(uniform_buffers[frame_index].mapped_memory, matrices, 3*sizeof(float[4][4]));
        

        begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkDependencyInfo dependency_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 0,
            .pImageMemoryBarriers = &compute_pipeline_barriers[frame_index],
            .memoryBarrierCount = 0,
            .pMemoryBarriers = NULL,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = NULL,
            .pNext = NULL
        };
        
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &compute_pipeline_barriers[frame_index][0]);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout, 0, 1, &compute_descriptor_sets[frame_index], 0, NULL);
        
        
        s = t*0.125;
        float d;
        d = 0.75f;
        d = 1/6.0f;
        //t *= 0.125;
        //float z[3] = {(cos(s) - cos(2.00*s)*0.5)*0.5, (sin(s) - sin(2.00*s)*0.5)*0.5, 0*0.25*t};
        //float z[3] = {(cos(s) - cos(6.00*s)*d)*0.5, (sin(s) - sin(6.00*s)*d)*0.5, 0.25*t};

        //float z[3] = {cos(s) + 2.0*cos(-4.00*s), sin(s) + 2.0*sin(-4.00*s), 0.125*t};
        //z[0] *= 0.75;
        //z[1] *= 0.75;

        float z[3] = {(cos(s) - cos(2.00*s)*0.5)*0.5, (sin(s) - sin(2.00*s)*0.5)*0.5, t};
        z[0] *= 1.0 + 0.25;
        z[1] *= 1.0 + 0.25;
        
        //float z[2] = {(cos(t) - cos(2.0*t)*0.5)*0.5+0.00625*cos(0.125*t), (sin(t) - sin(2.0*t)*0.5+0.00625*sin(0.125*t))*0.5};
        vkCmdPushConstants(command_buffer, compute_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 12, z);
        vkCmdDispatch(command_buffer, texture_width/32 + (texture_width % 32 != 0), texture_height/32 + (texture_height % 32 != 0), 1);

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &compute_pipeline_barriers[frame_index][1]);

        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

        VkRenderPassBeginInfo render_pass_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pipeline.render_pass,
            .framebuffer = swap_resources.framebuffers[image_index],
            .renderArea = {
                .offset = {0, 0},
                .extent = swap_resources.extent
            },
            .clearValueCount = 1,
            .pClearValues = &clear_color,
            .pNext = NULL
        };

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_pipeline.graphics_pipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = swap_resources.extent.width,
            .height = swap_resources.extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swap_resources.extent
        };

        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        VkBuffer vertex_buffers[] = {vertex_buffer.buffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);//Parameters 2, 3 specifies the offsets and how many vertex buffers to bind. Parameters 4, 5 specifies the array of vertex buffers to write and what offset to start reading from
        vkCmdBindIndexBuffer(command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);
        
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_pipeline.pipeline_layout, 0, 1, &graphics_descriptor_sets[frame_index], 0, NULL);
        
        vkCmdDrawIndexed(command_buffer, vector_count(index_vector), 1, 0, 0, 0);
        vkCmdEndRenderPass(command_buffer);

        end_command_buffer(command_buffer);

        result = end_frame(frame, swap_resources.swapchain, device.queues.graphics_queue, device.queues.graphics_queue, image_index);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreate_swap_resources(&swap_resources, &context, &device, &render_pipeline, window);
        } else if(result != VK_SUCCESS) {
            error(1, "Failed to present swap chain image");
        }

        frame_index = (frame_index + 1) % frames_in_flight;
    }

    vkDeviceWaitIdle(device.logical_device);

    clean_up_swap_resources(&swap_resources, &device);
    destroy_buffer(&vertex_buffer, device.logical_device);
    destroy_buffer(&index_buffer, device.logical_device);
    clean_up_frames(frames, frames_in_flight, device.logical_device);
    vkDestroyCommandPool(device.logical_device, command_pool, NULL);
    clean_up_render_pipeline(&render_pipeline, &device);

    vkDestroyDescriptorPool(device.logical_device, descriptor_pool, NULL);
    
    clean_up_device_context(&device);
    clean_up_context(&context);
    
    terminate_window(window);
    return 0;
}
*/

int main(int argc, const char * argv[]) {
    engine_t entropy_engine;
    initialise_engine(&entropy_engine);
    run_fractal(&entropy_engine);
    terminate_engine(&entropy_engine);
}