#version 450 core

layout(location=0) in vec2 position;

layout(push_constant) uniform Matrix {
	mat4 mat;
} matrix;

void main() {
	gl_Position = matrix.mat * vec4(position, 0.0, 1.0);
} 