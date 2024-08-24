#version 450 core

layout(location=0) in vec2 position;

layout(set=0, binding=0) uniform Data {
	mat4 mat;
	vec4 color;
} data;

layout(set=0, binding=1) uniform sampler2D texture_sampler;

layout (location = 0) out vec2 UV;

void main() {
	gl_Position = data.mat * vec4(position, 0.0, 1.0);
	UV = position;
} 