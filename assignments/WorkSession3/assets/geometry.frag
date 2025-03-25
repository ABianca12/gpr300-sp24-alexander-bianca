#version 450

layout (location= 0) out vec2 in_position;
layout (location= 1) out vec2 in_texcoord;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D albedo;
uniform sampler2D lighting;

void main()
{
	vec3 color = texture(albedo, vs_texcoord).rgb;
	vec3 light = texture(lighting, vs_texcoord).rgb;

	FragColor = vec4(color * light, 1.0);
}