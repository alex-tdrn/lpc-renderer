#pragma once
#include "AutoName.h"
#include "glm/glm.hpp"

class Camera : public AutoName<Camera>
{
private:
	glm::vec3 translation{-2.2f, 1.65f, 2.2f};
	float yaw = -45.0f;
	float pitch = -30.0f;
	float fov = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

private:
	glm::mat4 getRotationMatrix() const;


public:
	std::string getNamePrefix() const override;
	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	void move(glm::vec3 amount);
	void rotate(glm::vec2 amount);
	void drawUI();
};