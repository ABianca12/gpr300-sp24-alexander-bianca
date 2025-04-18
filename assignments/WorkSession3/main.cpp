#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
int shadowScreenWidth = 800;
int shadowScreenHeight = 600;
float prevFrameTime;
float deltaTime;

// Cache
#include <ew/shader.h>

#include <ew/model.h>

#include <ew/camera.h>
#include <ew/cameraController.h>
ew::Camera camera;
ew::CameraController camControl;

#include <ew/transform.h>
ew::Transform suzanneTransform;

#include <ew/texture.h>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include <ew/procGen.h>
ew::Mesh sphere;
ew::Mesh lightSphere;
ew::Mesh plane;

#include <cstdlib>

struct Framebuffer
{
	GLuint fbo;
	GLuint color;
	GLuint depth;
	GLuint position;
	GLuint normal;
	GLuint material;

	void init()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

		glGenTextures(1, &position);
		glBindTexture(GL_TEXTURE_2D, position);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, position, 0);

		glGenTextures(1, &normal);
		glBindTexture(GL_TEXTURE_2D, normal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normal, 0);

		glGenTextures(1, &material);
		glBindTexture(GL_TEXTURE_2D, material);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, material, 0);

		GLuint arr[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, arr);

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
} framebuffer;

struct LightFramebuffer
{
	GLuint fbo;
	GLuint color;
	GLuint depth;

	void init()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} lightFramebuffer;

struct DepthBuffer
{
	GLuint fbo;
	GLuint depth;

	void init()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			shadowScreenWidth, shadowScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, depth);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} depthBuffer;

struct FullScreenQuad
{
	GLuint vao;
	GLuint vbo;
} fullscreenQuad;

static float quadVertecies[] =
{
	// pos (x, y) texcoord (u, v)
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f,  1.0f, 1.0f, 1.0f,
};

struct Material
{
	float ambientCoff;
	float diffuseCoff;
	float specularCoff;
	float shine;
};

Material defaultMaterial = { 0.0, 0.5, 0.5, 128 };

const int maxLights = 100;

glm::vec3 lightPositions[maxLights];
glm::vec3 lightColors[maxLights];
float lightRadius = 4;

void LightDataInit()
{
	srand(0);
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			glm::vec3 lightColor = glm::vec3(rand() % 2, rand() % 2, rand() % 2);
			glm::vec3 lightPos = glm::vec3(i * 2.0f, 1.5, j * 2.0f);

			lightPositions[i * 10 + j] = lightPos;
			lightColors[i * 10 + j] = lightColor;
		}
	}
}

