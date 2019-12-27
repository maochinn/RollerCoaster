#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

//#include "Bspline.h"
#include "Spline.h"
#include "BufferObject.h"
#include "ObjModel.h"
#include "glu.h"

class Train
{
public:
	enum MoveAxis {MOVE_X = 0, MOVE_Y, MOVE_Z};
public:
	//Shader shader;

	Train(ObjModel* head_car, ObjModel* rail, ObjModel* rail_tie, ObjModel* column,
		/*for rail*/Shader spline_shader, Spline::SplineType type, float tension,
		glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);

	void save(std::string name);
	void load(std::string path);

	void update(float dt);

	void renderTrack(Shader* shader = nullptr);
	void renderRail(Shader* shader = nullptr, float interval = 0.5f);
	void renderColumn(Shader* shader = nullptr);

	void addControlPoint();
	void deleteControlPoint();
	void nextControlPoint();
	void beforeControlPoint();
	void rotateControlPointX(float degree);	//rotate by LCS's axis-x
	void rotateControlPointZ(float degree);	//rotate by LCS's axis-z
	void translateControlPoint(glm::vec3 vector);


	void addCar(ObjModel model);
	void eraseCar();
	void setCarVelocity(float v);
	glm::vec3 getCarFront();
	glm::vec3 getCarPosition();
	glm::vec3 getCarFrontTopPosition();
	glm::vec3 getCarTopPosition();
	glm::vec3 getTrainCenterPosition();
	float getCarVelocity() { return this->car_velocity; }

	void setTension(float tension);
	void setSplineType(Spline::SplineType type);

	//to manipulate control point
	void doPick(glm::vec2 screen_pos);
	void doMove(glm::vec2 screen_pos, MoveAxis move_axis);
private:
	Spline spline;
	std::vector<ObjModel> cars;
	ObjModel rail;
	ObjModel rail_tie;
	ObjModel column;

	float car_position;
	float car_velocity;	//m/sec
	glm::vec3 world_car_velocity;	//define in world space

	void initialize();
};