#version 450

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D albedo;
uniform sampler2D lightingTex;
uniform sampler2D light;

void main()
{
	vec3 color = texture(albedo, vs_texcoord).rgb;
	vec3 lightColor = texture(lightingTex, vs_texcoord).rgb;
	vec4 lighting = texture(light, vs_texcoord).rgba;

	vec3 completeLighting = lightColor * color;
	completeLighting = mix(completeLighting, lighting.rgb, lighting.a);

	FragColor = vec4(completeLighting, 1.0);
}