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
    float x_min, x_max, y_min, y_max;
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



mesh_t create_donut_mesh(double R, double r, uint32_t m, uint32_t n) {
    uint32_t vertex_count = (m+1)*(n+1);
    uint32_t index_count = 6*m*n;
    vertex_t (*vertices) = malloc(vertex_count*sizeof(vertex_t));
    uint16_t (*indices) = malloc(index_count*sizeof(uint16_t));

    double phi = 2*M_PI/(double)m;
    double theta = 2*M_PI/(double)n;

    uint32_t M = 8, N = 6;
    for(int32_t i = 0; i < m+1; i++) {
        for(int32_t j = 0; j < n+1; j++) {
            vertex_t vertex = {
                .position = {
                    .x = (R + r*cos(theta*j))*cos(phi*i),
                    .y = (R + r*cos(theta*j))*sin(phi*i),
                    .z = r*sin(theta*j)
                },
                .color = {
                    .r = 1.f,
                    .g = 1.f,
                    .b = 1.f,
                    .alpha = 0.f
                },
                .texture_coordinates = {
                    .u = M*i/(double)m,
                    .v = (M>>2)*i/(double)m+N*j/(double)n
                }
            };

            vertices[i*(n+1) + j] = vertex;
        }
    }

    for(int32_t i = 0; i < m; i++) {
        for(int32_t j = 0; j < n; j++) {
            indices[i*6*n + j*6 + 0] = i*(n+1) + j;
            indices[i*6*n + j*6 + 1] = (i + 1)*(n+1) + j;
            indices[i*6*n + j*6 + 2] = i*(n+1) + (j + 1);
            indices[i*6*n + j*6 + 3] = (i + 1)*(n+1) + (j + 1);
            indices[i*6*n + j*6 + 4] = i*(n+1) + (j + 1);
            indices[i*6*n + j*6 + 5] = (i + 1)*(n+1) + j;
        }
    }

    mesh_t mesh = {
        .index_count = index_count,
        .indices = indices,
        .vertex_count = vertex_count,
        .vertices = vertices
    };

    return mesh;
}

mesh_t create_square_mesh() {
    uint32_t vertex_count = 4;
    uint32_t index_count = 6;
    vertex_t (*vertices) = malloc(vertex_count*sizeof(vertex_t));
    uint16_t (*indices) = malloc(index_count*sizeof(uint16_t));

    for(uint32_t i = 0; i < vertex_count; i++) {
        vertex_t vertex = {
            .position = {
                .x = (float)(1 & i)*2.f-1.f,
                .y = (float)(1 & i>>1)*2.f-1.f,
                .z = 0
            },
            .color = {
                .r = 1.f,
                .g = 1.f,
                .b = 1.f,
                .alpha = 0.f
            },
            .texture_coordinates = {
                .u = (float)(1 & i)*1.f,
                .v = (float)(1 & i>>1)*1.f
            }
        };

        vertices[i] = vertex;
    }


    mesh_t mesh = {
        .index_count = index_count,
        .indices = indices,
        .vertex_count = vertex_count,
        .vertices = vertices
    };

    indices[0] = 0;
    indices[1] = 2;
    indices[2] = 1;
    indices[3] = 1;
    indices[4] = 2;
    indices[5] = 3;

    return mesh;
}

fractal_data_t initialise_fractal_data(renderer_t *renderer) {
    uint32_t frames_in_flight = renderer->frame_count;

    image_t *fractal_images = malloc(frames_in_flight*sizeof(image_t));
    VkImageView *fractal_image_views = malloc(frames_in_flight*sizeof(VkImage));

    VkImageMemoryBarrier *begin_barriers = malloc(frames_in_flight*sizeof(VkImageMemoryBarrier));
    VkImageMemoryBarrier *end_barriers = malloc(frames_in_flight*sizeof(VkImageMemoryBarrier));

    uint32_t texture_width = 2048, texture_height = 2048;
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
    mesh_t model = create_donut_mesh(1.0625, 1.0, 128, 128);
    //mesh_t model = create_square_mesh();

    VkCommandBuffer init_command_buffer[2];
    create_primary_command_buffer(init_command_buffer, renderer->logical_device, renderer->command_pool, 2);

    buffer_t vertex_buffer = create_vertex_buffer(renderer, model.vertex_count, sizeof(vertex_t), model.vertices, renderer->queues.graphics_queue, init_command_buffer[0]);
    buffer_t index_buffer = create_index_buffer(renderer, model.index_count, model.indices, renderer->queues.graphics_queue, init_command_buffer[1]);
    
    free(model.indices);
    free(model.vertices);

    host_buffer_t scene_buffer[renderer->frame_count];

    vector3_t eye = {0.5f, 0.5f, 0.5f};
    vector3_t object = {0.f, 0.f, 0.f};
    vector3_t up = {0.f, 0.25f, 1.f};

    float aspect_ratio = (float)renderer->extent.width/(float)renderer->extent.height;
    scene_data_t scene_data = {
        .view = camera_matrix(eye, object, up),
        .projection = perspective_matrix(M_PI_2, aspect_ratio, 0.01f, 100.0f),
    };

    //scene_data.view = identity_matrix();
    //scene_data.projection = identity_matrix();

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
    
    render_object_t mesh = {
        .first_index = 0,
        .index_count = model.index_count,
        .index_buffer = index_buffer.buffer,
        .vertex_buffer = &vertex_buffer.buffer,
        .push_constant = {
            .model = identity_matrix(),
            .t = 0
        },
        .material_instance = &fractal_material
    };

    double t = 0, d_t;
    clock_t time_start = clock();

    frame_t *current_frame;
    while(!window_should_close(engine->window)) {
        window_update();

        current_frame = &renderer->frames[frame_index];
        uint32_t image_index = begin_frame(engine, frame_index);

        d_t = (double)(clock() - time_start)/CLOCKS_PER_SEC - t;
        t += d_t;
        float s = 0.025f*t;
        double theta = 2.5 + 0.05*s;

        aspect_ratio = (float)renderer->extent.width/(float)renderer->extent.height;
        scene_data = (scene_data_t){
            .view = camera_matrix(eye, object, up),
            .projection = perspective_matrix(M_PI*(0.75 + 0.125*cos(t)), aspect_ratio, 0.01f, 100.0f),
        };
        memcpy(scene_buffer[frame_index].mapped_memory, &scene_data, sizeof(scene_data_t));


        complex float z = 0.5*((cos(theta) - cos(2.00*theta)*0.5) + (sin(theta) - sin(2.00*theta)*0.5)*I);
        z *= 1.15f;
        compute_push_constants_t push = {
            .x_min = -1.f,
            .x_max =  1.f,
            .y_min = -1.f,
            .y_max =  1.f,
            .z = z,
            .t = 4*s,
            .padding = 0
        };

        fractal_material.descriptor = material_sets[frame_index];
        update_fractal(&fractal_data, current_frame->command_buffer, push, frame_index);

        vector3_t axis = {cos(2.0*s)-sin(2.0*s), sin(2.0*s)-cos(2.0*s), cos(2.0*s)};
        VkClearValue clear_color = {{{0.0625f*(cos(s)-sin(s)), 0.25*0.0625f*(sin(s)-cos(s)), 0.0625f*(cos(s))}}};
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

        mesh.push_constant.model = rotation_matrix(axis, 5.0*s);
        mesh.push_constant.t = s;
        //mesh.push_constant.model = identity_matrix();

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

int main(int argc, const char * argv[]) {
    engine_t entropy_engine;
    initialise_engine(&entropy_engine);
    run_fractal(&entropy_engine);
    terminate_engine(&entropy_engine);
}