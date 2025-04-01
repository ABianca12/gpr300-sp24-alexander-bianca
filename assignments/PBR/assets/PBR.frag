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
uniform vec3 lightPosition = vec3 (1.0, 2.0, 1.0);

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
	float metalic;
	float roughness;
}; uniform Material material;

const float pi = 3.141592;

// Cahce dot products
float NDotV = 0.0;
float NDotL = 0.0;
float NDotH = 0.0;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F()
{
	return vec3(1.0);
}

vec3 G()
{
	return vec3(1.0);
}

float D()
{
	//float alpha = pow(material.roughness, 2.0);
	float alpha2 = pow(material.roughness, 4.0);
	float denom = pi * pow(pow(NDotH, 2.0) * (alpha2 - 1.0) + 1.0, 2.0);
	return alpha2 / denom;
}

// describes specular
vec3 CookTorrence(vec3 frenel)
{
	vec3 numer = D() * frenel * G();
	float denom = 4.0 * NDotV * NDotL;

	return numer / denom;
}

vec3 BDRF(vec3 pos, vec3 inDir, float NDotV)
{
	vec3 lambert = vec3(1.0, 0.0, 0.0) / pi;

	vec3 F0 = lambert;

	vec3 Ks = fresnelSchlick(NDotV, F0);
	vec3 Kd = (1 - Ks) * (1.0 - material.metalic);

	vec3 diffuse = (Kd * lambert);
	vec3 specular = Ks * CookTorrence(Ks);

	return diffuse + specular;
}

vec3 outGoingLight(vec3 out_direction, vec3 fragPos)
{
	vec3 emitted = vec3(0.0);
	vec3 radiance = vec3(0.0);

	vec3 incoming = lightColor;
	vec3 lightDir = normalize(lightPosition - fragPos);
	vec3 viewDir = normalize(eyePosition - fragPos);

	vec3 h = normalize(lightDir + viewDir);

	NDotL = dot(vs_surface.worldNormal, lightDir);
	NDotV = dot(vs_surface.worldNormal, viewDir);
	NDotH = dot(vs_surface.worldNormal, h);
	vec3 bdrf = BDRF(fragPos, lightDir, NDotV);

	radiance += bdrf * incoming * NDotL;

	return radiance + emitted;
}

void main()
{
	// Cache dot products

	// Light pointing straight down
	vec3 toLight = -lightDirection;
	vec3 toEye = normalize(eyePosition - vs_surface.worldPosition);

	vec3 normal = texture(_NormalMap, vs_surface.texcoord).rgb;
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