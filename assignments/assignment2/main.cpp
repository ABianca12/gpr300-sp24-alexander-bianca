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
int shadowWidth = 256;
int shadowHeight = 256;
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

#include <ew/procGen.h>
ew::Mesh plane;

static glm::vec4 lightOrbitRadius = { 2.0f, 2.0f, -2.0f, 1.0f };

struct Material
{
	// Ambient coefficent 0-1
	float aCoff = 1.0;
	// Diffuse coefficent 0-1
	float dCoff = 0.5;
	// Specular coefficent 0-1
	float sCoff = 0.5;
	// Size of specular highlight
	float shine = 125;
} material;

struct Light
{
	glm::vec3 color;
	glm::vec3 position;
} light;

struct DepthBuffer
{
	GLuint fbo;
	GLuint depth;

	void init()
	{
		//buffer code
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		//create depth
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("frame buffer is not complete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} depthBuffer;

void resetCam(ew::Camera* camera, ew::CameraController* camControl)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	camControl->yaw = camControl->pitch = 0;
}

// render loop
void qwerty(ew::Shader shader, ew::Shader shadowPass, ew::Mesh plane, ew::Model model, GLuint texture, GLint normalMap, GLFWwindow* window, float deltaTime)
{
	const auto viewProj = camera.projectionMatrix() * camera.viewMatrix();

	float nearPlane = 1.0f, farPlane = 7.5f;

	glm::mat4 lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(light.position, glm::vec3(0.0f), glm::vec3(0.0, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProj * lightView;

	//glViewport(0, 0, shadowWidth, shadowHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer.fbo);
	{
		glEnable(GL_DEPTH_TEST);
		
		glClear(GL_DEPTH_BUFFER_BIT);

		shadowPass.use();

		shadowPass.setMat4("model", glm::mat4(1.0f));
		shadowPass.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		// draw
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glViewport(0, 0, screenWidth, screenHeight);

	// 1. pipeline defenition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Backface culling
	glEnable(GL_DEPTH_TEST); // Depth testing

	// 2. gfx pass, a pass is what is going onto the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

	shader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	shader.setInt("shadowMap", 0);

	//shader.setMat4("model", glm::mat4(1.0f));
	shader.setMat4("transform_model", suzanneTransform.modelMatrix());
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader.setMat4("camera_viewproj", viewProj);

	shader.setFloat("material.aCoff", material.aCoff);
	shader.setFloat("material.dCoff", material.dCoff);
	shader.setFloat("material.sCoff", material.sCoff);
	shader.setFloat("material.shine", material.shine);

	shader.setVec3("light.color", light.color);
	shader.setVec3("light.position", light.position);

	model.draw();

	plane.draw();
}

int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader blinnShader = ew::Shader("assets/blinnphong.vert", "assets/blinnphong.frag");
	ew::Shader shadowPass = ew::Shader("assets/shadowPass.vert", "assets/shadowPass.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.fbx");

	plane.load(ew::createPlane(50.0, 50.0, 1.0));

	// Initalize camera
	camera.position = glm::vec3( 0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3( 0.0f, 0.0f, 0.0f );
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	GLuint pavingTexture = ew::loadTexture("assets/stone_wall_04_diff_4k.jpg");
	GLuint pavingNormalMap = ew::loadTexture("assets/stone_wall_04_nor_gl_4k.jpg");

	depthBuffer.init();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		camControl.move(window, &camera, deltaTime);

		suzanneTransform.rotation = glm::rotate(suzanneTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		// This is our main render
		qwerty(blinnShader, shadowPass, plane, suzanne, pavingTexture, pavingNormalMap, window, deltaTime);

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

	ImGui::Image((ImTextureID)(intptr_t)depthBuffer.depth, ImVec2(800, 600));

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

