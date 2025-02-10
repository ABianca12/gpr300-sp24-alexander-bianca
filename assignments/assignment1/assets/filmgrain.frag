#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;

uniform float strength = 0.2;

void main()
{	
	// From: https://gameidea.org/2023/12/01/film-grain-shader/
	float noise = (fract(sin(dot(vs_texcoord, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 2.0;

	vec3 color = texture(texture0, vs_texcoord).rgb;

	vec3 color2 = color - noise * strength;
	
	FragColor = vec4(color2, 1.0);
}