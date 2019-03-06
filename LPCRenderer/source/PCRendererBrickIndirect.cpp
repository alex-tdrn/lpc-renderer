#include "PCRendererBrickIndirect.h"
#include "Shader.h"
#include "PointCloud.h"
#include "Scene.h"
#include "imgui.h"

enum class RenderMode
{
	basic,
	lit,
	litColoured
};

namespace
{
	Shader basicShader{"shaders/pcBrickIndirect.vert", "shaders/pcBrickIndirect.frag"};
	Shader litShader{"shaders/pcBrickIndirectLit.vert", "shaders/pcLitDisk.frag", "shaders/pcLitDisk.geom"};
	Shader litColouredShader{"shaders/pcBrickIndirectLit.vert", "shaders/pcBrickIndirect.frag"};
	RenderMode renderMode = RenderMode::basic;
	
	bool backFaceCulling = true;
	int pointSize = 2;
	float diskRadius = 0.0005f;
	int positionSize = 16;
}

PCRendererBrickIndirect::PCRendererBrickIndirect()
	:PCRenderer(&basicShader)
{
}

bool PCRendererBrickIndirect::needNormals() const
{
	return renderMode != RenderMode::basic;
}

bool PCRendererBrickIndirect::needColours() const
{
	return renderMode == RenderMode::litColoured;
}

void PCRendererBrickIndirect::updatePositions16()
{
	static std::vector<DrawCommand> indirectDraws;
	static std::vector<std::uint16_t> compressedPositions;

	int brickIndex = -1;
	for(auto brick : cloud->getAllBricks())
	{
		brickIndex++;
		if(brick.positions.empty())
			continue;
		DrawCommand brickDrawCommand{};
		brickDrawCommand.count = brick.positions.size();
		brickDrawCommand.first = compressedPositions.size();
		brickDrawCommand.baseInstance = brickIndex;
		indirectDraws.push_back(std::move(brickDrawCommand));
		for(auto const& position : brick.positions)
			compressedPositions.push_back(packPosition32(position));
	}
	indirectDrawCount = indirectDraws.size();

	bindVAO();
	DrawBuffer.write({{(std::byte const*)indirectDraws.data(), indirectDraws.size() * sizeof(indirectDraws.front())}});
	DrawBuffer.bind();
	VBOPositions.write({{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front())}});
	VBOPositions.bind();
	glEnableVertexAttribArray(0);//Compressed Positions
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, 0, (void*)(0));

	compressedPositions.clear();
	indirectDraws.clear();
}

void PCRendererBrickIndirect::updatePositions32()
{
	static std::vector<DrawCommand> indirectDraws;
	static std::vector<std::uint32_t> compressedPositions;

	int brickIndex = -1;
	for(auto brick : cloud->getAllBricks())
	{
		brickIndex++;
		if(brick.positions.empty())
			continue;
		DrawCommand brickDrawCommand{};
		brickDrawCommand.count = brick.positions.size();
		brickDrawCommand.first = compressedPositions.size();
		brickDrawCommand.baseInstance = brickIndex;
		indirectDraws.push_back(std::move(brickDrawCommand));
		for(auto const& position : brick.positions)
			compressedPositions.push_back(packPosition1024(position));
	}
	indirectDrawCount = indirectDraws.size();

	bindVAO();
	DrawBuffer.write({{(std::byte const*)indirectDraws.data(), indirectDraws.size() * sizeof(indirectDraws.front())}});
	DrawBuffer.bind();
	VBOPositions.write({{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front())}});
	VBOPositions.bind();
	glEnableVertexAttribArray(0);//Compressed Positions
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)(0));

	compressedPositions.clear();
	indirectDraws.clear();
}

void PCRendererBrickIndirect::update()
{
	if(positionSize == 16)
		updatePositions16();
	else
		updatePositions32();
	mainShader->use();
	mainShader->set("positionSize", positionSize);
	
	if(needNormals())
	{
		static std::vector<glm::vec3> normals;

		for(auto const& brick : cloud->getAllBricks())
		{
			if(brick.positions.empty())
				continue;
			for(glm::vec3 normal : brick.normals)
				normals.push_back(normal);
		}

		VBONormals.write({{(std::byte const*)normals.data(), normals.size() * sizeof(normals.front())}});
		VBONormals.bind();
		
		glEnableVertexAttribArray(1);//Normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));

		normals.clear();
	}
	else
	{
		VBONormals.free();
		glDisableVertexAttribArray(1);
	}
}

void PCRendererBrickIndirect::render(Scene const* scene)
{
	PCRenderer::render(scene);

	mainShader->set("cloudOrigin", cloud->getBounds().first);
	mainShader->set("brickSize", cloud->getBrickSize());
	mainShader->set("subdivisions", glm::uvec3(cloud->getSubdivisions()));

	if(renderMode == RenderMode::lit)
	{
		mainShader->set("diskRadius", diskRadius);
		mainShader->set("backFaceCulling", backFaceCulling);
		mainShader->set("specularColor", scene->getSpecularColor());
		mainShader->set("shininess", scene->getShininess());
		mainShader->set("ambientStrength", scene->getAmbientStrength());
		mainShader->set("ambientColor", scene->getBackgroundColor());
		mainShader->set("light.color", scene->getLightColor());
		mainShader->set("light.direction", glm::vec3(scene->getCamera().getViewMatrix() * glm::vec4(scene->getLightDirection(), 0.0f)));
	}
	glPointSize(pointSize);

	bindVAO();
	DrawBuffer.bind();
	glMultiDrawArraysIndirect(GL_POINTS, nullptr, indirectDrawCount, 0);

}

void PCRendererBrickIndirect::drawUI()
{
	PCRenderer::drawUI();
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);
	ImGui::Text("Position Size");
	if(ImGui::RadioButton("32", positionSize == 32))
	{
		positionSize = 32;
		update();
		cloud->setBrickPrecision(1024);
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("16", positionSize == 16))
	{
		positionSize = 16;
		update();
		cloud->setBrickPrecision(32);
	}

	ImGui::Text("Render Mode");

	if(ImGui::RadioButton("Basic", renderMode == RenderMode::basic))
	{
		renderMode = RenderMode::basic;
		mainShader = &basicShader;
		update();
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit", renderMode == RenderMode::lit))
	{
		renderMode = RenderMode::lit;
		mainShader = &litShader;
		update();
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit Coloured", renderMode == RenderMode::litColoured))
	{
		renderMode = RenderMode::litColoured;
		mainShader = &litColouredShader;
		update();
	}
	
	if(needNormals() && cloud && !cloud->hasNormals())
	{
		ImGui::Text("Current render mode needs normals");
		ImGui::Text("but none are present in the dataset!");
	}

	if(needColours() && false)
	{
		ImGui::Text("Current render mode needs colours");
		ImGui::Text("but none are present in the dataset!");
	}

	if(renderMode != RenderMode::basic)
	{
		ImGui::DragFloat("Disk Radius", &diskRadius, 0.00001f, 0.00001f, 0.005f, "%.5f");
		ImGui::Checkbox("Backface Culling", &backFaceCulling);
	}
}

void PCRendererBrickIndirect::reloadShaders()
{
	basicShader.reload();
	litShader.reload();
	litColouredShader.reload();
}
