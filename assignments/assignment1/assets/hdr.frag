#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;

uniform sampler2D hdrBuffer;

uniform float exposure = 0;

void main()
{
	const float gamma = 2.0;

	vec3 hdrColor = texture(texture0, vs_texcoord).rgb;

	vec3 map = vec3(1.0) - exp(-hdrColor * exposure);

	map = pow(map, vec3(gamma));

	FragColor = vec4(map, 1.0);
}