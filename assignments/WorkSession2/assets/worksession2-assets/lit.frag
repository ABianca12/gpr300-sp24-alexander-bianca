#version 450

out vec4 fragColor; // Color of the fragment

uniform sampler2D waterSpec;
uniform sampler2D waterTex;
uniform sampler2D waterWarp;
uniform vec3 waterColor;
in vec3 toCamera;
in vec2 vs_texcoords;

uniform float tiling;
uniform float time;
uniform float waveStrength;
uniform	float warpScale;
uniform	float textureScale;
uniform	float specScale;

const vec3 reflectWater = vec3(1.0, 0.0, 0.0);

void main()
{
	vec2 uv = vs_texcoords;

	vec2 warpUV = vs_texcoords * waveStrength;
	vec2 warpScroll = vec2(0.5, 0.5) * time;
	vec2 warp = texture(waterWarp, warpUV + warpScroll).xy * warpScale;
	warp = (warp * 2.0 - 1.0);

	// Albedo
	vec2 albedoUV = vs_texcoords * waveStrength;
	vec2 albedoScroll = vec2(-0.5, 0.5) * time;
	vec4 albedo = texture(waterTex, albedoUV + warp + albedoScroll);

	// Specular
	vec2 specUV = vs_texcoords * specScale;
	vec3 smp1 = texture(waterSpec, specUV = vec2(1.0, 0.0) * time).rgb;
	vec3 smp2 = texture(waterSpec, specUV = vec2(1.0, 1.0) * time).rgb;
	vec3 spec  = smp1 + smp2;

	float fresnel = dot(normalize(toCamera), vec3(0.0, 1.0, 0.0));

	fragColor = vec4(waterColor + vec3(albedo.a), 1.0);
}
	