#version 460

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_uv;

layout( push_constant ) uniform object_block {
    mat4 model;
    float t;
} push;

layout(set = 0, binding = 0) uniform scene {
    mat4 view;
    mat4 projection;
} scene_data;

void main() {
    gl_Position = vec4(in_position, 1.0) * push.model * scene_data.view * scene_data.projection;
    //gl_Position = vec4(in_position, 1.0);
    out_color = in_color;
    out_uv = in_uv;
}
