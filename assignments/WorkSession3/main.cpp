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

struct Framebuffer
{
	GLuint fbo;
	GLuint color;
	GLuint depth;
	GLuint light;
	GLuint lighting;
	GLuint position;
	GLuint normal;

	void init()
	{
		// Bind framebuffer
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// Create color0 texture attachment
		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Bind color0 attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

		// Create color1 texture attachment
		glGenTextures(1, &position);
		glBindTexture(GL_TEXTURE_2D, position);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Bind color1 attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, position, 0);

		// Create color1 texture attachment
		glGenTextures(1, &normal);
		glBindTexture(GL_TEXTURE_2D, normal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Bind color1 attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normal, 0);

		// Create lighting texture attachment
		glGenTextures(1, &lighting);
		glBindTexture(GL_TEXTURE_2D, lighting);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Bind lighting attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, lighting, 0);

		// Configure drawing to multiple buffers (albedo, position, normal, light)
		GLuint arr[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, arr);

		// Create depth texture attachment
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Bind depth texture attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
} framebuffer;

struct FullScreenQuad
{
	GLuint vao;
	GLuint vbo;
} fullscreenQuad;

static float quadVertices[] =
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
	float ambientCoff = 0.5;
	float diffuseCoff = 0.5;
	float specularCoff = 0.5;
	float shine = 128.0;
} material;

struct PointLight
{
	glm::vec3 position;
	float radius;
	glm::vec4 color;
} pointLight;

const int maxLights = 128;
PointLight pointLights[maxLights];

void resetCam(ew::Camera* camera, ew::CameraController* camControl)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	camControl->yaw = camControl->pitch = 0;
}

void renderGeometry(ew::Shader shader, ew::Model suzanne, GLuint texture, GLuint normalMap, float deltaTime)
{
	shader.use();
	shader.setInt("mainTex", 0);
	shader.setMat4("camera_viewproj", camera.projectionMatrix() * camera.viewMatrix());

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	// 1. pipeline defenition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Backface culling
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // Depth testing
	glDepthFunc(GL_LESS);

	// 2. gfx pass, a pass is what is going onto the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMap);

			suzanneTransform.rotation = glm::rotate(suzanneTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", glm::translate(glm::vec3(i * 2.0, 0, j * 2.0)));

			suzanne.draw();
		}
	}
	// color, normals, position
	// draw spheres
	// Geo shader with color normal and postion combined with blinnphong rendered over it
	
	//shaderPassShader.use();
	//shaderPassShader.setInt("texturePos", 1);
	//shaderPassShader.setInt("textureNormal", 2);
	//shaderPassShader.setInt("oldShaderPass", 3);
	//shaderPassShader.setVec3("camPos", camera.position);
	//shaderPassShader.setVec3("light.pos", glm::vec3(2.0, 5, 2.0));
	//sphere.draw();

	//shaderPassShader.setFloat("material.ambientCoff", material.ambientCoff);
	//shaderPassShader.setFloat("material.diffuseCoff", material.diffuseCoff);
	//shaderPassShader.setFloat("material.specularCoff", material.specularCoff);
	//shaderPassShader.setFloat("material.shine", material.shine);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderLightVolumes(ew::Shader shader, ew::Mesh sphere)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	// 1. pipeline defenition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Backface culling
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // Depth testing
	glDepthFunc(GL_LESS);

	// 2. gfx pass, a pass is what is going onto the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.normal);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer.position);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);

	shader.use();

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			shader.use();
			shader.setInt("texturePos", 1);
			shader.setInt("textureNormal", 2);
			shader.setInt("oldShaderPass", 3);
			shader.setVec3("camPos", camera.position);
			shader.setInt("light.color", 2);
			shader.setMat4("light.position", glm::translate(glm::vec3(i * 2.0, 0, j * 2.0)));

			shader.setFloat("material.ambientCoff", material.ambientCoff);
			shader.setFloat("material.diffuseCoff", material.diffuseCoff);
			shader.setFloat("material.specularCoff", material.specularCoff);
			shader.setFloat("material.shine", material.shine);

			sphere.draw();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {
	GLFWwindow* window = initWindow("Work Session 3", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader geoShader = ew::Shader("assets/geometry.vert", "assets/geometry.frag");
	ew::Shader shaderPassShader = ew::Shader("assets/shaderPass.vert", "assets/shaderPass.frag");
	ew::Shader lightVisibilityShader = ew::Shader("assets/lightVisibility.vert", "assets/lightVisibility.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.fbx");

	sphere.load(ew::createSphere(pointLight.radius, 10));

	// Initalize camera
	camera.position = glm::vec3( 0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3( 0.0f, 0.0f, 0.0f );
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	camera.farPlane = 1000;

	GLuint texture = ew::loadTexture("assets/stone_wall_04_diff_4k.jpg");
	GLuint normalMap = ew::loadTexture("assets/stone_wall_04_nor_gl_4k.jpg");

	framebuffer.init();

	// Initalize fullscreen quad
	glGenVertexArrays(1, &fullscreenQuad.vao);
	glGenBuffers(1, &fullscreenQuad.vbo);

	// bind vao and vbo
	glBindVertexArray(fullscreenQuad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuad.vbo);

	// buffer data to vbo
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

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

		sphere.load(ew::createSphere(5.0, 10.0));

		//RENDER
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// This is our main render
		camControl.move(window, &camera, deltaTime);
		renderGeometry(litShader, suzanne, texture, normalMap, deltaTime); // color, position, normal
		renderLightVolumes(shaderPassShader, sphere);

		// render to fullscreen quad:
		glBindVertexArray(fullscreenQuad.vao);
		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, framebuffer.position);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, framebuffer.normal);

		geoShader.use();
		geoShader.setInt("albedo", 0);
		geoShader.setInt("lightingTex", 1);
		geoShader.setInt("light", 2);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// forward render light spheres:
		// TODO:



		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.light, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.depth, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.normal, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.position, ImVec2(800, 600));

	ImGui::Begin("Settings");

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

