#version 450 core

layout(location=0) in vec2 position;
layout(location=1) in vec2 UV;

layout(set=0, binding=0) uniform TransformMatrix {
	mat4 mat;
} transform_matrix;

layout (location = 0) out vec2 out_UV;

void main() {
	gl_Position = transform_matrix.mat * vec4(position, 0.0, 1.0);
	out_UV = UV;
} 