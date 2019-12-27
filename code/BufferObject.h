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