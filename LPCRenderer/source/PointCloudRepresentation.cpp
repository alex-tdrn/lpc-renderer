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
	:VAO(other.VAO), VBO(std::move(other.VBO))
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
	other.VAO = 0;
	return *this;
}

void PointCloudRepresentation::free()
{
	glDeleteVertexArrays(1, &VAO);
}

void PointCloudRepresentation::update(bool shrinkToFit, bool useNormals, std::vector<PointCloud const*>& clouds)
{
	if(clouds.empty())
		return;
	glBindVertexArray(VAO);
	auto bufferSize = 0;
	auto vertexCount = 0;
	std::vector<std::pair<std::byte const*, std::size_t>> vertexData;

	for(auto cloud : clouds)
	{	
		std::size_t cloudSize = cloud->getSize() * 3 * sizeof(float);
		vertexData.emplace_back((std::byte const*)cloud->getPositions().data(), cloudSize);
		bufferSize += cloudSize;
		vertexCount += cloud->getSize();
	}
	if(clouds.front()->hasNormals() && useNormals)
	{
		for(auto cloud : clouds)
		{
			std::size_t cloudSize = cloud->getSize() * 3 * sizeof(float);
			vertexData.emplace_back((std::byte const*)cloud->getNormals().data(), cloudSize);
			bufferSize += cloudSize;
		}
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (bufferSize / 2));
	}
	else
	{
		glDisableVertexAttribArray(1);
	}
	VBO.write(shrinkToFit, std::move(vertexData));
	glDrawArrays(GL_POINTS, 0, vertexCount);
	VBO.lock();
}
