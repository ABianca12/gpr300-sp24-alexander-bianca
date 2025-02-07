#include "framebuffer.h"

namespace ab
{
	Framebuffer createFrameBuffer(unsigned int width, GLuint height, GLuint colorFormat)
	{
		return Framebuffer();
	}

	Framebuffer ab::createHDR_FramBuffer(unsigned int width, unsigned int height)
	{
		ab::Framebuffer hdrBuffer;
		hdrBuffer.width = width;
		hdrBuffer.height = height;


		glBindTexture(GL_TEXTURE_2D, hdrBuffer.colorBuffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	}
}
