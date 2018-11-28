#include "Renderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "imgui.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "Mesh.h"

void Renderer::render(Scene* scene) const
{
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
	if(!scene)
		return;
	glPointSize(pointSize);
	ShaderManager::debug()->use();
	ShaderManager::debug()->set("color", meshColor);
	ShaderManager::debug()->set("view", scene->getCamera().getViewMatrix());
	ShaderManager::debug()->set("projection", scene->getCamera().getProjectionMatrix());

	for(auto const& prop : scene->getProps())
	{
		if(!prop.enabled())
			continue;
		if(highlightProps)
		{
			if(prop.highlighted())
				ShaderManager::debug()->set("color", highlightColor);
			else
				ShaderManager::debug()->set("color", meshColor);
		}
		ShaderManager::debug()->set("model", prop.getModelMatrix());
		if(frustumCulling)
		{
			prop.getMesh()->setCullMatrix(scene->getCamera().getProjectionMatrix() * scene->getCamera().getViewMatrix() * prop.getModelMatrix());
		}
		prop.getMesh()->getRepresentation().use();
	}
	if(frustumCulling)
		frustumCulling = false;
}

std::string Renderer::getNamePrefix() const
{
	return "Debug Renderer";
}

void Renderer::drawUI()
{
	ImGui::PushID(this);
	ImGui::ColorEdit3("Background", &backgroundColor.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::ColorEdit3("Mesh Color", &meshColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::Checkbox("Highlight Props", &highlightProps);
	if(highlightProps)
		ImGui::ColorEdit3("Highlight Color", &highlightColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::SliderFloat("Point Size", &pointSize, 1.0f, 16.0f);
	if(ImGui::Button("Cull Points"))
		frustumCulling = true;
	ImGui::PopID();
}