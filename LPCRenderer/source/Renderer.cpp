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
	auto activeShader = [&](){
		switch(renderMode)
		{
			case Renderer::RenderMode::debugNormals:
				return ShaderManager::pcDebugNormals();
			case Renderer::RenderMode::lit:
				return ShaderManager::pcLit();
			case Renderer::RenderMode::litDisk:
				return ShaderManager::pcLitDisk();
			default:
				if(compressPointClouds)
					return ShaderManager::pcBarebonesCompressed();
				else
					return ShaderManager::pcBarebones();
		}
	}();

	if(renderMode == RenderMode::barebones || renderMode == RenderMode::lit)
		glPointSize(pointSize);
	else
		glPointSize(1.0f);

	glm::mat4 m = scene->getModelMatrix();
	glm::mat4 v = scene->getCamera().getViewMatrix();
	glm::mat4 p = scene->getCamera().getProjectionMatrix();
	currentPointCloud = scene->getPointCloud();
	/*if(decimation)
		currentPointCloud = currentPointCloud->decimated(decimationMaxVertices);*/

	activeShader->use();
	activeShader->set("model", m);
	activeShader->set("view", v);
	activeShader->set("projection", p);

	activeShader->set("diffuseColor", scene->getDiffuseColor());
	if(renderMode == RenderMode::lit || renderMode == RenderMode::litDisk)
	{
		activeShader->set("backFaceCulling", backFaceCulling);
		activeShader->set("specularColor", scene->getSpecularColor());
		activeShader->set("shininess", scene->getShininess());
		activeShader->set("ambientStrength", scene->getAmbientStrength());
		activeShader->set("ambientColor", backgroundColor);
		activeShader->set("light.color", scene->getLightColor());
		glm::vec3 viewSpaceLightDirection = v * glm::vec4(scene->getLightDirection(), 0.0f);
		activeShader->set("light.direction", viewSpaceLightDirection);
		if(renderMode == RenderMode::litDisk)
		{
			activeShader->set("diskRadius", diskRadius);
		}
	}
	else if(renderMode == RenderMode::debugNormals)
	{
		glLineWidth(debugNormalsLineThickness);
		activeShader->set("lineLength", debugNormalsLineLength);
	}

	currentPointCloudBuffer %= pointCloudBufffers.size();

	pointCloudBufffers[currentPointCloudBuffer]
		.update(shrinkBuffersToFit, useNormalsIfAvailable, compressPointClouds, currentPointCloud);
	currentPointCloudBuffer++;
}

std::string Renderer::getNamePrefix() const
{
	return "Renderer";
}

void Renderer::drawUI()
{
	ImGui::PushID(this);
	ImGui::Checkbox("Shrink Buffers To Fit", &shrinkBuffersToFit);
	nBuffers = pointCloudBufffers.size();
	ImGui::InputInt("# of Buffers", &nBuffers, 1);
	if(nBuffers <= 0)
		nBuffers = 1;
	if(nBuffers != pointCloudBufffers.size())
	{
		pointCloudBufffers.resize(nBuffers);
	}
	if(currentPointCloud)
	{
		ImGui::Text("Rendering %lu Vertices", renderedVertices);
		ImGui::SameLine();
		if(currentPointCloud->hasNormals() && useNormalsIfAvailable)
			ImGui::Text("With Normals");
		else
			ImGui::Text("Without Normals");
	}
	ImGui::Checkbox("Decimation", &decimation);
	if(decimation)
	{
		ImGui::SameLine();
		ImGui::Text("Max Vertices");
		ImGui::DragInt("###InputMaxVertices", &decimationMaxVertices, 10'000, 1, std::numeric_limits<int>::max());
		if(decimationMaxVertices < 1)
			decimationMaxVertices = 1;
	}

	ImGui::Separator();
	ImGui::Checkbox("Compress Pointclouds", &compressPointClouds);
	ImGui::Text("Rendering Mode");
	if(ImGui::RadioButton("Barebones", renderMode == RenderMode::barebones))
	{
		renderMode = RenderMode::barebones;
		useNormalsIfAvailable = false;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Debug Normals", renderMode == RenderMode::debugNormals))
	{
		renderMode = RenderMode::debugNormals;
		useNormalsIfAvailable = true;
	}
	if(ImGui::RadioButton("Lit", renderMode == RenderMode::lit))
	{
		renderMode = RenderMode::lit;
		useNormalsIfAvailable = true;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit Disk", renderMode == RenderMode::litDisk))
	{
		renderMode = RenderMode::litDisk;
		useNormalsIfAvailable = true;
	}
	if(renderMode == RenderMode::barebones || renderMode == RenderMode::lit)
		ImGui::SliderInt("Point Size", &pointSize, 1, 16);

	if(renderMode == RenderMode::lit || renderMode == RenderMode::litDisk)
		ImGui::Checkbox("Backface Culling", &backFaceCulling);

	if(renderMode == RenderMode::litDisk)
		ImGui::DragFloat("Disk Radius", &diskRadius, 0.00001f, 0.00001f, 0.005f, "%.5f");

	if(renderMode == RenderMode::debugNormals)
	{
		ImGui::DragFloat("Line Length", &debugNormalsLineLength, 0.0001f, 0.0001f, 0.01f, "%.4f");
		ImGui::SliderInt("Line Thickness", &debugNormalsLineThickness, 1, 16);
	}

	ImGui::PopID();
}
