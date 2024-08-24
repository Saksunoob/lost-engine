#version 450 core

layout(location=0) in vec2 position;

layout(set=0, binding=0) uniform Data {
	mat4 mat;
	vec4 color;
} data;

void main() {
	gl_Position = data.mat * vec4(position, 0.0, 1.0);
} 