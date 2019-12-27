#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Texture2D
{
public:
	Texture2D(const char* path)
	{
		cv::Mat img;
		//cv::imread(path, cv::IMREAD_COLOR).convertTo(img, CV_32FC3, 1 / 255.0f);	//unsigned char to float
		img = cv::imread(path, cv::IMREAD_COLOR);

		this->size.x = img.cols;
		this->size.y = img.rows;

		cv::cvtColor(img, img, CV_BGR2RGB);

		glGenTextures(1, &this->id);

		glBindTexture(GL_TEXTURE_2D, this->id);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if(img.channels() == 3)
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, img.cols, img.rows, 0, GL_RGB, GL_FLOAT, img.data);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img.cols, img.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
		else if (img.channels() == 4)
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, img.cols, img.rows, 0, GL_RGBA, GL_FLOAT, img.data);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.cols, img.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
		glBindTexture(GL_TEXTURE_2D, 0);

		img.release();
	}
	void bind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, this->id);
	}
	static void unbind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glm::ivec2 size;
private:
	GLuint id;
	
};