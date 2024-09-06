#version 450 core

layout(location=0) in vec2 position;
layout(location=1) in vec2 UV;

layout(set=0, binding=0) uniform SlicedData {
	mat4 mat;
    mat4 proj;
	vec2 slice_scale;
    vec2 texture_size;
	vec4 slice_borders;
} sliced_data;

layout (location = 0) out vec2 out_UV;

void main() {
	gl_Position = sliced_data.proj * sliced_data.mat * vec4(position, 0.0, 1.0);
	out_UV = UV;
} 