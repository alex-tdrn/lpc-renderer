#pragma once
#include "GPUBuffer.h"
#include "glm/glm.hpp"
#include "glad/glad.h"
#include <optional>
#include <array>
#include <vector>

class PointCloud;

class PointCloudRepresentation
{
private:
	unsigned int VAO = 0;
	GPUBuffer VBO{GL_ARRAY_BUFFER};
	GPUBuffer SSBO{GL_SHADER_STORAGE_BUFFER};

public:
	PointCloudRepresentation();
	PointCloudRepresentation(PointCloudRepresentation const&) = delete;
	PointCloudRepresentation(PointCloudRepresentation&&);
	~PointCloudRepresentation();
	PointCloudRepresentation& operator=(PointCloudRepresentation const&) = delete;
	PointCloudRepresentation& operator=(PointCloudRepresentation&&);

private:
	void free();

public:
	void update(bool shrinkToFit, bool useNormals, bool compress, PointCloud const* cloud);

};
