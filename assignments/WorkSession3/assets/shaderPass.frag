#version 450

layout (location = 3) out vec4 fragColor3;

in vec2 vs_texcoord;

uniform sampler2D texturePos;
uniform sampler2D textureNormal;
//uniform sampler2D oldShaderPass;
uniform vec3 camPos;

struct PointLight
{
	vec3 color;
	vec3 pos;
	float radius;
} light;

#define maxLights 128
uniform PointLight pointLights[maxLights];

struct Material 
{
	float ambientCoff;
	float diffuseCoff;
	float specularCoff;
	float shine;
} material;

vec3 blinnPhong(vec3 normal, vec3 fragPos, vec3 lightDir)
{
	vec3 viewDir = normalize(camPos - fragPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float nDotL = max(dot(normal, lightDir), 0.0);
	float nDotH = max(dot(normal, halfDir), 0.0);

	vec3 diffuse = vec3(nDotL * material.diffuseCoff);
	vec3 specular = vec3(pow(nDotH, material.shine) * material.specularCoff);

	return (diffuse + specular);
}

float attenuateExponential(float distance, float radius){
	float i = clamp(1.0 - pow(distance/radius,4.0),0.0,1.0);
	return i * i;
	
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 pos){
	vec3 diff = light.pos - pos;
	//Direction toward light position
	vec3 toLight = normalize(diff);
	//TODO: Usual blinn-phong calculations for diffuse + specular
	vec3 lightColor = blinnPhong(normal, pos, toLight) * light.color;
	//Attenuation
	float d = length(diff); //Distance to light
	lightColor *= attenuateExponential(d, light.radius); //See below for attenuation options
	return lightColor;
}

void main()
{
	vec3 posColor = texture(texturePos, vs_texcoord).xyz;
	vec3 normalColor = texture(textureNormal, vs_texcoord).xyz;
	vec3 totalLight = vec3(0);

//	//vec3 oldShaderPassColor = texture(oldShaderPass, vs_texcoord).xyz;
//
	vec3 lightDir = normalize(light.pos - posColor);
//
//	vec3 blinnPhongLighting = blinnPhong(normalColor, posColor, lightDir);

	//totalLight += lightDir

	//vec3 completeLighting = light.color * (material.ambientCoff + blinnPhongLighting);

	//fragColor3 = vec4(completeLighting + oldShaderPassColor, 1.0);
}