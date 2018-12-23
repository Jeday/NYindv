#include "Camera.h"






void Camera::bake_transform()
{
	transofrm = glm::mat4{
	   glm::vec4(right.x,            up.x,            forward.x,       0),
	   glm::vec4(right.y,            up.y,            forward.y,       0),
	   glm::vec4(right.z,            up.z,            forward.z,       0),
	   glm::vec4(-glm::dot(right, pos), -glm::dot(up, pos), -glm::dot(forward, pos),  1)
	};
}

Camera::Camera(glm::vec3 eye, glm::vec3 target, glm::vec3 _up)
{
	pos = eye;
	forward = glm::normalize(eye - target);
	right = glm::normalize(glm::cross(_up, forward));
	up = glm::cross(forward, right);
	transofrm = glm::mat4(1.0);
	bake_transform();
}




void Camera::MoveCam(float fd,float rd, float anglex, float angley, float tilt)
{
	glm::mat3 rot;
	if (tilt != 0) {
		 rot = glm::mat3(glm::rotate(tilt, forward)); // roll 
		up = rot * up;
		right = rot * right;
	}
	if (anglex != 0) {
		rot = glm::mat3(glm::rotate(anglex, up)); // yawn
		forward = rot * forward;
		right = rot * right;
	}

	if (angley != 0) {
		rot = glm::mat3(glm::rotate(angley, right)); //pitch
		forward = rot * forward;
		up = rot * up;
	}
	
	if (fd != 0) {
		pos -= forward * fd;
	}
	if (rd != 0) {
		pos -= right * rd;
	}

	bake_transform();
	
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::mat4(transofrm);
}

void Camera::ReposeCam(glm::vec3 e, glm::vec3 target, glm::vec3 _up)
{
	pos = e;
	forward = glm::normalize(e - target);
	right = glm::normalize(glm::cross(_up, forward));
	up = glm::cross(forward, right);
	bake_transform();

}

glm::vec3 Camera::get_eye() {
	return pos;
}

Camera::~Camera()
{
}
