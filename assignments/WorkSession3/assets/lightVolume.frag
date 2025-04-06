#version 450

layout (location = 0) out vec4 fragColor;

uniform sampler2D texturePos;
uniform sampler2D textureNormal;
uniform sampler2D textureAlbedo;
uniform sampler2D textureMaterial;
uniform vec3 camPos;

struct PointLight
{
	vec3 color;
	vec3 pos;
	float radius;
};

uniform PointLight light;

//struct Material 
//{
//	float ambientCoff;
//	float diffuseCoff;
//	float specularCoff;
//	float shine;
//};

vec3 blinnPhong(vec3 lightDir, vec3 worldPos, vec3 normal, vec2 UV, vec4 matTexture)
{
	vec3 viewDir = normalize(camPos - worldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float nDotL = max(dot(normal, lightDir), 0.0);
	float nDotH = max(dot(normal, halfDir), 0.0);

	vec3 diffuse = vec3(nDotL * matTexture.g);
	vec3 specular = vec3(pow(nDotH, matTexture.a) * matTexture.b);

	return (diffuse + specular) * light.color;
}

float calculateAttentuation(float dist, float radius)
{
	float clampValue = clamp(1.0 - pow(dist / radius, 4.0), 0.0, 1.0);
	return clampValue * clampValue;
}

void main()
{
	vec2 UV = gl_FragCoord.xy / textureSize(textureNormal, 0);
	vec3 normColor = texture(textureNormal, UV).rgb;
	vec3 posColor = texture(texturePos, UV).rgb;
	vec3 albedo = texture(textureAlbedo, UV).rgb;
	vec4 matTexture = texture(textureMaterial, UV).rgba;

	vec3 toLight = light.pos - posColor;
	vec3 toLightNorm = normalize(toLight);

	float attentuation = calculateAttentuation(length(toLight), light.radius);

	vec3 lighting = blinnPhong(toLightNorm, posColor, normColor, UV, matTexture) + albedo * matTexture.r;

	vec3 lightColor = lighting * attentuation;
	fragColor = vec4(lightColor * albedo, 1.0f);
}