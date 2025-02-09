#version 450

// Sourced from https://medium.com/@vlh2604/real-time-radial-and-tangential-lens-distortion-with-opengl-a55b7493e207

out vec4 FragColor;
in vec2 vs_texcoord;
in vec4 color;

uniform sampler2D texture0;
uniform vec3 radialDisortion;
uniform vec2 tangentDistortion;

vec2 RadialDisortion(vec2 coord, float k1, float k2, float k3)
{
	float radius = length(coord);
	float distortionFactor = 1.0 + k1 * pow(radius, 2) + k2 * pow(radius, 4) + k3 * pow(radius, 6);
	return distortionFactor * coord;
}

vec2 TangentDisortion(vec2 coord, float p1, float p2)
{
	float x = coord.x;
	float y = coord.y;
	float radius2 = x * x + y * y;
	float deltx = 2.0 * p1 * x * y + p2 * (radius2 + 2.0 * x * x);
	float delty = p1 * (radius2 + 2.0 * y * y) + 2.0 * p2 * x * y;
	return vec2(deltx, delty);
}

void main()
{
	vec2 coords = vs_texcoord * 2.0 - 1.0;

	// Apply Radial Disortion
	vec2 radialDisortionCoords = RadialDisortion(coords, radialDisortion.x, radialDisortion.y, radialDisortion.z);
	
	// Apply Tangental Disortion
	vec2 tangentDistortionCoords = TangentDisortion(coords, tangentDistortion.x, tangentDistortion.y);

	// Combine distored coords
	vec2 distortedCoords = radialDisortionCoords + tangentDistortionCoords;

	distortedCoords = (distortedCoords + 1.0) / 2.0;

	FragColor = color * texture(texture0, distortedCoords);
}