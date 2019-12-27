#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

#include "Shader.h"

class Spline
{
public:
	struct ControlPoint
	{
		glm::vec3 position;
		glm::vec3 orientation;	//up vector
		ControlPoint(glm::vec3 pos, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
			:position(pos), orientation(up) {}
		ControlPoint() :
			position(glm::vec3(0.0f)), 
			orientation(glm::vec3(0.0f, 1.0f, 0.0f)) {}
	};
	struct Parameter
	{
		double t;	//b-spline parameter
		double s;	//distance parameter
	};
	enum SplineType {CardinalCubic, CubicBspline};

public:
	Spline(Shader shader, SplineType type, float tension,
		glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int interval_num = 100)
		:shader(shader), type(type), tension(tension), DIVIDE_LINE(interval_num) 
	{
		control_points.push_back(ControlPoint(p1));
		control_points.push_back(ControlPoint(p2));
		control_points.push_back(ControlPoint(p3));
		control_points.push_back(ControlPoint(p4));

		this->now_point_idx = -1;

		updateCurves();
	}
	
	void saveSpline(std::string name)
	{
		FILE* fp;
		std::string str = name + ".txt";
		fopen_s(&fp, str.c_str(), "w+");
		fprintf(fp, "%d\n", this->control_points.size());
		for (auto& point : control_points)
		{
			fprintf(fp, "p %f %f %f\n",
				point.position.x,
				point.position.y,
				point.position.z);
			fprintf(fp, "o %f %f %f\n",
				point.orientation.x,
				point.orientation.y,
				point.orientation.z);
		}
		fclose(fp);
	}
	void loadSpline(std::string path)
	{
		std::ifstream file(path.c_str());
		if (file.bad())
		{
			printf("open file '%d' failed!", path.c_str());
			return;
		}
		std::stringstream ss;
		ss << file.rdbuf();
		int amount;
		char temp;
		ss >> amount;
		this->control_points.clear();
		for (int i = 0; i < amount; i++)
		{
			ControlPoint point;
			ss >> temp >> point.position.x >> point.position.y >> point.position.z;
			ss >> temp >> point.orientation.x >> point.orientation.y >> point.orientation.z;
			this->control_points.push_back(point);
		}
		updateCurves();
	}
	
