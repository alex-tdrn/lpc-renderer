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

void PointCloudRepresentation::updateAndUse(PointCloud const* cloud, bool useNormalsIfAvailable)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);
	vertexCount = cloud->getSize();
	bufferSize = vertexCount * 3 * sizeof(float);
	if(cloud->hasNormals() && useNormalsIfAvailable)
	{
		bufferSize *= 2;
		glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize / 2, cloud->getPositions().data());
		glBufferSubData(GL_ARRAY_BUFFER, bufferSize / 2, bufferSize / 2, cloud->getNormals().data());
		if(!normalsAttributeEnabled)
		{
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (bufferSize / 2));
			normalsAttributeEnabled = true;
		}
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, bufferSize, cloud->getPositions().data(), GL_STREAM_DRAW);
		if(normalsAttributeEnabled)
		{
			glDisableVertexAttribArray(1);
			normalsAttributeEnabled = false;
		}
	}

	glDrawArrays(GL_POINTS, 0, vertexCount);
	glBindVertexArray(0);
}

void PointCloudRepresentation::use() const
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, vertexCount);
	glBindVertexArray(0);
}
