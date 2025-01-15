#version 450

in Surface
{
	vec3 worldPosition; // Vertex world space position
	vec3 worldNormal; // Vertex world space normal
	vec2 texcoord;
}vs_surface;

out vec4 fragColor; // Color of the fragment

uniform sampler2D _MainTex;

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

void main()
{
	// Diffusion
	// Because of screen distortion from different screen resolutions
	vec3 normal = normalize(vs_surface.worldNormal);

	// Light pointing straight down
	vec3 toLight = -lightDirection;
	float diffuseFactor = max(dot(normal, toLight), 0.0);
	
	// Specular
	vec3 toEye = normalize(eyePosition - vs_surface.worldPosition);

	// Blinn-phong half angle
	vec3 halfAngle = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, halfAngle), 0.0), material.shine);

	vec3 lightColor = (material.dCoff * diffuseFactor + material.sCoff * specularFactor) * lightColor;
	lightColor += ambientLight * material.aCoff;

	// Amount of light diffusely reflecting off surface
	//vec3 diffuseColor = lightColor * diffuseFactor;

	vec3 albedo = texture(_MainTex, vs_surface.texcoord).rgb;
	
	//vec3 objectColor = albedo * diffuseColor;
	//fragColor = vec4(objectColor * diffuseColor, 1.0);

	fragColor = vec4(albedo * lightColor, 1.0);
}