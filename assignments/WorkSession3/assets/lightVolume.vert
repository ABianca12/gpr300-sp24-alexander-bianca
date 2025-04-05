#version 450

layout (location = 0) in vec3 in_position;

uniform mat4 cameraViewproj;
uniform mat4 model;

void main()
{
	gl_Position = cameraViewproj * model * vec4(in_position, 1.0);
}