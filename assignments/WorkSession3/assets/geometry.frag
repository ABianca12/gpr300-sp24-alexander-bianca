#version 450

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D gAlbedo;
uniform sampler2D gPos;
uniform sampler2D gNormal;

void main()
{
	vec3 color = texture(gAlbedo, vs_texcoord).rgb;
	FragColor = vec4(color, 1.0);
}