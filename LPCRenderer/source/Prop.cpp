#include "Prop.h"
#include "Mesh.h"
#include "MeshManager.h"

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

glm::mat4 Prop::getModelMatrix() const
{
	return transform * glm::scale(glm::mat4{1.0f}, glm::vec3{scale});
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
	ImGui::DragFloat("Scale", &scale, 0.1f);
	ImGui::PopID();
}

