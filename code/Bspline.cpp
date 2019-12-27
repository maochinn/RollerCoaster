#include "Bspline.h"

Bspline::Bspline(Shader shader,
	glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int interval_num)
	:shader(shader)
{
	this->c_points.push_back(p1);
	this->c_points.push_back(p2);
	this->c_points.push_back(p3);
	this->c_points.push_back(p4);
	this->c_points.push_back(p1);
	this->c_points.push_back(p2);
	this->c_points.push_back(p3);
	
	this->now_c_point_index = 0;

	this->interval_num = interval_num;

	this->curves_vao.vao = -1;
	this->c_points_vao.vao = -1;

	this->createVAO();
	this->calculateOrientation();
	//create axis vao
	{
		float axis_vertices[] = {
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		this->axis_vao.count = 6;
		glGenVertexArrays(1, &this->axis_vao.vao);
		glGenBuffers(1, this->axis_vao.vbo);

		glBindVertexArray(this->axis_vao.vao);

		glBindBuffer(GL_ARRAY_BUFFER, this->axis_vao.vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0); // Unbind VAO
	}
}
Bspline::~Bspline()
{
	this->deleteVAO();
}
/*
curve_amount:	amount of bspline curve
ds			:	在每個curve中的每個點之間的距離，原則上這邊的ds是粗略值
*/
void Bspline::createParameterTable(glm::vec3* knot_points, int curve_amount, int interval_num)
{
	//free table
	for (int i = 0; i < this->curve_parameter_table.size(); i++)
		this->curve_parameter_table[i].clear();
	this->curve_parameter_table.clear();

	double s = 0.0;
	double t = 0.0;
	double dt = 1.0 / (interval_num - 1);
	//calculate curve length
	for (int i = 0; i < curve_amount; i++)
	{
		std::vector<Parameter> curve;
		Parameter p;
		p.t = t;
		p.s = s;
		curve.push_back(p);
		for (int j = 0; j < interval_num - 1; j++)
		{
			double ds = glm::length(
				knot_points[i * interval_num + j + 1] -
				knot_points[i * interval_num + j]);
			t += dt;
			s += ds;
			curve.push_back(Parameter{t, s});
		}
		this->curve_parameter_table.push_back(curve);
	}
}
void Bspline::createVAO()
{
	//B-spline = G*M*T
	glm::mat4 M;
	{
		M[0][0] = -1.0f;
		M[1][0] = 3.0f;
		M[2][0] = -3.0f;
		M[3][0] = 1.0f;

		M[0][1] = 3.0f;
		M[1][1] = -6.0f;
		M[2][1] = 0.0f;
		M[3][1] = 4.0f;

		M[0][2] = -3.0f;
		M[1][2] = 3.0f;
		M[2][2] = 3.0f;
		M[3][2] = 1.0f;

		M[0][3] = 1.0f;
		M[1][3] = 0.0f;
		M[2][3] = 0.0f;
		M[3][3] = 0.0f;
	}
	int curve_amount = (int)this->c_points.size() - 3;
	int interval_knot_amount = (this->interval_num);	//每個interval會有幾個knot point

	int knot_amount = curve_amount * interval_knot_amount;
	glm::vec3* knot_points = new glm::vec3[knot_amount];

	for (int i = 0; i < curve_amount; i++)
	{
		glm::vec4 G_x(c_points[i].x, c_points[i + 1].x, c_points[i + 2].x, c_points[i + 3].x);
		glm::vec4 G_y(c_points[i].y, c_points[i + 1].y, c_points[i + 2].y, c_points[i + 3].y);
		glm::vec4 G_z(c_points[i].z, c_points[i + 1].z, c_points[i + 2].z, c_points[i + 3].z);
		for (int j = 0;j < this->interval_num;j++)
		{
			float t = (float)j / interval_num;
			glm::vec4 T(t*t*t, t*t, t, 1.0f);

			glm::vec4 MT = M * T;
			MT *= 0.166667f;

			knot_points[i * interval_knot_amount + j].x = glm::dot(G_x, MT);
			knot_points[i * interval_knot_amount + j].y = glm::dot(G_y, MT);
			knot_points[i * interval_knot_amount + j].z = glm::dot(G_z, MT);
		}
	}

	this->deleteVAO();

	//update curves
	this->curves_vao.count = knot_amount;
	glGenVertexArrays(1, &this->curves_vao.vao);
	glGenBuffers(1, this->curves_vao.vbo);

	glBindVertexArray(this->curves_vao.vao);

	glBindBuffer(GL_ARRAY_BUFFER, this->curves_vao.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * knot_amount, knot_points, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO


	//update control points
	this->c_points_vao.count = (unsigned int)this->c_points.size();
	glGenVertexArrays(1, &this->c_points_vao.vao);
	glGenBuffers(1, this->c_points_vao.vbo);

	glBindVertexArray(this->c_points_vao.vao);

	glBindBuffer(GL_ARRAY_BUFFER, this->c_points_vao.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * this->c_points.size(), &this->c_points[0],GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO

	if (this->curve_length.empty() != true)
		this->curve_length.clear();
	//calculate curve length
	for (int i = 0; i < curve_amount; i++)
	{
		double sum = 0.0;;
		for (int j = 0; j < this->interval_num-1; j++)
		{
			sum += glm::length(
				knot_points[i * interval_knot_amount + j + 1] - 
				knot_points[i * interval_knot_amount + j]);
		}
		curve_length.push_back(sum);
	}
	this->createParameterTable(knot_points, curve_amount, interval_knot_amount);

	return;

}
void Bspline::deleteVAO()
{
	if (this->curves_vao.vao > 0)
	{
		//delete VAO VBO
		glDeleteVertexArrays(1, &this->curves_vao.vao);
		glDeleteBuffers(1, this->curves_vao.vbo);
	}
	if (this->c_points_vao.vao > 0)
	{
		//delete VAO VBO
		glDeleteVertexArrays(1, &this->c_points_vao.vao);
		glDeleteBuffers(1, this->c_points_vao.vbo);
	}

}
void Bspline::renderCurves()
{
	this->shader.Use();

	glUniformMatrix4fv(glGetUniformLocation(this->shader.Program, "u_model"), 1, GL_FALSE, &glm::mat4()[0][0]);
	glUniform3f(glGetUniformLocation(this->shader.Program, "u_color"), 0.0f, 1.0f, 0.0f);

	glBindVertexArray(this->curves_vao.vao);
	glDrawArrays(GL_LINE_LOOP, 0, this->curves_vao.count);
}
void Bspline::renderControlPoints()
{
	this->shader.Use();

	glPointSize(10.0);

	glUniformMatrix4fv(glGetUniformLocation(this->shader.Program, "u_model"), 1, GL_FALSE, &glm::mat4()[0][0]);
	glUniform3f(glGetUniformLocation(shader.Program, "u_color"), 0.0f, 1.0f, 0.0f);

	glBindVertexArray(this->c_points_vao.vao);
	glDrawArrays(GL_POINTS, 0, this->c_points_vao.count);
}
void Bspline::addControlPoint()
{
	int index = this->now_c_point_index;
	int pre_index = index - 1;
	if (pre_index < 0)
		pre_index = (int)this->c_points.size() - 4;
	glm::vec3 p = 0.5f*(this->c_points[pre_index] + this->c_points[index]);
	this->addControlPoint(p);
}
void Bspline::addControlPoint(glm::vec3 p)
{
	int index = this->now_c_point_index;
	this->c_points.insert(this->c_points.begin() + index, p);
	this->c_points_orientation.insert(
		this->c_points_orientation.begin() + index, this->newOrientation(index));
	if (index <= 2)
	{
		this->c_points.insert(c_points.end() -1 - 2 + index, this->c_points[index]);
		this->c_points.pop_back();
		this->c_points_orientation.insert(
			this->c_points_orientation.end() - 1 -2 + index, this->c_points_orientation[index]);
		this->c_points_orientation.pop_back();
	}
	this->createVAO();
}
void Bspline::deleteControlPoint()
{
	if (this->c_points.size() <= 7)
		return;
	int index = this->now_c_point_index;
	this->c_points.erase(this->c_points.begin() + index);
	this->c_points_orientation.erase(this->c_points_orientation.begin() + index);
	if (index <= 2)
	{
		this->c_points.erase(this->c_points.end() - 1 - 2 + index);
		this->c_points.push_back(this->c_points[2]);
		this->c_points_orientation.erase(this->c_points_orientation.end() - 1 - 2 + index);
		this->c_points_orientation.push_back(this->c_points_orientation[2]);
	}
	this->createVAO();
}
Orientation Bspline::newOrientation(int index)
{
	Orientation temp;
	glm::vec3 world_up(0.0f, 1.0f, 0.0f);
	if (index + 1 >= this->c_points.size())
		temp.u = glm::normalize(this->c_points[3] - this->c_points[index]);
	else
		temp.u = glm::normalize(this->c_points[index + 1] - this->c_points[index]);
	temp.w = glm::cross(temp.u, world_up);
	temp.v = glm::cross(temp.w, temp.u);
	return temp;
}
void Bspline::calculateOrientation()
{
	//calculate orientation
	glm::vec3 world_up(0.0f, 1.0f, 0.0f);
	for (int i = 0; i<this->c_points.size(); i++)
	{
		this->c_points_orientation.push_back(this->newOrientation(i));
	}
}
void Bspline::renderContralPointsOrientaion()
{
	this->shader.Use();

	for (int i = 0; i < this->c_points.size(); i++)
	{
		glm::mat4 model_matrix;
		{
			model_matrix[0][0] = this->c_points_orientation[i].u.x;
			model_matrix[0][1] = this->c_points_orientation[i].u.y;
			model_matrix[0][2] = this->c_points_orientation[i].u.z;
			model_matrix[3][0] = this->c_points[i].x;

			model_matrix[1][0] = this->c_points_orientation[i].v.x;
			model_matrix[1][1] = this->c_points_orientation[i].v.y;
			model_matrix[1][2] = this->c_points_orientation[i].v.z;
			model_matrix[3][1] = this->c_points[i].y;

			model_matrix[2][0] = this->c_points_orientation[i].w.x;
			model_matrix[2][1] = this->c_points_orientation[i].w.y;
			model_matrix[2][2] = this->c_points_orientation[i].w.z;
			model_matrix[3][2] = this->c_points[i].z;

		}
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
		if(i == this->now_c_point_index)
			glUniform3f(glGetUniformLocation(shader.Program, "u_color"), 0.0f, 1.0f, 1.0f);
		else
			glUniform3f(glGetUniformLocation(shader.Program, "u_color"), 1.0f, 0.0f, 0.0f);

		
		glBindVertexArray(this->axis_vao.vao);
		glDrawArrays(GL_LINES, 0, this->axis_vao.count);
	}
}
void Bspline::renderKnotPoints()
{
	//i = 3~m
	//t_3 = 0, t_4 = 1...
	//t_i - t_i+1 = 1
	int t_i = 0;	//t_i	//t_3
	int t_i_1 = 1;	//t_i+1	//t_4
	int i = 3;	//curve Q_i
	int m = (int)this->c_points.size()-1;	//control points:p[0], p[1] ~ , p[m]
	double delta_t = 1.0 / interval_num;
	double distance = 0.5;
	double new_dt = delta_t;
	for (double t = 0.0 ;t<m-2;t+=new_dt)
	{
		glm::mat4 model_matrix = this->calculateKnotPointModelMatrix(t, t+delta_t);
		//new_dt = this->getReparameterDeltaT(t, distance);
		//glm::mat4 model_matrix = this->calculateKnotPointModelMatrix(t, t + new_dt);

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform3f(glGetUniformLocation(shader.Program, "u_color"), 1.0f, 1.0f, 0.0f);

		glBindVertexArray(this->axis_vao.vao);
		glDrawArrays(GL_LINES, 0, this->axis_vao.count);
	}
}
/*
double Bspline::getReparameterT(double t, double distance)
{
	t += this->getReparameterDeltaT(t, distance);
	while (t >= this->getMaxParameter())
		t -= this->getMaxParameter();
	while (t < this->getMinParameter())
		t += this->getMaxParameter();
	while (t >= this->getMaxParameter())
		t -= this->getMaxParameter();
	return t;
}
double Bspline::getReparameterDeltaT(double t, double distance)
{
	while (t >= this->getMaxParameter())
		t -= this->getMaxParameter();
	while (t < this->getMinParameter())
		t += this->getMaxParameter();
	while (t >= this->getMaxParameter())
		t -= this->getMaxParameter();

	int i = (int)t;
	double new_dt = distance / this->curve_length[i];
	double tt = t + new_dt;
	int ii = (int)tt;
	if (ii > i)
	{
		double d0 = (i + 1) - t;
		double d1 = distance - d0;
		if (i + 1 >= this->curve_length.size())
			ii = 0;
		else ii = i + 1;
		new_dt = (d0 / this->curve_length[i]) + (d1 / this->curve_length[ii]);
	}
	else if(ii < i)
	{
		double d0 = t - i;
		double d1 = distance - d0;
		if (i - 1 < 0)
			ii = this->curve_length.size() - 1;
		else
			ii = i - 1;
		new_dt = (d0 / this->curve_length[i]) + (d1 / this->curve_length[ii]);
	}
	return new_dt;
}*/
glm::mat4 Bspline::calculateKnotPointModelMatrix(double t, double tt)
{
	glm::mat4 M;
	{
		M[0][0] = -1.0f;
		M[1][0] = 3.0f;
		M[2][0] = -3.0f;
		M[3][0] = 1.0f;

		M[0][1] = 3.0f;
		M[1][1] = -6.0f;
		M[2][1] = 0.0f;
		M[3][1] = 4.0f;

		M[0][2] = -3.0f;
		M[1][2] = 3.0f;
		M[2][2] = 3.0f;
		M[3][2] = 1.0f;

		M[0][3] = 1.0f;
		M[1][3] = 0.0f;
		M[2][3] = 0.0f;
		M[3][3] = 0.0f;

		M *= 0.166667f;
	}

	while(t >= this->getMaxParameter())
		t -= this->getMaxParameter();

	while(tt >= this->getMaxParameter())
		tt -= this->getMaxParameter();

	int m = (int)this->c_points.size() - 1;

	//this knot in Q_(i+3) curve
	int i = ((int)t);
	t = t - i;
	int ii = (int)tt;
	tt = tt - ii;

	if (tt > m - 2)
	{
		tt = 0.0;
	}
		

	glm::vec4 T(t*t*t, t*t, t, 1.0f);
	glm::vec4 TT(tt*tt*tt, tt*tt, tt, 1.0f);

	glm::vec4 MT = M * T;
	
	//calculate kont point position
	glm::vec4 G_x(c_points[i].x, c_points[i + 1].x, c_points[i + 2].x, c_points[i + 3].x);
	glm::vec4 G_y(c_points[i].y, c_points[i + 1].y, c_points[i + 2].y, c_points[i + 3].y);
	glm::vec4 G_z(c_points[i].z, c_points[i + 1].z, c_points[i + 2].z, c_points[i + 3].z);

	//Qi(t)
	glm::vec3 Q_t;
	Q_t.x = glm::dot(G_x, MT);
	Q_t.y = glm::dot(G_y, MT);
	Q_t.z = glm::dot(G_z, MT);

	//calculate kont point position
	if (ii + 3 >= c_points.size())
	{
		G_x = glm::vec4(c_points[ii].x, c_points[ii + 1].x, c_points[ii + 2].x, c_points[3].x);
		G_y = glm::vec4(c_points[ii].y, c_points[ii + 1].y, c_points[ii + 2].y, c_points[3].y);
		G_z = glm::vec4(c_points[ii].z, c_points[ii + 1].z, c_points[ii + 2].z, c_points[3].z);
	}
	else
	{
		G_x = glm::vec4(c_points[ii].x, c_points[ii + 1].x, c_points[ii + 2].x, c_points[ii + 3].x);
		G_y = glm::vec4(c_points[ii].y, c_points[ii + 1].y, c_points[ii + 2].y, c_points[ii + 3].y);
		G_z = glm::vec4(c_points[ii].z, c_points[ii + 1].z, c_points[ii + 2].z, c_points[ii + 3].z);
	}

	//Qii(tt)
	glm::vec3 Q_tt;
	Q_tt.x = glm::dot(G_x, M*TT);
	Q_tt.y = glm::dot(G_y, M*TT);
	Q_tt.z = glm::dot(G_z, M*TT);

	//calculate knot point orientation
	//for front vector;
	G_x = glm::vec4(c_points_orientation[i].u.x, c_points_orientation[i + 1].u.x, c_points_orientation[i + 2].u.x, c_points_orientation[i + 3].u.x);
	G_y = glm::vec4(c_points_orientation[i].u.y, c_points_orientation[i + 1].u.y, c_points_orientation[i + 2].u.y, c_points_orientation[i + 3].u.y);
	G_z = glm::vec4(c_points_orientation[i].u.z, c_points_orientation[i + 1].u.z, c_points_orientation[i + 2].u.z, c_points_orientation[i + 3].u.z);

	glm::vec3 u;
	//u.x = glm::dot(G_x, MT);
	//u.y = glm::dot(G_y, MT);
	//u.z = glm::dot(G_z, MT);
	u = glm::normalize(Q_tt - Q_t);

	//for up vector;
	G_x = glm::vec4(c_points_orientation[i].v.x, c_points_orientation[i + 1].v.x, c_points_orientation[i + 2].v.x, c_points_orientation[i + 3].v.x);
	G_y = glm::vec4(c_points_orientation[i].v.y, c_points_orientation[i + 1].v.y, c_points_orientation[i + 2].v.y, c_points_orientation[i + 3].v.y);
	G_z = glm::vec4(c_points_orientation[i].v.z, c_points_orientation[i + 1].v.z, c_points_orientation[i + 2].v.z, c_points_orientation[i + 3].v.z);

	glm::vec3 v;
	v.x = glm::dot(G_x, MT);
	v.y = glm::dot(G_y, MT);
	v.z = glm::dot(G_z, MT);
	

	//for up vector;
	//G_x = glm::vec4(c_points_orientation[i].w.x, c_points_orientation[i + 1].w.x, c_points_orientation[i + 2].w.x, c_points_orientation[i + 3].w.x);
	//G_y = glm::vec4(c_points_orientation[i].w.y, c_points_orientation[i + 1].w.y, c_points_orientation[i + 2].w.y, c_points_orientation[i + 3].w.y);
	//G_z = glm::vec4(c_points_orientation[i].w.z, c_points_orientation[i + 1].w.z, c_points_orientation[i + 2].w.z, c_points_orientation[i + 3].w.z);

	glm::vec3 w;
	//w.x = glm::dot(G_x, MT);
	//w.y = glm::dot(G_y, MT);
	//w.z = glm::dot(G_z, MT);
	w = glm::cross(u, v);
	v = glm::cross(w, u);
	v = glm::normalize(v);
	w = glm::normalize(w);

	glm::mat4 model_matrix;
	{
		model_matrix[0][0] = u.x;
		model_matrix[0][1] = u.y;
		model_matrix[0][2] = u.z;
		model_matrix[3][0] = Q_t.x;

		model_matrix[1][0] = v.x;
		model_matrix[1][1] = v.y;
		model_matrix[1][2] = v.z;
		model_matrix[3][1] = Q_t.y;

		model_matrix[2][0] = w.x;
		model_matrix[2][1] = w.y;
		model_matrix[2][2] = w.z;
		model_matrix[3][2] = Q_t.z;
	}
	return model_matrix;
}
void Bspline::nextControlPoint()
{
	this->now_c_point_index++;
	if (this->now_c_point_index >= this->c_points.size()-3)
		this->now_c_point_index = 0;
	this->updateOrientation(this->now_c_point_index);
	if (this->now_c_point_index <= 2)
	{
		int index = this->now_c_point_index + ((int)this->c_points.size() - 3);
		this->updateOrientation(index);
	}
	printf("now control point index: %d\n", this->now_c_point_index);
}
void Bspline::beforeControlPoint()
{
	this->now_c_point_index--;
	if (this->now_c_point_index < 0)
		this->now_c_point_index = (int)this->c_points.size()-4;
	if (this->now_c_point_index <= 2)
	{
		int index = this->now_c_point_index + ((int)this->c_points.size() - 3);
		this->updateOrientation(index);
	}
	this->updateOrientation(this->now_c_point_index);
	printf("now control point index: %d\n", this->now_c_point_index);
}
void Bspline::rotateControlPoint(float degree)
{
	Orientation& now_orien = this->c_points_orientation[this->now_c_point_index];
	glm::mat4 rotate_matrix;
	rotate_matrix = glm::rotate(rotate_matrix, glm::radians(degree), now_orien.u);

	now_orien.v = rotate_matrix * glm::vec4(now_orien.v, 0.0f);
	now_orien.w = glm::cross(now_orien.u, now_orien.v);

	if (this->now_c_point_index <= 2)
	{
		int index = this->now_c_point_index + ((int)this->c_points.size() - 3);
		Orientation& orien = this->c_points_orientation[index];
		orien.v = now_orien.v;
		orien.w = now_orien.w;
	}
}
void Bspline::translateControlPoint(glm::vec3 vector)
{
	glm::vec3& now_point = this->c_points[this->now_c_point_index];
	//glm::vec3 translate = this->getControlPointModelMatrix(this->now_c_point_index) * glm::vec4(vector, 0.0f);
	now_point += vector;
	this->updateOrientation(this->now_c_point_index);
	//if(this->now_c_point_index - 1>=0)
		//this->updateOrientation(this->now_c_point_index-1);
	if (this->now_c_point_index <= 2)
	{
		int index = this->now_c_point_index + ((int)this->c_points.size() - 3);
		this->c_points[index] = now_point;

		this->updateOrientation(index);
		//this->updateOrientation(index-1);
	}
	this->createVAO();
}
void Bspline::updateOrientation(int index)
{
	Orientation& now = this->c_points_orientation[index];
	if (index + 1 >= c_points.size())
		now.u = glm::normalize(this->c_points[3] - this->c_points[index]);
	else
		now.u = glm::normalize(this->c_points[index + 1] - this->c_points[index]);
	now.w = glm::cross(now.u, now.v);
	now.v = glm::cross(now.w, now.u);
}
int Bspline::getMaxParameter()
{
	int m = (int)this->c_points.size() - 1;
	return m - 2;
}
int Bspline::getMinParameter()
{
	return 0;
}
glm::mat4 Bspline::getControlPointModelMatrix(int index)
{
	glm::mat4 mtx;
	mtx[0][0] = this->c_points_orientation[index].u.x;
	mtx[0][1] = this->c_points_orientation[index].u.y;
	mtx[0][2] = this->c_points_orientation[index].u.z;
	mtx[3][0] = this->c_points[index].x;

	mtx[1][0] = this->c_points_orientation[index].v.x;
	mtx[1][1] = this->c_points_orientation[index].v.y;
	mtx[1][2] = this->c_points_orientation[index].v.z;
	mtx[3][1] = this->c_points[index].y;

	mtx[2][0] = this->c_points_orientation[index].w.x;
	mtx[2][1] = this->c_points_orientation[index].w.y;
	mtx[2][2] = this->c_points_orientation[index].w.z;
	mtx[3][2] = this->c_points[index].z;

	return mtx;
}
double Bspline::getReparameterT(double distance)
{
	distance = fabs(fmod(distance, this->getRailLength()));
	for (auto curve = this->curve_parameter_table.begin();curve<this->curve_parameter_table.end();curve++)
	{
		//目標距離是否在這條曲線中
		if (distance < curve->back().s)
		{
			for (auto point = curve->begin(); point < curve->end(); point++)
			{
				if (distance < point->s)
				{
					Parameter p1 = *(point-1);
					Parameter p2 = *point;
					//linear interpolation
					return (p1.t*(p2.s - distance) + p2.t*(distance - p1.s)) / (p2.s - p1.s);
				}
			}
		}
	}
}
double Bspline::getRailLength()
{
	return this->curve_parameter_table.back().back().s;
}