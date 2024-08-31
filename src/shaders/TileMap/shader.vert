#version 450 core

layout(location=0) in vec2 position;
layout(location=1) in vec2 UV;

layout(set=0, binding=0) uniform TilemapInfo {
	mat4 mat;
	ivec2 tilemap_size;
	ivec2 atlas_size;
	ivec4 tilemap[256*256];
} tilemap_info;

layout (location = 0) out vec2 out_UV;

void main() {
	gl_Position = tilemap_info.mat * vec4(position, 0.0, 1.0);
	out_UV = UV;
} 