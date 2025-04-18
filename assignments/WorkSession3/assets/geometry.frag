#version 450

layout (location = 0) out vec4 fragColor0;
layout (location = 1) out vec4 fragColor1;
layout (location = 2) out vec4 fragColor2;
layout (location = 3) out vec4 fragColor3;

in vec3 outNormal;

in Surface
{
	vec3 worldPosition;
	vec3 worldNormal;
	vec2 texcoord;
}vs_surface;

struct Material 
{
	float ambientCoff;
	float diffuseCoff;
	float specularCoff;
	float shine;
};

uniform Material material;

uniform sampler2D mainTex;

void main()
{
	vec3 worldNormalNorm = normalize(vs_surface.worldNormal);

	vec3 objectColor0 = texture(mainTex, vs_surface.texcoord).rgb;
	vec3 position = vec3(vs_surface.worldPosition.xyz);
	vec3 normal = vec3(worldNormalNorm.xyz);

	fragColor0 = vec4(objectColor0, 1.0);
	fragColor1 = vec4(position, 1.0);
	fragColor2 = vec4(normal, 1.0);
	fragColor3 = vec4(material.ambientCoff, material.diffuseCoff, material.specularCoff, material.shine);
}