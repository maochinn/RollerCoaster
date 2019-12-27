#if defined(NANOGUI_GLAD)
#if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
#define GLAD_GLAPI_EXPORT
#endif

#include <glad/glad.h>
#else
#if defined(__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif
#endif

#include <glad\glad.h>
#include <GLFW/glfw3.h>

#include <nanogui/nanogui.h>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Cube.h"
#include "Sphere.h"
#include "Texture.h"
#include "Train.h"
#include "HeightMap.h"
#include "CubeMap.h"
#include "DynamicCubeMap.h"
#include "ParticleGenerator.h"
#include "ShadowMap.h"
#include "Light.h"

enum ViewModel { EDIT_MODEL = 0, FREE_MODEL, TRAIN_MODEL };


GLfloat last_x = 400, last_y = 300;

bool right_mouse = false;	//when press = true, release = false
bool left_mouse = false;
bool first_mouse = true;
bool left_first = true;

std::string save_name = "train";
std::string load_path = "train.txt";

bool keys[1024] = { 0 };

int width = 1200;	//width of window
int height = 800;	//height of window

ViewModel view_model = EDIT_MODEL;
Train::MoveAxis move_axis = Train::MoveAxis::MOVE_Y;
Spline::SplineType rail_type = Spline::SplineType::CubicBspline;
float tension = 0.5f;

GLFWwindow* window = nullptr;
nanogui::Screen* screen = nullptr;
nanogui::ComboBox* cobo = nullptr;

Camera* camera = nullptr;
float camera_center_distance = 20.0f;



//Bspline* rail = nullptr;
Train* train = nullptr;
HeightMap* wave = nullptr;
HeightMap* taiwan = nullptr;

Texture2D* texture = nullptr;
Texture2D* texture2 = nullptr;
Texture2D* texture_smoke = nullptr;
UBO* matrix = nullptr;
UBO* light = nullptr;
UBO* view = nullptr;


ObjModel* track_head = nullptr;
ObjModel* track_body = nullptr;
ObjModel* sleeper = nullptr;
ObjModel* rail = nullptr;
ObjModel* column = nullptr;

Cube* texture_cube = nullptr;

CubeMap* skybox = nullptr;
CubeMap* wave_skybox = nullptr;
DynamicCubeMap* dynamic_skybox = nullptr;

ParticleGenerator* smoke = nullptr;

ShadowMap* shadow_map = nullptr;

DirectLight* sun;

void initialize();
void setCallBack();
void setGUI();
void setUBO();
void setKeyboard(float dt);
void update(float dt);
void renderSceneDepth();
void renderScene();

int main(int /* argc */, char** /* argv */) 
{
	glfwInit();

	glfwSetTime(0);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Create a GLFWwindow object
	window = glfwCreateWindow(width, height, "Basic", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Could not initialize GLAD!");
	glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM

	glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);


	initialize();
	setCallBack();
	setGUI();





	GLfloat current_frame = 0.0f;
	GLfloat last_frame = 0.0f;
	GLfloat delta_time = 0.0f;
	GLfloat frame_time = 0.0f;
	int frame = 0;
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		if (frame_time >= 1.0f)
		{
			system("cls");
			printf("FPS:%f", (float)frame / frame_time);
			frame_time = 0.0f;
			frame = 0;
		}

		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		glfwGetFramebufferSize(window, &width, &height);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		frame_time += delta_time;
		frame++;




		setKeyboard(delta_time);

		update(delta_time);

		//render depth map to shadow map
		{
			glm::vec3 pos = train->getTrainCenterPosition() - sun->direction * 50.0f;
			shadow_map->setLight(pos, sun->direction, glm::vec2(50.0f, 50.0f));

			shadow_map->bindFBO();
			glViewport(0, 0, shadow_map->resolution.x, shadow_map->resolution.y);
			// Clear all relevant buffers
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
			glClear(GL_DEPTH_BUFFER_BIT);


			//set UBO
			glm::mat4 view = shadow_map->getViewMtx();
			glm::mat4 projection = shadow_map->getProjectionMtx();
			glBindBuffer(GL_UNIFORM_BUFFER, matrix->ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection[0][0]);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view[0][0]);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			renderSceneDepth();
		}



		//render six face to create dynamic cubemap
		/*
		if (wave_skybox != nullptr)
		{
			delete wave_skybox;
			wave_skybox = nullptr;
		}

		for (int i = 0; i < 6; i++)
		{
			GLenum face = dynamic_skybox->face_type[i];
			//bind frame buffer
			dynamic_skybox->bindFrameBufferToFace(face);
			glViewport(0, 0, dynamic_skybox->resolution.x, dynamic_skybox->resolution.y);
			// Clear all relevant buffers
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			setUBO();

			glm::mat4 view = dynamic_skybox->getViewMtx(wave->getPosition(), face);
			glm::mat4 projection = dynamic_skybox->getProjectionMtx();
			glBindBuffer(GL_UNIFORM_BUFFER, matrix->ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection[0][0]);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view[0][0]);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			renderScene();
		}
		wave_skybox = new CubeMap(dynamic_skybox->shader, dynamic_skybox->getTextureId());
		*/
		wave_skybox = skybox;
		//render
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setUBO();
		renderScene();


		// Draw nanogui
		screen->drawContents();
		screen->drawWidgets();


		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

