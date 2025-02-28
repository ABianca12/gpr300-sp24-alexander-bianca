#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;
layout (location = 3) in vec3 in_tangent;

uniform mat4 model;
uniform mat4 cameraViewproj;
uniform mat4 lightSpaceMatrix;

out Surface
{
	vec3 vs_fragWorldPos;
	vec3 vs_normal;
	vec2 vs_texcoord;
	vec4 vs_fragLightPos;
	mat3 vs_TBN;
} vs_out;

void main()
{
	vs_out.vs_fragWorldPos = vec3(model * vec4(in_position, 1.0));
	vs_out.vs_normal = transpose(inverse(mat3(model))) * in_normal;

	vec3 T = normalize(vec3(model * vec4(in_tangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(in_normal, 0.0)));

	T = normalize(T - dot(T, N) * N);

	vec3 B = cross(N, T);

	vs_out.vs_TBN = transpose(mat3(T, B, N));
	vs_out.vs_texcoord = in_texcoord;
	vs_out.vs_fragLightPos = lightSpaceMatrix * vec4(vs_out.vs_fragWorldPos, 1.0);

	gl_Position = cameraViewproj * model * vec4(in_position, 1.0);
}