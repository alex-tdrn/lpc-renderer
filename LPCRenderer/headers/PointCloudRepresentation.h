#pragma once
#include "glm/glm.hpp"

#include <optional>
#include <array>
#include <vector>

class PointCloud;

class PointCloudRepresentation
{
private:
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	std::size_t vertexCount = 0;
	std::size_t bufferSize = 0;
	std::size_t capacity = 0;

public:
	PointCloudRepresentation();
	PointCloudRepresentation(PointCloudRepresentation const&) = delete;
	PointCloudRepresentation(PointCloudRepresentation&&);
	~PointCloudRepresentation();
	PointCloudRepresentation& operator=(PointCloudRepresentation const&) = delete;
	PointCloudRepresentation& operator=(PointCloudRepresentation&&);

private:
	void freeBuffers();

public:
	void shrink();
	void updateAndUse(PointCloud const* cloud, bool useNormalsIfAvailable, bool orphaning = false);
	void use() const;

};