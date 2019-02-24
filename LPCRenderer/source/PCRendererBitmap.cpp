#include "PCRendererBitmap.h"
#include "Shader.h"
#include "PointCloud.h"
#include "imgui.h"

#include <bitset>

namespace
{
	Shader basicShader{"shaders/pcBrickIndirect.vert", "shaders/pcBrickIndirect.frag"};
	Shader unpack32Shader{"shaders/pcUnpackBitmap32.comp"};
	Shader unpack16Shader{"shaders/pcUnpackBitmap16.comp"};
	Shader unpack8Shader{"shaders/pcUnpackBitmap8.comp"};
	Shader unpack4Shader{"shaders/pcUnpackBitmap4.comp"};
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
	constexpr std::size_t bitmapSize = 4;
	using BrickBitmap = std::bitset<bitmapSize * bitmapSize * bitmapSize>;
	static BrickBitmap auxBitmap;
	static std::vector<BrickBitmap> bitmaps;
	bitmaps.clear();
	std::vector<std::uint32_t> bitmapIndices;
	int brickIndex = -1;
	totalBrickCount = 0;
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
			auxBitmap.set(idx, true);
		}
		bitmapIndices.push_back(brickIndex);
		bitmaps.push_back(auxBitmap);
	}

	bindVAO();
	SSBOBitmaps.write({{(std::byte const*)bitmaps.data(), sizeInBytes(bitmaps)}});
	SSBOBitmapIndices.write({{(std::byte const*)bitmapIndices.data(),sizeInBytes(bitmapIndices)}});
	std::size_t positionCount = batchSize * bitmapSize * bitmapSize * bitmapSize;
	SSBOPackedPositions.reserve((positionCount + positionCount % 2) * sizeof(std::uint16_t));
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
	unpack4Shader.use();

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
		Counter.clear();

		unpack4Shader.use();
		unpack4Shader.set("bitmapsOffset", totalBrickCount - remainingBricks);
		int brickCount = batchSize;
		if(remainingBricks < batchSize)
			brickCount = remainingBricks;
		glDispatchCompute(brickCount, 1, 1);
		mainShader->use();
		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

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
	unpack32Shader.reload();
	unpack16Shader.reload();
	unpack8Shader.reload();
	unpack4Shader.reload();
}
