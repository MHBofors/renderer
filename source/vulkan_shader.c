

#include "vulkan_shader.h"
#include "vulkan_utils.h"

uint32_t parse_file(const char *file_name, char **p_buffer) {
    FILE *p_file = fopen(file_name, "rb");

    if(p_file == NULL) {
        printf("Failed to open file: %s\n", file_name);
        exit(1);
    }

    fseek(p_file, 0, SEEK_END);
    uint32_t file_size = ftell(p_file);
    fseek(p_file, 0, SEEK_SET);

    char *buffer = malloc(file_size);
    *p_buffer = buffer;
    
    uint32_t bytes_read = fread(buffer, 1, file_size, p_file);
    if(file_size != bytes_read) {
        printf("Failed to read file: %s\nMissing %u bytes", file_name, file_size-bytes_read);
        exit(1);
    }
    fclose(p_file);
    
    return file_size;
}

void load_shader_module(VkShaderModule *shader_module, VkDevice logical_device, const char *file_name) {
    char *shader_binary;
    uint32_t binary_size;
    binary_size = parse_file(file_name, &shader_binary);
    
    /* 
        Naïve casting only works if the endianness of the binary and uint32 is the same.
        Should make a more general solution
    */

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = binary_size,
        .pCode = (uint32_t *)shader_binary
    };

    if(vkCreateShaderModule(logical_device, &create_info, NULL, shader_module)) {
        error(1, "Failed to create shader module");
    }

    free(shader_binary);
}

VkPipelineShaderStageCreateInfo create_shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits shader_stage) {
    VkPipelineShaderStageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shader_stage,
        .module = shader_module,
        .pName = "main",
        .pSpecializationInfo = NULL
    };
    
    return create_info;
}