#pragma once
#include "GPUBuffer.h"
#include "glm/glm.hpp"
#include "glad/glad.h"
#include <optional>
#include <array>
#include <vector>

class PointCloud;
enum class Compression
{
	none,
	brickGS,
	brickVS
};

class PointCloudRepresentation
{
private:
	unsigned int VAO = 0;
	GPUBuffer VBO{GL_ARRAY_BUFFER};
	GPUBuffer SSBO{GL_SHADER_STORAGE_BUFFER};
	std::size_t vertexCount = 0;
	Compression compression = Compression::none;

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
	void update(bool shrinkToFit, bool useNormals, Compression compression, PointCloud const* cloud);
	void render();
};
