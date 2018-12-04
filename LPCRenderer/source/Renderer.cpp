#include "Renderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "imgui.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "PointCloud.h"

Renderer::Renderer()
{
	if(pointCloudBufffers.empty())
		pointCloudBufffers.resize(nBuffers);
}

void Renderer::render(Scene* scene) const
{
	if(!scene)
		return;
	glm::vec3 backgroundColor = scene->getBackgroundColor();
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
	if(!scene->getPointCloud())
		return;

	glPointSize(pointSize);

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
	else if(activeShader == ShaderManager::pcDebugNormals())
	{
		activeShader->set("lineLength", debugNormalsLineLength);
	}

	currentPointCloud = scene->getPointCloud();
	if(decimation)
		currentPointCloud = currentPointCloud->decimated(maxVertices);
	if(frustumCulling)
		currentPointCloud = currentPointCloud->culled(p * v * m);
	currentPointCloudBuffer %= pointCloudBufffers.size();
	pointCloudBufffers[currentPointCloudBuffer++].updateAndUse(currentPointCloud, useNormalsIfAvailable, bufferOrphaning);
}

std::string Renderer::getNamePrefix() const
{
	return "Renderer";
}

void Renderer::drawUI()
{
	ImGui::PushID(this);
	ImGui::Checkbox("Buffer Orphaning", &bufferOrphaning);
	ImGui::SameLine();
	if(ImGui::Button("Shrink Buffers"))
		for(auto& buffer : pointCloudBufffers)
			buffer.shrink();
	nBuffers = pointCloudBufffers.size();
	ImGui::SliderInt("# of Buffers", &nBuffers, 1, 8);
	if(nBuffers != pointCloudBufffers.size())
	{
		pointCloudBufffers.resize(nBuffers);
	}
	if(currentPointCloud)
	{
		ImGui::Text("Rendering %lu Vertices", currentPointCloud->getSize());
		ImGui::SameLine();
		if(currentPointCloud->hasNormals() && useNormalsIfAvailable)
			ImGui::Text("With Normals");
		else
			ImGui::Text("Without Normals");
	}
	ImGui::Checkbox("Decimation", &decimation);
	ImGui::SameLine();
	ImGui::Checkbox("Frustum Culling", &frustumCulling);
	if(decimation)
	{
		ImGui::Text("Max Vertices");
		ImGui::SameLine();
		ImGui::DragInt("###InputMaxVertices", &maxVertices, 10'000, 1, std::numeric_limits<int>::max());
		if(maxVertices < 1)
			maxVertices = 1;
	}

	ImGui::Text("Rendering Method");
	ImGui::SameLine();
	if(ImGui::RadioButton("Barebones", activeShader == ShaderManager::pcBarebones()))
	{
		activeShader = ShaderManager::pcBarebones();
		useNormalsIfAvailable = false;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit", activeShader == ShaderManager::pcLit()))
	{
		activeShader = ShaderManager::pcLit();
		useNormalsIfAvailable = true;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Debug Normals", activeShader == ShaderManager::pcDebugNormals()))
	{
		activeShader = ShaderManager::pcDebugNormals();
		useNormalsIfAvailable = true;
	}
	ImGui::SliderFloat("Point Size", &pointSize, 1.0f, 8.0f);
	if(activeShader == ShaderManager::pcDebugNormals())
		ImGui::DragFloat("Line Length", &debugNormalsLineLength, 0.001f);
	ImGui::PopID();
}