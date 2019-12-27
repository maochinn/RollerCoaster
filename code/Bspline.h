#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "BufferObject.h"
#include "Shader.h"

struct Orientation
{
	glm::vec3 u;	//front vector
	glm::vec3 v;	//up vector
	glm::vec3 w;	//right vector
};
struct Parameter
{
	double t;	//b-spline parameter
	double s;	//distance parameter
};

class Bspline
{
public:
	Shader shader;

	Bspline(Shader shader,
		glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int interval_num = 100);
	~Bspline();

	void addControlPoint();
	void addControlPoint(glm::vec3 p);
	void deleteControlPoint();

	void nextControlPoint();
	void beforeControlPoint();
	void rotateControlPoint(float degree);
	void translateControlPoint(glm::vec3 vector);

	/*local coordinate of t -> tt*/
	glm::mat4 calculateKnotPointModelMatrix(double t, double tt);
	int getMaxParameter();
	int getMinParameter();
	double getReparameterT(double distance);
	double getRailLength();
	

	void renderCurves();
	void renderControlPoints();
	void renderContralPointsOrientaion();
	void renderKnotPoints();
private:
	VAO curves_vao;
	VAO c_points_vao;
	VAO axis_vao;
	int now_c_point_index;

	std::vector<glm::vec3> c_points;	//control point
	std::vector<Orientation> c_points_orientation;
	std::vector<double> curve_length;
	std::vector<std::vector<Parameter>> curve_parameter_table;	//curve[Q]: Qth curve
	int interval_num;	//一段區間中要分割成幾段


	void createVAO();
	void createParameterTable(glm::vec3* knot_points, int curve_amount, int interval_num);
	void deleteVAO();

	void calculateOrientation();
	void updateOrientation(int index);
	glm::mat4 getControlPointModelMatrix(int index);

	Orientation newOrientation(int index);
	/*
	t: this knot point parameter
	tt: next knot point parameter
	*/
	
	

};
