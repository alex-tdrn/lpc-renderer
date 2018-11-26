#pragma once
#include <optional>
#include <array>
#include <vector>

struct Vertex
{
	float x;
	float y;
	float z;
};

class MeshRepresentation
{
private:
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	std::size_t vertexCount = 0;

public:
	MeshRepresentation(std::vector<Vertex> const& vertices);
	MeshRepresentation(MeshRepresentation const&) = delete;
	MeshRepresentation(MeshRepresentation&&);
	~MeshRepresentation();
	MeshRepresentation& operator=(MeshRepresentation const&) = delete;
	MeshRepresentation& operator=(MeshRepresentation&&) = delete;

public:
	void use() const;

};
