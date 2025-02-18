#version 450

out vec4 FragColor;

//uniforms
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform sampler2D shadowMap;

in vec3 vsPosition;
in vec3 vsNormal;
in vec2 vsTexcoord;

struct Material
{
	// Ambient coefficent 0-1
	vec3 aCoff;
	// Diffuse coefficent 0-1
	vec3 dCoff;
	// Specular coefficent 0-1
	vec3 sCoff;
	// Size of specular highlight
	float shine;
} material;

struct Light
{
	vec3 color;
	vec3 position;
};

float shadowCalc(vec4 fragPosLight)
{
	// Perspective divide
	vec3 projCoords = fragPosLight.xyz / fragPosLight.xyz;
	projCoords = (projCoords * 0.5) + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float shadow = (currentDepth > closestDepth) ? 1.0 : 0.0;

	return shadow;
}

vec3 blinnphong(vec3 normal, vec3 fragPos)
{
	// Normalize inputs
	vec3 viewDirection = normalize(cameraPos - fragPos);
	vec3 lightDirection = normalize(lightPos - fragPos);
	vec3 halfDirection = normalize(lightDirection + viewDirection);

	// Dot products
	float ndot1 = max(dot(normal, lightDirection), 0.0);
	float ndoth = max(dot(normal, halfDirection), 0.0);

	vec3 diffuse = ndot1 * material.dCoff;
	vec3 specular = pow(ndoth, material.shine * 128.0) * material.sCoff;

	return (diffuse + specular);
}

void main()
{
	vec3 normal = normalize(vsNormal);
	float shadow = shadowCalc(fragPosLight);

	vec3 lighting = blinnPhong(normal, vsPosition);
	lighting *= (1.0 - shadow);
	lighting *= vec3(1.0) * material.Ambient;
	lighting = light.Color;

	vec3 objectColor = normal * 0.5 + 0.5;

	FragColor = vec4(objectColor * lighting, 1.0);
}