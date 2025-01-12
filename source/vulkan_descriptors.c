
#include "vulkan_descriptors.h"

#define MAX_INFOS 32

descriptor_layout_builder_t initialise_layout_builder() {
    uint32_t array_size = 8;
    VkDescriptorSetLayoutBinding *bindings = malloc(array_size*sizeof(VkDescriptorSetLayoutBinding));
    
    if(bindings == NULL) {
        error(1, "Failed to allocate binding array!\n");
    }

    return (descriptor_layout_builder_t){
        .array_size = array_size,
        .binding_count = 0,
        .bindings = bindings
    };
}

void free_layout_builder(descriptor_layout_builder_t *layout_builder) {
    free(layout_builder->bindings);
}



void add_binding(descriptor_layout_builder_t *layout_builder, uint32_t binding, VkDescriptorType type, VkShaderStageFlags shader_stage) {
    if(layout_builder->array_size == layout_builder->binding_count) {
        layout_builder->array_size <<= 1;
        VkDescriptorSetLayoutBinding *bindings = realloc(layout_builder->bindings, layout_builder->array_size*sizeof(VkDescriptorSetLayoutBinding));
        
        if(bindings == NULL) {
            error(1, "Failed to allocate binding array!\n");
        } else {
            layout_builder->bindings = bindings;
        }
    }

    VkDescriptorSetLayoutBinding layout_binding = {
        .binding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .stageFlags = shader_stage
    };

    layout_builder->bindings[layout_builder->binding_count++] = layout_binding;
}

void clear_bindings(descriptor_layout_builder_t *layout_builder) {
    layout_builder->binding_count = 0;
}

VkDescriptorSetLayout build_layout(descriptor_layout_builder_t *layout_builder, VkDevice logical_device) {
    VkDescriptorSetLayout layout;
    
    create_descriptor_set_layout(&layout, logical_device, layout_builder->bindings, layout_builder->binding_count);
    return layout;
}





descriptor_allocator_t initialise_allocator(uint32_t type_count, VkDescriptorType *types, uint32_t *type_ratios) {
    uint32_t ratio_sum = 0;

    for(uint32_t i = 0; i < type_count; i++) {
        ratio_sum += type_ratios[i];
    }

    uint32_t max_sets = ratio_sum - 1;
    max_sets |= max_sets >> 1;
    max_sets |= max_sets >> 2;
    max_sets |= max_sets >> 4;
    max_sets |= max_sets >> 8;
    max_sets |= max_sets >> 16;
    max_sets++;//smallest power of two larger than ratio_sum

    VkDescriptorPool *free_pools = malloc(max_sets*sizeof(VkDescriptorPool));
    VkDescriptorPool *full_pools = malloc(max_sets*sizeof(VkDescriptorPool));

    VkDescriptorType *type_array = malloc(type_count*sizeof(VkDescriptorType));
    uint32_t *ratio_array = malloc(type_count*sizeof(uint32_t));

    if((free_pools && full_pools && type_array && ratio_array) == NULL) {
        error(1, "Failed to allocate pool array.\n");
    }

    return (descriptor_allocator_t) {
        .array_size = max_sets,
        .free_count = 0,
        .free_pools = free_pools,
        .full_pools = 0,
        .full_pools = full_pools,
    };
}
void free_allocator(descriptor_allocator_t *allocator);

void create_pool(descriptor_allocator_t *allocator);
void clear_pools(descriptor_allocator_t *allocator) {

}

VkDescriptorSet allocate_set(descriptor_allocator_t *allocator, VkDescriptorSetLayout layout);




descriptor_writer_t initialise_writer() {
    uint32_t array_size = 8;
    VkDescriptorImageInfo *image_infos = malloc(array_size*sizeof(VkDescriptorImageInfo));
    VkDescriptorBufferInfo *buffer_infos = malloc(array_size*sizeof(VkDescriptorBufferInfo));
    VkWriteDescriptorSet *writes = malloc(array_size*sizeof(VkWriteDescriptorSet));
    
    if(!(image_infos && buffer_infos && writes)) {
        error(1, "Failed to allocate descriptor write arrays!\n");
    }

    return (descriptor_writer_t) {
        .write_array_size = array_size,
        .write_count = 0,
        .writes = writes
    };
}

