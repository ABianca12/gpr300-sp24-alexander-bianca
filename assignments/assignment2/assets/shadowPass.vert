#version 450

layout (location= 0) in vec3 in_position; // vertx pos in model space

// what is getting passed to gpu
out vec3 vs_normal;

uniform mat4 model;
uniform mat4 lightViewProj;

void main()
{
	vec4 worldPos = model * vec4(in_position, 1.0);
	gl_Position = lightViewProj * worldPos;
}