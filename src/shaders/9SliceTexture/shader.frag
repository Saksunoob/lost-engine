#version 450 core

layout (location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform SlicedData {
	mat4 mat;
    mat4 proj;
	vec2 slice_scale;
    vec2 texture_size;
	vec4 slice_borders;
} sliced_data;

layout(set=0, binding=1) uniform sampler2D texture_sampler;

layout (location = 0) in vec2 UV;

void main() {
    vec2 scale = vec2(length(vec3(sliced_data.mat[0].xyz)), length(vec3(sliced_data.mat[1].xyz)));
    scale /= sliced_data.slice_scale;
	vec2 pixelCoord = UV * scale;  // Convert to pixel coordinates

    // Determine which region of the 9-slice we're in
    bool inLeft   = pixelCoord.x < sliced_data.slice_borders.x;
    bool inRight  = pixelCoord.x > (scale.x - sliced_data.slice_borders.y);
    bool inTop    = pixelCoord.y < sliced_data.slice_borders.z;
    bool inBottom = pixelCoord.y > (scale.y - sliced_data.slice_borders.w);

    vec2 adjustedCoord = pixelCoord;

    // Adjust the coordinates for stretching/scaling
    if (inLeft) {
        adjustedCoord[0] = pixelCoord[0];
    } else if (inRight) {
        adjustedCoord[0] = (sliced_data.texture_size[0] - sliced_data.slice_borders[1]) + pixelCoord[0] - (scale[0] - sliced_data.slice_borders[1]);
    } else {
        adjustedCoord[0] = sliced_data.slice_borders[0] + mod(pixelCoord[0]-sliced_data.slice_borders[0], sliced_data.texture_size[0] - sliced_data.slice_borders[0] - sliced_data.slice_borders[1]);
    }

    if (inTop) {
        adjustedCoord[1] = pixelCoord[1];
    } else if (inBottom) {
        adjustedCoord[1] = (sliced_data.texture_size[1] - sliced_data.slice_borders[3]) + pixelCoord[1] - (scale[1] - sliced_data.slice_borders[3]);
    } else {
        adjustedCoord[1] = sliced_data.slice_borders[2] + mod(pixelCoord[1]-sliced_data.slice_borders[2], sliced_data.texture_size[1] - sliced_data.slice_borders[2] - sliced_data.slice_borders[3]);
    }

    // Sample the texture with the adjusted coordinates
    outColor = texture(texture_sampler, adjustedCoord / sliced_data.texture_size);
    //outColor = vec4(adjustedCoord / sliced_data.texture_size, 0.0, 1.0);
}