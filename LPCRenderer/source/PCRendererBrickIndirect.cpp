#include "PCRendererBrickIndirect.h"
#include "Shader.h"
#include "PointCloud.h"
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
	int pointSize = 2;
	int positionSize = 16;
}

PCRendererBrickIndirect::PCRendererBrickIndirect()
	:PCRenderer(&basicShader)
{
}

void PCRendererBrickIndirect::updatePositions16()
{
	static std::vector<DrawCommand> indirectDraws;
	indirectDraws.clear();
	static std::vector<std::uint16_t> compressedPositions;
	compressedPositions.clear();
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
	VBO.write({{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front())}});
	VBO.bind();
	glEnableVertexAttribArray(0);//Compressed Positions
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, 0, (void*)(0));
}

void PCRendererBrickIndirect::updatePositions32()
{
	static std::vector<DrawCommand> indirectDraws;
	indirectDraws.clear();
	static std::vector<std::uint32_t> compressedPositions;
	compressedPositions.clear();
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
	VBO.write({{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front())}});
	VBO.bind();
	glEnableVertexAttribArray(0);//Compressed Positions
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)(0));
}

void PCRendererBrickIndirect::update()
{
	if(positionSize == 16)
		updatePositions16();
	else
		updatePositions32();
	basicShader.use();
	basicShader.set("positionSize", positionSize);
	
}

void PCRendererBrickIndirect::render(Scene const* scene)
{
	PCRenderer::render(scene);

	mainShader->set("cloudOrigin", cloud->getBounds().first);
	mainShader->set("brickSize", cloud->getBrickSize());
	mainShader->set("subdivisions", glm::uvec3(cloud->getSubdivisions()));

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
}

void PCRendererBrickIndirect::reloadShaders()
{
	basicShader.reload();
}
