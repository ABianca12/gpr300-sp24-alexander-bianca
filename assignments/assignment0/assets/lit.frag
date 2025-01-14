#version 450

in Surface
{
	vec3 worldPosition;
	vec3 worldNormal;
	vec2 texcoord;
}vs_surface;

out vec4 fragColor;

uniform sampler2D _MainTex;

// Light Source
uniform vec3 lightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 lightColor = vec3(1.0, 0.0, 1.0);

void main()
{
	// Because of screen distortion from different screen resolutions
	vec3 normal = normalize(vs_surface.worldNormal);

	vec3 toLight = lightDirection;
	float difuseFactor = max(dot(normal, toLight), 0.0);

	vec3 difuseColor = lightColor * difuseFactor;

	vec3 albedo = texture(_MainTex, vs_surface.texcoord).rgb;
	vec3 objectColor = albedo * difuseColor;

	//fragColor = vec4(vs_normal * 0.5, 1.0);
	fragColor = vec4(objectColor * difuseColor, 1.0);
}