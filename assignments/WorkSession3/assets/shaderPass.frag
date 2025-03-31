#version 450

layout (location = 0) out vec4 fragColor0;

uniform sampler2D texturePos;
uniform sampler2D textureNormal;
uniform sampler2D textureAlbedo;
uniform vec3 camPos;

struct PointLight
{
	vec3 color;
	vec3 pos;
	float r;
} light;

struct Material 
{
	float ambientCoff;
	float diffuseCoff;
	float specularCoff;
	float shine;
} material;

vec3 blinnPhong(vec3 normal, vec3 worldPos, vec3 lightDir)
{
	vec3 viewDir = normalize(camPos - worldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float nDotL = max(dot(normal, lightDir), 0.0f);
	float nDotH = max(dot(normal, halfDir), 0.0f);

	vec3 diffuse = vec3(nDotL * material.diffuseCoff);
	vec3 specular = vec3(pow(nDotH, material.shine) * material.specularCoff);

	return (diffuse + specular);
}

float calculateAttentuation(float dist, float radius)
{
	float clampValue = clamp(1.0f - pow(dist / radius, 4.0f), 0.0f, 1.0f);
	return clampValue * clampValue;
}

void main()
{
	vec2 UV = gl_FragCoord.xy / textureSize(textureNormal, 0);
	vec3 posColor = texture(texturePos, UV).xyz;
	vec3 normalColor = texture(textureNormal, UV).xyz;
	vec3 albedo = texture(textureAlbedo, UV).rgb;

	vec3 toLight = light.pos - posColor;
	vec3 toLightNorm = normalize(toLight);

	float attentuation = calculateAttentuation(length(toLight), light.r);

	vec3 lightColor = blinnPhong(toLightNorm, posColor, normalColor) * light.color * attentuation;
	fragColor0 = vec4(lightColor * albedo, 1.0f);
}