#include "framebuffer.h"

namespace ab
{
	Framebuffer createFrameBuffer(unsigned int width, unsigned int height, GLuint colorFormat)
	{
		ab::Framebuffer framebuffer;
		framebuffer.width = width;
		framebuffer.height - height;

		// Initalize framebuffers
		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		// Color attachment
		glGenTextures(1, &framebuffer.colorBuffer[0]);
		glBindTexture(GL_TEXTURE_2D, framebuffer.colorBuffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Bind framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.colorBuffer[0], 0);

		// Check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Buffer not complete!");
			return Framebuffer();
		}

		// Unbinding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return framebuffer;

	}

	Framebuffer ab::createHDR_FramBuffer(unsigned int width, unsigned int height)
	{
		ab::Framebuffer hdrBuffer;
		hdrBuffer.width = width;
		hdrBuffer.height = height;


		glBindTexture(GL_TEXTURE_2D, hdrBuffer.colorBuffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

		return Framebuffer();
	}
}
