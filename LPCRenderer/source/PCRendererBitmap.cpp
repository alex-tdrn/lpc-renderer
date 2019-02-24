#include "PCRendererBitmap.h"
#include "Shader.h"
#include "PointCloud.h"
#include "imgui.h"

#include <bitset>

namespace
{
	Shader basicShader{"shaders/pcBrickIndirect.vert", "shaders/pcBrickIndirect.frag"};
	Shader unpackShader{"shaders/pcUnpackBitmap.comp"};
	int pointSize = 2;
}

PCRendererBitmap::PCRendererBitmap()
	:PCRenderer(&basicShader)
{
	Counter.reserve(4);
}

void PCRendererBitmap::update()
{

	std::vector<DrawCommand> indirectDraws;
	static std::vector<std::uint16_t> compressedPositions;
	constexpr unsigned int bitmapSize = 32;
	using BrickBitmap = std::bitset<bitmapSize * bitmapSize * bitmapSize>;
	static BrickBitmap auxBitmap;
	static std::vector<BrickBitmap> bitmaps;
	compressedPositions.clear();
	bitmaps.clear();
	int brickIndex = -1;
	for(auto brick : cloud->getAllBricks())
	{
		brickIndex++;
		if(brick.positions.empty())
			continue;
		DrawCommand brickDrawCommand{};
		brickDrawCommand.count = 0;
		brickDrawCommand.baseInstance = brickIndex;
		auxBitmap.reset();
		for(auto const& position : brick.positions)
		{
			glm::ivec3 coordinates(position * float(bitmapSize));
			int idx = 0;
			idx += coordinates.x;//jump points
			idx += coordinates.y * bitmapSize;//jump lines
			idx += coordinates.z * bitmapSize * bitmapSize;//jump surfaces
			if(!auxBitmap.test(idx))
			{
				compressedPositions.push_back(packPosition(position));
				brickDrawCommand.count++;
			}
			auxBitmap.set(idx, true);
		}
		indirectDraws.push_back(std::move(brickDrawCommand));
		bitmaps.push_back(auxBitmap);
	}
	indirectDrawCount = indirectDraws.size();

	bindVAO();
	SSBOBitmap.write({{(std::byte const*)bitmaps.data(), bitmaps.size() * sizeof(bitmaps.front())}});

	SSBOPackedPositions.reserve(compressedPositions.size() * sizeof(compressedPositions.front()));
	SSBOPackedPositions.bind(GL_ARRAY_BUFFER);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, 0, (void*)(0));

	SSBODrawCommands.write({{(std::byte const*)indirectDraws.data(), indirectDraws.size() * sizeof(indirectDraws.front())}});
}

void PCRendererBitmap::render(Scene const* scene)
{
	PCRenderer::render(scene);

	mainShader->set("cloudOrigin", cloud->getBounds().first);
	mainShader->set("brickSize", cloud->getBrickSize());
	mainShader->set("subdivisions", glm::uvec3(cloud->getSubdivisions()));

	bindVAO();

	unpackShader.use();
	SSBOBitmap.bindBase(0);
	SSBOPackedPositions.clear();
	SSBOPackedPositions.bind(GL_ARRAY_BUFFER);
	SSBOPackedPositions.bindBase(1);
	SSBODrawCommands.bindBase(2);
	SSBODrawCommands.bind(GL_DRAW_INDIRECT_BUFFER);
	Counter.clear();
	Counter.bindBase(0);

	glDispatchCompute(indirectDrawCount, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
	mainShader->use();

	glPointSize(pointSize);

	glMultiDrawArraysIndirect(GL_POINTS, nullptr, indirectDrawCount, 0);


}

void PCRendererBitmap::drawUI()
{
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);
}
