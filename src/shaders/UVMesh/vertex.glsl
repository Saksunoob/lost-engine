#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 transform;
uniform mat4 projection;

out vec2 UV;

void main() {
	gl_Position = projection * transform * vec4(aPos, -1.0, 1.0);
	UV = aUV;
}