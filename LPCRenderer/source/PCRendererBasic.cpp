#include "PCRendererBasic.h"
#include "Shader.h"
#include "PointCloud.h"
#include "imgui.h"

#include <memory>

static Shader mainShader{"shaders/pcBasic.vert", "shaders/pcBasic.frag"};

PCRendererBasic::PCRendererBasic()
	: PCRenderer(&(::mainShader))
{

}


PCRendererBasic::~PCRendererBasic()
{
}

void PCRendererBasic::update()
{
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

void PCRendererBasic::render() const
{
	glPointSize(pointSize);

	bindVAO();
	glDrawArrays(GL_POINTS, 0, vertexCount);
}

void PCRendererBasic::drawUI()
{
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);
}
