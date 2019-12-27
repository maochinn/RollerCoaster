#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Shader.h"
#include "Texture.h"
#include "BufferObject.h"



class HeightMap
{
public:
	Shader shader;

	HeightMap(const char* folder_path, int amount, 
		Shader shader, glm::vec3 size = glm::vec3(20.0f, 1.0f, 20.0f), glm::vec3 position = glm::vec3(0.0f))
		:shader(shader), size(size), position(position)
	{
		this->size = size;
		this->animation_idx = 0;

		//load textures
		for (int i = 0; i < amount; i++)
		{
			std::stringstream ss;
			std::string index;
			ss << std::setw(std::to_string(amount - 1).length()) << std::setfill('0') << i;
			index = ss.str();

			std::string file_path = std::string(folder_path)+ "\\" + index + ".png";
			textures.push_back(Texture2D(file_path.c_str()));
		}
		//generate VAO
		{
			int width = textures[0].size.x;
			int height = textures[0].size.y;
			int vertices_amount = (width + 1) * (height + 1);
			int face_amount = width * height * 2;
			glm::vec3* postions = new glm::vec3[vertices_amount];
			glm::vec2* text_pos = new glm::vec2[vertices_amount];
			glm::ivec3* elements = new glm::ivec3[face_amount];
			//vertices
			for (int j = 0; j <= height; j++)
				for (int i = 0; i <= width; i++)
				{
					//normalize to [-0.5, 0.5]
					postions[i + j * (width + 1)].x = (float)i / width - 0.5f;
					postions[i + j * (width + 1)].y = 0.0f;
					postions[i + j * (width + 1)].z = (float)j / height - 0.5f;
					//normalize to [0, 1]
					text_pos[i + j * (width + 1)].x = (float)i / width;
					text_pos[i + j * (width + 1)].y = (float)j / height;
				}
			int count = 0;
			//face element
			for (int j = 0; j < height; j++)
				for (int i = 0; i < width; i++)
				{
					elements[count][0] = i + j * (width + 1);
					elements[count][1] = (i + 1) + j * (width + 1);
					elements[count++][2] = (i + 1) + (j + 1) *  (width + 1);

					elements[count][0] = i + j * (width + 1);
					elements[count][1] = (i + 1) + (j + 1) * (width + 1);
					elements[count++][2] = i + (j + 1) * (width + 1);
				}
			//create vao
			this->vao.elementAmount = face_amount * 3;

			glGenVertexArrays(1, &vao.vao);
			glGenBuffers(2, vao.vbo);
			glGenBuffers(1, &vao.ebo);

			glBindVertexArray(vao.vao);

			// Position attribute
			glBindBuffer(GL_ARRAY_BUFFER, vao.vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices_amount, postions, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			//texel
			glBindBuffer(GL_ARRAY_BUFFER, vao.vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices_amount, text_pos, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			//element
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * face_amount, elements, GL_STATIC_DRAW);
			glBindVertexArray(0); // Unbind VAO
		}
	}
	void update()
	{
		this->animation_idx++;
		if (this->animation_idx >= this->textures.size())
			this->animation_idx = 0;
	}
	void render()
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		shader.Use();

		this->textures[animation_idx].bind(0);
		glUniform1i(glGetUniformLocation(this->shader.Program, "u_texture0"), 0);

		glm::mat4 model;
		model = glm::translate(model, this->position);
		model = glm::scale(model, this->size);

		glUniformMatrix4fv(glGetUniformLocation(this->shader.Program, "u_model"), 1, GL_FALSE, &model[0][0]);
		glUniform3f(glGetUniformLocation(this->shader.Program, "u_material.ambient"), 0.2f, 0.2f, 0.3f);
		glUniform3f(glGetUniformLocation(this->shader.Program, "u_material.diffuse"), 0.5f, 0.5f, 0.6f);
		glUniform3f(glGetUniformLocation(this->shader.Program, "u_material.specular"), 0.1f, 0.1f, 0.1f);
		glUniform1f(glGetUniformLocation(this->shader.Program, "u_material.shininess"), 16.0f);
		glUniform2fv(glGetUniformLocation(this->shader.Program, "u_texture_size"), 1, &glm::vec2(this->textures[animation_idx].size)[0]);


		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(this->vao.vao);
		glDrawElements(GL_TRIANGLES, this->vao.elementAmount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glm::vec3 getPosition() { return this->position; }
	glm::vec3 getSize() { return this->size; }
private:
	std::vector<Texture2D> textures;
	VAO vao;

	int animation_idx;

	glm::vec3 position;
	glm::vec3 size;
	
};