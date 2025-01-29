#version 460

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 uv;

layout( push_constant ) uniform object_block {
    mat4 model;
    float t;
} push;

layout(set = 1, binding = 0) uniform texture2D texture_image;
layout(set = 1, binding = 1) uniform sampler texture_sampler;

layout(location = 0) out vec4 out_color;

/*
float X = fract((gl_FragCoord.x/800 - 1.0)*16);
float Y = fract((gl_FragCoord.y/800 - 1.0)*16);

float X = gl_FragCoord.x/800 - 1.0;
float Y = gl_FragCoord.y/800 - 1.0;
*/

float pi = 3.14159;
void main() {
    out_color = vec4(texture(sampler2D(texture_image, texture_sampler), uv));
}
