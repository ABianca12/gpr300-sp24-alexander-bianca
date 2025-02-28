#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

out Surface
{
	vec3 vs_normal;
	vec3 vs_lightPosition;
	vec3 vs_worldPosition;
	vec2 vs_texcoord;
} vs_in;

uniform mat4 model;
uniform mat4 cameraViewProj;
uniform mat4 lightViewProj;

void main()
{
	vec4 worldPos = model * vec4(in_position, 1.0);

	vs_in.vs_worldPosition = worldPos.xyz;
	vs_in.vs_lightPosition = vec3(lightViewProj * worldPos);
	vs_in.vs_normal = transpose(inverse(mat3(model))) * in_normal;
	vs_in.vs_texcoord = in_texcoord;

	gl_Position = cameraViewProj * worldPos;
}