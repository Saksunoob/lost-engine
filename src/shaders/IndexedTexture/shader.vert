#version 450 core

layout(location=0) in vec2 position;
layout(location=1) in vec2 UV;

layout(set=0, binding=0) uniform ImageInfo {
	mat4 mat[10000];
	uvec4 indices[2500];
	ivec2 atlas_size;
} image_info;

layout (location = 0) out vec2 out_UV;
layout (location = 1) out uint index;

void main() {
	gl_Position = image_info.mat[gl_InstanceIndex] * vec4(position, 0.0, 1.0);
	out_UV = UV;
	index = image_info.indices[gl_InstanceIndex/4][gl_InstanceIndex%4];
} 