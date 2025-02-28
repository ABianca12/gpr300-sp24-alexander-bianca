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
int currentShader = 0;

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

#include <ab/framebuffer.h>
ab::Framebuffer framebuffer;
ab::Framebuffer hdrFramebuffer;

static std::vector<std::string> effects
{
	"None",
	"Blur",
	"Box Blur",
	"Chromatic Aberration",
	"Edge Detection",
	"Film Grain",
	"Fog",
	"Greyscale",
	"HDR",
	"Inverted",
	"Lens Disortion",
	"Sharp",
	"Vignette",
};

static std::vector<ew::Shader> shaders;

// Effect variables
float blurStrength = 16.0f;
float boxBlurStrength = 8.0f;
float chromeRed = 0.009;
float chromeGreen = 0.006;
float chromeBlue = -0.006;
float edgeStrength = -10.0f;
float filmGrainStrength = 0.2f;
float fogOffset = 1.0;
float fogSteepness = 1.0;
float hdrExposure = 2.0;
glm::vec3 radialDisortion = glm::vec3(1.0);
glm::vec2 tangentDistortion = glm::vec2(1.0);
float vignetteStrength = 1.0;

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
void renderLoop(ew::Shader shader, ew::Model model, GLuint texture, GLint normalMap, GLFWwindow* window, float deltaTime)
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
	shaders.push_back(fullscreen_shader);
	ew::Shader blur_fullscreen_shader = ew::Shader("assets/fullscreen.vert", "assets/blur.frag");
	shaders.push_back(blur_fullscreen_shader);
	ew::Shader box_blur_shader = ew::Shader("assets/boxblur.vert", "assets/boxblur.frag");
	shaders.push_back(box_blur_shader);
	ew::Shader chromematic_fullscreen_shader = ew::Shader("assets/fullscreen.vert", "assets/chromematic.frag");
	shaders.push_back(chromematic_fullscreen_shader);
	ew::Shader edge_shader = ew::Shader("assets/edgeDetection.vert", "assets/edgeDetection.frag");
	shaders.push_back(edge_shader);
	ew::Shader filmgrain_shader = ew::Shader("assets/filmgrain.vert", "assets/filmgrain.frag");
	shaders.push_back(filmgrain_shader);
	ew::Shader fog_shader = ew::Shader("assets/fog.vert", "assets/fog.frag");
	shaders.push_back(fog_shader);
	ew::Shader greyscale_fullscreen_shader = ew::Shader("assets/fullscreen.vert", "assets/greyscalefullscreen.frag");
	shaders.push_back(greyscale_fullscreen_shader);
	ew::Shader hdr_shader = ew::Shader("assets/hdr.vert", "assets/hdr.frag");
	shaders.push_back(hdr_shader);
	ew::Shader inverse_fullscreen_shader = ew::Shader("assets/inversefullscreen.vert", "assets/inversefullscreen.frag");
	shaders.push_back(inverse_fullscreen_shader);
	ew::Shader lendisortion_shader = ew::Shader("assets/lendisortion.vert", "assets/lendisortion.frag");
	shaders.push_back(lendisortion_shader);
	ew::Shader sharp_shader = ew::Shader("assets/sharp.vert", "assets/sharp.frag");
	shaders.push_back(sharp_shader);
	ew::Shader vignette_shader = ew::Shader("assets/vignette.vert", "assets/vignette.frag");
	shaders.push_back(vignette_shader);

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

	framebuffer = ab::createHDR_Framebuffer(screenWidth, screenHeight);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER Clear default buffer
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// This is our main render
		renderLoop(lit_shader, suzanne, pavingTexture, pavingNormalMap, window, deltaTime);

		// fullscreen pipeline
		glDisable(GL_DEPTH_TEST);

		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// render fullscreen quad

		switch (currentShader)
		{
			case 0:
				// None
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				break;
			case 1:
				// Blur
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setFloat("strength", blurStrength);
				break;
			case 2:
				// Box Blur
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setFloat("strength", boxBlurStrength);
				break;
			case 3:
				// Chrome
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setFloat("offsetR", chromeRed);
				shaders.at(currentShader).setFloat("offsetG", chromeGreen);
				shaders.at(currentShader).setFloat("offsetB", chromeBlue);
				break;
			case 4:
				// Edge
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setFloat("strength", edgeStrength);
				break;
			case 5:
				// Filmgrain
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setFloat("strength", filmGrainStrength);
			case 6:
				// Fog
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setInt("depth", 1);
				shaders.at(currentShader).setFloat("uOffset", fogOffset);
				shaders.at(currentShader).setFloat("uSteepness", fogSteepness);
				break;
			case 7:
				// Grey scale
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				break;
			case 8:
				// HDR
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setFloat("exposure", hdrExposure);
				break;
			case 9:
				// Inverted Colors
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				break;
			case 10:
				// Lens Distortion
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setVec3("radialDisortion", radialDisortion);
				shaders.at(currentShader).setVec2("tangentDistortion", tangentDistortion);
				break;
			case 11:
				// Sharp
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				break;
			case 12:
				// Vignette
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				shaders.at(currentShader).setFloat("strength", vignetteStrength);
				break;
			default:
				shaders.at(currentShader).use();
				shaders.at(currentShader).setInt("texture0", 0);
				break;
		}

		//shaders.at(currentShader).use();
		//shaders.at(currentShader).setInt("texture0", 0);
		glBindVertexArray(fullscreenQuad.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.colorBuffer[0]);
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

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.colorBuffer[0], ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.colorBuffer[1], ImVec2(800, 600));

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

	if (ImGui::CollapsingHeader("Effects"))
	{
		if (ImGui::Button("None"))
		{
			currentShader = 0;
		}
		
		if (ImGui::Button("Blur"))
		{
			currentShader = 1;
		
		}
		if (ImGui::CollapsingHeader("Blur Settings"))
		{
			ImGui::SliderFloat("Strength", &blurStrength, 10.0f, 30.0f);
		}

		if (ImGui::Button("Box Blur"))
		{
			currentShader = 2;
		}

		if (ImGui::CollapsingHeader("Box Blur Settings"))
		{
			ImGui::SliderFloat("Strength", &boxBlurStrength, 5.0f, 30.0f);
		}
		
		if (ImGui::Button("Chromematic"))
		{
			currentShader = 3;
		}
		
		if (ImGui::CollapsingHeader("Chromematic Settings"))
		{
			ImGui::SliderFloat("Red", &chromeRed, -0.010f, 0.010f);
			ImGui::SliderFloat("Green", &chromeGreen, -0.010f, 0.010f);
			ImGui::SliderFloat("Blue", &chromeBlue, -0.010f, 0.010f);
		}

		if (ImGui::Button("Edge Detection"))
		{
			currentShader = 4;
		}
		
		if (ImGui::CollapsingHeader("Edge Detection Settings"))
		{
			ImGui::SliderFloat("Strength", &edgeStrength, -15.0f, -5.0f);
		}

		if (ImGui::Button("Film Grain"))
		{
			currentShader = 5;
		}

		if (ImGui::CollapsingHeader("Film Grain Settings"))
		{
			ImGui::SliderFloat("Strength", &filmGrainStrength, 0.0f, 1.0f);
		}

		if (ImGui::Button("Fog"))
		{
			currentShader = 6;
		}

		if (ImGui::CollapsingHeader("Fog Settings"))
		{
			ImGui::SliderFloat("Fog Offset", &fogOffset, -2.0f, 2.0f);
			ImGui::SliderFloat("Fog Steepness", &fogSteepness, 0.0f, 1.0f);
		}

		if (ImGui::Button("Grey Scale"))
		{
			currentShader = 7;
		}

		if (ImGui::CollapsingHeader("Grey Scale Settings"))
		{
			ImGui::Text("None :(");
		}

		if (ImGui::Button("HDR"))
		{
			currentShader = 8;
		}

		if (ImGui::CollapsingHeader("HDR Settings"))
		{
			ImGui::SliderFloat("Exposure", &hdrExposure, 0.0f, 10.0f);
		}

		if (ImGui::Button("Inverted Colors"))
		{
			currentShader = 9;
		}

		if (ImGui::CollapsingHeader("Inverted Colors Settings"))
		{
			ImGui::Text("None :(");
		}

		if (ImGui::Button("Lens Disortion"))
		{
			currentShader = 10;
		}

		if (ImGui::CollapsingHeader("Lens Disortion Settings"))
		{
			ImGui::SliderFloat3("Radial Distortion", &radialDisortion.x, 1.0f, 100.0f);
			ImGui::SliderFloat2("Tangent Distortion", &tangentDistortion.x, 1.0f, 100.0f);
		}

		if (ImGui::Button("Sharp"))
		{
			currentShader = 11;
		}

		if (ImGui::CollapsingHeader("Sharp Settings"))
		{
			ImGui::Text("None :(");
		}

		if (ImGui::Button("Vignette"))
		{
			currentShader = 12;
		}

		if (ImGui::CollapsingHeader("Vignette Settings"))
		{
			ImGui::SliderFloat("Strength", &vignetteStrength, 0.0f, 10.0f);
		}
		
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

