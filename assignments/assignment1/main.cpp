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

struct Framebuffer
{
	GLuint fbo;
	GLuint color0;
	GLuint color1;
	GLuint depth;
} framebuffer;

struct FullScreenQuad
{
	GLuint vao;
	GLuint vbo;
} fullscreenQuad;
// Equivelent to Framebuffer fullscreenbuffer

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

void resetCam(ew::Camera* camera, ew::CameraController* camControl)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	camControl->yaw = camControl->pitch = 0;
}

// render loop
void qwerty(ew::Shader shader, ew::Model model, GLuint texture, GLint normalMap, GLFWwindow* window, float deltaTime)
{
	// Bind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	//RENDER Clear default buffer
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	glBindTexture(GL_TEXTURE_2D, normalMap);

	shader.use();
	shader.setMat4("transform_model", glm::mat4(1.0f));
	shader.setMat4("camera_viewproj", camera.projectionMatrix() * camera.viewMatrix());
	shader.setVec3("eyePosition", camera.position);
	shader.setFloat("material.aCoff", material.aCoff);
	shader.setFloat("material.dCoff", material.dCoff);
	shader.setFloat("material.sCoff", material.sCoff);
	shader.setFloat("material.shine", material.shine);
	shader.setInt("_Maintex", 0);
	shader.setInt("_NormalMap", 1);

	suzanneTransform.rotation = glm::rotate(suzanneTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

	shader.setMat4("transform_model", suzanneTransform.modelMatrix());

	camControl.move(window, &camera, deltaTime);

	model.draw();

	// Unbind frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader lit_shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader fullscreen_shader = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader inverse_fullscreen_shader = ew::Shader("assets/inversefullscreen.vert", "assets/inversefullscreen.frag");
	ew::Shader greyscale_fullscreen_shader = ew::Shader("assets/fullscreen.vert", "assets/greyscalefullscreen.frag");
	ew::Shader blur_fullscreen_shader = ew::Shader("assets/fullscreen.vert", "assets/blur.frag");
	ew::Shader chromematic_fullscreen_shader = ew::Shader("assets/fullscreen.vert", "assets/chromematic.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.fbx");

	// Initalize camera
	camera.position = glm::vec3( 0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3( 0.0f, 0.0f, 0.0f );
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	//brick_color.jpg PavingStones138.png
	GLuint pavingTexture = ew::loadTexture("assets/stone_wall_04_diff_4k.jpg");
	GLuint pavingNormalMap = ew::loadTexture("assets/stone_wall_04_nor_gl_4k.jpg");

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

	// Initalize framebuffers
	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	// Color attachment
	glGenTextures(1, &framebuffer.color0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

	// Check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Buffer not complete!");
		return 0;
	}

	// Unbinding framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER Clear default buffer
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// This is our main render
		qwerty(lit_shader, suzanne, pavingTexture, pavingNormalMap, window, deltaTime);

		// fullscreen pipeline
		glDisable(GL_DEPTH_TEST);

		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// render fullscreen quad
		chromematic_fullscreen_shader.use();
		chromematic_fullscreen_shader.setInt("texture0", 0);
		glBindVertexArray(fullscreenQuad.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

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

	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("Ambient", &material.aCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse", &material.dCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular", &material.sCoff, 0.0f, 1.0f);
		ImGui::SliderFloat("Shine", &material.shine, 2.0f, 1024.0f);
	}

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(800, 600));

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

