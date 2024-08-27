#version 450 core

layout(location=0) in vec2 position;

layout(set=0, binding=0) uniform Data {
	mat4 mat[10000];
	vec4 color[10000];
};

layout (location = 0) out vec4 outColor;

void main() {
	gl_Position = mat[gl_InstanceIndex] * vec4(position, 0.0, 1.0);
	outColor = color[gl_InstanceIndex];
} 