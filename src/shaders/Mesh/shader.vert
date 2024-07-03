#version 450 core

layout(location=0) in vec2 position;

/*layout(binding=0) uniform Matricies {
	mat4 trans;
	mat4 proj;
} matricies;*/

void main() {
	gl_Position = /*matricies.proj * matricies.trans * */vec4(position, 0.0, 1.0);
}