#version 450

out vec4 FragColor;

struct Material
{
	float ambient;
	float diffuse;
	float specular;
	float shininess;
};

struct Light
{
	vec3 color;
	vec3 position;
};

in Surface
{
	in vec3 vs_normal;
	in vec3 vs_lightPosition;
	in vec3 vs_worldPosition;
	in vec2 vs_texcoord;
} vs_out;

uniform Material material;
uniform sampler2D shadowMap;
uniform vec3 cameraPosition;
uniform Light light;
uniform float bias = 0.005;

float shadowCalculation(vec3 fragPositionLightspace)
{
	// Perspective divide
	vec3 projCoords = fragPositionLightspace;
	projCoords = (projCoords * 0.5) + 0.5;

	float lightDepth = texture(shadowMap, projCoords.xy).r;
	float cameraDepth = projCoords.z;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
	    {
		    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += cameraDepth - bias > pcfDepth ? 1.0 : 0.0;        
	    }    
	}

	shadow /= 9.0;

	return shadow;
}

vec3 blinnphong(vec3 normal, vec3 fragPos)
{
	// Norms
	vec3 viewDir = normalize(cameraPosition - fragPos);
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// Dot products
	float ndotl = max(dot(normal,lightDir),0.0);
	float ndoth = max(dot(normal,halfwayDir),0.0);

	// Light
	vec3 diffuse = vec3(ndotl * material.diffuse);
	vec3 specular =  vec3(pow(ndoth, material.shininess) * material.specular);

	return (diffuse + specular);
}

void main()
{
	vec3 normal = normalize(vs_out.vs_normal);
	float shadow = shadowCalculation(vs_out.vs_lightPosition);

	vec3 lighting = blinnphong(normal, vs_out.vs_worldPosition);
	lighting *= (1.0 - shadow);
	lighting *= light.color;
	lighting += vec3(1.0) * material.ambient;

	vec3 objectColor = normal * 0.5 + 0.5;

	FragColor = vec4(objectColor * lighting,1.0);
}