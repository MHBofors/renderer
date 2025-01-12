#include "renderer.h"

#ifdef __APPLE__
    const char device_extension_count = 2;
    const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};
#else
    const char device_extension_count = 1;
    const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif

const uint32_t frames_in_flight = 3;

void initialise_engine(engine_t *engine) {
    initialise_window(&engine->window);
    initialise_renderer(&engine->renderer, engine->window);
}

void terminate_engine(engine_t *engine) {
    terminate_window(engine->window);
    terminate_renderer(&engine->renderer);
}



void initialise_renderer(renderer_t *renderer, window_t window) {
    uint32_t window_extension_count;
    get_window_extensions(&window_extension_count, NULL);
    
    const char *window_extensions[window_extension_count];
    get_window_extensions(&window_extension_count, window_extensions);

    create_instance(&renderer->instance, window_extension_count, window_extensions);
    create_debug_messenger(renderer->instance, &renderer->debug_messenger);
    create_surface(&renderer->surface, renderer->instance, window);

    select_physical_device(&renderer->physical_device, renderer->instance, renderer->surface);
    create_logical_device(&renderer->logical_device, renderer->physical_device, &renderer->queues, device_extension_count, device_extensions);

    setup_swapchain(renderer, window);
    setup_render_pass(renderer);
    setup_framebuffers(renderer);

    queue_family_indices indices = find_queue_families(renderer->physical_device);
    renderer->graphics_family = indices.graphics_family;
    create_command_pool(&renderer->command_pool, renderer->logical_device, indices.graphics_family);

    setup_frame_resources(renderer, frames_in_flight);
}

void terminate_renderer(renderer_t *renderer) {
    vkDeviceWaitIdle(renderer->logical_device);

    destroy_framebuffers(renderer);
    terminate_swapchain(renderer);
    
    destroy_frame_resources(renderer);
    vkDestroyCommandPool(renderer->logical_device, renderer->command_pool, NULL);
    
    destroy_render_pass(renderer);

    vkDestroyDevice(renderer->logical_device, NULL);

    destroy_debug_utils_messenger_EXT(renderer->instance, renderer->debug_messenger, NULL);
    vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
    vkDestroyInstance(renderer->instance, NULL);
}



void setup_swapchain(renderer_t *renderer, window_t window) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->physical_device, renderer->surface, &capabilities);
    
    VkPresentModeKHR present_mode = choose_swap_present_mode(renderer->physical_device, renderer->surface);
    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(renderer->physical_device, renderer->surface);
    VkExtent2D extent = choose_swap_extent(&capabilities, window);
    uint32_t min_image_count = capabilities.minImageCount + 1;

    renderer->swapchain_image_format = surface_format.format;
    renderer->extent = extent;

    create_swapchain(&renderer->swapchain, renderer->logical_device, renderer->physical_device, renderer->surface, min_image_count, extent);

    vkGetSwapchainImagesKHR(renderer->logical_device, renderer->swapchain, &renderer->swapchain_image_count, NULL);

    renderer->swapchain_images = malloc(renderer->swapchain_image_count*sizeof(VkImage));
    renderer->swapchain_image_views = malloc(renderer->swapchain_image_count*sizeof(VkImageView));
    vkGetSwapchainImagesKHR(renderer->logical_device, renderer->swapchain, &renderer->swapchain_image_count, renderer->swapchain_images);

    for(uint32_t i = 0; i < renderer->swapchain_image_count; i++) {
        renderer->swapchain_image_views[i] = create_image_view(renderer->swapchain_images[i], renderer->logical_device, 1, renderer->swapchain_image_format);
    }
}

void terminate_swapchain(renderer_t *renderer) {
    for(uint32_t i = 0; i < renderer->swapchain_image_count; i++) {
        vkDestroyImageView(renderer->logical_device, renderer->swapchain_image_views[i], NULL);
    }

    vkDestroySwapchainKHR(renderer->logical_device, renderer->swapchain, NULL);

    free(renderer->swapchain_images);
    free(renderer->swapchain_image_views);
}

void recreate_swapchain(renderer_t *renderer, window_t window) {
    int width = 0, height = 0;
    get_framebuffer_size(window, &width, &height);
    
    while(width == 0 || height == 0) {
        get_framebuffer_size(window, &width, &height);
        window_wait_events();
    }

    vkDeviceWaitIdle(renderer->logical_device);

    terminate_swapchain(renderer);
    setup_swapchain(renderer, window);
    setup_framebuffers(renderer);
}



void setup_render_pass(renderer_t *renderer) {
    create_render_pass_simple(&renderer->render_pass, renderer->logical_device, renderer->swapchain_image_format);
}

void destroy_render_pass(renderer_t *renderer) {
    vkDestroyRenderPass(renderer->logical_device, renderer->render_pass, NULL);
}



void setup_framebuffers(renderer_t *renderer) {
    renderer->framebuffers = malloc(renderer->swapchain_image_count*sizeof(VkFramebuffer));

    for(uint32_t i = 0; i < renderer->swapchain_image_count; i++) {
        create_framebuffer(&renderer->framebuffers[i], renderer->logical_device, renderer->render_pass, 1, &renderer->swapchain_image_views[i], renderer->extent);
    }
}

