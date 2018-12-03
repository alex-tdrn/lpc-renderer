#include "PointCloudRepresentation.h"
#include "PointCloud.h"
#include "glad/glad.h"

PointCloudRepresentation::PointCloudRepresentation()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) (0));
	glBindVertexArray(0);
}

PointCloudRepresentation::PointCloudRepresentation(PointCloudRepresentation&& other)
	:VAO(other.VAO), VBO(other.VBO)
{
	other.VAO = 0;
	other.VBO = 0;
}

PointCloudRepresentation::~PointCloudRepresentation()
{
	freeBuffers();
}

PointCloudRepresentation& PointCloudRepresentation::operator=(PointCloudRepresentation &&other)
{
	freeBuffers();
	VAO = other.VAO;
	VBO = other.VBO;
	vertexCount = other.vertexCount;
	other.VAO = 0;
	other.VBO = 0;
	return *this;
}

void PointCloudRepresentation::freeBuffers()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void PointCloudRepresentation::updateAndUse(PointCloud const* cloud)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), nullptr, GL_STREAM_DRAW);
	vertexCount = cloud->getSize();
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), cloud->getPositions().data(), GL_STREAM_DRAW);

	glDrawArrays(GL_POINTS, 0, vertexCount);
	glBindVertexArray(0);
}

void PointCloudRepresentation::use() const
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, vertexCount);
	glBindVertexArray(0);
}
