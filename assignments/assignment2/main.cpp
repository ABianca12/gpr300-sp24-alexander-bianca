// Thank you to Alex Daley for helping me out with this, gave me a lot of help with shadow calc

#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Cache
#include <ew/shader.h>

#include <ew/model.h>

#include <ew/camera.h>
#include <ew/CameraController.h>
ew::Camera camera;
ew::Camera lightCamera;
ew::CameraController camControl;

#include <ew/transform.h>
ew::Transform modelTransform;

#include <ew/texture.h>

#include <ew/procGen.h>
ew::Mesh plane;

#include <ab/framebuffer.h>
ab::Framebuffer shadowFramebuffer;

#include "glm/gtx/transform.hpp"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

int screenWidth = 1080;
int screenHeight = 1080;
int shadowWidth = 500;
int shadowHeight = 500;
float prevFrameTime;
float deltaTime;

struct Material
{
	float aCoff = 1.0;
	float dCoff = 0.5;
	float sCoff = 0.5;
	float shine = 128;
} material;

struct Light
{
	float bias = 0.004;
	float lightX = 0.0f;
	float lightY = 3.0f;
	float lightZ = 0.0f;
	float lightR = 1.0f;
	float lightG = 1.0f;
	float lightB = 1.0f;

	glm::vec3 color = glm::vec3(1.0);
	glm::vec3 position = glm::vec3(0.0, -1.0, 0.0);
} light;


void resetCam(ew::Camera* camera, ew::CameraController* camControl)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	camControl->yaw = camControl->pitch = 0;
}

void render(ew::Shader& shader, ew::Shader& shadowPass, ew::Model& model, ew::Mesh plane, float deltaTime)
{
	const auto lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	const auto lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const auto lightViewProj = lightProj * lightView;

	modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer.fbo);
	{
		glViewport(0, 0, screenWidth, screenHeight);

		glCullFace(GL_FRONT);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		shadowPass.use();
		shadowPass.setMat4("model", modelTransform.modelMatrix());
		shadowPass.setMat4("lightViewProj", lightViewProj);

		model.draw();
	}

	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, screenWidth, screenHeight);

	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	shader.use();

	shader.setMat4("model", modelTransform.modelMatrix());
	shader.setMat4("cameraViewProj", lightCamera.projectionMatrix() * lightCamera.viewMatrix());
	shader.setVec3("cameraPosition", lightCamera.position);
	shader.setMat4("lightViewProj", lightViewProj);

	shader.setVec3("light.position", glm::vec3(light.lightX, light.lightY, light.lightZ));
	shader.setVec3("light.color", glm::vec3(light.lightR, light.lightG, light.lightB));
	shader.setInt("shadowMap", 0);

	shader.setFloat("material.ambient", material.aCoff);
	shader.setFloat("material.diffuse", material.dCoff);
	shader.setFloat("material.specular", material.sCoff);
	shader.setFloat("material.shine", material.shine);

	shader.setFloat("bias", light.bias);

	model.draw();

	shader.setMat4("model", glm::translate(glm::vec3(0.0, -2.0, 0.0)));

	plane.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {

	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader blinnPhong = ew::Shader("assets/blinnphong.vert", "assets/blinnphong.frag");
	ew::Shader shadowPass = ew::Shader("assets/shadowPass.vert", "assets/shadowPass.frag");

	ew::Model suzanne = ew::Model("assets/Suzanne.obj");
	GLuint texture = ew::loadTexture("assets/stone_wall_04_diff_4k.jpg");

	ew::Mesh plane;
	plane.load(ew::createPlane(50, 50, 1));

	lightCamera.position = { 0.0f, 0.0f, 5.0f };
	lightCamera.target = { 0.0f, 0.0f, 0.0f };
	lightCamera.aspectRatio = (float)screenWidth / screenHeight;
	lightCamera.fov = 60.0f;

	shadowFramebuffer = ab::CreateShadowFrameBuffer(screenWidth, screenHeight);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		render(blinnPhong, shadowPass, suzanne, plane, deltaTime);
		camControl.move(window, &lightCamera, deltaTime);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");

	if (ImGui::Button("Reset Camera"))
	{
		resetCamera(&lightCamera, &camControl);
	}

	ImGui::Image((ImTextureID)(intptr_t)shadowFramebuffer.depth, ImVec2(800, 600));

	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("Ambient", &material.aCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse", &material.dCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular", &material.sCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Shine", &material.shine, 2.0f, 1024.0f);
	}

	if (ImGui::CollapsingHeader("Directional Light"))
	{
		ImGui::SliderFloat("Light X", &light.lightX, -10.0f, 10.0f);
		ImGui::SliderFloat("Light Y", &light.lightY, -10.0f, 10.0f);
		ImGui::SliderFloat("Light Z", &light.lightZ, -10.0f, 10.0f);

		if (ImGui::CollapsingHeader("Light Color"))
		{
			ImGui::SliderFloat("Red", &light.lightR, 0.0f, 1.0f);
			ImGui::SliderFloat("Green", &light.lightG, 0.0f, 1.0f);
			ImGui::SliderFloat("Blue", &light.lightB, 0.0f, 1.0f);
		}

		ImGui::SliderFloat("Shadow Bias", &light.bias, 0.002f, 0.01f);
	}

	ImGui::End();

	ImGui::render();
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