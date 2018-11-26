#include "Camera.h"
#include "OSWindow.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "imgui.h"

#include <algorithm>

glm::mat4 Camera::getRotationMatrix() const
{
	return glm::eulerAngleYX(glm::radians(yaw), glm::radians(pitch));
}

std::string Camera::getNamePrefix() const
{
	return "Camera";
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::inverse(glm::translate(glm::mat4{1.0f}, translation) * getRotationMatrix());
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::perspective(glm::radians(fov), OSWindow::getAspectRatio(), nearPlane, farPlane);
}

void Camera::move(glm::vec3 amount)
{
	amount.z *= -1;
	translation += glm::mat3(getRotationMatrix()) * amount;
}

void Camera::rotate(glm::vec2 amount)
{
	yaw += amount.y;
	yaw -= int(yaw / 360.0f) * 360.0f;
	float const pitchLimit = 85.0f;
	pitch = std::clamp(pitch + amount.x, -pitchLimit, +pitchLimit);
}

void Camera::drawUI()
{
	ImGui::AlignTextToFramePadding();
	ImGui::DragFloat("FOV", &fov, 0.1f);
	ImGui::DragFloat("Near Plane", &nearPlane, 0.01f);
	ImGui::DragFloat("Far Plane", &farPlane, 0.1f);
	ImGui::Text("Yaw %.2f", yaw);
	ImGui::Text("Pitch %.2f", pitch);
	ImGui::Text("Translation %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
}