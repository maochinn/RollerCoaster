#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "BufferObject.h"
#include "Shader.h"
class DynamicCubeMap
{
public:
	const GLenum face_type[6] = 
	{
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	Shader shader;
	const glm::ivec2 resolution;
	DynamicCubeMap(Shader shader, glm::ivec2 resolution)
		:shader(shader), resolution(resolution)
	{	
		//create empty cube map
		this->id = this->loadTexture(resolution.x, resolution.y);

		//create FBO
		this->fbo = this->generateFBO(this->id, resolution.x, resolution.y);

		//create VAO
		this->vao = this->generateVAO();
	}
	/*render to texture of face*/
	void bindFrameBufferToFace(/*cube map type*/ GLenum type)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo.buffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, type, this->id, 0);
	}
	glm::mat4 getViewMtx(/*position of camera*/glm::vec3 pos, GLenum type)
	{
		glm::mat4 view_matrix;
		if (type == GL_TEXTURE_CUBE_MAP_POSITIVE_X)
		{
			//right
			view_matrix = glm::lookAt(
				pos, 
				pos + glm::vec3(0.1f, 0.0f, 0.0f), 
				glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else if (type == GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
		{
			//left
			view_matrix = glm::lookAt(
				pos,
				pos + glm::vec3(-0.1f, 0.0f, 0.0f),
				glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else if (type == GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
		{
			//top
			view_matrix = glm::lookAt(
				pos,
				pos + glm::vec3(0.0f, 0.1f, 0.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (type == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
		{
			//bottom
			view_matrix = glm::lookAt(
				pos,
				pos + glm::vec3(0.0f, -0.1f, 0.0f),
				glm::vec3(0.0f, 0.0f, -1.0f));
		}
		else if (type == GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
		{
			//back
			view_matrix = glm::lookAt(
				pos,
				pos + glm::vec3(0.0f, 0.0f, 0.1f),
				glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else if (type == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
		{
			//front
			view_matrix = glm::lookAt(
				pos,
				pos + glm::vec3(0.0f, 0.0f, -0.1f),
				glm::vec3(0.0f, -1.0f, 0.0f));
		}
		return view_matrix;
	}
	glm::mat4 getProjectionMtx()
	{
		return glm::perspective(glm::radians(90.0f), (float)this->resolution.x / (float)this->resolution.y, 0.1f, 1000.0f);
	}
	glm::mat4 getOrthoProjectionMtx(glm::vec2 range)
	{
		return glm::ortho(-range.x, range.x, -range.y, range.y, 0.0f, 1000.0f);
	}
	void render()
	{
		this->shader.Use();

		this->bind(0);
		glUniform1i(glGetUniformLocation(this->shader.Program, "u_cubemap"), 0);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		glBindVertexArray(this->vao.vao);
		glDrawArrays(GL_TRIANGLES, 0, this->vao.count);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
	}
	void bindFBO()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo.buffer);
	}
	void bind(int bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
	}
	GLuint getTextureId() { return this->id; }
private:
	
	GLuint id;	//texture id
	
	VAO vao;
	FBO fbo;


	GLuint loadTexture(int width, int height)
	{
		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		//6 face
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		return id;
	}
	FBO generateFBO(GLuint texture_id, int width, int height)
	{
		FBO fbo;
		fbo.texture = texture_id;
		//GLuint fbo;
		glGenFramebuffers(1, &fbo.buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo.buffer);
		//defalut bind positive x face
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, texture_id, 0);
		// Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		//GLuint rbo;
		glGenRenderbuffers(1, &fbo.rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, fbo.rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // Use a single renderbuffer object for both a depth AND stencil buffer.
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo.rbo); // Now actually attach it
																									  // Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return fbo;
	}
	VAO generateVAO()
	{
		float skyboxVertices[] = {
			// Positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
		};

		VAO vao;

		vao.count = 36;
		glGenVertexArrays(1, &vao.vao);
		glGenBuffers(1, vao.vbo);
		glBindVertexArray(vao.vao);
		glBindBuffer(GL_ARRAY_BUFFER, vao.vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glBindVertexArray(0);

		return vao;
	}
};