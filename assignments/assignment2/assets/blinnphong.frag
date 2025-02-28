#version 450

out vec4 FragColor;

in Surface
{
	vec3 vs_fragWorldPos;
	vec3 vs_normal;
	vec2 vs_texcoord;
	vec4 vs_fragLightPos;
	mat3 vs_TBN;
} vs_in;

struct Material
{
	float aCoff;
	float dCoff;
	float sCoff;
	float shine;
};

uniform Material material;

uniform vec3 lightColor = vec3(1.0, 0.0, 1.0);
uniform vec3 lightPosition = vec3(0.0, -1.0, 0.0);
uniform vec3 lightAmbient = vec3(1.0);

uniform sampler2D mainTex;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;

uniform vec3 cameraPos;

uniform float bias;

float shadowCalc(vec4 vs_fragLightPos)
{
	// Perspective divide
	vec3 projCoords = vs_fragLightPos.xyz / vs_fragLightPos.xyz;
	projCoords = (projCoords * 0.5) + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;

	return shadow;
}

vec3 blinnphong()
{
	vec3 normal = normalize(vs_in.vs_normal);
	normal = normalize(normal * 2.0 - 1.0);

	vec3 toLight = lightPosition;
	toLight = -toLight;

	float diffuse = max(dot(normal, toLight), 0.0);
	vec3 diffuseColor = lightColor * diffuse;

	vec3 toCam = normalize(toLight - vs_in.vs_fragWorldPos);

	vec3 lightCamNormal = normalize(toLight + toCam);

	float specular = pow(max(dot(normal, lightCamNormal), 0.0), material.shine);

	vec3 lightColor = (material.dCoff * diffuseColor + material.sCoff * specular) * lightColor;

	return lightColor;
}

void main()
{
	vec3 lightColor = blinnphong();

	vec3 objectColor = texture(mainTex, vs_in.vs_texcoord).rbg;

	float shadow = shadowCalc(vs_in.vs_fragLightPos);

	vec3 finalColor = ((lightAmbient * material.aCoff) + (1.0 - shadow) * lightColor) * objectColor;

	FragColor = vec4(finalColor, 1.0);
}