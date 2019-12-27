#include <glm/glm.hpp>
#include <vector>
#include <glad\glad.h>

#include "BufferObject.h"
#include "Shader.h"
#include "Texture.h"

#define MAX_PARTICLE 300
#define MAX_LIFE 3.0f


struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 color;
	float life;
	Particle()
		: position(0.0f), velocity(0.0f), color(1.0f), life(0.0f) {};
};

class ParticleGenerator
{
public:
	ParticleGenerator(Shader shader, Texture2D texture, int amount):
		shader(shader), texture(texture)
	{
		this->respawn_time = 0.0f;	//sec

		/*generate VAO*/
		GLfloat particle_quad[] = 
		{
			//postion			//texture
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f,
			0.5f, -0.5f, 0.0f,	1.0f, 0.0f,
			0.5f, 0.5f, 0.0f,	1.0f, 1.0f,

			0.5f, 0.5f, 0.0f,	1.0f, 1.0f,
			-0.5f, 0.5f, 0.0f,	0.0f, 1.0f,
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f
		};
		vao.count = 6;
		glGenVertexArrays(1, &this->vao.vao);
		glGenBuffers(1, vao.vbo);
		glBindVertexArray(this->vao.vao);
		// Fill mesh buffer
		glBindBuffer(GL_ARRAY_BUFFER, vao.vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
		// Set mesh attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

		glBindVertexArray(0);

		if (amount > MAX_PARTICLE)
			amount = MAX_PARTICLE;
		for (int i = 0;i<amount; ++i)
			this->particles.push_back(Particle());
	}
	void render(/*front direction of camere*/glm::vec3 camera_dir)
	{
		glm::vec3 v = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 w = -camera_dir;
		glm::vec3 u = glm::cross(v, w);
		glm::mat4 model;
		model[0] = glm::vec4(u.x, u.y, u.z, 0.0f);
		model[1] = glm::vec4(v.x, v.y, v.z, 0.0f);
		model[2] = glm::vec4(w.x, w.y, w.z, 0.0f);
		model[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// Use additive blending to give it a 'glow' effect
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		this->shader.Use();
		//set uniform
		glUniformMatrix4fv(glGetUniformLocation(this->shader.Program, "u_model"), 1, GL_FALSE, &model[0][0]);
		this->texture.bind(0);
		glUniform1i(glGetUniformLocation(this->shader.Program, "u_texture0"), 0);

		int amount = 0;
		for (Particle particle : this->particles)
		{
			if (particle.life > 0.0f)
			{
				std::stringstream ss;
				std::string index;
				ss << amount++;
				index = ss.str();

				glUniform3fv(glGetUniformLocation(this->shader.Program, ("u_offsets[" + index + "]").c_str()), 1, &particle.position[0]);
				glUniform4fv(glGetUniformLocation(this->shader.Program, ("u_colors[" + index + "]").c_str()),  1, &particle.color[0]);
			}
		}
		//render instance
		glBindVertexArray(this->vao.vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, this->vao.count, amount);
		glBindVertexArray(0);

		// Don't forget to reset to default blending mode
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void update(glm::vec3 position, float dt)
	{
		this->respawn_time += dt;
		
		
		this->position = position;
		//std::cout << this->position.x << "  " << this->position.z << std::endl;
		for (Particle& p : this->particles)
		{
			if (this->respawn_time > 0.1f && p.life <= 0.0f)
			{
				respawnParticle(p, glm::vec3(2.0f), glm::vec3(0.0f, 3.0f, 0.0f));
				this->respawn_time = 0.0f;
				break;
			}
				
		}
		for (Particle& p : this->particles)
		{
			p.life -= dt;
			if (p.life > 0.0f)
			{
				p.position += p.velocity * dt;
				p.color.a -= dt * 0.25f;
			}
		}
	}
	glm::vec3 position;
	std::vector<Particle> particles;
private:
	VAO vao;
	Shader shader;
	Texture2D texture;
	float respawn_time;
	void respawnParticle(Particle& p, glm::vec3 offset_range, glm::vec3 v)
	{
		glm::vec3 offset(
			(rand() / (RAND_MAX + 1.0)) * offset_range.x * 0.5f,
			(rand() / (RAND_MAX + 1.0)) * offset_range.y * 0.5f,
			(rand() / (RAND_MAX + 1.0)) * offset_range.z * 0.5f);
		p.position = this->position;
		p.velocity = v;
		p.color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		p.life = MAX_LIFE;
	}
	
};