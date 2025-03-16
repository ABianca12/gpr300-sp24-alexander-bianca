#version 450

layout (location = 4) out vec4 fragColor4;

uniform vec3 color;

void main() 
{
	fragColor4 = vec4(color, 1.0);
}