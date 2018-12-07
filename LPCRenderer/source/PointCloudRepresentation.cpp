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

void PointCloudRepresentation::growAndBindBuffer()
{
	if(buffer)
	{
		GLenum waitReturn = GL_UNSIGNALED;
		Profiler::beginFenceWait();
		while(waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
			waitReturn = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		Profiler::endFenceWait();
	}
	glDeleteSync(fence);

	if(bufferSize > capacity)
	{
		Profiler::recordGPUAllocation(bufferSize - capacity);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glDeleteBuffers(1, &VBO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferStorage(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT);
		buffer = (std::byte*)(glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT));

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) (0));

		capacity = bufferSize;
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
	}
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
	glBindVertexArray(VAO);
	vertexCount = cloud->getSize();
	bufferSize = vertexCount * 3 * sizeof(float);
	if(cloud->hasNormals() && useNormalsIfAvailable)
	{
		bufferSize *= 2;
		growAndBindBuffer();

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (bufferSize / 2));
	}
	else
	{
		growAndBindBuffer();
		std::memcpy(buffer, cloud->getPositions().data(), bufferSize);
		glDisableVertexAttribArray(1);
	}

	glDrawArrays(GL_POINTS, 0, vertexCount);
	fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glBindVertexArray(0);
}

void PointCloudRepresentation::updateAndUse(std::vector<PointCloud const*>& clouds, bool useNormalsIfAvailable, bool orphaning)
{
	glBindVertexArray(VAO);
	vertexCount = 0;
	for(auto cloud : clouds)
		vertexCount += cloud->getSize();
	bufferSize = vertexCount * 3 * sizeof(float);
	if(clouds.front()->hasNormals() && useNormalsIfAvailable)
		bufferSize *= 2;

	growAndBindBuffer();
	std::size_t offset = 0;
	for(auto cloud : clouds)
	{
		std::size_t cloudSize = cloud->getSize() * 3 * sizeof(float);
		std::memcpy((char*) (buffer) + offset, cloud->getPositions().data(), cloudSize);
		offset += cloudSize;
	}
	if(clouds.front()->hasNormals() && useNormalsIfAvailable)
	{
		for(auto cloud : clouds)
		{
			std::size_t cloudSize = cloud->getSize() * 3 * sizeof(float);
			std::memcpy((char*) (buffer) + offset, cloud->getNormals().data(), cloudSize);
			offset += cloudSize;
		}
		glDisableVertexAttribArray(1);
	}
	else
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) (bufferSize / 2));
	}

	glDrawArrays(GL_POINTS, 0, vertexCount);
	fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glBindVertexArray(0);
}

void PointCloudRepresentation::use() const
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, vertexCount);
	glBindVertexArray(0);
}
