#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform float offsetR = 0.009;
uniform float offsetG = 0.006;
uniform float offsetB = -0.006;

const vec3 offset = vec3(offsetR, offsetG, offsetB);
const vec2 direction = vec2(1.0);

uniform sampler2D texture0;

void main()
{
	FragColor.r = texture(texture0, vs_texcoord + (direction * vec2(offset.r))).r;
	FragColor.g = texture(texture0, vs_texcoord + (direction * vec2(offset.g))).g;
	FragColor.b = texture(texture0, vs_texcoord + (direction * vec2(offset.b))).b;
}