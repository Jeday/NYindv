#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/glut.h>
#include <SOIL2.h>
#include <cmath>
#include <utility>
#include <iostream>
#include <vector>
#include <list>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp> 
#include<glm/trigonometric.hpp>

#include "GLShader.h"
#include "GLobject.h"
#include "Camera.h"


std::vector<GLobject*> scene;
GLShader * shaderwrap;
int VertShaderPhong, FragShaderPhong;
std::vector<int> VertShaders;
std::vector<int> FragShaders;
glm::mat4 Matrix_projection;

int w = 0, h = 0;
float rotateX = 0;
float rotateY = 0;
float scaleX = 1;
float scaleY = 1;


//x: -80..80, y: -80..80
std::vector<glm::mat4> forest{
	

};




std::vector<glm::mat4> SingleEmptyInstance{
	glm::mat4(1.0f)
};

std::vector<std::vector<glm::mat4>> instancing_trasforms{
	SingleEmptyInstance, // ground
	SingleEmptyInstance, // snowbase
	forest, // trees
	SingleEmptyInstance,
	SingleEmptyInstance,
};


int oldTimeSinceStart = 0;
bool animate = false;

float angle = 0;



Camera cam(glm::vec3(10.0f,0,10.0f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));




// параметры источника освещения
struct Light
{
public: 
	glm::vec4 light_position;
	glm::vec4 light_ambient;
	glm::vec4 light_diffuse;
	glm::vec4 light_specular;
	glm::vec3 light_attenuation;
	glm::vec3 spot_direction;
	float spot_cutoff;
	float spot_exp;
};


std::vector<Light> lights;

std::vector<std::string> pathsVert = {
"shader_phong_struct.vert"
};

std::vector<std::string> pathsFrag = {
"shader_phong_struct.frag"
};

void LoadShaders() {
	VertShaderPhong = shaderwrap->load_shader(pathsVert[0], GL_VERTEX_SHADER);
	FragShaderPhong = shaderwrap->load_shader(pathsFrag[0], GL_FRAGMENT_SHADER);
}

