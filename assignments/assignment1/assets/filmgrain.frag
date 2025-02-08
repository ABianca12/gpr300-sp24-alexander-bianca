#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;

float strength = 0.1;

uniform float time;

uniform float toRadians = 3.14 / 180;

void main()
{	
	float noise = fract(100 * sin(vs_texcoord.x + vs_texcoord.y * time) * toRadians);

	vec2 textSize = textureSize(texture0, 0).xy;

	vec2 texCoord = vs_texcoord.xy / textSize;

	vec4 color = texture(texture0, texCoord);

	strength *= noise;

	color.rgb += strength;
	
	FragColor = color;
}