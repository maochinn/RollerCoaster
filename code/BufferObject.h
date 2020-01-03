#pragma once
#include <glad\glad.h>

struct VAO
{
	GLuint vao;
	GLuint vbo[3];
	GLuint ebo;
	union
	{
		unsigned int elementAmount;	//for draw element
		unsigned int count;			//for draw array
	};
};
struct UBO
{
	GLuint ubo;
	GLsizeiptr size;
};
struct FBO
{
	GLuint buffer;	//frame buffer
	GLuint texture;	//attach to color buffer
	GLuint rbo;	//attach to depth and stencil
};
static void displayVAO(VAO vao);
static void displayVBO(GLuint vbo, int vertex_size, int vertex_type, int vertex_stride);
static void displayEBO(GLuint ebo, int stride = 3);

static void displayVAO(VAO vao)
{
	for (int i = 0; i < 3; i++)
	{
		if (vao.vbo[i] <= 0)
			break;

		int vertex_size;
		int vertex_type;
		int vertex_stride;
		glBindVertexArray(vao.vao);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertex_size);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertex_type);  // GL_FLOAT
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertex_stride);
		glBindVertexArray(0);

		printf("VBO_%d:\n", i);
		displayVBO(vao.vbo[i], vertex_size, vertex_type, vertex_stride);
	}
	if (vao.ebo > 0)
	{
		puts("EBO:");
		displayEBO(vao.ebo);
	}
}
static void displayVBO(GLuint vbo, int vertex_size, int vertex_type, int vertex_stride)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	int buffer_size;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
	float* buffer = (float*)malloc(buffer_size);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, buffer);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (vertex_type == GL_FLOAT)
	{
		vertex_type = 4;
		int number = buffer_size / vertex_type;
		vertex_stride /= vertex_type;
		for (int i = 0; i < number;)
		{
			for (int j = 0; j < vertex_stride; j++)
			{
				if (j < vertex_size)
					printf("%f, ", buffer[i++]);
				else
					i++;
			}

			puts("");
		}
	}
	
	free(buffer);
}
static void displayEBO(GLuint ebo, int stride)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	int buffer_size;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
	unsigned int* buffer = (unsigned int*)malloc(buffer_size);
	glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, buffer_size, buffer);
	GLenum temp = glGetError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	int number = buffer_size / sizeof(unsigned int);
	for (int i = 0; i < number;)
	{
		for (int j = 0; j < stride; j++)
			printf("%d, ", buffer[i++]);
		puts("");
	}

	free(buffer);
}