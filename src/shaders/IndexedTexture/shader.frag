#version 450 core

layout (location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform ImageInfo {
	mat4 mat[10000];
	uvec4 indices[2500];
	ivec2 atlas_size;
} image_info;

layout(set=0, binding=1) uniform sampler2D atlas;

layout (location = 0) in vec2 UV;
layout (location = 1) in flat uint index;

void main() {
	uint tile_y = index/image_info.atlas_size.x;
	uint tile_x = index - tile_y*image_info.atlas_size.x;
	vec2 cell_size = 1./image_info.atlas_size;
	vec2 start = vec2(tile_x,tile_y)*cell_size;

	vec2 rel_pos = UV*cell_size;

	vec4 color = texture(atlas, start+rel_pos);

	if (color.a < 0.5) {
		discard;
	}
	outColor = color;
}