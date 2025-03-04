#version 450

in vec3 outNormal;

in Surface
{
	vec3 worldPosition;
	vec3 worldNormal;
	vec2 texcoord;
	mat3 TBN;
}vs_surface;

out vec4 FragAlbedo;
out vec4 FragPos;
out vec4 FragNormal;

void main()
{
	vec3 objectColor = vs_surface.worldNormal.xyz * 0.5 + 0.5;
	FragAlbedo = vec4(objectColor, 1.0);
	FragPos = vec4(vs_surface.worldPosition, 1.0);
	FragNormal = vec4(vs_surface.worldNormal, 1.0);
}