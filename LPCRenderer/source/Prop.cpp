#include "Prop.h"
#include "Mesh.h"
#include "MeshManager.h"
#include "Scene.h"

#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

Prop::Prop(Mesh* mesh, glm::mat4 transform)
	:mesh(mesh), transform(transform)
{

}

std::string Prop::getNamePrefix() const
{
	return "Prop";
}

bool Prop::enabled() const
{
	return _enabled;
}

void Prop::enable()
{
	_enabled = true;
}

void Prop::disable()
{
	_enabled = false;
}

bool Prop::highlighted() const
{
	return _highlighted;
}

void Prop::setHighlighted(bool flag)
{
	_highlighted = flag;
}

glm::mat4 Prop::getModelMatrix() const
{
	glm::mat4 globalScaling{1.0f};
	if(scene)
		globalScaling = glm::scale(globalScaling, glm::vec3{scene->getGlobalScaling()});
	return globalScaling * transform * glm::scale(glm::mat4{1.0f}, glm::vec3{scaling});
}

void Prop::setTransformation(glm::mat4 t)
{
	transform = t;
}

Mesh *Prop::getMesh() const
{
	return mesh;
}

void Prop::drawUI()
{
	ImGui::PushID(this);
	ImGui::Checkbox("Enabled", &_enabled);
	if(ImGui::BeginCombo("Mesh Used", !mesh ? "None" : mesh->getName().data()))
	{
		int id = 0;
		for(auto& mesh : MeshManager::getAll())
		{
			ImGui::PushID(id++);
			bool isSelected = this->mesh == mesh.get();
			ImGui::Selectable(mesh->getName().data(), &isSelected);
			if(isSelected)
			{
				ImGui::SetItemDefaultFocus();
				this->mesh = mesh.get();
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	ImGui::DragFloat("Scaling", &scaling, 0.1f);
	constexpr float rotateAmount = glm::radians(0.1f);
	ImGui::Text("Rotate");
	ImGui::SameLine();
	ImGui::Button("+X");
	if(ImGui::IsItemActive())
		transform = glm::rotate(transform, +rotateAmount, glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("-X");
	if(ImGui::IsItemActive())
		transform = glm::rotate(transform, -rotateAmount, glm::vec3{1.0f, 0.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("+Y");
	if(ImGui::IsItemActive())
		transform = glm::rotate(transform, +rotateAmount, glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("-Y");
	if(ImGui::IsItemActive())
		transform = glm::rotate(transform, -rotateAmount, glm::vec3{0.0f, 1.0f, 0.0f});
	ImGui::SameLine();
	ImGui::Button("+Z");
	if(ImGui::IsItemActive())
		transform = glm::rotate(transform, +rotateAmount, glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::SameLine();
	ImGui::Button("-Z");
	if(ImGui::IsItemActive())
		transform = glm::rotate(transform, -rotateAmount, glm::vec3{0.0f, 0.0f, 1.0f});
	ImGui::PopID();
}

