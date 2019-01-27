#include "PointCloudRepresentation.h"
#include "PointCloud.h"
#include "glad/glad.h"
#include "Profiler.h"

PointCloudRepresentation::PointCloudRepresentation()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
}

PointCloudRepresentation::PointCloudRepresentation(PointCloudRepresentation&& other)
	:VAO(other.VAO), VBO(std::move(other.VBO)), SSBO(std::move(other.SSBO))
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
	other.VAO = 0;
	return *this;
}

void PointCloudRepresentation::free()
{
	glDeleteVertexArrays(1, &VAO);
}

void PointCloudRepresentation::update(bool shrinkToFit, bool useNormals, bool compress, PointCloud const* cloud)
{
	auto const& bricks = cloud->getBricks();
	glBindVertexArray(VAO);
	auto vertexCount = 0;

	if(!compress)
	{
		auto bufferSize = 0;
		static std::vector<std::pair<std::byte const*, std::size_t>> vertexData;
		vertexData.clear();

		for(auto const& brick : bricks)
		{	
			std::size_t brickSize = brick.getPositions().size() * 3 * sizeof(float);
			vertexData.emplace_back((std::byte const*)brick.getPositions().data(), brickSize);
			bufferSize += brickSize;
			vertexCount += brick.getPositions().size();
		}
		if(useNormals && !bricks.front().getNormals().empty())
		{
			for(auto const& brick : bricks)
			{
				std::size_t brickSize = brick.getNormals().size() * 3 * sizeof(float);
				vertexData.emplace_back((std::byte const*)brick.getNormals().data(), brickSize);
				bufferSize += brickSize;
			}
			VBO.write(shrinkToFit, std::move(vertexData));
			VBO.bind();
			glEnableVertexAttribArray(0);//Positions
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) (0));
			glEnableVertexAttribArray(1);//Normals
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (bufferSize / 2));
		}
		else
		{
			VBO.write(shrinkToFit, std::move(vertexData));
			VBO.bind();
			glEnableVertexAttribArray(0);//Positions
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) (0));
			glDisableVertexAttribArray(1);//Normals
		}
	}
	//else
	//{
	//	vertexCount = clouds.size();
	//	static std::vector<glm::vec3> origins;
	//	static std::vector<glm::vec3> sizes;
	//	static std::vector<std::uint32_t> bufferOffsets;
	//	static std::vector<std::uint32_t> bufferLengths;
	//	static std::vector<std::pair<std::byte const*, std::size_t>> positionsData;

	//	origins.clear();
	//	sizes.clear();
	//	bufferOffsets.clear();
	//	bufferOffsets.push_back(0);
	//	bufferLengths.clear();
	//	positionsData.clear();
	//	for(auto cloud : clouds)
	//	{
	//		origins.push_back(cloud->asBlock()->origin);
	//		sizes.push_back(cloud->asBlock()->size);
	//		bufferOffsets.push_back(cloud->asBlock()->positions.size() + bufferOffsets.back());
	//		bufferLengths.push_back(cloud->asBlock()->positions.size());
	//		positionsData.emplace_back((std::byte const*)cloud->asBlock()->positions.data(), cloud->asBlock()->positions.size() * sizeof(std::uint32_t));
	//	}
	//	bufferOffsets.pop_back();
	//	std::size_t bufferSize = origins.size() * sizeof(float) * 3;
	//	VBO.write(shrinkToFit, {
	//		{(std::byte const*)origins.data(), bufferSize},
	//		{(std::byte const*)sizes.data(), bufferSize},
	//		{(std::byte const*)bufferOffsets.data(), bufferOffsets.size() * sizeof(std::uint32_t)},
	//		{(std::byte const*)bufferLengths.data(), bufferLengths.size() * sizeof(std::uint32_t)}
	//	});
	//	VBO.bind();
	//	glEnableVertexAttribArray(0);//Origins
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) (0));
	//	glEnableVertexAttribArray(1);//Sizes
	//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (bufferSize));
	//	glEnableVertexAttribArray(2);//Offsets
	//	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (void*) (bufferSize * 2));
	//	glEnableVertexAttribArray(3);//Lengths
	//	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, (void*) (bufferSize * 2 + bufferOffsets.size() * sizeof(std::uint32_t)));

	//	SSBO.write(shrinkToFit, std::move(positionsData));
	//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO.getID());

	//	//TODO
	//}

	glDrawArrays(GL_POINTS, 0, vertexCount);
	VBO.lock();
	if(compress)
		SSBO.lock();
}
