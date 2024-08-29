#version 450 core

layout (location = 0) out vec4 outColor;

layout (location=0) in vec4 color;

void main() {
	if (color.a < 0.5) {
		discard;
	}
	outColor = color;
}