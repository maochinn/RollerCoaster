#pragma once
#include <glad/glad.h>

int gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
	const GLdouble modelMatrix[16],
	const GLdouble projMatrix[16],
	const GLint viewport[4],
	GLdouble* objx, GLdouble* objy, GLdouble* objz);
void gluPickMatrix(GLdouble x, GLdouble y, GLdouble deltax, GLdouble deltay, GLint viewport[4]);
