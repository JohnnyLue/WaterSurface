#pragma once
#include <iostream>

class WaterFrameBuffers {

	 int REFLECTION_WIDTH = 320*4;
	 int REFLECTION_HEIGHT = 180*4;
	 int REFRACTION_WIDTH = 1280;
	 int REFRACTION_HEIGHT = 720;

	 GLuint reflectionFrameBuffer;
	 GLuint reflectionTexture;
	 GLuint reflectionDepthBuffer;
	 
	 GLuint refractionFrameBuffer;
	 GLuint refractionTexture;
	 GLuint refractionDepthTexture;

public:
	WaterFrameBuffers() {//call when loading the game
		initialiseReflectionFrameBuffer();
		initialiseRefractionFrameBuffer();
	}

	void cleanUp() {//call when closing the game
		glDeleteFramebuffers(1,&reflectionFrameBuffer);
		glDeleteTextures(1,&reflectionTexture);
		glDeleteRenderbuffers(1, &reflectionDepthBuffer);
		glDeleteFramebuffers(1, &refractionFrameBuffer);
		glDeleteTextures(1, &refractionTexture);
		glDeleteTextures(1, &refractionDepthTexture);
	}

	void bindReflectionFrameBuffer() {//call before rendering to this FBO
		bindFrameBuffer(reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
	}

	 void bindRefractionFrameBuffer() {//call before rendering to this FBO
		bindFrameBuffer(refractionFrameBuffer, REFRACTION_WIDTH, REFRACTION_HEIGHT);
	}

	 void unbindCurrentFrameBuffer() {//call to switch to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 590, 590);
	}

	 int getReflectionTexture() {//get the resulting texture
		return reflectionTexture;
	}

	 int getRefractionTexture() {//get the resulting texture
		return refractionTexture;
	}

	 int getRefractionDepthTexture() {//get the resulting depth texture
		return refractionDepthTexture;
	}

	 void initialiseReflectionFrameBuffer() {
		reflectionFrameBuffer = createFrameBuffer();
		reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
		reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
		unbindCurrentFrameBuffer();
	}

	 void initialiseRefractionFrameBuffer() {
		refractionFrameBuffer = createFrameBuffer();
		refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
		refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
		unbindCurrentFrameBuffer();
	}

	 void bindFrameBuffer(int frameBuffer, int width, int height) {
		glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, width, height);
	}

	 int createFrameBuffer() {
		 GLuint frameBuffer;
		 glGenFramebuffers(1, &frameBuffer);
		 glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		 glDrawBuffer(GL_COLOR_ATTACHMENT0);
		return frameBuffer;
	}

	 int createTextureAttachment(int width, int height) {
		 GLuint texColorBuffer;
		 glGenTextures(1, &texColorBuffer);
		 glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 glBindTexture(GL_TEXTURE_2D, 0);
		 glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
		return texColorBuffer;
	}

	 int createDepthTextureAttachment(int width, int height) {
		 GLuint texture;
		 glGenTextures(1, &texture);
		 glBindTexture(GL_TEXTURE_2D, texture);
		 glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 glBindTexture(GL_TEXTURE_2D, 0);
		 glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

		 return texture;
	}

	 int createDepthBufferAttachment(int width, int height) {
		 
		 GLuint depthBuffer;
		 glGenRenderbuffers(1, &depthBuffer);
		 glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		 glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // Use a single renderbuffer object for both a depth AND stencil buffer.
		 glBindRenderbuffer(GL_RENDERBUFFER, 0);
		 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		return depthBuffer;
	}

};