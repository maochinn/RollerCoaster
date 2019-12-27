#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include<utility>
#include <glm/glm.hpp>
#include <algorithm>

#include "BufferObject.h"


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
};
struct Material
{
	glm::vec3 Ka;	//ambient
	glm::vec3 Kd;	//diffuse
	glm::vec3 Ks;	//specular
	GLfloat Ns;	//exponent
	GLfloat d;	//alpha
};
struct ObjVAO
{
	char name[80];
	VAO vao;
	Material material;
	glm::vec3 min_pos;	//minimum position of AABB
	glm::vec3 max_pos;	//maximum position of AABB

};


/*simple obj loader*/
/*only for triangle face*/
/*use phong's illumination model*/
/*only read position and normal of vertex*/
class ObjModel
{
public:
	Shader shader;
	glm::vec3 min_pos;	//minimum position of AABB
	glm::vec3 max_pos;	//maximum position of AABB

	ObjModel(Shader shader, const char* path, const char* name)
		:shader(shader)
	{
		this->loadOBJ(path);
		this->name = name;
	}
	void render(glm::vec3 position)
	{
		glm::mat4 model;
		model = glm::translate(model, position);
		render(model);
	}
	void render(glm::mat4 model_matrix, Shader* shader = nullptr)
	{
		if (shader == nullptr)
		{
			this->shader.Use();

			glUniformMatrix4fv(glGetUniformLocation(this->shader.Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

			for (auto& obj : this->obj_vaos)
			{
				glUniform3fv(glGetUniformLocation(this->shader.Program, "u_material.ambient"), 1, &obj.material.Ka[0]);
				glUniform3fv(glGetUniformLocation(this->shader.Program, "u_material.diffuse"), 1, &obj.material.Kd[0]);
				glUniform3fv(glGetUniformLocation(this->shader.Program, "u_material.specular"), 1, &obj.material.Ks[0]);
				glUniform1f(glGetUniformLocation(this->shader.Program, "u_material.shininess"), obj.material.Ns);

				glBindVertexArray(obj.vao.vao);
				glDrawArrays(GL_TRIANGLES, 0, obj.vao.count);
				glBindVertexArray(0);
			}
		}
		else
		{
			shader->Use();

			glUniformMatrix4fv(glGetUniformLocation(shader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

			for (auto& obj : this->obj_vaos)
			{
				glUniform3fv(glGetUniformLocation(shader->Program, "u_material.ambient"), 1, &obj.material.Ka[0]);
				glUniform3fv(glGetUniformLocation(shader->Program, "u_material.diffuse"), 1, &obj.material.Kd[0]);
				glUniform3fv(glGetUniformLocation(shader->Program, "u_material.specular"), 1, &obj.material.Ks[0]);
				glUniform1f(glGetUniformLocation(shader->Program, "u_material.shininess"), obj.material.Ns);

				glBindVertexArray(obj.vao.vao);
				glDrawArrays(GL_TRIANGLES, 0, obj.vao.count);
				glBindVertexArray(0);
			}
		}
		
	}

private:
	std::map<std::string, Material> materials;
	std::vector<ObjVAO> obj_vaos;
	std::string name;


	void loadOBJ(const char* path)
	{
		std::ifstream file;
		std::stringstream stream;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open(path);
			stream << file.rdbuf();
			file.close();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::OBJ::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<Vertex> vertices;

		while (!stream.eof())
		{
			std::string str;
			stream >> str;
			if (str[0] == '#')
			{
				//is comment
			}
			else if (str[0] == 'm' && str[1] == 't' && str[2] == 'l')
			{
				//mtllib
				std::string folder(path);
				std::string name;
				stream >> name;

				std::string path = folder.substr(0, folder.rfind('/')) + "/" +  name;

				this->loadMTL(path.c_str());

			}
			else if (str[0] == 'u' && str[1] == 's' && str[2] == 'e' && str[3] == 'm')
			{
				//usemtl
				std::string name;
				stream >> name;

				this->obj_vaos.back().material = this->materials[name];
			}
			else if (str[0] == 'o')
			{
				//is object
				std::string name;
				stream >> name;

				if (this->obj_vaos.empty() == false)
					generateVAO(this->obj_vaos.back(), vertices);

				ObjVAO obj;
				
				std::size_t length = std::min(strlen(name.c_str()), (std::size_t)80);
				strncpy_s(obj.name, name.c_str(), length);

				this->obj_vaos.push_back(obj);
			
				vertices.clear();
			}
			else if (str[0] == 'v' && str[1] == 'n')
			{
				glm::vec3 normal;
				stream >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}
			else if (str[0] == 'v')
			{
				glm::vec3 position;
				stream >> position.x >> position.y >> position.z;
				positions.push_back(position);
			}
			else if (str[0] == 'f')
			{
				//only for triangle face
				for (int i = 0; i < 3; i++)
				{
					Vertex v;
					int position_idx, normal_idx;
					char c;
					stream >> position_idx >> c >> c >> normal_idx;

					v.position = positions[position_idx-1];
					v.normal = normals[normal_idx-1];

					vertices.push_back(v);
				}
			}
			else
			{
				//
			}
			std::getline(stream, str, '\n');
		}
		//last obj data
		if (this->obj_vaos.empty() == false)
			generateVAO(this->obj_vaos.back(), vertices);

		//compute min and max
		{
			glm::vec3 min(FLT_MAX);
			glm::vec3 max(-FLT_MAX);
			for (auto obj : obj_vaos)
			{
				if (obj.min_pos.x < min.x)
					min.x = obj.min_pos.x;
				if (obj.min_pos.y < min.y)
					min.y = obj.min_pos.y;
				if (obj.min_pos.z < min.z)
					min.z = obj.min_pos.z;

				if (obj.max_pos.x > max.x)
					max.x = obj.max_pos.x;
				if (obj.max_pos.y > max.y)
					max.y = obj.max_pos.y;
				if (obj.max_pos.z > max.z)
					max.z = obj.max_pos.z;
			}
			this->min_pos = min;
			this->max_pos = max;
		}
		
	}
	void loadMTL(const char* path)
	{
		std::stringstream stream("empty string!");
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open(path);
			stream << file.rdbuf();
			file.close();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::MTLLIB::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		std::string name;
		while (!stream.eof())
		{
			std::string str;
			stream >> str;
			if (str[0] == '#')
			{
				//comment
			}
			else if (str[0] == 'n' && str[1] == 'e')
			{
				//newmtl
				
				stream >> name;

				Material material;
				{
					material.Ka = glm::vec3(0.2f);
					material.Kd = glm::vec3(0.3f);
					material.Ks = glm::vec3(0.1f);
					material.d = 1.0f;
					material.Ns = 16.0f;
				}

				this->materials.insert(std::pair<std::string, Material>(name, material));
			}
			else if (str[0] == 'N' && str[1] == 's')
			{
				stream >> this->materials[name].Ns;
			}
			else if (str[0] == 'K' && str[1] == 'a')
			{
				stream >> this->materials[name].Ka.r;
				stream >> this->materials[name].Ka.g;
				stream >> this->materials[name].Ka.b;
			}
			else if (str[0] == 'K' && str[1] == 'd')
			{
				stream >> this->materials[name].Kd.r;
				stream >> this->materials[name].Kd.g;
				stream >> this->materials[name].Kd.b;
			}
			else if (str[0] == 'K' && str[1] == 's')
			{
				stream >> this->materials[name].Ks.r;
				stream >> this->materials[name].Ks.g;
				stream >> this->materials[name].Ks.b;
			}
			else if (str[0] == 'd')
			{
				stream >> this->materials[name].d;
			}
			std::getline(stream, str, '\n');
		}
	}
	ObjVAO generateVAO(ObjVAO& obj, std::vector<Vertex> vertices)
	{

		glGenVertexArrays(1, &obj.vao.vao);
		glGenBuffers(1, obj.vao.vbo);

		glBindVertexArray(obj.vao.vao);

		glBindBuffer(GL_ARRAY_BUFFER, obj.vao.vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));

		glBindVertexArray(0);

		obj.vao.count = vertices.size();

		//find min and max position
		glm::vec3 min(FLT_MAX);
		glm::vec3 max(-FLT_MAX);
		for (auto v : vertices)
		{
			if (v.position.x < min.x)
				min.x = v.position.x;
			if (v.position.y < min.y)
				min.y = v.position.y;
			if (v.position.z < min.z)
				min.z = v.position.z;

			if (v.position.x > max.x)
				max.x = v.position.x;
			if (v.position.y > max.y)
				max.y = v.position.y;
			if (v.position.z > max.z)
				max.z = v.position.z;
		}
		obj.min_pos = min;
		obj.max_pos = max;
		return obj;
	}
};