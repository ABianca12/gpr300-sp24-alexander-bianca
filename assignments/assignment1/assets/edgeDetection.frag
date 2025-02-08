#version 450

out vec4 fragColor; // Color of the fragment
in vec2 vs_texcoord;

uniform sampler2D tex;

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

const float strength = 16.0;
const float kernel[9] = float[](
	1.0, 1.0, 1.0,
	1.0, -10.0, 1.0,
	1.0, 1.0, 1.0
);

void main()
{
	vec3 average = vec3(0.0);

	for (int i = 0; i < 9; i++)
	{
		vec3 textureColor = texture(tex, vs_texcoord + offsets[i]).rgb;
		average += textureColor * (kernel[i]/strength);
	}

	vec3 albido = texture(tex, vs_texcoord).rgb;
	fragColor = vec4(vec3(average), 1.0);
}