void RenderLights(ew::Shader lightVisibility)
{
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	lightVisibility.use();
	lightVisibility.setMat4("cameraViewproj", camera.projectionMatrix() * camera.viewMatrix());

	for (int i = 0; i < maxLights; i++)
	{
		lightVisibility.setVec3("color", lightColors[i]);
		lightVisibility.setMat4("model", glm::translate(lightPositions[i]));
		lightSphere.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void resetCam(ew::Camera* camera, ew::CameraController* camControl)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	camControl->yaw = camControl->pitch = 0;
}

void RenderMonkey(ew::Shader geoShader, GLuint texture, ew::Transform& suzanneTransform, ew::Model& model, float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				suzanneTransform.rotation = glm::rotate(suzanneTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

				geoShader.use();
				geoShader.setFloat("material.ambientCoff", defaultMaterial.ambientCoff);
				geoShader.setFloat("material.diffuseCoff", defaultMaterial.diffuseCoff);
				geoShader.setFloat("material.specularCoff", defaultMaterial.specularCoff);
				geoShader.setFloat("material.shine", defaultMaterial.shine);

				geoShader.setMat4("cameraViewproj", camera.projectionMatrix() * camera.viewMatrix());
				geoShader.setMat4("model", glm::translate(glm::vec3(i * 2.0f, 0, j * 2.0f)));
				geoShader.setInt("mainTex", 0);

				model.draw();
			}
		}
	}

	geoShader.setMat4("model", glm::translate(glm::vec3(0.0f, -5.0f, 0.0f)));

	plane.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderLightVolumes(ew::Shader lightVolume)
{
	glBindFramebuffer(GL_FRAMEBUFFER, lightFramebuffer.fbo);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthMask(GL_FALSE);

	lightVolume.use();
	lightVolume.setMat4("cameraViewproj", camera.projectionMatrix() * camera.viewMatrix());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer.position);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, framebuffer.normal);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, framebuffer.material);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, depthBuffer.depth);

	lightVolume.setInt("texturePos", 1);
	lightVolume.setInt("textureNormal", 2);
	lightVolume.setInt("textureAlbedo", 0);
	lightVolume.setInt("textureMaterial", 3);

	for (int i = 0; i < maxLights; i++)
	{
		lightVolume.setVec3("camPos", camera.position);

		lightVolume.setVec3("light.color", lightColors[i]);
		lightVolume.setVec3("light.pos", lightPositions[i]);
		lightVolume.setFloat("light.radius", lightRadius);

		lightVolume.setMat4("model", glm::translate(lightPositions[i]));
		sphere.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void RenderShaderPass(ew::Shader shaderPass)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(fullscreenQuad.vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightFramebuffer.color);

	shaderPass.use();

	shaderPass.setInt("albedo", 0);
	shaderPass.setInt("lighting", 1);
	shaderPass.setInt("material", 2);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

int main() {
	GLFWwindow* window = initWindow("Work Session 3", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader geoShader = ew::Shader("assets/geometry.vert", "assets/geometry.frag");
	ew::Shader lightVolume = ew::Shader("assets/lightVolume.vert", "assets/lightVolume.frag");
	ew::Shader shaderPass = ew::Shader("assets/shaderPass.vert", "assets/shaderPass.frag");
	ew::Shader lightVisibility = ew::Shader("assets/lightVisibility.vert", "assets/lightVisibility.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.fbx");

	// Initalize camera
	camera.position = glm::vec3( 0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3( 0.0f, 0.0f, 0.0f );
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	camera.farPlane = 1000;

	GLuint texture = ew::loadTexture("assets/stone_wall_04_diff_4k.jpg");
	GLuint normalMap = ew::loadTexture("assets/stone_wall_04_nor_gl_4k.jpg");

	framebuffer.init();
	lightFramebuffer.init();
	LightDataInit();
	depthBuffer.init();

	sphere.load(ew::createSphere(lightRadius, 10));
	lightSphere.load(ew::createSphere(0.5f, 4));
	plane.load(ew::createPlane(100, 100, 100));

	// Initalize fullscreen quad
	glGenVertexArrays(1, &fullscreenQuad.vao);
	glGenBuffers(1, &fullscreenQuad.vbo);

	// bind vao and vbo
	glBindVertexArray(fullscreenQuad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuad.vbo);

	// buffer data to vbo
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertecies), &quadVertecies, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // positions
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1); // texcoords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		camControl.move(window, &camera, deltaTime);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderMonkey(geoShader, texture, suzanneTransform, suzanne, time);

		RenderLightVolumes(lightVolume);

		RenderShaderPass(shaderPass);

		RenderLights(lightVisibility);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Text("Color");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color, ImVec2(800, 600));
	ImGui::Text("Normal");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.normal, ImVec2(800, 600));
	ImGui::Text("Position");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.position, ImVec2(800, 600));
	ImGui::Text("Material");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.material, ImVec2(800, 600));
	ImGui::Text("Lighting");
	ImGui::Image((ImTextureID)(intptr_t)lightFramebuffer.color, ImVec2(800, 600));

	ImGui::Begin("Settings");

	ImGui::Text("Material Settings");
	ImGui::SliderFloat("ambientCoff", &defaultMaterial.ambientCoff, 0.0, 1.0);
	ImGui::SliderFloat("diffuseCoff", &defaultMaterial.diffuseCoff, 0.0, 1.0);
	ImGui::SliderFloat("specularCoff", &defaultMaterial.specularCoff, 0.0, 1.0);
	ImGui::SliderFloat("shine", &defaultMaterial.shine, 2.0, 1024.0);

	ImGui::Text("Light Settings");
	ImGui::SliderFloat("Light Radius", &lightRadius, 0.0, 10.0);

	if (ImGui::Button("Reset Camera"))
	{
		resetCam(&camera, &camControl);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	// The proc address is where the the calls are located in the specific gpu
	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

