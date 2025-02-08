#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;

const float offset = 1.0 / 300.0;
const vec2 offsets[9] = vec2[](
	vec2(-offset, offset), // top left
	vec2(0.0, offset), // top center
	vec2(offset, offset), // top right

	vec2(-offset, 0.0), // middle left
	vec2(0.0, 0.0), // middle center
	vec2(offset, 0.0), // middle right

	vec2(-offset, -offset), // bottom left
	vec2(0.0, -offset), // bottom center
	vec2(offset, -offset) // bottom right
);

const float kernel[9] = float[](
	1.0, -1.0, 1.0,
	-1.0, 7.0, -1.0,
	1.0, -1.0, 1.0
);

void main()
{
	vec3 average = vec3(0.0);

	for (int i; i < 9; i++)
	{
		// Where we are on the texture
		vec3 local = texture(texture0, vs_texcoord + offsets[i]).rgb;
		average += local * (kernel[i]);
	}

	vec3 albedo = texture(texture0, vs_texcoord).rgb;
	FragColor = vec4(vec3(average),1.0);
}