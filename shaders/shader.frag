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
vec2 kaleido(vec2 uv)
{
	float th = atan(uv.y, uv.x);
	float r = pow(length(uv), 1.);

    float p1 = sin(2. * pi / 10.);
    float q = 2. * pi / ( 5. + 4. * p1);
    th = abs(mod(th, q) - 0.5 * q);
	return vec2(cos(th), sin(th)) * pow(r, 1.3 + 1.3 / (1.3 + sin(2. * pi / 3.))) * .1;
}

void main() {
    //kaleido(10*(uv - 0.5))
    out_color = vec4(texture(sampler2D(texture_image, texture_sampler), uv+push.t*vec2(-1,1)));
}