void initialize()
{
	//ubo
	//matrices for 3D persective projection of uniform buffer
	matrix = new UBO();
	matrix->size = 2 * sizeof(glm::mat4);
	glGenBuffers(1, &matrix->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, matrix->ubo);
	glBufferData(GL_UNIFORM_BUFFER, matrix->size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	light = new UBO();
	light->size = 4 * sizeof(glm::vec4);
	glGenBuffers(1, &light->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, light->ubo);
	glBufferData(GL_UNIFORM_BUFFER, light->size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	view = new UBO();
	view->size = 2 * sizeof(glm::vec4);
	glGenBuffers(1, &view->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, view->ubo);
	glBufferData(GL_UNIFORM_BUFFER, view->size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Create a nanogui screen and pass the glfw pointer to initialize
	screen = new nanogui::Screen();
	screen->initialize(window, true);

	Shader shader("code/shaders/noTexture.vert", "code/shaders/noTexture.frag");
	Shader simple_shader("code/shaders/simple.vert", "code/shaders/simple.frag");
	Shader t_shader("code/shaders/singleTexture.vert", "code/shaders/singleTexture.frag");
	Shader wave_map_shader("code/shaders/heightMapShader.vert", "code/shaders/waveShader.frag");
	Shader height_map_shader("code/shaders/heightMapShader.vert", "code/shaders/heightMapShader.frag");
	Shader cube_map_shader("code/shaders/cubeMap.vert", "code/shaders/cubeMap.frag");
	Shader particle_shader("code/shaders/particleShader.vert", "code/shaders/particleShader.frag");
	Shader simple_depth_shader("code/shaders/simpleDepth.vert", "code/shaders/simpleDepth.frag");
	Shader shadow_map_shader("code/shaders/shadowMap.vert", "code/shaders/shadowMap.frag");

	texture = new Texture2D("img/kurumi.png");
	texture2 = new Texture2D("img/chara_kurumi.png");
	texture_smoke = new Texture2D("img/kenney_particlePack/PNG/smoke_01.png");

	texture_cube = new Cube(t_shader);

	track_head = new ObjModel(shader, "model/track.obj", "track_head");
	track_body = new ObjModel(shader, "model/body.obj", "track_body");
	rail = new ObjModel(shader, "model/rail.obj", "rail");
	sleeper = new ObjModel(shader, "model/sleeper.obj", "rail_tie");
	column = new ObjModel(shader, "model/columnj.obj", "columnj");

	wave = new HeightMap("img/waves5/waves5", 200, wave_map_shader,
		glm::vec3(500.0f, 1.0f, 500.0f));
	taiwan = new HeightMap("img/Taiwan", 1, height_map_shader, 
		glm::vec3(100.0f, 10.0f, 100.0f), glm::vec3(0.0f, 4.5f, 0.0f));

	train = new Train(track_head, rail, sleeper, column,
		simple_shader, rail_type, tension, glm::vec3(-10.0f, 10.0f, -10.0f), glm::vec3(-5.0f, 10.0f, 5.0f),
		glm::vec3(5.0f, 10.0f, 5.0f), glm::vec3(5.0f, 10.0f, -5.0f));

	camera = new Camera();

	skybox = new CubeMap(cube_map_shader,
		"img/skybox/right.jpg",
		"img/skybox/left.jpg",
		"img/skybox/top.jpg",
		"img/skybox/bottom.jpg",
		"img/skybox/back.jpg",
		"img/skybox/front.jpg");
	dynamic_skybox = new DynamicCubeMap(cube_map_shader, glm::ivec2(512, 512));

	smoke = new ParticleGenerator(particle_shader, *texture_smoke, 200);

	sun = new DirectLight
	{
		glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)),
		glm::vec3(0.15f, 0.15f, 0.1f),
		glm::vec3(0.7f, 0.7f, 0.4f),
		glm::vec3(0.2f, 0.2f, 0.2f)
	};

	shadow_map = new ShadowMap(simple_depth_shader, shadow_map_shader, glm::ivec2(1280, 1280));
}
void setGUI()
{
	// Create nanogui gui
	nanogui::Window* setting_window = new nanogui::Window(screen, "float window");
	setting_window->setPosition(nanogui::Vector2i(15, 15));

	setting_window->setPosition(nanogui::Vector2i(425, 300));
	nanogui::GridLayout* layout =
		new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2,
			nanogui::Alignment::Middle, 15, 5);
	layout->setColAlignment(
		{ nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
	layout->setSpacing(0, 10);
	setting_window->setLayout(layout);

	//setting_window->setLayout(new nanogui::GroupLayout());

	(new nanogui::TextBox(setting_window, save_name))
		->setCallback([&](const std::string& str) {save_name = str; return true; });
	(new nanogui::Button(setting_window, "Save"))
		->setCallback([&]() {train->save(save_name); });

	(new nanogui::TextBox(setting_window, load_path))
		->setCallback([&](const std::string& str) {load_path = str; return true; });
	(new nanogui::Button(setting_window, "Load"))
		->setCallback([&]() {train->load(load_path); });

	/* No need to store a pointer, the data structure will be automatically
	freed when the parent window is deleted */
	new nanogui::Label(setting_window, "test label", "sans-bold");
	nanogui::ComboBox* cobo = new nanogui::ComboBox(setting_window, { "EDIT MODEL", "FREE MODEL", "TRAIN MODEL"});
	cobo->setSelectedIndex((int)view_model);
	cobo->setCallback([](int i) {view_model = (ViewModel)i; });

	(new nanogui::Button(setting_window, "Add Control Point"))
		->setCallback([&]() {train->addControlPoint(); });
	(new nanogui::Button(setting_window, "Delete Control Point"))
		->setCallback([&]() {train->deleteControlPoint(); });

	(new nanogui::Button(setting_window, "Add Car"))
		->setCallback([&]() {train->addCar(*track_body); });
	(new nanogui::Button(setting_window, "Delete Car"))
		->setCallback([&]() {train->eraseCar(); });

	nanogui::FloatBox<float>* float_box = new nanogui::FloatBox<float>(setting_window, 0.5f);
	float_box->setCallback([&](float value) 
		{
			tension = value;
			train->setTension(tension);
			return false; 
		});
	float_box->setMinMaxValues(0.1f, 2.0f);
	float_box->setEditable(true);
	float_box->setSpinnable(true);

	float_box = new nanogui::FloatBox<float>(setting_window, 0.5f);
	float_box->setCallback([&](float value)
		{
			train->setCarVelocity(value);
			return false;
		});
	float_box->setValue(train->getCarVelocity());
	float_box->setMinMaxValues(0.0f, 100.0f);
	float_box->setEditable(true);
	float_box->setValueIncrement(2.0f);
	float_box->setSpinnable(true);

	cobo = new nanogui::ComboBox(setting_window, { "Move X", "Move Y", "Move Z" });
	cobo->setSelectedIndex((int)move_axis);
	cobo->setCallback([](int i) {move_axis = (Train::MoveAxis)i; });

	cobo = new nanogui::ComboBox(setting_window, { "Cardinal Cubic", "Cubic Bspline"});
	cobo->setSelectedIndex((int)rail_type);
	cobo->setCallback([&](int i) 
		{
			rail_type = (Spline::SplineType)i;
			train->setSplineType(rail_type);
		});


	screen->setVisible(true);
	screen->performLayout();
}
void setCallBack()
{
	glfwSetCursorPosCallback(window,
		[](GLFWwindow*, double x_pos, double y_pos) {
			screen->cursorPosCallbackEvent(x_pos, y_pos);
			if (right_mouse || left_mouse)
			{
				if (first_mouse)
				{
					last_x = (float)x_pos;
					last_y = (float)y_pos;
					first_mouse = false;
				}

				GLfloat xoffset = (float)x_pos - last_x;
				GLfloat yoffset = last_y - (float)y_pos;  // Reversed since y-coordinates go from bottom to left

				if (right_mouse)
				{
					if (view_model == EDIT_MODEL)
						camera->ProcessMouseMovement(xoffset, yoffset);
					else if (view_model == FREE_MODEL || view_model == TRAIN_MODEL)
						camera->ProcessMouseMovement(-xoffset, -yoffset);
				}
				if (left_mouse)
				{
					glm::f64vec2 mouse;
					glfwGetCursorPos(window, &mouse.x, &mouse.y);
					train->doMove(mouse, move_axis);
				}
			
			}
			last_x = (float)x_pos;
			last_y = (float)y_pos;
		}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow* window, int button, int action, int modifiers)
		{
			screen->mouseButtonCallbackEvent(button, action, modifiers);
			if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
				right_mouse = true;
			else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
				right_mouse = false;
			else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			{
				left_mouse = true;
				glm::f64vec2 mouse;
				glfwGetCursorPos(window, &mouse.x, &mouse.y);
				train->doPick(mouse);
			}
				
			else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			{
				left_mouse = false;
				glm::f64vec2 mouse;
				glfwGetCursorPos(window, &mouse.x, &mouse.y);
				train->doPick(mouse);
			}

		}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow*, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
			if (key >= 0 && key < 1024)
			{
				if (action == GLFW_PRESS)
					keys[key] = true;
				else if (action == GLFW_RELEASE)
					keys[key] = false;
			}
			screen->keyCallbackEvent(key, scancode, action, mods);
		}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow*, unsigned int codepoint) {
			screen->charCallbackEvent(codepoint);
		}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow*, int count, const char** filenames)
		{
			screen->dropCallbackEvent(count, filenames);
		}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow*, double x, double y)
		{
			if (screen->scrollCallbackEvent(x, y))
				return;
			if (view_model == EDIT_MODEL)
			{
				camera_center_distance -= y;
			}
			else if (view_model == FREE_MODEL || view_model == TRAIN_MODEL)
			{
				camera->ProcessMouseScroll(y);
			}
			
		});

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow*, int width, int height) {
			screen->resizeCallbackEvent(width, height);
		}
	);
}
void setUBO()
{
	if (view_model == EDIT_MODEL)
	{
		glm::vec3 pos = train->getTrainCenterPosition() - camera->Front* camera_center_distance;
		camera->setCameraWorldPosition(pos);
	}
	else if (view_model == TRAIN_MODEL)
	{
		camera->setCameraWorldPosition(train->getCarFrontTopPosition());
		camera->Front = train->getCarFront();
	}

	//update ubo_perspective
	glm::mat4 view_matrix = camera->GetViewMatrix();
	glm::mat4 projection_matrix;
	projection_matrix = glm::perspective(glm::radians(camera->Zoom), (GLfloat)width / (GLfloat)height, 0.01f, 1000.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, matrix->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, light->ubo);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &sun->direction[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(glm::vec4), &sun->ambient[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::vec4), sizeof(glm::vec4), &sun->diffuse[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::vec4), sizeof(glm::vec4), &sun->specular[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, view->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &camera->Front[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(glm::vec4), &camera->Position[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//set static render pipeline matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&projection_matrix[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&view_matrix[0][0]);
}
void update(float dt)
{
	train->update(dt);
	smoke->update(train->getCarTopPosition(), dt);
	wave->update();
	taiwan->update();
}
void renderSceneDepth()
{
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/0, matrix->ubo, 0, matrix->size);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	train->renderTrack(&shadow_map->shader);
	train->renderRail(&shadow_map->shader);
	train->renderColumn(&shadow_map->shader);

	taiwan->shader.Use();
	glUniform1i(glGetUniformLocation(taiwan->shader.Program, "u_have_shadow"), 0);
	taiwan->render();
}
void renderScene()
{
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/0, matrix->ubo, 0, matrix->size);
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/1, light->ubo, 0, light->size);
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/2, view->ubo, 0, view->size);
	shadow_map->shadow_shader.Use();
	shadow_map->bind(0);
	glUniform1i(glGetUniformLocation(shadow_map->shadow_shader.Program, "u_shadow_map"), 0);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map->shadow_shader.Program, "u_light_space_matrix"), 1, GL_FALSE, &shadow_map->getViewProjectionMtx()[0][0]);


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	/*{
		glm::mat4 model = rail->calculateKnotPointModelMatrix
		(
			rail->getReparameterT(rocket_current),
			rail->getReparameterT(rocket_next)
		);
		cube->shader.Use();
		glUniformMatrix4fv(glGetUniformLocation(cube->shader.Program, "u_model"), 1, GL_FALSE, &model[0][0]);
		cube->render(glm::vec3(0.0f, 1.0f, 0.0f));
	}*/


	train->renderTrack(&shadow_map->shadow_shader);
	train->renderRail(&shadow_map->shadow_shader);
	train->renderColumn(&shadow_map->shadow_shader);


	texture_cube->shader.Use();
	shadow_map->bind(0);
	glUniform1i(glGetUniformLocation(texture_cube->shader.Program, "u_material.t_diffuse"), 0);
	shadow_map->bind(1);
	glUniform1i(glGetUniformLocation(texture_cube->shader.Program, "u_material.t_specular"), 1);
	texture_cube->render(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	if (wave_skybox != nullptr)
	{
		wave->shader.Use();
		wave_skybox->bind(1);
		glUniform1i(glGetUniformLocation(wave->shader.Program, "u_cubemap"), 1);
		wave->shader.Use();
		shadow_map->bind(2);
		glUniform1i(glGetUniformLocation(wave->shader.Program, "u_shadow_map"), 2);
		glUniformMatrix4fv(glGetUniformLocation(wave->shader.Program, "u_light_space_matrix"), 1, GL_FALSE, &shadow_map->getViewProjectionMtx()[0][0]);
		wave->render();

		skybox->render();
	}
	else
		skybox->render();

	//bind shadow map
	taiwan->shader.Use();
	glUniform1i(glGetUniformLocation(taiwan->shader.Program, "u_have_shadow"), 1);
	shadow_map->bind(1);
	glUniform1i(glGetUniformLocation(taiwan->shader.Program, "u_shadow_map"), 2);
	glUniformMatrix4fv(glGetUniformLocation(wave->shader.Program, "u_light_space_matrix"), 1, GL_FALSE, &shadow_map->getViewProjectionMtx()[0][0]);
	taiwan->render();

	smoke->render(camera->Front);
}
void setKeyboard(/*delta time*/float dt)
{
	if (keys[GLFW_KEY_W])
		camera->ProcessKeyboard(FORWARD, dt);
	if (keys[GLFW_KEY_S])
		camera->ProcessKeyboard(BACKWARD, dt);
	if (keys[GLFW_KEY_A])
		camera->ProcessKeyboard(LEFT, dt);
	if (keys[GLFW_KEY_D])
		camera->ProcessKeyboard(RIGHT, dt);

	if (keys[GLFW_KEY_UP])
	{
		train->beforeControlPoint();
		keys[GLFW_KEY_UP] = false;
	}
	else if (keys[GLFW_KEY_DOWN])
	{
		train->nextControlPoint();
		keys[GLFW_KEY_DOWN] = false;
	}

	if (keys[GLFW_KEY_R])
	{
		train->rotateControlPointX(0.5f);
	}
	else if (keys[GLFW_KEY_F])
	{
		train->rotateControlPointX(-0.5f);
	}
	if (keys[GLFW_KEY_INSERT])
	{
		glm::vec3 vector = glm::inverse(camera->GetViewMatrix()) * glm::vec4(0.01f, 0.0f, 0.0f, 0.0f);
		//train->translateControlPoint(vector);
	}
	else if (keys[GLFW_KEY_DELETE])
	{
		glm::vec3 vector = glm::inverse(camera->GetViewMatrix()) * glm::vec4(-0.01f, 0.0f, 0.0f, 0.0f);
		//train->translateControlPoint(vector);
	}
	if (keys[GLFW_KEY_HOME])
	{
		glm::vec3 vector = glm::inverse(camera->GetViewMatrix()) * glm::vec4(0.0f, 0.01f, 0.0f, 0.0f);
		//train->translateControlPoint(vector);
	}
	else if (keys[GLFW_KEY_END])
	{
		glm::vec3 vector = glm::inverse(camera->GetViewMatrix()) * glm::vec4(0.0f, -0.01f, 0.0f, 0.0f);
		//train->translateControlPoint(vector);
	}
	if (keys[GLFW_KEY_PAGE_UP])
	{
		glm::vec3 vector = glm::inverse(camera->GetViewMatrix()) * glm::vec4(0.0f, 0.0f, 0.01f, 0.0f);
		//train->translateControlPoint(vector);
	}
	else if (keys[GLFW_KEY_PAGE_DOWN])
	{
		glm::vec3 vector = glm::inverse(camera->GetViewMatrix()) * glm::vec4(0.0f, 0.0f, -0.01f, 0.0f);
		//train->translateControlPoint(vector);
	}

	if (keys[GLFW_KEY_Z])
	{
		train->addControlPoint();
		keys[GLFW_KEY_Z] = false;
	}
	if (keys[GLFW_KEY_C])
	{
		train->deleteControlPoint();
		keys[GLFW_KEY_C] = false;
	}
	if (keys[GLFW_KEY_Q])
	{
		train->addCar(*track_body);
		keys[GLFW_KEY_Q] = false;
	}
	if (keys[GLFW_KEY_E])
	{
		train->eraseCar();
		keys[GLFW_KEY_E] = false;
	}
	if (keys[GLFW_KEY_SPACE])
	{
		train->setCarVelocity(5.0f);
		keys[GLFW_KEY_E] = false;
	}
}