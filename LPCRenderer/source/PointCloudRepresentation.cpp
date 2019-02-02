#include "PointCloudRepresentation.h"
#include "PointCloud.h"
#include "glad/glad.h"
#include "Profiler.h"

struct DrawCommand
{
	unsigned int count = 0;
	unsigned int instanceCount = 1;
	unsigned int first = 0;
	unsigned int baseInstance = 0;
};

PointCloudRepresentation::PointCloudRepresentation()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
}

PointCloudRepresentation::PointCloudRepresentation(PointCloudRepresentation&& other)
	:VAO(other.VAO), VBO(std::move(other.VBO)), SSBO(std::move(other.SSBO)), DrawBuffer(std::move(other.DrawBuffer))
{
	other.VAO = 0;
}

PointCloudRepresentation::~PointCloudRepresentation()
{
	free();
}

PointCloudRepresentation& PointCloudRepresentation::operator=(PointCloudRepresentation &&other)
{
	free();
	VAO = other.VAO;
	VBO = std::move(other.VBO);
	SSBO = std::move(other.SSBO);
	DrawBuffer = std::move(other.DrawBuffer);
	other.VAO = 0;
	return *this;
}

void PointCloudRepresentation::free()
{
	glDeleteVertexArrays(1, &VAO);
}

void PointCloudRepresentation::update(bool shrinkToFit, bool useNormals, Compression compression, PointCloud const* cloud)
{
	auto const& bricks = cloud->getAllBricks();
	glBindVertexArray(VAO);
	vertexCount = 0;
	this->compression = compression;

	switch (compression)
	{
		case Compression::none:
		{
			std::size_t bufferSize = 0;
			static std::vector<std::pair<std::byte const*, std::size_t>> vertexData;
			vertexData.clear();
			static std::vector<glm::vec3> globalPositions;
			for (auto const& brick : bricks)
				vertexCount += brick.positions.size();

			globalPositions.clear();
			globalPositions.reserve(vertexCount);
			for (auto const& brick : bricks)
			{
				if (brick.positions.empty())
					continue;
				std::byte const* startAddress = (std::byte const*)(globalPositions.data());
				std::size_t brickSize = brick.positions.size() * 3 * sizeof(float);
				vertexData.emplace_back((std::byte const*)(globalPositions.data() + bufferSize / 3 / sizeof(float)), brickSize);
				for (glm::vec3 position : brick.positions)
					globalPositions.push_back(cloud->convertToWorldPosition(brick.indices, position));
				bufferSize += brickSize;
			}
			if (useNormals && cloud->hasNormals())
			{
				for (auto const& brick : bricks)
				{
					if (brick.positions.empty())
						continue;
					std::size_t brickSize = brick.normals.size() * 3 * sizeof(float);
					vertexData.emplace_back((std::byte const*)brick.normals.data(), brickSize);
					bufferSize += brickSize;
				}
				VBO.write(shrinkToFit, std::move(vertexData));
				VBO.bind();
				glEnableVertexAttribArray(0);//Positions
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
				glEnableVertexAttribArray(1);//Normals
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(bufferSize / 2));
			}
			else
			{
				VBO.write(shrinkToFit, std::move(vertexData));
				VBO.bind();
				glEnableVertexAttribArray(0);//Positions
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
				glDisableVertexAttribArray(1);//Normals
			}
			break;
		}
		case Compression::brickGS:
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

			for (auto brick : bricks)
			{
				if (brick.positions.empty())
					continue;
				vertexCount++;
				brickIndices.push_back(brick.indices);
				for (auto const& position : brick.positions)
					compressedPositions.push_back(glm::packUnorm4x8(glm::vec4(position, 0.0f)));

				bufferOffsets.push_back(bufferOffsets.back() + brick.positions.size());
				bufferLengths.push_back(brick.positions.size());
			}
			bufferOffsets.pop_back();
			std::size_t brickIndicesBufferSize = brickIndices.size() * sizeof(brickIndices.front());
			std::size_t bufferOffsetsBufferSize = bufferOffsets.size() * sizeof(bufferOffsets.front());
			VBO.write(shrinkToFit, {
				{ (std::byte const*)brickIndices.data(), brickIndicesBufferSize },
				{ (std::byte const*)bufferOffsets.data(), bufferOffsetsBufferSize },
				{ (std::byte const*)bufferLengths.data(), bufferLengths.size() * sizeof(bufferLengths.front()) }
				});
			VBO.bind();
			glEnableVertexAttribArray(0);//Indices
			glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, 0, (void*)(0));
			glEnableVertexAttribArray(1);//Buffer Offsets
			glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, (void*)(brickIndicesBufferSize));
			glEnableVertexAttribArray(2);//Buffer Lengths
			glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (void*)(brickIndicesBufferSize + bufferOffsetsBufferSize));

			SSBO.write(shrinkToFit, { { (std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front()) } });
			SSBO.bindBase(0);
			break;
		}
		case Compression::brickIndirect:
		{
			SSBO.free();
			std::vector<DrawCommand> indirectDraws;
			static std::vector<std::uint32_t> compressedPositions;
			compressedPositions.clear();
			int brickIndex = -1;
			for (auto brick : bricks)
			{
				brickIndex++;
				if (brick.positions.empty())
					continue;
				vertexCount+= brick.positions.size();
				DrawCommand brickDrawCommand{};
				brickDrawCommand.count = brick.positions.size();
				brickDrawCommand.first = compressedPositions.size();
				brickDrawCommand.baseInstance = brickIndex;
				indirectDraws.push_back(std::move(brickDrawCommand));
				for (auto const& position : brick.positions)
					compressedPositions.push_back(glm::packUnorm4x8(glm::vec4(position, 0.0f)));
			}
			indirectDrawCount = indirectDraws.size();
			DrawBuffer.write(shrinkToFit, {{(std::byte const*)indirectDraws.data(), indirectDraws.size() * sizeof(indirectDraws.front())}});
			DrawBuffer.bind();
			VBO.write(shrinkToFit, {{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front()) }});
			VBO.bind();
			glEnableVertexAttribArray(0);//Compressed Positions
			glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)(0));
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			break;
		}
	}

}

void PointCloudRepresentation::render()
{
	switch (compression)
	{
	case Compression::none:
	case Compression::brickGS:
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, vertexCount);
		break;
	case Compression::brickIndirect:
		glBindVertexArray(VAO);
		glMultiDrawArraysIndirect(GL_POINTS, nullptr, indirectDrawCount, 0);
		break;
	}
}
