#include "PCRendererBrickGS.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "PointCloud.h"
#include "imgui.h"

namespace
{
	Shader basicShader{"shaders/pcBrickGS.vert", "shaders/pcBrickGS.frag", "shaders/pcBrickGS.geom"};
	int pointSize = 2;
}

PCRendererBrickGS::PCRendererBrickGS()
	: PCRenderer(&basicShader)
{
}

void PCRendererBrickGS::update()
{
	static std::vector<glm::u8vec3> brickIndices;
	static std::vector<std::uint32_t> bufferOffsets;
	static std::vector<std::uint32_t> bufferLengths;
	static std::vector<std::uint32_t> compressedPositions;
	brickIndices.clear();
	bufferOffsets.clear();
	bufferLengths.clear();
	bufferOffsets.push_back(0);
	compressedPositions.clear();
	brickCount = 0;
	for(auto brick : cloud->getAllBricks())
	{
		if(brick.positions.empty())
			continue;
		brickCount++;
		brickIndices.push_back(brick.indices);
		for(auto const& position : brick.positions)
			compressedPositions.push_back(glm::packUnorm4x8(glm::vec4(position, 0.0f)));

		bufferOffsets.push_back(bufferOffsets.back() + brick.positions.size());
		bufferLengths.push_back(brick.positions.size());
	}
	bufferOffsets.pop_back();
	std::size_t brickIndicesBufferSize = brickIndices.size() * sizeof(brickIndices.front());
	std::size_t bufferOffsetsBufferSize = bufferOffsets.size() * sizeof(bufferOffsets.front());

	bindVAO();
	VBO.write({
		{(std::byte const*)brickIndices.data(), brickIndicesBufferSize},
		{(std::byte const*)bufferOffsets.data(), bufferOffsetsBufferSize},
		{(std::byte const*)bufferLengths.data(), bufferLengths.size() * sizeof(bufferLengths.front())}
		});
	VBO.bind();
	glEnableVertexAttribArray(0);//Indices
	glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, 0, (void*)(0));
	glEnableVertexAttribArray(1);//Buffer Offsets
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, (void*)(brickIndicesBufferSize));
	glEnableVertexAttribArray(2);//Buffer Lengths
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (void*)(brickIndicesBufferSize + bufferOffsetsBufferSize));

	SSBO.write({{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front())}});
	SSBO.bindBase(0);
}

void PCRendererBrickGS::render(Scene const* scene)
{
	PCRenderer::render(scene);

	glPointSize(pointSize);

	mainShader->set("cloudOrigin", cloud->getBounds().first);
	mainShader->set("brickSize", cloud->getBrickSize());

	bindVAO();
	glDrawArrays(GL_POINTS, 0, brickCount);
}

void PCRendererBrickGS::drawUI()
{
	PCRenderer::drawUI();
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);
}

void PCRendererBrickGS::reloadShaders()
{
	basicShader.reload();
}
