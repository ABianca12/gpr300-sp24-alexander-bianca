#version 450

layout (location= 0) out vec4 fragColor0;
layout (location= 1) out vec4 fragColor1;
layout (location= 2) out vec4 fragColor2;

in vec3 outNormal;

in Surface
{
	vec3 worldPosition;
	vec3 worldNormal;
	vec2 texcoord;
}vs_surface;

uniform sampler2D mainTex;

void main()
{
	vec3 normal = normalize(vs_surface.worldNormal);

	vec3 objectColor0 = texture(mainTex, vs_surface.texcoord).rgb;
	vec3 objectColor1 = vec3(vs_surface.worldPosition.xyz);
	vec3 objectColor2 = vec3(normal.xyz);

	fragColor0 = vec4(objectColor0, 1.0);
	fragColor1 = vec4(objectColor1, 1.0);
	fragColor2 = vec4(objectColor2, 1.0);
}