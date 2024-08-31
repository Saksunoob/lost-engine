#version 450 core

layout (location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform TilemapInfo {
	mat4 mat;
	ivec2 tilemap_size;
	ivec2 atlas_size;
	ivec4 tilemap[256*256];
} tilemap_info;

layout(set=0, binding=1) uniform sampler2D atlas;

layout (location = 0) in vec2 UV;

int get_tilemap(int index) {
	int i = index/4;
	int c = index%4;
	return tilemap_info.tilemap[i][c];
}

void main() {
	ivec2 pos = ivec2(UV * tilemap_info.tilemap_size);
	int tile_id = get_tilemap(pos.y*tilemap_info.tilemap_size.x+pos.x);
	int tile_y = tile_id/tilemap_info.atlas_size.x;
	int tile_x = tile_id - tile_y*tilemap_info.atlas_size.x;
	vec2 cell_size = 1./tilemap_info.atlas_size;
	vec2 start = vec2(tile_x,tile_y)*cell_size;

	vec2 rel_pos = mod(UV,(1./tilemap_info.tilemap_size))/(vec2(tilemap_info.atlas_size)/vec2(tilemap_info.tilemap_size));

	vec4 color = texture(atlas, start+rel_pos);

	if (color.a < 0.5) {
		discard;
	}
	outColor = color;
}