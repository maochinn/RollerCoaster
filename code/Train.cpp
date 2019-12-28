#include "code/Train.h"

//local function

//get cross point by two lines
//for 2D space
glm::vec2 _getCrossPoint(glm::vec2 s1, glm::vec2 e1, glm::vec2 s2, glm::vec2 e2)
{
	glm::vec2 d1 = e1 - s1;
	glm::vec2 d2 = e2 - s2;

	float a1 = d1.x;
	float b1 = -d2.x;
	float c1 = s2.x - s1.x;

	float a2 = d1.y;
	float b2 = -d2.y;
	float c2 = s2.y - s1.y;

	float demon = a1 * b2 - b1 * a2;
	if (demon == 0.0f)	//parallel
		return s2;
	else
	{
		float demon2 = a1 * c2 - c1 * a2;
		float s = demon2 / demon;
		return s2 + d2 * s;
	}
	return s2;
}


Train::Train(ObjModel* head_car, ObjModel* rail, ObjModel* rail_tie, ObjModel* column,
	/*for rail*/Shader spline_shader, Spline::SplineType type, float tension,
	glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4)
	:spline(spline_shader, type, tension, p1, p2, p3, p4), rail(*rail), rail_tie(*rail_tie), column(*column)
{
	this->initialize();
	this->addCar(*head_car);
}
void Train::save(std::string name)
{
	spline.saveSpline(name);
}
void Train::load(std::string path)
{
	spline.loadSpline(path);
}
void Train::update(float dt)
{
	this->car_position += this->car_velocity * dt;
	if (car_position >= spline.getSplineLength())
		this->car_position = 0.0f;

	glm::vec3 now_pos = spline.getModelMatrixDistance(this->car_position)[3];
	glm::vec3 next_pos;
	if (this->car_velocity >= 0)
		next_pos = spline.getModelMatrixDistance(this->car_position + 1.0f)[3];
	else
		next_pos = spline.getModelMatrixDistance(this->car_position - 1.0f)[3];

	this->world_car_velocity = (next_pos - now_pos) / dt;

	float acceleration = -this->world_car_velocity.y * 0.1f;
	if (this->car_velocity >= 0)
		this->car_velocity = this->car_velocity + acceleration * dt;
	else	
		this->car_velocity = this->car_velocity - acceleration * dt;


	//this->car_velocity -= 1.0f* dt;
	//if (this->car_velocity < 0.0f)
	//	this->car_velocity = 0.01f;
}
void Train::renderTrack(Shader* shader)
{
	//this->shader.Use();
	float now = this->car_position;

	for (auto car : this->cars)
	{
		glm::vec3 car_size = car.max_pos - car.min_pos;

		glm::mat4 offset_matrix;
		offset_matrix = glm::translate(offset_matrix, glm::vec3(0.0f, car_size.y * 0.6f, 0.0f));
		glm::mat4 model_matrix = offset_matrix * spline.getModelMatrixDistance(now);

		car.render(model_matrix, shader);

		now -= car_size.x + 0.1f;
		if (now < 0.0f)
			now += spline.getSplineLength();
	}

	this->spline.renderSpline();
	this->spline.renderControlPoints();
}
void Train::renderRail(Shader* shader, float interval)
{
	float max_distance = spline.getSplineLength();
	float now = interval;

	for (; now <= max_distance;now += interval)
	{
		glm::mat4 model_matrix = spline.getModelMatrixDistance(now);
		//left and right rail
		glm::mat4 left_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, -0.5f))* glm::scale(glm::mat4(), glm::vec3(interval+0.1f, 1.0f, 1.0f));
		glm::mat4 right_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, 0.5f))* glm::scale(glm::mat4(), glm::vec3(interval+0.1f, 1.0f, 1.0f));
		//rail tie
		glm::mat4 tie_matrix = glm::translate(model_matrix, glm::vec3(0.1f));


		//render parallel rails
		this->rail.render(left_matrix, shader);
		this->rail.render(right_matrix, shader);
		//render rail ties
		this->rail_tie.render(tie_matrix, shader);


	}
	

	spline.renderSpline();
}
void Train::renderColumn(Shader* shader)
{
	float interval = 5.0f;


	float max_distance = this->spline.getSplineLength();
	float now = interval;
	for (; now <= max_distance; now += interval)
	{
		glm::mat4 model_matrix = spline.getModelMatrixDistance(now);
		model_matrix[0][0] = model_matrix[1][1] = model_matrix[2][2] = 1.0f;
		model_matrix[0][1] = model_matrix[0][2] =
			model_matrix[1][0] = model_matrix[1][2] =
			model_matrix[2][0] = model_matrix[2][1] = 0.0f;
		float height = model_matrix[3].y;
		model_matrix = glm::scale(model_matrix, glm::vec3(1.0f, height, 1.0f));
		this->column.render(model_matrix, shader);
	}
	
}
void Train::addControlPoint() { spline.addControlPoint(); }
void Train::deleteControlPoint() { spline.deleteControlPoint(); }
void Train::nextControlPoint() { spline.nextControlPoint(); }
void Train::beforeControlPoint() { spline.beforeControlPoint(); }
void Train::rotateControlPointX(float degree) { spline.rotateControlPointX(degree); }
void Train::rotateControlPointZ(float degree) { spline.rotateControlPointZ(degree); }
void Train::translateControlPoint(glm::vec3 vector) { //spline.translateControlPoint(vector); }
}
void Train::addCar(ObjModel model){this->cars.push_back(model);}
void Train::eraseCar()
{
	if (this->cars.size() <= 1)
		return;
	this->cars.pop_back();
}
void Train::setCarVelocity(float v) { this->car_velocity = v; }
glm::vec3 Train::getCarPosition()
{
	glm::mat4 model = spline.getModelMatrixDistance(car_position);
	return glm::vec3(model[3]);
}
glm::vec3 Train::getCarFront()
{
	glm::mat4 model = spline.getModelMatrixDistance(car_position);
	return glm::vec3(model[0]);
}
glm::vec3 Train::getCarFrontTopPosition()
{
	glm::mat4 model = spline.getModelMatrixDistance(car_position);

	glm::vec3 car_size = cars[0].max_pos - cars[0].min_pos;
	model = glm::translate(model, glm::vec3(car_size.x, car_size.y, 0.0f));

	return glm::vec3(model[3]);
}
glm::vec3 Train::getCarTopPosition()
{
	glm::mat4 model = spline.getModelMatrixDistance(car_position);

	glm::vec3 car_size = cars[0].max_pos - cars[0].min_pos;
	model = glm::translate(model, glm::vec3(0.0f, car_size.y, 0.0f));

	return glm::vec3(model[3]);
}
glm::vec3 Train::getTrainCenterPosition()
{
	int amount = spline.getControlPointAmount();
	glm::vec3 center(0.0f);
	for (int i = 0;i<amount; i++)
	{
		center += spline.getControlPointPosition(i);
	}
	return center /(float)amount;
}