void destroy_framebuffers(renderer_t *renderer) {
    for(uint32_t i = 0; i < renderer->swapchain_image_count; i++) {
        vkDestroyFramebuffer(renderer->logical_device, renderer->framebuffers[i], NULL);
    }

    free(renderer->framebuffers);
}



void setup_frame_resources(renderer_t *renderer, uint32_t frame_count) {
    renderer->frame_count = frame_count;
    renderer->frames = malloc(renderer->frame_count*sizeof(frame_t));

    if(renderer->frames == NULL) {
        error(1, "Failed to allocate frames\n");
    }

    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for(uint32_t i = 0; i < frame_count; i++) {
        if(vkCreateSemaphore(renderer->logical_device, &semaphore_create_info, NULL, &renderer->frames[i].image_available_semaphore) != VK_SUCCESS ||
           vkCreateSemaphore(renderer->logical_device, &semaphore_create_info, NULL, &renderer->frames[i].render_finished_semaphore) != VK_SUCCESS ||
           vkCreateFence(renderer->logical_device, &fence_create_info, NULL, &renderer->frames[i].in_flight_fence) != VK_SUCCESS) {
            error(1, "Failed to create frame sync resources");
        }

        create_command_pool(&renderer->frames[i].command_pool, renderer->logical_device, renderer->graphics_family);
        create_primary_command_buffer(&renderer->frames[i].command_buffer, renderer->logical_device, renderer->command_pool, 1);
    }
}

void destroy_frame_resources(renderer_t *renderer) {
    for(uint32_t i = 0; i < renderer->frame_count; i++) {
        clean_up_frame(&renderer->frames[i], renderer->logical_device);
    }

    free(renderer->frames);
}



image_t create_image(renderer_t *renderer, uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits sample_count, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
    image_t image;
    
    VkExtent3D image_extent = {
        .width = width,
        .height = height,
        .depth = 1
    };
    
    VkImageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = image_extent,
        .mipLevels = mip_levels,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = sample_count,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if(vkCreateImage(renderer->logical_device, &create_info, NULL, &image.image) != VK_SUCCESS) {
        error(1, "Failed to create image\n");
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(renderer->logical_device, image.image, &memory_requirements);

    uint32_t memory_type = select_memory_type(renderer->physical_device, memory_requirements.memoryTypeBits, properties);
    if(memory_type == ~0) {
        error(1, "Failed to find suitable memory type\n");
    }

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type
    };

    if(vkAllocateMemory(renderer->logical_device, &alloc_info, NULL, &image.memory) != VK_SUCCESS) {
        error(1, "Failed to allocate image memory\n");
    }

    vkBindImageMemory(renderer->logical_device, image.image, image.memory, 0);
    return image;
}

void destroy_image(image_t *allocated_image, VkDevice logical_device) {
    vkDestroyImage(logical_device, allocated_image->image, NULL);
    vkFreeMemory(logical_device, allocated_image->memory, NULL);
}

void transition_image(VkCommandBuffer command_buffer, VkImage image, VkImageLayout source_layout, VkImageLayout destination_layout, VkPipelineStageFlags source_stage, VkPipelineStageFlags destination_stage, uint32_t mip_levels) {

}



