#version 450

layout (location= 0) in vec3 in_position; // vertx pos in model space
layout (location= 1) in vec3 in_normal; // Vertex pos in model space
layout (location= 2) in vec2 in_texcoord;

uniform mat4 transform_model;
uniform mat4 camera_viewproj;
uniform mat3 lightViewProj;

// what is getting passed to gpu
out vec3 vsFragWorldPos;
out vec3 vsFragLightPos;
out vec3 vs_normal;
out vec2 vs_texcoord;

void main()
{
	vec4 worldPos = model * vec4(in_position, 1.0);

	vsFragWorldPos = worldPos.xyz;
	vsFragLightPos = lightViewProj * worldPos;
	vs_normal = transpose(inverse(mat3(transform_model))) * in_normal;
	vs_texcoord = in_texcoord;
	gl_Position = camera_viewproj * worldPos;
}