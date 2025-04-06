#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 in_texcoord;

out vec3 outNormal;

out Surface
{
	vec3 worldPosition;
	vec3 worldNormal;
	vec2 texcoord;
}vs_surface;

uniform mat4 cameraViewproj;
uniform mat4 model;

void main()
{
	vs_surface.worldPosition = vec3(model * vec4(in_position, 1.0));
	vs_surface.worldNormal = transpose(inverse(mat3(model))) * in_normal;

	vs_surface.texcoord = in_texcoord;
	gl_Position = cameraViewproj * model * vec4(in_position, 1.0);
}