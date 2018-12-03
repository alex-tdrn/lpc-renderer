#include "Renderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "imgui.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "PointCloud.h"

void Renderer::render(Scene* scene) const
{
	if(!scene)
		return;
	glm::vec3 backgroundColor = scene->getBackgroundColor();
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
	if(!scene->getPointCloud())
		return;

	if(activeShader == ShaderManager::pcBarebones())
		glPointSize(pointSize);
	else
		glPointSize(1.0f);

	activeShader->use();

	glm::mat4 m = scene->getModelMatrix();
	activeShader->set("model", m);
	glm::mat4 v = scene->getCamera().getViewMatrix();
	activeShader->set("view", v);
	glm::mat4 p = scene->getCamera().getProjectionMatrix();
	activeShader->set("projection", p);

	activeShader->set("diffuseColor", scene->getDiffuseColor());
	if(activeShader == ShaderManager::pcLit())
	{
		activeShader->set("specularColor", scene->getSpecularColor());
		activeShader->set("shininess", scene->getShininess());
		activeShader->set("ambientStrength", scene->getAmbientStrength());
		activeShader->set("light.color", scene->getLightColor());
		activeShader->set("light.direction", scene->getLightDirection());

	}
	PointCloud const* cloud = scene->getPointCloud();
	if(decimation)
		cloud = cloud->decimated(maxVertices);
	if(frustumCulling)
		cloud = cloud->culled(p * v * m);
	pointCloudRepresentation.updateAndUse(cloud);
}

std::string Renderer::getNamePrefix() const
{
	return "Renderer";
}

void Renderer::drawUI()
{
	ImGui::PushID(this);
	
	ImGui::Checkbox("Decimation", &decimation);
	if(decimation)
	{
		ImGui::InputInt("###InputMaxVertices", &maxVertices);
		if(maxVertices < 1)
			maxVertices = 1;
	}
	ImGui::Checkbox("Frustum Culling", &frustumCulling);

	ImGui::Text("Rendering Method");
	ImGui::SameLine();
	if(ImGui::RadioButton("Barebones", activeShader == ShaderManager::pcBarebones()))
		activeShader = ShaderManager::pcBarebones();
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit", activeShader == ShaderManager::pcLit()))
		activeShader = ShaderManager::pcLit();
	if(activeShader == ShaderManager::pcBarebones())
		ImGui::SliderFloat("Point Size", &pointSize, 1.0f, 8.0f);

	ImGui::PopID();
}