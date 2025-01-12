#version 460

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 uv;
layout(set = 0, binding = 1) uniform texture2D texture_image;
layout(set = 0, binding = 2) uniform sampler texture_sampler;

layout(location = 0) out vec4 out_color;

/*
float X = fract((gl_FragCoord.x/800 - 1.0)*16);
float Y = fract((gl_FragCoord.y/800 - 1.0)*16);
*/
float X = gl_FragCoord.x/800 - 1.0;
float Y = gl_FragCoord.y/800 - 1.0;
float pi = 3.14159;

void main() {
    out_color = vec4(frag_color * texture(sampler2D(texture_image, texture_sampler), uv));
    out_color.x *= (sin(37*pi*(X*X-Y*Y-X*Y)) + 1)/2;
    out_color.y *= (sin(11*pi*(1/(exp(X*X-Y)))) + 1)/2;
    out_color.z *= (sin(13*pi*1/(0.1+X*X+Y*Y)) + 1)/2;
}
