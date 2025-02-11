#version 450

layout (location= 0) in vec3 in_position; // vertx pos in model space
layout (location= 1) in vec3 in_normal; // Vertex pos in model space
layout (location= 2) in vec2 in_texcoord;

// what is getting passed to gpu
out vec2 vs_texcoords;
out vec3 toCamera;

uniform mat4 transform_model;
uniform mat4 camera_viewproj;
uniform vec3 cameraPos;
uniform float time;
uniform float strength;
uniform float scale;

float calcSurface(float x, float z) {
  float scale = 10.0;
  float y = 0.0;
  y += (sin(x * 1.0 / scale + time * 1.0) + sin(x * 2.3 / scale + time * 1.5) + sin(x * 3.3 / scale + time * 0.4)) / 3.0;
  y += (sin(z * 0.2 / scale + time * 1.8) + sin(z * 1.8 / scale + time * 1.8) + sin(z * 2.8 / scale + time * 0.8)) / 3.0;
  return y;
}

void main()
{
	vec3 pos = in_position;
	pos += calcSurface(pos.x, pos.y) * strength;
	vs_texcoords = in_texcoord;

	vec4 worldPos = transform_model * vec4(pos, 1.0);
	toCamera = cameraPos - worldPos.xyz;
	gl_Position = camera_viewproj * worldPos;
}