	void renderControlPoints()
	{
		for (int i = 0;i<control_points.size();i++)
		{
			if(i == this->now_point_idx)
				renderControlPoint(
					control_points[i].position, 
					control_points[i].orientation, 
					glm::vec3(1.0f, 0.0f, 0.0f));
			else
				renderControlPoint(
					control_points[i].position,
					control_points[i].orientation,
					glm::vec3(0.8f, 0.8f, 0.8f));

		}
	}
	void renderControlPoint(glm::vec3 pos, glm::vec3 orient, glm::vec3 color)
	{
		glUseProgram(0);

		float size = 0.5;
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(pos.x, pos.y, pos.z);
		float theta1 = -glm::degrees(atan2(orient.z, orient.x));
		glRotatef(theta1, 0, 1, 0);
		float theta2 = -glm::degrees(acos(orient.y));
		glRotatef(theta2, 0, 0, 1);

		glBegin(GL_QUADS);
		glColor3fv(&color[0]);
		glNormal3f(0, 0, 1);
		glVertex3f(size, size, size);
		glVertex3f(-size, size, size);
		glVertex3f(-size, -size, size);
		glVertex3f(size, -size, size);

		glNormal3f(0, 0, -1);
		glVertex3f(size, size, -size);
		glVertex3f(size, -size, -size);
		glVertex3f(-size, -size, -size);
		glVertex3f(-size, size, -size);

		// no top - it will be the point

		glNormal3f(0, -1, 0);
		glVertex3f(size, -size, size);
		glVertex3f(-size, -size, size);
		glVertex3f(-size, -size, -size);
		glVertex3f(size, -size, -size);

		glNormal3f(1, 0, 0);
		glVertex3f(size, size, size);
		glVertex3f(size, -size, size);
		glVertex3f(size, -size, -size);
		glVertex3f(size, size, -size);

		glNormal3f(-1, 0, 0);
		glVertex3f(-size, size, size);
		glVertex3f(-size, size, -size);
		glVertex3f(-size, -size, -size);
		glVertex3f(-size, -size, size);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(0, 1.0f, 0);
		glVertex3f(0, 3.0f * size, 0);
		glNormal3f(1.0f, 0.0f, 1.0f);
		glVertex3f(size, size, size);
		glNormal3f(-1.0f, 0.0f, 1.0f);
		glVertex3f(-size, size, size);
		glNormal3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(-size, size, -size);
		glNormal3f(1.0f, 0.0f, -1.0f);
		glVertex3f(size, size, -size);
		glNormal3f(1.0f, 0.0f, 1.0f);
		glVertex3f(size, size, size);
		glEnd();
		glPopMatrix();
	}
	void renderOrientation(glm::mat4 model)
	{
		glm::vec3 u = model[0] * 5.0f;
		glm::vec3 v = model[1] * 5.0f;
		glm::vec3 w = model[2] * 5.0f;
		glm::vec3 pos = model[3];
		
		glUseProgram(0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3fv(&pos[0]);
		glVertex3fv(&(pos + u)[0]);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3fv(&pos[0]);
		glVertex3fv(&(pos + v)[0]);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3fv(&pos[0]);
		glVertex3fv(&(pos + w)[0]);
		glEnd();
		glPopMatrix();
	}
	void renderSpline()
	{
		glUseProgram(0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLineWidth(3);
		glBegin(GL_LINE_LOOP);
		glColor3ub(0, 255, 0);
		for(auto& curve: this->curve_knot_points)
			for (auto& knot_point : curve)
			{
				glVertex3fv(&(knot_point)[0]);
			}
		glEnd();
		glPopMatrix();
	}

	float getSplineLength()
	{
		return this->curve_parameter_table.back().back().s;
	}
	glm::mat4 getModelMatrixParameter(float p)
	{
		int m = this->control_points.size();

		//Qi ith curve
		int i = (int)p;
		//Qi(t) parameter of now pos 
		float t = p - i;
		//Qi(tt) parameter of next pos
		float tt = std::min(1.0f, t + 0.01f);

		glm::mat4 M;
		if (type == CubicBspline)
		{
			M[0] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
			M[1] = glm::vec4(3.0f, -6.0f, 3.0f, 0.0f);
			M[2] = glm::vec4(-3.0f, 0.0f, 3.0f, 0.0f);
			M[3] = glm::vec4(1.0f, 4.0f, 1.0f, 0.0f);
			M *= 0.166667f;
		}
		else if(type == CardinalCubic)
		{
			M[0] = glm::vec4(-1.0f, (2.0f / tension) - 1.0f, (-2.0f / tension) + 1.0f, 1.0f);
			M[1] = glm::vec4(2.0f, (-3.0f / tension) + 1.0f, (3.0f / tension) - 2.0f, -1.0f);
			M[2] = glm::vec4(-1.0f, 0.0f, 1.0f, 0.0f);
			M[3] = glm::vec4(0.0f, 1.0f / tension, 0.0f, 0.0f);
			M *= tension;
		}

		glm::vec4 T(t * t * t, t * t, t, 1.0f);
		glm::vec4 TT(tt * tt * tt, tt * tt, tt, 1.0f);

		glm::vec4 M_T = M * T;
		glm::vec4 M_TT = M * TT;

		//calculate kont point position
		//now pos Qi(t)
		glm::vec3 Q_t;
		//next pos Qi(tt)
		glm::vec3 Q_tt;
		{
			glm::vec4 G_x(
				control_points[i].position.x,
				control_points[(i + 1) % m].position.x,
				control_points[(i + 2) % m].position.x,
				control_points[(i + 3) % m].position.x);
			glm::vec4 G_y(
				control_points[i].position.y,
				control_points[(i + 1) % m].position.y,
				control_points[(i + 2) % m].position.y,
				control_points[(i + 3) % m].position.y);
			glm::vec4 G_z(
				control_points[i].position.z,
				control_points[(i + 1) % m].position.z,
				control_points[(i + 2) % m].position.z,
				control_points[(i + 3) % m].position.z);
			//Qi(t)
			Q_t.x = glm::dot(G_x, M_T);
			Q_t.y = glm::dot(G_y, M_T);
			Q_t.z = glm::dot(G_z, M_T);

			Q_tt.x = glm::dot(G_x, M_TT);
			Q_tt.y = glm::dot(G_y, M_TT);
			Q_tt.z = glm::dot(G_z, M_TT);
		}
		//calculate knot point orient
		glm::vec3 Q_t_orient;
		{
			glm::vec4 G_x(
				control_points[i].orientation.x,
				control_points[(i + 1) % m].orientation.x,
				control_points[(i + 2) % m].orientation.x,
				control_points[(i + 3) % m].orientation.x);
			glm::vec4 G_y(
				control_points[i].orientation.y,
				control_points[(i + 1) % m].orientation.y,
				control_points[(i + 2) % m].orientation.y,
				control_points[(i + 3) % m].orientation.y);
			glm::vec4 G_z(
				control_points[i].orientation.z,
				control_points[(i + 1) % m].orientation.z,
				control_points[(i + 2) % m].orientation.z,
				control_points[(i + 3) % m].orientation.z);
			//Qi(t)
			Q_t_orient.x = glm::dot(G_x, M_T);
			Q_t_orient.y = glm::dot(G_y, M_T);
			Q_t_orient.z = glm::dot(G_z, M_T);
		}

		glm::vec3 up = glm::normalize(Q_t_orient);
		glm::vec3 u = glm::normalize(Q_tt - Q_t);
		glm::vec3 w = glm::cross(u, up);
		glm::vec3 v = glm::cross(w, u);

		glm::mat4 model;

		model[0] = glm::vec4(u, 0.0f);
		model[1] = glm::vec4(v, 0.0f);
		model[2] = glm::vec4(w, 0.0f);
		model[3] = glm::vec4(Q_t, 1.0f);

		return model;
	}
	glm::mat4 getModelMatrixDistance(float d)
	{
		float distance = fabs(fmod(d, getSplineLength()));
		for (auto curve = this->curve_parameter_table.begin(); curve < this->curve_parameter_table.end(); curve++)
		{
			//目標距離是否在這條曲線中
			if (distance < curve->back().s)
			{
				for (auto point = curve->begin(); point < curve->end(); point++)
				{
					if (distance < point->s)
					{
						Parameter p1 = *(point - 1);
						Parameter p2 = *point;
						//linear interpolation
						float t = (p1.t * (p2.s - distance) + p2.t * (distance - p1.s)) / (p2.s - p1.s);

						return getModelMatrixParameter(t);
					}
				}
			}
		}
	}
	glm::vec3 getControlPointPosition(int idx)
	{
		if (idx < 0 || idx >= control_points.size())
			return glm::vec3(0.0f);

		return control_points[idx].position;
	}
	glm::vec3 getNowControlPointPosition() { return getControlPointPosition(now_point_idx); }
	int getControlPointAmount() { return this->control_points.size(); }
	bool selectControlPoint() 
	{
		if (now_point_idx < 0 || now_point_idx >= control_points.size())
			return false;
		else
			return true;
	}

	void setNowControlPointPos(glm::vec3 pos) 
	{ 
		if (now_point_idx < 0 || now_point_idx >= control_points.size())
			return;

		this->control_points[now_point_idx].position = pos; 
		updateCurves();
	}
	void setTension(float tension)
	{
		this->tension = tension;
		updateCurves();
	}
	void setSplineType(SplineType type)
	{
		this->type = type;
		updateCurves();
	}


	void addControlPoint()
	{
		int idx = this->now_point_idx;

		if (idx < 0 || idx >= control_points.size())
			return;
		int next_idx = (idx + 1) % control_points.size();

		ControlPoint now = control_points[idx];
		ControlPoint next = control_points[next_idx];

		ControlPoint new_point{
			0.5f * (now.position + next.position),
			0.5f * (now.orientation + next.orientation) };

		this->now_point_idx = next_idx;
		this->control_points.insert(this->control_points.begin() + next_idx, new_point);
		this->updateCurves();
	}
	void deleteControlPoint()
	{
		int idx = this->now_point_idx;
		if (idx < 0 || idx >= control_points.size())
			return;
		if (control_points.size() <= 4)
			return;

		this->control_points.erase(control_points.begin() + idx);
		this->updateCurves();
	}
	void nextControlPoint()
	{
		this->now_point_idx = (now_point_idx + 1) % control_points.size();
	}
	void beforeControlPoint()
	{
		this->now_point_idx = (now_point_idx - 1) % control_points.size();
	}
	void rotateControlPointX(float degree)
	{
		int idx = this->now_point_idx;

		if (idx < 0 || idx >= control_points.size())
			return;


		glm::vec3 front = glm::normalize(
			control_points[(idx+1)%control_points.size()].position - control_points[idx].position);
		glm::vec3 right = glm::normalize(glm::cross(front, control_points[idx].orientation));
		glm::vec3 up = glm::cross(right, front);
		glm::mat4 rotate = glm::rotate(glm::mat4(), glm::radians(degree), front);
		glm::vec3 new_orient = rotate * glm::vec4(up, 0.0f);

		control_points[idx].orientation = glm::normalize(new_orient);
	}
	void rotateControlPointZ(float degree)
	{
		int idx = this->now_point_idx;

		if (idx < 0 || idx >= control_points.size())
			return;

		glm::vec3 left = getModelMatrixParameter(idx)[2];
		glm::vec3 new_orient =
			glm::vec4(control_points[idx].orientation, 0.0f) *
			glm::rotate(glm::mat4(), glm::radians(degree), left);

		control_points[idx].orientation = glm::normalize(new_orient);
	}


public:
	Shader shader;
	int now_point_idx;
	float tension;

	std::vector<ControlPoint> control_points;
private:


	void updateCurves()
	{
		//free curve
		for (auto curve : this->curve_knot_points)
			curve.clear();
		this->curve_knot_points.clear();

		for (int i = 0; i < this->control_points.size(); i++)
		{
			std::vector <glm::vec3> knot_points;
			for (int j = 0; j < DIVIDE_LINE; j++)
			{
				float t = (float)j / DIVIDE_LINE;
				glm::mat4 model = this->getModelMatrixParameter(i + t);
				glm::vec3 pos = glm::vec3(model[3]);

				knot_points.push_back(pos);
			}
			this->curve_knot_points.push_back(knot_points);
		}

		//free table
		for (auto curve : this->curve_parameter_table)
			curve.clear();
		this->curve_parameter_table.clear();

		double s = 0.0;
		double t = 0.0;
		double dt = 1.0 / (DIVIDE_LINE - 1);
		//calculate curve length
		for (auto knot_points : this->curve_knot_points)
		{
			std::vector<Parameter> segment;
			Parameter p;
			p.t = t;
			p.s = s;
			segment.push_back(p);
			for (int j = 0; j < knot_points.size() - 1; j++)
			{
				double ds = glm::length(
					knot_points[j + 1] - knot_points[j]);
				t += dt;
				s += ds;
				segment.push_back(Parameter{ t, s });
			}
			this->curve_parameter_table.push_back(segment);
		}
	}

private:
	std::vector<std::vector<Parameter>> curve_parameter_table;	//curve[Q]: Qth curve
	std::vector<std::vector<glm::vec3>> curve_knot_points;

	int DIVIDE_LINE;	//amount of interval of a curve
	SplineType type;
};