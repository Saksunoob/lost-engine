#version 450 core

layout(location=0) in vec2 position;

layout(push_constant) uniform Push {
	mat4 mat;
	vec4 color;
} push;

void main() {
	gl_Position = push.mat * vec4(position, 0.0, 1.0);
} 