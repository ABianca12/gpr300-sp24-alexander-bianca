#version 450

in vec3 outNormal;

in Surface
{
	vec3 worldPosition; // Vertex world space position
	vec3 worldNormal; // Vertex world space normal
	vec2 texcoord;
	mat3 TBN;
}vs_surface;

out vec4 fragColor; // Color of the fragment

uniform sampler2D _MainTex;
uniform sampler2D _NormalMap;

// Light Source
uniform vec3 lightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 lightColor = vec3(1.0, 0.0, 1.0);

// Specular
uniform vec3 eyePosition;

// Ambient
uniform vec3 ambientLight = vec3(0.3, 0.4, 0.46);

struct Material
{
	// Ambient coefficent 0-1
	float aCoff;
	// Diffuse coefficent 0-1
	float dCoff;
	// Specular coefficent 0-1
	float sCoff;
	// Size of specular highlight
	float shine;
}; uniform Material material;

vec3 BDRF(vec3 pos, vec3 inDir)
{
	// lambert
	return vec3(1.0, 0.0, 0.0);
}

// vec3 viewDir = normalize(cameraPos - fragPos);
// vec3 lightDir = normalize(lightPos - fragPos);

vec3 outGoingLight(vec3 out_direction, vec3 fragPos)
{
	vec3 emitted = vec3(0.0);
	vec3 radiance = vec3(0.0);

	vec3 incoming = lightColor;
	vec3 lightDir = normalize(vs_surface.worldPosition - fragPos);
	vec3 bdrf = BDRF(fragPos, lightDir);
	float NDotL = dot(vs_surface.worldNormal, lightDir);

	radiance += bdrf * incoming * NDotL;

	return radiance + emitted;
}

void main()
{
	// Light pointing straight down
	vec3 toLight = -lightDirection;
	vec3 toEye = normalize(eyePosition - vs_surface.worldPosition);

	vec3 normal = texture(_NormalMap, vs_surface.texcoord).rgb;
	normal = normal * 2.0 - 1.0;
	normal = normalize(vs_surface.TBN * normal);

	float diffuseFactor = max(dot(normal, toLight), 0.0);

	// Blinn-phong half angle
	vec3 halfAngle = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, halfAngle), 0.0), material.shine);

	vec3 lightColor = (material.dCoff * diffuseFactor + material.sCoff * specularFactor) * lightColor;
	lightColor += ambientLight * material.aCoff;
	
	vec3 objectColor = texture(_MainTex, vs_surface.texcoord).rgb;
	fragColor = vec4(objectColor * outGoingLight(lightDirection, vs_surface.worldPosition), 1.0);
}