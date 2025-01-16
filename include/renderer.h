#ifndef renderer_h
#define renderer_h

#include "window.h"
#include "vulkan_render_pipeline.h"
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_buffers.h"
#include "material.h"

typedef struct buffer_t {
    VkBuffer buffer;
    VkDeviceMemory memory;
} buffer_t;

typedef struct host_buffer_t {
    VkBuffer buffer;
    VkDeviceMemory memory;
    void *mapped_memory;
} host_buffer_t;

typedef struct image_t {
    VkImage image;
    VkDeviceMemory memory;
} image_t;

typedef struct host_image_t {
    VkImage image;
    VkDeviceMemory memory;
    void *mapped_memory;
} host_image_t;

typedef struct frame_t {
    VkSemaphore image_available_semaphore, render_finished_semaphore;
    VkFence in_flight_fence;

    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
} frame_t;

typedef struct renderer_t {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    device_queues queues;
    uint32_t graphics_family;

    VkSwapchainKHR swapchain;
    VkExtent2D extent;
    VkFormat swapchain_image_format;
    uint32_t swapchain_image_count;
    VkImage *swapchain_images;
    VkImageView *swapchain_image_views;
    VkFramebuffer *framebuffers;

    VkRenderPass render_pass;
    VkFormat depth_image_format;
    image_t *depth_images;
    VkImageView *depth_image_views;

    VkCommandPool command_pool;
    VkDescriptorPool global_pool;

    uint32_t frame_index;
    uint32_t frame_count;
    frame_t *frames;
} renderer_t;

typedef struct engine_t {
    window_t window;
    renderer_t renderer;
} engine_t;

void initialise_engine(engine_t *engine);
void terminate_engine(engine_t *engine);
void run(engine_t *engine);

void initialise_renderer(renderer_t *renderer, window_t window);
void terminate_renderer(renderer_t *renderer);

void setup_swapchain(renderer_t *renderer, window_t window);
void recreate_swapchain(renderer_t *renderer, window_t window);
void terminate_swapchain(renderer_t *renderer);

void setup_render_pass(renderer_t *renderer);
void destroy_render_pass(renderer_t *renderer);

void setup_framebuffers(renderer_t *renderer);
void destroy_framebuffers(renderer_t *renderer);

void setup_depth_resources(renderer_t *renderer);
void destroy_depth_resources(renderer_t *renderer);

void setup_frame_resources(renderer_t *renderer, uint32_t frame_count);
void destroy_frame_resources(renderer_t *renderer);

host_buffer_t create_host_buffer(renderer_t *renderer, VkDeviceSize device_size, VkQueue queue);
buffer_t create_vertex_buffer(renderer_t *renderer, uint32_t vertex_count, size_t vertex_size, void *vertices, VkQueue queue, VkCommandBuffer command_buffer);
buffer_t create_index_buffer(renderer_t *renderer, uint32_t index_count, uint16_t indices[], VkQueue queue, VkCommandBuffer command_buffer);
image_t create_image(renderer_t *renderer, uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits sample_count, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
void destroy_image(image_t *allocated_image, VkDevice logical_device);
void destroy_buffer(buffer_t *buffer, VkDevice logical_device);
void destroy_host_buffer(host_buffer_t *buffer, VkDevice logical_device);

void create_frame(frame_t *frame, VkDevice logical_device, VkCommandPool command_pool);
frame_t *create_frames(VkDevice logical_device, VkCommandPool command_pool, uint32_t frame_count);
void clean_up_frame(frame_t *frames, VkDevice logical_device);
void clean_up_frames(frame_t *frames, uint32_t frame_count, VkDevice logical_device);


uint32_t begin_frame(engine_t *engine, uint32_t frame_index);
void end_frame(engine_t *engine, uint32_t frame_index, uint32_t image_index);
void draw_frame(engine_t *engine, uint32_t frame_index);
void draw_mesh(frame_t *frame, render_object_t *object, VkDescriptorSet global_descriptor);

#endif /* renderer_h */