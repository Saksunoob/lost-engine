#version 450 core

out vec4 FragColor;

in vec2 UV;
uniform sampler2D texture_image;

void main() {
	FragColor = texture(texture_image, UV);
}