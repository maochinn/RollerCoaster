#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>



class Shader
{
public:
	GLuint Program;
	// Constructor generates the shader on the fly
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		// 1. Retrieve the vertex/fragment source code from filePath
		// Vertex Shader
		std::string code = this->readCode(vertexPath);
		GLuint vertex = this->compileShader(GL_VERTEX_SHADER, code.c_str());
		// Fragment Shader
		code = this->readCode(fragmentPath);
		GLuint fragment = this->compileShader(GL_FRAGMENT_SHADER, code.c_str());

		
		// Shader Program
		GLint success;
		GLchar infoLog[512];
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);

	}
	Shader(const GLchar* vertexPath, const GLchar* geometrytPath, const GLchar* fragmentPath)
	{
		// 1. Retrieve the vertex/fragment source code from filePath
		// Vertex Shader
		std::string code = this->readCode(vertexPath);
		GLuint vertex = this->compileShader(GL_VERTEX_SHADER, code.c_str());
		// Geometry Shader
		code = this->readCode(geometrytPath);
		GLuint geomrtry = this->compileShader(GL_GEOMETRY_SHADER, code.c_str());
		// Fragment Shader
		code = this->readCode(fragmentPath);
		GLuint fragment = this->compileShader(GL_FRAGMENT_SHADER, code.c_str());


		// Shader Program
		GLint success;
		GLchar infoLog[512];
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, geomrtry);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(geomrtry);
		glDeleteShader(fragment);
	}
	// Uses the current shader
	void Use()
	{
		glUseProgram(this->Program);
	}
private:
	std::string readCode(const GLchar* path)
	{
		std::string code;
		std::ifstream shader_file;
		// ensures ifstream objects can throw exceptions:
		shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// Open files
			shader_file.open(path);
			//if (!vShaderFile)std::cout << vertexPath << " fail to open" << std::endl;
			std::stringstream shader_stream;
			// Read file's buffer contents into streams
			shader_stream << shader_file.rdbuf();
			// close file handlers
			shader_file.close();
			// Convert stream into string
			code = shader_stream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		return code;
	}
	GLuint compileShader(GLenum shader_type, const char* code)
	{
		GLuint shader_number;
		GLint success;
		GLchar infoLog[512];
		// Vertex Shader
		shader_number = glCreateShader(shader_type);
		glShaderSource(shader_number, 1, &code, NULL);
		glCompileShader(shader_number);
		// Print compile errors if any
		glGetShaderiv(shader_number, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader_number, 512, NULL, infoLog);
			if(shader_type == GL_VERTEX_SHADER)
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			else if(shader_type == GL_GEOMETRY_SHADER)
				std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
			else if (shader_type == GL_FRAGMENT_SHADER)
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		return shader_number;
	}
};

#endif