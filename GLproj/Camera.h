#pragma 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp> 
#include<glm/trigonometric.hpp>
class Camera
{
	glm::vec3 forward; // eye - target
	glm::vec3 right; // 
	glm::vec3 up; // 
	glm::vec3 pos; // postion of the camerea, eye

	glm::mat4 transofrm;

	void bake_transform();

public:
	Camera(glm::vec3 eye, glm::vec3 target , glm::vec3  up);
	void MoveCam(float fd, float rd, float anglex, float angley, float tilt = 0);
	glm::mat4 getViewMatrix();
	void ReposeCam(glm::vec3 eye, glm::vec3 target, glm::vec3  up);
	glm::vec3 get_eye();
	~Camera();
};

