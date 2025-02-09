#version 450

out vec4 fragColor; // Color of the fragment
in vec2 vs_texcoord;

uniform sampler2D texture0;

uniform float strength = 1.0;

void main()
{
	vec2 transform = vs_texcoord - 0.5;
	float transformNormal = length(transform);

	vec3 color = texture(texture0, vs_texcoord).rgb;
	color *= (1 - transformNormal * strength);

	fragColor = vec4(color, 1.0);
}