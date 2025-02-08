#pragma once

#include "../ew/external/glad.h"
#include <stdio.h>

namespace ab
{
	struct Framebuffer
	{
		GLuint fbo;
		GLuint colorBuffer[8];
		GLuint depthBuffer;
		unsigned int width;
		unsigned int height;
	};

	Framebuffer createFramebuffer(unsigned int width, unsigned int height, GLuint colorFormat);
	Framebuffer createHDR_Framebuffer(unsigned int width, unsigned int height);
}