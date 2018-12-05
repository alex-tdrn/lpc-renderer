#include "Scene.h"
#include "PointCloudManager.h"

#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

Scene::Scene(PointCloud* cloud)
	:cloud(cloud)
{
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
		for(auto& cloud : PointCloudManager::getAll())
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
	static float rotateAmount = glm::radians(0.1f);
	ImGui::SliderFloat("Rotation Speed", &rotateAmount, glm::radians(0.1f), glm::radians(10.0f));
	ImGui::Text("Rotate");
	ImGui::SameLine();
	ImGui::Button("+X");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, +rotateAmount, glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("-X");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, -rotateAmount, glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("+Y");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, +rotateAmount, glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("-Y");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, -rotateAmount, glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("+Z");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, +rotateAmount, glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::SameLine();
	ImGui::Button("-Z");
	if(ImGui::IsItemActive())
		modelMatrix = glm::rotate(modelMatrix, -rotateAmount, glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::PopID();
}
