#include "PCRendererBrickIndirect.h"
#include "Shader.h"
#include "PointCloud.h"
#include "imgui.h"

namespace
{
	Shader basicShader{"shaders/pcBrickIndirect.vert", "shaders/pcBrickIndirect.frag"};
	int pointSize = 2;
}

PCRendererBrickIndirect::PCRendererBrickIndirect()
	:PCRenderer(&basicShader)
{
}

void PCRendererBrickIndirect::update()
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
			compressedPositions.push_back(packPosition(position));
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
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);
}
