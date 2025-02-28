#include "framebuffer.h"

namespace ab
{
	Framebuffer createFramebuffer(unsigned int width, unsigned int height, GLuint colorFormat)
	{
		ab::Framebuffer framebuffer;
		framebuffer.width = width;
		framebuffer.height = height;

		// Initalize framebuffers
		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		// Color attachment
		glGenTextures(1, &framebuffer.colorBuffer[0]);
		glBindTexture(GL_TEXTURE_2D, framebuffer.colorBuffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Depth attachment
		glGenRenderbuffers(1, &framebuffer.depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// Bind framebuffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer.depthBuffer);

		// Attaching Buffers
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

	Framebuffer ab::createHDR_Framebuffer(unsigned int width, unsigned int height)
	{
		ab::Framebuffer hdrBuffer;
		hdrBuffer.width = width;
		hdrBuffer.height = height;

		// Initalize framebuffers
		glGenFramebuffers(1, &hdrBuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer.fbo);

		// Color attachment
		glGenTextures(1, &hdrBuffer.colorBuffer[0]);
		glBindTexture(GL_TEXTURE_2D, hdrBuffer.colorBuffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrBuffer.colorBuffer[0], 0);

		// Brightness attachment
		glGenTextures(1, &hdrBuffer.colorBuffer[1]);

		glBindTexture(GL_TEXTURE_2D, hdrBuffer.colorBuffer[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Attach buffers
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, hdrBuffer.colorBuffer[1], 0);

		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		glGenTextures(1, &hdrBuffer.depthBuffer);

		glBindTexture(GL_TEXTURE_2D, hdrBuffer.depthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, hdrBuffer.depthBuffer, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("frame buffer is not complete!");
			return Framebuffer();

		}

		// Unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return hdrBuffer;
	}

	Framebuffer ab::CreateShadowFrameBuffer(unsigned int width, unsigned int height)
	{
		Framebuffer framebuffer;

		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		glGenTextures(1, &framebuffer.depth);
		glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Frame buffer is not complete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return framebuffer;
	}
}
