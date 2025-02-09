#version 450

// Sourced from https://www.youtube.com/watch?v=3xGKu4T4SCU

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform sampler2D depth;

vec3 fogColor = vec3(1.0);

float near = 0.1f;
float far = 100.0f;

uniform float uOffset;
uniform float uSteepness;

float LinearizeDepth(float depth) 
{
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));	
}

float logisticDepth(float depth, float steepness = 0.5f, float offset = 5.0f)
{
	float z = LinearizeDepth(depth);
	return (1 / (1 + exp(-steepness * (z - offset))));
}

void main()
{
	float depth = logisticDepth(texture(depth, vs_texcoord).r, uSteepness, uOffset);
	vec3 color = mix(texture(texture0, vs_texcoord).rgb, fogColor, depth);
	FragColor = vec4(color, 1.0);
}