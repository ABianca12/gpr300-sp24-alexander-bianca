#version 450

out vec4 fragColor;

in vec2 vs_texcoord;

uniform sampler2D albedo;
uniform sampler2D lighting;
uniform sampler2D material;

uniform vec3 camPos;

void main() 
{
   vec3 objectColor = texture(albedo, vs_texcoord).rgb;
   vec3 lightingColor = texture(lighting, vs_texcoord).rgb;
   vec3 matTexture = texture(material, vs_texcoord).rgb;
   
   vec3 completeLighting = objectColor * lightingColor;

   fragColor = vec4(completeLighting, 1.0);
}