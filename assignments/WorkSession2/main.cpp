#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
//#include <ew/externalshader.h>

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

#include <tuple>

#include <ew/procGen.h>
ew::Mesh plane;

struct
{
	glm::vec3 color = glm::vec3(0.0f, 0.31f, 0.3f);
	float tiling = 1.0f;
	float waveStrength = 1.0;
	float waveScale = 1.0;
	float textureScale = 1.0;
	float specScale = 1.0;

} debug;

void resetCam(ew::Camera* camera, ew::CameraController* camControl)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	camControl->yaw = camControl->pitch = 0;
}

// render loop
void qwerty(ew::Shader shader, ew::Mesh plane, GLuint texture, GLuint texture2, GLuint texture3, GLFWwindow* window, float deltaTime)
{
	// 1. pipeline defenition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Backface culling
	glEnable(GL_DEPTH_TEST); // Depth testing

	// 2. gfx pass, a pass is what is going onto the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture3);

	shader.use();
	shader.setVec3("cameraPos", camera.position);
	shader.setMat4("camera_viewproj", camera.projectionMatrix() * camera.viewMatrix());
	shader.setMat4("transform_model", glm::mat4(1.0f));
	shader.setVec3("waterColor", debug.color);
	shader.setFloat("tiling", debug.tiling);
	shader.setFloat("time", glfwGetTime());
	shader.setFloat("warpStrength", debug.waveStrength);
	shader.setFloat("specScale", debug.specScale);
	shader.setFloat("textureScale", debug.textureScale);
	shader.setFloat("waveScale", debug.waveScale);
	shader.setInt("waterSpec", 0);
	shader.setInt("waterTex", 1);
	shader.setInt("waterWave", 2);

	camControl.move(window, &camera, deltaTime);

	plane.draw();
}

int main() {
	GLFWwindow* window = initWindow("WorkSession2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader lit_shader = ew::Shader("assets/worksession2-assets/lit.vert", "assets/worksession2-assets/lit.frag");

	plane.load(ew::createPlane(50.0, 50.0, 100.0));

	// Initalize camera
	camera.position = glm::vec3( 0.0f, 10.0f, 5.0f);
	camera.target = glm::vec3( 0.0f, 0.0f, 0.0f );
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	GLuint waterSpec = ew::loadTexture("assets/worksession2-assets/doubledash/wave_spec.png");
	GLuint watertex = ew::loadTexture("assets/worksession2-assets/doubledash/wave_tex.png");
	GLuint waterWarp = ew::loadTexture("assets/worksession2-assets/doubledash/wave_warp.png");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// This is our main render
		qwerty(lit_shader, suzzane, plane, waterSpec, watertex, waterWarp, window, deltaTime);

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

	ImGui::ColorEdit3("Water Color", &debug.color[0]);
	ImGui::SliderFloat("Tiling", &debug.tiling, 1.0f, 10.0f);
	ImGui::SliderFloat("Wave Strength", &debug.waveStrength, 1.0, 10.0);
	ImGui::SliderFloat("Spec Scale", &debug.specScale, 1.0, 10.0);
	ImGui::SliderFloat("Texture Scale", &debug.textureScale, 1.0, 10.0);
	ImGui::SliderFloat("Warp Scale", &debug.waveScale, 1.0, 10.0);

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

