#version 450 core

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 mat;
	vec4 color;
} push;

void main() {
	outColor = push.color;
}