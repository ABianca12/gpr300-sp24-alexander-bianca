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

	Framebuffer createFrameBuffer(unsigned int width, GLuint height, GLuint colorFormat);
	Framebuffer createHDR_FramBuffer(unsigned int width, unsigned int height);
}