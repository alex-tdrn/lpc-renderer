#include "DebugRenderer.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "Mesh.h"
#include "glad/glad.h"
std::string DebugRenderer::getNamePrefix() const
{
	return "Debug Renderer";
}

void DebugRenderer::render(Scene* scene) const
{
	Renderer::render();
	glPointSize(pointSize);
	ShaderManager::debug()->use();
	ShaderManager::debug()->set("color", meshColor);
	ShaderManager::debug()->set("view", scene->getCamera().getViewMatrix());
	ShaderManager::debug()->set("projection", scene->getCamera().getProjectionMatrix());

	for(auto const& prop : scene->getProps())
	{
		if(!prop.enabled())
			continue;
		ShaderManager::debug()->set("model", prop.getModelMatrix());
		prop.getMesh()->getRepresentation()->use();
	}
}

void DebugRenderer::drawUI()
{
	Renderer::drawUI();
	ImGui::ColorEdit3("Mesh Color", &meshColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
	ImGui::SliderFloat("Point Size", &pointSize, 1.0f, 16.0f);
}