#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "BufferObject.h"
class ShadowMap
{
public:
	Shader shader;
	Shader shadow_shader;
	glm::ivec2 resolution;
	ShadowMap(Shader shader, Shader shadow_shader, glm::ivec2 resolution)
		:shader(shader), shadow_shader(shadow_shader)
	{
		this->resolution = resolution;
		this->fbo = this->generateFBO();
		this->id = this->fbo.texture;
	}
	glm::mat4 getViewProjectionMtx() { return this->projection_matrix * this->view_matrix; }
	glm::mat4 getViewMtx() { return this->view_matrix; }
	glm::mat4 getProjectionMtx() { return this->projection_matrix; }
	void setLight(glm::vec3 position, glm::vec3 direction, glm::vec2 range)
	{
		this->projection_matrix = glm::ortho(-range.x, range.x, -range.y, range.y, 0.1f, 1000.0f);
		this->view_matrix = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));
	}
	void bindFBO()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo.buffer);
	}
	void bind(int bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, this->id);
	}
private:
	GLuint id;	//texture id
	FBO fbo;
	
	glm::mat4 view_matrix;	//in light space
	glm::mat4 projection_matrix;	//in light space

	FBO generateFBO()
	{
		FBO fbo;

		// Framebuffers
		glGenFramebuffers(1, &fbo.buffer);
		// - Create depth texture
		glGenTextures(1, &fbo.texture);
		glBindTexture(GL_TEXTURE_2D, fbo.texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->resolution.x, this->resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.buffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo.texture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		return fbo;
	}
};