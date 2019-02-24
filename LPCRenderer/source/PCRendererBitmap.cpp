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
	int batchSize = 1;
}

PCRendererBitmap::PCRendererBitmap()
	:PCRenderer(&basicShader)
{
	Counter.reserve(4);
}

void PCRendererBitmap::update()
{

	constexpr std::size_t bitmapSize = 32;
	using BrickBitmap = std::bitset<bitmapSize * bitmapSize * bitmapSize>;
	static BrickBitmap auxBitmap;
	static std::vector<BrickBitmap> bitmaps;
	std::vector<std::uint32_t> bitmapIndices;
	bitmaps.clear();
	int brickIndex = -1;
	for(auto brick : cloud->getAllBricks())
	{
		brickIndex++;
		if(brick.positions.empty())
			continue;
		totalBrickCount++;
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
				auxBitmap.set(idx, true);
			}
		}
		bitmapIndices.push_back(brickIndex);
		bitmaps.push_back(auxBitmap);
	}

	bindVAO();
	SSBOBitmaps.write({{(std::byte const*)bitmaps.data(), sizeInBytes(bitmaps)}});
	SSBOBitmapIndices.write({{(std::byte const*)bitmapIndices.data(),sizeInBytes(bitmapIndices)}});

	SSBOPackedPositions.reserve(batchSize * bitmapSize * bitmapSize * bitmapSize * sizeof(std::uint16_t));
	SSBOPackedPositions.bind(GL_ARRAY_BUFFER);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, 0, (void*)(0));

	SSBODrawCommands.reserve(batchSize * sizeof(DrawCommand));
}

void PCRendererBitmap::render(Scene const* scene)
{
	PCRenderer::render(scene);

	mainShader->set("cloudOrigin", cloud->getBounds().first);
	mainShader->set("brickSize", cloud->getBrickSize());
	mainShader->set("subdivisions", glm::uvec3(cloud->getSubdivisions()));

	bindVAO();
	SSBOBitmaps.bindBase(0);
	SSBOBitmapIndices.bindBase(1);
	SSBOPackedPositions.bindBase(2);
	SSBOPackedPositions.bind(GL_ARRAY_BUFFER);
	SSBODrawCommands.bindBase(3);
	SSBODrawCommands.bind(GL_DRAW_INDIRECT_BUFFER);
	Counter.bindBase(0);
	glPointSize(pointSize);

	int remainingBricks = totalBrickCount;
	while(remainingBricks > 0)
	{
		//SSBOPackedPositions.clear();
		Counter.clear();

		unpackShader.use();
		unpackShader.set("bitmapsOffset", totalBrickCount - remainingBricks);
		int brickCount = batchSize;
		if(remainingBricks < batchSize)
			brickCount = remainingBricks;
		glDispatchCompute(brickCount, 1, 1);
		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

		mainShader->use();
		glMultiDrawArraysIndirect(GL_POINTS, nullptr, brickCount, 0);
		remainingBricks -= batchSize;
	}

}

void PCRendererBitmap::drawUI()
{
	PCRenderer::drawUI();
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);
	if(ImGui::InputInt("Batch Size", &batchSize, 1, 10))
	{
		if(batchSize < 1)
			batchSize = 1;
		update();
	}

	ImGui::Text("Memory Bitmaps: ");
	ImGui::SameLine();
	drawMemoryConsumption(SSBOBitmaps.size());

	ImGui::Text("Memory Bitmap Indices: ");
	ImGui::SameLine();
	drawMemoryConsumption(SSBOBitmapIndices.size());

	ImGui::Text("Memory Packed Positions: ");
	ImGui::SameLine();
	drawMemoryConsumption(SSBOPackedPositions.size());

	ImGui::Text("Memory Draw Commands: ");
	ImGui::SameLine();
	drawMemoryConsumption(SSBODrawCommands.size());

}

void PCRendererBitmap::reloadShaders()
{
	basicShader.reload();
	unpackShader.reload();
}
