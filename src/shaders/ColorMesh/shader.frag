#version 450 core

layout (location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform Data {
	mat4 mat;
	vec4 color;
} data;

void main() {
	outColor = data.color;
}