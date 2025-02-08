#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform vec3 radialDisortion;

void main()
{
	vec3 average = vec3(0.0);

	for (int i; i < 9; i++)
	{
		// Where we are on the texture
		vec3 local = texture(texture0, vs_texcoord + offsets[i]).rgb;
		average += local * (kerne[i] / strength);
	}

	vec3 albedo = texture(texture0, vs_texcoord).rgb;
	FragColor = vec4(vec3(average),1.0);
}