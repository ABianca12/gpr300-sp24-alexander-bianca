#version 450

out vec4 fragColor; // Color of the fragment

uniform sampler2D _MainTex;
uniform vec3 waterColor;
in vec3 toCamera;
in vec2 vs_texcoords;

uniform float tiling;

uniform float time;

uniform float b1;
uniform float b2;

const vec3 reflectWater = vec3(1.0, 0.0, 0.0);

void main()
{
	float fresnelFactor = dot(normalize(toCamera), vec3(0.0, 1.0, 0.0));
	//vec2 direction = normalize(vec2(1.0));
	vec2 uv = (vs_texcoords * tiling) + time;

	uv.y += 0.01 * (sin(uv.x * 3.5 + time * 0.35) + sin(uv.x * 4.8 + time * 1.05) + sin(uv.x * 7.3 + time * 0.45)) / 3.0;
    uv.x += 0.12 * (sin(uv.y * 4.0 + time * 0.5) + sin(uv.y * 6.8 + time * 0.75) + sin(uv.y * 11.3 + time * 0.2)) / 3.0;
    uv.y += 0.12 * (sin(uv.x * 4.2 + time * 0.64) + sin(uv.x * 6.3 + time * 1.65) + sin(uv.x * 8.2 + time * 0.45)) / 3.0;

	vec4 smp1 = texture(_MainTex, uv);
	vec4 smp2 = texture(_MainTex, uv + vec2(0.2));

	//vec3 color = mix(reflectWater, waterColor, fresnelFactor);
	vec3 color = waterColor + vec3(smp1.r * b1 - smp2.r * b2);

	vec3 objectColor = texture(_MainTex, vs_texcoords * tiling + time).rgb;
	fragColor = vec4(color, 1.0);
}
	