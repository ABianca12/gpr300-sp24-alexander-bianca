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
#include <ew/cameraController.h>
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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
int shadowWidth = 256;
int shadowHeight = 256;
float prevFrameTime;
float deltaTime;

GLuint texture;
GLuint normalMap;

struct Material
{
	float aCoff = 1.0;
	float dCoff = 0.5;
	float sCoff = 0.5;
	float shine = 128;
} material;

struct Light
{
	glm::vec3 color = glm::vec3(1.0);
	glm::vec3 position = glm::vec3(0.0, -1.0, 0.0);
	float bias = 0.01;
} light;

void resetCam(ew::Camera* camera, ew::CameraController* camControl)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	camControl->yaw = camControl->pitch = 0;
}

// render loop
void render(ew::Shader& shader, ew::Shader& shadowPass, glm::mat4 lightMatrix, ew::Mesh plane,
	ew::Model& model, GLFWwindow* window, float deltaTime)
{
	modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer.fbo);
	{
		glViewport(0, 0, shadowWidth, shadowHeight);

		glCullFace(GL_FRONT);
		glEnable(GL_DEPTH_TEST);

		shadowPass.use();
		shadowPass.setMat4("model", modelTransform.modelMatrix());
		shadowPass.setMat4("lightSpaceMatrix", lightMatrix);

		model.draw();
	}

	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, screenWidth, screenHeight);

	// 1. pipeline defenition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Backface culling
	glEnable(GL_DEPTH_TEST); // Depth testing

	// 2. gfx pass, a pass is what is going onto the screen
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowFramebuffer.depthBuffer);

	shader.use();

	shader.setMat4("model", modelTransform.modelMatrix());
	shader.setMat4("lightSpaceMatrix", lightMatrix);
	shader.setMat4("cameraViewproj", camera.projectionMatrix() * camera.viewMatrix());
	shader.setVec3("cameraPos", camera.position);

	shader.setFloat("bias", light.bias);

	shader.setFloat("material.aCoff", material. aCoff);
	shader.setFloat("material.dCoff", material.dCoff);
	shader.setFloat("material.sCoff", material.sCoff);
	shader.setFloat("material.shine", material.shine);

	shader.setInt("mainTex", 0);
	shader.setInt("normalMap", 1);
	shader.setInt("shadowMap", 2);

	model.draw();

	plane.draw();
}

int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader blinnShader = ew::Shader("assets/blinnphong.vert", "assets/blinnphong.frag");
	ew::Shader shadowPass = ew::Shader("assets/shadowPass.vert", "assets/shadowPass.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.fbx");
	ew::Mesh plane = ew::createPlane(10, 10, 100);

	// Initalize camera
	camera.position = glm::vec3( 0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3( 0.0f, 0.0f, 0.0f );
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	lightCamera.position = light.position;
	lightCamera.target = glm::vec3( 0.0f, 0.0f, 0.0f );
	lightCamera.orthographic = true;
	lightCamera.orthoHeight = 10.0f;
	lightCamera.aspectRatio = (float)screenWidth / screenHeight;
	lightCamera.fov = 60.0f;

	texture = ew::loadTexture("assets/stone_wall_04_diff_4k.jpg");
	normalMap = ew::loadTexture("assets/stone_wall_04_nor_gl_4k.jpg");
	shadowFramebuffer = ab::CreateShadowFrameBuffer(shadowWidth, shadowHeight);

	float nearPlane = 1.0f, farPlane = 7.5f;

	glm::mat4 lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(
		glm::vec3(2.0f, 4.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		camControl.move(window, &camera, deltaTime);

		glm::mat4 lightSpaceMatrix = lightProj * lightView;

		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render(blinnShader, shadowPass, lightSpaceMatrix, plane, suzanne, window, deltaTime);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");

	if (ImGui::Button("Reset Camera"))
	{
		resetCam(&camera, &camControl);
	}

	ImGui::Image((ImTextureID)(intptr_t)shadowFramebuffer.depthBuffer, ImVec2(800, 600));

	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("Ambient", &material.aCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse", &material.dCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular", &material.sCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Shine", &material.shine, 2.0f, 1024.0f);
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

