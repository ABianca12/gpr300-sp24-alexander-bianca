#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

out vec3 outNormal;

uniform mat4 cameraViewproj;
uniform mat4 model;

void main()
{
	gl_Position = cameraViewproj * model * vec4(in_position, 1.0);
}