#include "Scene.h"
#include "PCManager.h"

#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include <algorithm>

Scene::Scene(PointCloud* cloud)
	:cloud(cloud)
{
	auto bounds = cloud->getBounds();
	glm::vec3 center = (bounds.first + bounds.second) / 2.0f;
	bounds.first -= center;
	float scale = -1.0f;
	for(int i = 0; i < 3; i++)
		scale = std::max(scale, -bounds.first[i]);
	glm::mat4 t = glm::translate(glm::mat4{1.0f}, -center);
	glm::mat4 s = glm::scale(glm::mat4{1.0f}, 1.0f / glm::vec3{scale});
	modelMatrix = s * t;
}

std::string Scene::getNamePrefix() const
{
	return "Scene";
}

PointCloud const* Scene::getPointCloud() const
{
	return cloud;
}

glm::mat4 Scene::getModelMatrix() const
{
	return glm::scale(modelMatrix, glm::vec3{scaling});
}

Camera& Scene::getCamera()
{
	return camera;
}

glm::vec3 Scene::getBackgroundColor() const
{
	return backgroundColor;
}

glm::vec3 Scene::getLightColor() const
{
	return lightColor;
}

glm::vec3 Scene::getLightDirection() const
{
	return lightDirection;
}

glm::vec3 Scene::getDiffuseColor() const
{
	return diffuseColor;
}

glm::vec3 Scene::getSpecularColor() const
{
	return specularColor;
}

float Scene::getShininess() const
{
	return shininess;
}

float Scene::getAmbientStrength() const
{
	return ambientStrength;
}

void Scene::drawUI()
{
	ImGui::PushID(this);
	ImGui::AlignTextToFramePadding();
	ImGui::ColorEdit3("Background", &backgroundColor.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::SameLine();
	ImGui::ColorEdit3("Light Color", &lightColor.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	//ImGui::ColorEdit3("Light Direction", &backgroundColor.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::ColorEdit3("Diffuse Color", &diffuseColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::SameLine();
	ImGui::ColorEdit3("Specular Color", &specularColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::SliderFloat("Shininess", &shininess, 0.0f, 64.0f);
	ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f);

	ImGui::Text("Camera");
	camera.drawUI();

	if(ImGui::BeginCombo("Point Cloud", !cloud ? "None" : cloud->getName().data()))
	{
		int id = 0;
		for(auto& cloud : PCManager::getAll())
		{
			ImGui::PushID(id++);
			bool isSelected = this->cloud == cloud.get();
			ImGui::Selectable(cloud->getName().data(), &isSelected);
			if(isSelected)
			{
				ImGui::SetItemDefaultFocus();
				this->cloud = cloud.get();
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	ImGui::DragFloat("Scaling", &scaling, 0.1f);

	static float rotationSpeed = 0.1f;
	ImGui::Text("Rotate Continuously By %.1f Degrees/Frame", rotationSpeed);
	ImGui::DragFloat("###RotationSpeed ", &rotationSpeed, 0.01f, 0.01f, 1.0f);
	ImGui::PushID(1);
	ImGui::Button("+X");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, +glm::radians(rotationSpeed), glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("-X");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, -glm::radians(rotationSpeed), glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("+Y");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, +glm::radians(rotationSpeed), glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("-Y");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, -glm::radians(rotationSpeed), glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("+Z");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, +glm::radians(rotationSpeed), glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::SameLine();
	ImGui::Button("-Z");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, -glm::radians(rotationSpeed), glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::PopID();

	static float rotationAmount = 90.0f;
	ImGui::Text("Rotate Once By %.1f Degrees", rotationAmount);
	ImGui::DragFloat("###RotationAmount ", &rotationAmount, 1.0f, 1.0f, 180.0f);
	ImGui::PushID(2);
	if(ImGui::Button("+X"))
		modelMatrix = glm::rotate(modelMatrix, +glm::radians(rotationAmount), glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	if(ImGui::Button("-X"))
		modelMatrix = glm::rotate(modelMatrix, -glm::radians(rotationAmount), glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	if(ImGui::Button("+Y"))
		modelMatrix = glm::rotate(modelMatrix, +glm::radians(rotationAmount), glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	if(ImGui::Button("-Y"))
		modelMatrix = glm::rotate(modelMatrix, -glm::radians(rotationAmount), glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	if(ImGui::Button("+Z"))
		modelMatrix = glm::rotate(modelMatrix, +glm::radians(rotationAmount), glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::SameLine();
	if(ImGui::Button("-Z"))
		modelMatrix = glm::rotate(modelMatrix, -glm::radians(rotationAmount), glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::PopID();

	ImGui::PopID();
}
