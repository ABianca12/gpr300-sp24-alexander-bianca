#version 450

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D albedo;
uniform sampler2D lighting;

void main()
{
	vec3 objectColor = texture(albedo, vs_texcoord).rgb;
	vec3 lightingColor = texture(lighting, vs_texcoord).rgb;
   
	vec3 completeLighting = objectColor * lightingColor;

	FragColor = vec4(completeLighting, 1.0);
}