#version 450

layout (location= 0) in vec3 in_position; // vertx pos in model space
layout (location = 1) in vec3 in_normal;
//layout (location = 2) in vec2 

out vec3 vs_normal;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
	gl_Position = lightSpaceMatrix * model * vec4(in_position, 1.0);
}