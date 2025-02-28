#version 450

layout(location = 0) in vec3 in_position;

uniform mat4 model;
uniform mat4 lightViewProj;

void main()
{
	vec4 world_position = model * vec4(in_position, 1.0);
	gl_Position = lightViewProj * world_position;
}