void Init(void)
{
	glClearColor(0, 0, 0, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Set_cam() {
	Matrix_projection = glm::perspective(glm::radians(60.0f), (float)w / h, 0.01f, 200.0f);
	Matrix_projection *= cam.getViewMatrix();
}

void Reshape(int x, int y)
{
	if (y == 0 || x == 0) return;

	w = x;
	h = y;
	glViewport(0, 0, w, h);

	Set_cam();
}


void set_light() {
	

	Light l2;
	l2.light_position = { 0,0,10,1 };
	l2.light_ambient = { 0.2,0.2,0.2,1 };
	l2.light_diffuse = { 0.3,0.3,0.3,1 };
	l2.light_specular = { 0.1,0.1,0.1,1 };
	l2.light_attenuation = { 0,0,0 };
	l2.spot_direction = { 0, 0, -1 };
	l2.spot_cutoff = 0;
	l2.spot_exp = 0;
	lights.push_back(l2);

	Light l1;
	l1.light_position = { 0,0,7,1 };
	l1.light_ambient = { 0.2,0.2,0.2,1 };
	l1.light_diffuse = { 0.6,0.6,0.0,1 };
	l1.light_specular = { 0.7,0.7,0.0,1 };
	l1.light_attenuation = { 0,0,0.03 };
	l1.spot_direction = { 0, 0, -1 };
	l1.spot_cutoff = std::cos(glm::radians(40.0f));
	l1.spot_exp = 50;
	lights.push_back(l1);
}

void Update(void) {
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(shaderwrap->ShaderProgram);
	shaderwrap->setUniformmat4("transform_viewProjection", false, Matrix_projection);
	shaderwrap->setUniformfv3("transform_viewPosition", cam.get_eye());
	shaderwrap->setUniform1s("material_texture", 0);
	shaderwrap->setUniform1i("lcount", lights.size());
	for (int i = 0; i < lights.size(); ++i) {
		std::string prefix = "l[" + std::to_string(i) + "].";
		shaderwrap->setUniformfv4(prefix+"light_position", lights[i].light_position);
		shaderwrap->setUniformfv4(prefix+"light_ambient", lights[i].light_ambient);
		shaderwrap->setUniformfv4(prefix + "light_diffuse", lights[i].light_diffuse);
		shaderwrap->setUniformfv4(prefix + "light_specular", lights[i].light_specular);
		shaderwrap->setUniformfv3(prefix + "light_attenuation", lights[i].light_attenuation);
		shaderwrap->setUniformfv3(prefix + "spot_direction", lights[i].spot_direction);
		shaderwrap->setUniform1f(prefix + "spot_cutoff", lights[i].spot_cutoff);
		shaderwrap->setUniform1f(prefix + "spot_exp", lights[i].spot_exp);
	}


	for (int i = 0; i < scene.size(); ++i) {
		shaderwrap->setUniformfv4("material_ambient", scene[i]->material_ambient);
		shaderwrap->setUniformfv4("material_diffuse", scene[i]->material_diffuse);
		shaderwrap->setUniformfv4("material_specular", scene[i]->material_specular);
		shaderwrap->setUniformfv4("material_emission", scene[i]->material_emission);
		shaderwrap->setUniform1f("material_shininess", scene[i]->material_shininess);
		
		shaderwrap->setUniform1b("use_texture", scene[i]->use_texture);	
		for (size_t j = 0; j < instancing_trasforms[i].size();j++)
		{
			auto trsf = instancing_trasforms[i][j] * scene[i]->object_transformation;
			glm::mat3 transform_normal = glm::inverseTranspose(glm::mat3(trsf));
			shaderwrap->setUniformmat4("transform_model", false, trsf );
			shaderwrap->setUniformmat3("transform_normal", false, transform_normal);
			scene[i]->drawObject();
		}
		
	}

	glUseProgram(0);

	glFlush();
	glutSwapBuffers();

}

void processPassiveMouseMotion(int x, int y) {
	static int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
	static int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	 float camx = -1.0f * (x - centerX)/w;
	 float camy = -1.0f * (y - centerY)/h;

	if (camx || camy) {
		
		
		if (x != centerX || y != centerY) {
			glutWarpPointer(centerX, centerY);
			cam.MoveCam(0,0, camx, camy, 0);
			Set_cam();
			glutPostRedisplay();
		}
	}
	

	
}


void keyboard(unsigned char key, int x, int y)	
{

	float forward = 0;
	float right = 0;
	float tilt = 0;
	switch (key)
	{
	case 'W':
		forward += 2;
		break;
	case 'w':
		forward += 1;
		break;
	case 's':

		forward -= 1;
		break;
	case 'a':
		right += 1;
		break;
	case 'd':
		right -= 1;
		break;
	case 'q':
		tilt += 0.05;
		break;
	case 'e':
		tilt -= 0.05;
		break;
	default:
		break;
	}
	cam.MoveCam(forward, right, 0, 0, tilt);
	Set_cam();
	glutPostRedisplay();
}


void specialKey(int key, int x, int y) {
	
	switch (key)
	{
	case GLUT_KEY_RIGHT:
		lights[0].spot_exp += 1;
		break;
	case GLUT_KEY_LEFT:
		
		lights[0].spot_exp -= 1;
		if (lights[0].spot_exp < 0)
			lights[0].spot_exp = 0;
		break;
	case GLUT_KEY_UP:
		angle +=1;
		if (angle >90) {
			angle = 90;
			lights[0].spot_cutoff = std::cos(glm::radians(90.0f));
		}
		else
			lights[0].spot_cutoff = std::cos(glm::radians(angle));
		break;
	case GLUT_KEY_DOWN:
		angle -= 1;
		if (angle <= 0) {
			angle = 0;
			lights[0].spot_cutoff = std::cos(glm::radians(180.0f));
		}
		else
			lights[0].spot_cutoff = std::cos(glm::radians(angle));
		break;
		break;
	case GLUT_KEY_F1:
		//animate = !animate;
	default:
		break;
	}

	
	glutPostRedisplay();
}

void animate_tree() {
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	if (!animate)
		return;

	glm::mat4 anim = glm::translate(glm::vec3{ 55, 16, 0 });
	anim *= glm::rotate(glm::radians((float)deltaTime / 100), glm::vec3{ 0, 0, 1 });
	anim *= glm::translate(glm::vec3{ -55, -16, 0 });
	scene[1]->object_transformation = anim * scene[1]->object_transformation;
	//glutPostRedisplay();
}

void load_scene() {
	

	// scene
	// 0 - ground
	// 1 - snow base
	// 2 - tree
	scene.push_back(GLobject::draw_ground(-80, 80, -80, 80, 50, 50));             
	scene[0]->material_ambient = { 0.2, 0.2, 0.2, 1 };
	scene[0]->material_diffuse = { 0.7, 0.7, 0.7, 1 };
	scene[0]->material_specular = { 0.7, 0.7, 0.7, 1 };
	scene[0]->use_texture = true;
	scene[0]->texture = SOIL_load_OGL_texture("resourses/snow.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y);
	
#ifndef _DEBUG
	scene.push_back(new GLobject("resourses/snow base.obj",""));
	scene[1]->object_transformation *= glm::translate(glm::vec3{ 5,5,0.8f }) *glm::rotate(glm::radians(90.0f), glm::vec3{ 0,0,1 }) * glm::rotate(glm::radians(90.0f), glm::vec3{ 1,0,0 })*glm::scale(glm::vec3{ 0.3,0.3,0.3 });
	scene.push_back(new GLobject("resourses/lowpolytree.obj", "", glm::vec3{0,1,0}));
	scene[2]->object_transformation *= glm::translate(glm::vec3{ 0,0,0.3f }) * glm::rotate(glm::radians(90.0f), glm::vec3{ 1,0,0 });
	scene.push_back(new GLobject("resourses/fireplace.obj", "resourses/fireplaceF.png"));
	scene[3]->object_transformation *= glm::translate(glm::vec3{ 0,0,-0.01 }) * glm::rotate(glm::radians(90.0f), glm::vec3{ 1,0,0 })* glm::scale(glm::vec3{ 0.1,0.1,0.1});
	scene[3]->material_emission = glm::vec4{ 0.5f,0.5f,0.5,1};
	scene.push_back(new GLobject("resourses/dome_tent.obj", "resourses/tent2.tga"));
	scene[4]->object_transformation*= glm::translate(glm::vec3{ 0,4.5f,0 }) *glm::rotate(glm::radians(90.0f), glm::vec3{ 1,0,0 })* glm::scale(glm::vec3{ 0.02,0.02,0.02 });
#endif
	for (size_t i = 0; i < scene.size(); i++)
	{
		scene[i]->BindAttributesToShader(*shaderwrap);
	}
}

void make_forest(int sz,float rscene) {
	std::vector<glm::mat4> forest;
	forest.reserve(sz);
	rscene *= rscene;
	for (size_t i = 0; i < sz; i++)
	{	
		float x = rand() % 160 - 80;
		float y = rand() % 160 - 80;
		float scl = 0.8 + 0.6 * (rand() % 1000) / 1000.0;
		while (x*x + y * y < rscene) {
			x = rand() % 160 - 80;
			y = rand() % 160 - 80;

		}
		forest.push_back(glm::translate(glm::vec3{ x,y,0 })* glm::scale(glm::vec3(1,1,scl)));
	}
	instancing_trasforms[2] = forest;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	w = 800;
	h = 600;
	glutCreateWindow("Happy New Year!");
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(Update);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);
	glutPassiveMotionFunc(processPassiveMouseMotion);
	GLenum err = glewInit();
	if (GLEW_OK != err)
		std::cout << glewGetErrorString(err) << std::endl;
	Init();

	make_forest(2000,10);
	shaderwrap = new GLShader();
	LoadShaders();
	shaderwrap->linkProgram(VertShaderPhong, FragShaderPhong);
	
	

	load_scene();

	
	
	shaderwrap->checkOpenGLerror();
	set_light();
	glutSetCursor(GLUT_CURSOR_NONE);
	glutMainLoop();

	return 0;         
}