void Train::setTension(float tension)
{
	this->spline.setTension(tension);
}
void Train::setSplineType(Spline::SplineType type)
{
	this->spline.setSplineType(type);
}

void Train::doPick(glm::vec2 screen_pos)
{
	glm::ivec2 viewport[2];
	glGetIntegerv(GL_VIEWPORT, &viewport[0][0]);

	glm::vec2 mouse(screen_pos.x, viewport[1].y - screen_pos.y);

	glm::mat4 projection;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection[0][0]);

	//set matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix((double)mouse.x, (double)(mouse.y),
		5, 5, &viewport[0][0]);
	glMultMatrixf(&projection[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);


	// draw the cubes, loading the names as we go
	for (size_t i = 0; i < spline.control_points.size(); ++i) 
	{
		glLoadName((GLuint)(i + 1));
		spline.renderControlPoint(
			spline.control_points[i].position,
			spline.control_points[i].orientation,
			glm::vec3(1.0f));
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) 
	{
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		spline.now_point_idx = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		spline.now_point_idx = -1;
}
void Train::doMove(glm::vec2 screen_pos, MoveAxis move_axis)
{
	if (spline.selectControlPoint() == false)
		return;

	glm::ivec2 viewport[2];
	glGetIntegerv(GL_VIEWPORT, &viewport[0][0]);

	glm::f64vec2 mouse(screen_pos.x, viewport[1].y - screen_pos.y);

	double view[16], projection[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, view);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	glm::f64vec3 start;
	glm::f64vec3 end;

	gluUnProject(mouse.x, mouse.y, 0.25, view, projection, &viewport[0][0], &start.x, &start.y, &start.z);
	gluUnProject(mouse.x, mouse.y, 0.75, view, projection, &viewport[0][0], &end.x, &end.y, &end.z);

	glm::vec3 now_point = spline.getNowControlPointPosition();
	glm::vec3 new_point;

	glm::vec3 axis;
	if (move_axis == MoveAxis::MOVE_X)
	{
		axis = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec2 pos = _getCrossPoint(
			glm::vec2(start),
			glm::vec2(end),
			glm::vec2(now_point),
			glm::vec2(now_point + axis));
		new_point = glm::vec3(pos.x, pos.y, now_point.z);
	}
	else if (move_axis == MoveAxis::MOVE_Y)
	{
		axis = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec2 pos = _getCrossPoint(
			glm::vec2(start),
			glm::vec2(end),
			glm::vec2(now_point),
			glm::vec2(now_point + axis));
		new_point = glm::vec3(pos.x, pos.y, now_point.z);
	}
	else if (move_axis == MoveAxis::MOVE_Z)
	{
		axis = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec2 pos = _getCrossPoint(
			glm::vec2(start.y, start.z),
			glm::vec2(end.y, end.z),
			glm::vec2(now_point.y, now_point.z),
			glm::vec2(now_point.y + axis.y, now_point.z + axis.z));
		new_point = glm::vec3(now_point.x, pos.x, pos.y);
	}
		




	spline.setNowControlPointPos(new_point);
}

void Train::initialize()
{
	this->car_position = 0.0f;
	this->car_velocity = 5.0f;
}