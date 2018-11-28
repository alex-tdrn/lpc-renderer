#pragma once
#include "glm/glm.hpp"

#include <optional>
#include <array>
#include <vector>
struct Vertex
{
	glm::vec3 position;
};

class MeshRepresentation
{
private:
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	std::size_t vertexCount = 0;

public:
	MeshRepresentation(std::vector<Vertex> const& vertices, std::size_t n);
	MeshRepresentation(std::vector<Vertex> const& vertices);
	MeshRepresentation(MeshRepresentation const&) = delete;
	MeshRepresentation(MeshRepresentation&&);
	~MeshRepresentation();
	MeshRepresentation& operator=(MeshRepresentation const&) = delete;
	MeshRepresentation& operator=(MeshRepresentation&&);

private:
	void freeBuffers();

public:
	void use() const;

};
