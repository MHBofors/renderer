

#ifndef vulkan_shader_h
#define vulkan_shader_h

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

uint32_t parse_file(const char *file_name, char **buffer);

void load_shader_module(VkShaderModule *shader_module, VkDevice logical_device, const char *file_name);

VkPipelineShaderStageCreateInfo create_shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits shader_stage);

#endif