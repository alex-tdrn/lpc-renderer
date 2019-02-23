#include "PCRendererUncompressed.h"
#include "Shader.h"
#include "PointCloud.h"
#include "Scene.h"
#include "Camera.h"
#include "imgui.h"

#include <memory>

enum class RenderMode
{
	basic,
	litPoint,
	litDisk,
	debugNormals
};

namespace
{
	Shader basicShader{"shaders/pcBasic.vert", "shaders/pcBasic.frag"};
	Shader litPointShader{"shaders/pcLit.vert", "shaders/pcLit.frag"};
	Shader litDiskShader{"shaders/pcLitDisk.vert", "shaders/pcLitDisk.frag", "shaders/pcLitDisk.geom"};
	Shader debugNormalsShader{"shaders/pcDebugNormals.vert", "shaders/pcDebugNormals.frag", "shaders/pcDebugNormals.geom"};

	RenderMode renderMode = RenderMode::basic;
	bool backFaceCulling = true;
	int pointSize = 2;
	float diskRadius = 0.0005f;
	float debugNormalsLineLength = 0.001f;
	int debugNormalsLineThickness = 2;
}

PCRendererUncompressed::PCRendererUncompressed()
	: PCRenderer(&basicShader)
{

}


PCRendererUncompressed::~PCRendererUncompressed()
{
}

bool PCRendererUncompressed::needNormals() const
{
	return renderMode != RenderMode::basic;
}

void PCRendererUncompressed::update()
{
	if(needNormals() && !cloud->hasNormals())
		return;
	vertexCount = 0;
	for(auto const& brick : cloud->getAllBricks())
		vertexCount += brick.positions.size();

	static std::vector<glm::vec3> positions;
	positions.reserve(vertexCount);

	std::vector<std::pair<std::byte const*, std::size_t>> vertexData;
	std::byte const* bufferStartAddress = (std::byte const*)(positions.data());
	std::size_t bufferOffset = 0;
	for(auto const& brick : cloud->getAllBricks())
	{
		if(brick.positions.empty())
			continue;
		std::size_t const elementSize = 3 * sizeof(float);
		std::size_t const brickSize = brick.positions.size() * elementSize;

		vertexData.emplace_back(bufferStartAddress + bufferOffset, brickSize);

		for(glm::vec3 position : brick.positions)
			positions.push_back(cloud->convertToWorldPosition(brick.indices, position));

		bufferOffset += brickSize;
	}

	bindVAO();
	VBO.write(true, std::move(vertexData));
	VBO.bind();
	glEnableVertexAttribArray(0);//Positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));

	positions.clear();
}

void PCRendererUncompressed::render(Scene const* scene)
{
	PCRenderer::render(scene);

	if(needNormals() && !cloud->hasNormals())
		return;
	glPointSize(pointSize);


	switch(renderMode)
	{
		case RenderMode::litDisk:
			mainShader->set("diskRadius", diskRadius);
			[[fallthrough]];
		case RenderMode::litPoint:
			mainShader->set("backFaceCulling", backFaceCulling);
			mainShader->set("specularColor", scene->getSpecularColor());
			mainShader->set("shininess", scene->getShininess());
			mainShader->set("ambientStrength", scene->getAmbientStrength());
			mainShader->set("ambientColor", scene->getBackgroundColor());
			mainShader->set("light.color", scene->getLightColor());
			mainShader->set("light.direction", glm::vec3(scene->getCamera().getViewMatrix() * glm::vec4(scene->getLightDirection(), 0.0f)));
			break;
		case RenderMode::debugNormals:
			glLineWidth(debugNormalsLineThickness);
			mainShader->set("lineLength", debugNormalsLineLength);
			break;
	}

	bindVAO();
	glDrawArrays(GL_POINTS, 0, vertexCount);
}

void PCRendererUncompressed::drawUI()
{
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);

	ImGui::Text("Render Mode");

	if(ImGui::RadioButton("Basic", renderMode == RenderMode::basic))
	{
		renderMode = RenderMode::basic;
		mainShader = &basicShader;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit SS Point", renderMode == RenderMode::litPoint))
	{
		renderMode = RenderMode::litPoint;
		mainShader = &litPointShader;
	}

	if(ImGui::RadioButton("Lit Disk", renderMode == RenderMode::litDisk))
	{
		renderMode = RenderMode::litDisk;
		mainShader = &litDiskShader;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Debug Normals", renderMode == RenderMode::debugNormals))
	{
		renderMode = RenderMode::debugNormals;
		mainShader = &debugNormalsShader;
	}
	if(needNormals() && !cloud->hasNormals())
		ImGui::Text("Current render mode needs normals, but none are present in the dataset!");


	switch(renderMode)
	{
		case RenderMode::litDisk:
			ImGui::DragFloat("Disk Radius", &diskRadius, 0.00001f, 0.00001f, 0.005f, "%.5f");
			[[fallthrough]];
		case RenderMode::litPoint:
			ImGui::Checkbox("Backface Culling", &backFaceCulling);
			break;
		case RenderMode::debugNormals:
			ImGui::DragFloat("Line Length", &debugNormalsLineLength, 0.0001f, 0.0001f, 0.01f, "%.4f");
			ImGui::SliderInt("Line Thickness", &debugNormalsLineThickness, 1, 16);
			break;
	}
}