host_buffer_t create_host_buffer(renderer_t *renderer, VkDeviceSize device_size, VkQueue queue) {
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
    void *mapped_memory;

    create_buffer(&buffer, &buffer_memory, renderer->logical_device, renderer->physical_device, device_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkMapMemory(renderer->logical_device, buffer_memory, 0, device_size, 0, &mapped_memory);

    host_buffer_t host_buffer = {
        .buffer = buffer,
        .memory = buffer_memory,
        .mapped_memory = mapped_memory
    };

    return host_buffer;
}

buffer_t create_vertex_buffer(renderer_t *renderer, uint32_t vertex_count, size_t vertex_size, void *vertices, VkQueue queue, VkCommandBuffer command_buffer) {
    VkDeviceSize buffer_size = vertex_count*vertex_size;
    buffer_t staging_buffer, vertex_buffer;

    create_buffer(&staging_buffer.buffer, &staging_buffer.memory, renderer->logical_device, renderer->physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void *data;
    vkMapMemory(renderer->logical_device, staging_buffer.memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices, buffer_size);
    vkUnmapMemory(renderer->logical_device, staging_buffer.memory);

    create_buffer(&vertex_buffer.buffer, &vertex_buffer.memory, renderer->logical_device, renderer->physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copy_buffer(vertex_buffer.buffer, staging_buffer.buffer, renderer->logical_device, command_buffer, queue, buffer_size);

    destroy_buffer(&staging_buffer, renderer->logical_device);
    return vertex_buffer;
}

buffer_t create_index_buffer(renderer_t *renderer, uint32_t index_count, uint16_t indices[], VkQueue queue, VkCommandBuffer command_buffer) {
    VkDeviceSize buffer_size = index_count*sizeof(uint16_t);
    buffer_t staging_buffer, index_buffer;

    create_buffer(&staging_buffer.buffer, &staging_buffer.memory, renderer->logical_device, renderer->physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void *data;
    vkMapMemory(renderer->logical_device, staging_buffer.memory, 0, buffer_size, 0, &data);
    memcpy(data, indices, buffer_size);
    vkUnmapMemory(renderer->logical_device, staging_buffer.memory);

    create_buffer(&index_buffer.buffer, &index_buffer.memory, renderer->logical_device, renderer->physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copy_buffer(index_buffer.buffer, staging_buffer.buffer, renderer->logical_device, command_buffer, queue, buffer_size);
    
    destroy_buffer(&staging_buffer, renderer->logical_device);
    return index_buffer;
}

void destroy_buffer(buffer_t *buffer, VkDevice logical_device) {
    vkDestroyBuffer(logical_device, buffer->buffer, NULL);
    vkFreeMemory(logical_device, buffer->memory, NULL);
}

void destroy_host_buffer(host_buffer_t *buffer, VkDevice logical_device) {
    vkDestroyBuffer(logical_device, buffer->buffer, NULL);
    vkFreeMemory(logical_device, buffer->memory, NULL);
}



void clean_up_frame(frame_t *frame, VkDevice logical_device) {
    vkDestroyFence(logical_device, frame->in_flight_fence, NULL);
    vkDestroyCommandPool(logical_device, frame->command_pool, NULL);
    vkDestroySemaphore(logical_device, frame->image_available_semaphore, NULL);
    vkDestroySemaphore(logical_device, frame->render_finished_semaphore, NULL);
}

void clean_up_frames(frame_t *frames, uint32_t frame_count, VkDevice logical_device) {
    for(uint32_t i = 0; i < frame_count; i++) {
        clean_up_frame(frames + i, logical_device);
    }

    free(frames);
}



void draw_mesh(frame_t *frame, render_object_t *object, VkDescriptorSet global_descriptor) {
    vkCmdBindPipeline(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object->material_instance->material_pipeline->pipeline);

    vkCmdBindDescriptorSets(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object->material_instance->material_pipeline->layout, 0, 1, &global_descriptor, 0, NULL);
    vkCmdBindDescriptorSets(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object->material_instance->material_pipeline->layout, 1, 1, &object->material_instance->descriptor, 0, NULL);
    vkCmdPushConstants(frame->command_buffer, object->material_instance->material_pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_data_t), &object->push_constant);
    
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(frame->command_buffer, 0, 1, object->vertex_buffer, offsets);
    vkCmdBindIndexBuffer(frame->command_buffer, object->index_buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(frame->command_buffer, object->index_count, 1, object->first_index, 0, 0);
}

void draw_frame(engine_t *engine, uint32_t frame_index) {
    uint32_t image_index = begin_frame(engine, frame_index);


    end_frame(engine, frame_index, image_index);
}

uint32_t begin_frame(engine_t *engine, uint32_t frame_index) {
    renderer_t *renderer = &engine->renderer;
    frame_t *frame = &renderer->frames[frame_index];
    
    vkWaitForFences(renderer->logical_device, 1, &frame->in_flight_fence, VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(renderer->logical_device, renderer->swapchain, UINT64_MAX, frame->image_available_semaphore, VK_NULL_HANDLE, &image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain(renderer, engine->window);
    } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        error(1, "Failed to acquire swap chain image!");
    }

    vkResetFences(renderer->logical_device, 1, &frame->in_flight_fence);
    vkResetCommandBuffer(frame->command_buffer, 0);
    begin_command_buffer(frame->command_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    return image_index;
}

void end_frame(engine_t *engine, uint32_t frame_index, uint32_t image_index) {
    renderer_t *renderer = &engine->renderer;
    frame_t *frame = &renderer->frames[frame_index];
    VkImage *image = &renderer->swapchain_images[image_index];

    VkSemaphore wait_semaphore[] = {frame->image_available_semaphore};
    VkSemaphore signal_semaphore[] = {frame->render_finished_semaphore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphore,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &frame->command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphore
    };

    end_command_buffer(frame->command_buffer);
    if(vkQueueSubmit(renderer->queues.graphics_queue, 1, &submit_info, frame->in_flight_fence) != VK_SUCCESS) {
        error(1, "Failed to submit draw command buffer");
    }

    VkSwapchainKHR swapchains[] = {renderer->swapchain};
    
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphore,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &image_index,
        .pResults = NULL
    };

    VkResult result = vkQueuePresentKHR(renderer->queues.graphics_queue, &present_info);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain(renderer, engine->window);
    } else if(result != VK_SUCCESS) {
        error(1, "Failed to present swap chain image");
    }
}

void main_loop(engine_t *engine) {
    uint32_t frame_index = 0;
    uint32_t frames_in_flight = engine->renderer.frame_count;

    while(!window_should_close(engine->window)) {
        window_update();

        draw_frame(engine, frame_index);
        frame_index = (frame_index + 1) % frames_in_flight;
    }
}