void free_writer(descriptor_writer_t *descriptor_writer) {
    clear_writes(descriptor_writer);
    free(descriptor_writer->writes);
}

void write_image(descriptor_writer_t *descriptor_writer, uint32_t binding, VkDescriptorType type, VkImageView image_view, VkImageLayout image_layout) {
    VkDescriptorImageInfo *image_info = malloc(sizeof(VkDescriptorImageInfo));
    *image_info = (VkDescriptorImageInfo){
        .imageLayout = image_layout,
        .imageView = image_view
    };
    
    /* If there is no write with same type/binding then add new write */
    if(descriptor_writer->write_array_size <= descriptor_writer->write_count) {
        VkWriteDescriptorSet *writes = realloc(descriptor_writer->writes, (descriptor_writer->write_array_size << 1)*sizeof(VkWriteDescriptorSet));
        
        if(writes != NULL) {
            descriptor_writer->writes = writes;
            descriptor_writer->write_array_size <<= 1;
        } else {
            error(1, "Failed to allocate image info array\n");
        }
    }

    descriptor_writer->writes[descriptor_writer->write_count++] = (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = image_info,
        .pBufferInfo = NULL
    };
}

void write_sampler(descriptor_writer_t *descriptor_writer, uint32_t binding, VkSampler sampler) {
    VkDescriptorImageInfo *sampler_info = malloc(sizeof(VkDescriptorImageInfo));
    *sampler_info = (VkDescriptorImageInfo){
        .sampler = sampler
    };
    
    /* If there is no write with same type/binding then add new write */
    if(descriptor_writer->write_array_size <= descriptor_writer->write_count) {
        VkWriteDescriptorSet *writes = realloc(descriptor_writer->writes, (descriptor_writer->write_array_size << 1)*sizeof(VkWriteDescriptorSet));
        
        if(writes != NULL) {
            descriptor_writer->writes = writes;
            descriptor_writer->write_array_size <<= 1;
        } else {
            error(1, "Failed to allocate image info array\n");
        }
    }

    descriptor_writer->writes[descriptor_writer->write_count++] = (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = sampler_info,
        .pBufferInfo = NULL
    };
}

void write_buffer(descriptor_writer_t *descriptor_writer, uint32_t binding, VkDescriptorType type, VkBuffer buffer, size_t size, size_t offset) {
    VkDescriptorBufferInfo *buffer_info = malloc(sizeof(VkDescriptorImageInfo));
    *buffer_info = (VkDescriptorBufferInfo){
        .buffer = buffer,
        .offset = offset,
        .range = size
    };
    
    /* If there is no write with same type/binding then add new write */
    if(descriptor_writer->write_array_size == descriptor_writer->write_count) {
        VkWriteDescriptorSet *writes = realloc(descriptor_writer->writes, (descriptor_writer->write_array_size << 1)*sizeof(VkWriteDescriptorSet));
        
        if(writes != NULL) {
            descriptor_writer->writes = writes;
            descriptor_writer->write_array_size <<= 1;
        } else {
            error(1, "Failed to allocate image info array\n");
        }
    }

    descriptor_writer->writes[descriptor_writer->write_count++] = (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = NULL,
        .pBufferInfo = buffer_info
    };
}

void clear_writes(descriptor_writer_t *descriptor_writer) {
    for(uint32_t i = 0; i < descriptor_writer->write_count; i++) {
        free(descriptor_writer->writes[i].pImageInfo);
        free(descriptor_writer->writes[i].pBufferInfo);
    }
    
    descriptor_writer->write_count = 0;
}

void update_set(descriptor_writer_t *descriptor_writer, VkDevice logical_device, VkDescriptorSet set) {
    for(uint32_t i = 0; i < descriptor_writer->write_count; i++) {
        descriptor_writer->writes[i].dstSet = set;
    }
    
    vkUpdateDescriptorSets(logical_device, descriptor_writer->write_count, descriptor_writer->writes, 0, NULL);
}
