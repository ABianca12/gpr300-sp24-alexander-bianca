#version 450

layout (location = 0) in vec3 in_position;

out vec3 normal;

uniform mat4 camera_viewproj;
uniform mat4 model;

void main()
{
	gl_Position = camera_viewproj * model * vec4(in_position, 1.0);
}