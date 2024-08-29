#version 450 core

layout (location = 0) out vec4 outColor;

layout(set=0, binding=1) uniform sampler2D texture_sampler;

layout (location = 0) in vec2 UV;

void main() {
	vec4 color = texture(texture_sampler, UV);
	if (color.a < 0.5) {
		discard;
	}
	outColor = color;
}