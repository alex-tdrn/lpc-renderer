#include "PointCloudRepresentation.h"
#include "PointCloud.h"
#include "glad/glad.h"
#include "Profiler.h"

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
	Profiler::recordGPUDeallocation(capacity);
}

void PointCloudRepresentation::shrink()
{
	Profiler::recordGPUDeallocation(capacity - bufferSize);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);
	capacity = bufferSize;
}

void PointCloudRepresentation::updateAndUse(PointCloud const* cloud, bool useNormalsIfAvailable, bool orphaning)
{
	auto growBuffer = [&](){

		if(bufferSize > capacity)
		{
			Profiler::recordGPUAllocation(bufferSize - capacity);
			glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);
			capacity = bufferSize;
		}
		else if(orphaning)
		{
			glBufferData(GL_ARRAY_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
		}
	};
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	vertexCount = cloud->getSize();
	bufferSize = vertexCount * 3 * sizeof(float);
	if(cloud->hasNormals() && useNormalsIfAvailable)
	{
		bufferSize *= 2;
		growBuffer();
		glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize / 2, cloud->getPositions().data());
		glBufferSubData(GL_ARRAY_BUFFER, bufferSize / 2, bufferSize / 2, cloud->getNormals().data());
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (bufferSize / 2));
	}
	else
	{
		growBuffer();
		glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, cloud->getPositions().data());
		glDisableVertexAttribArray(1);
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
