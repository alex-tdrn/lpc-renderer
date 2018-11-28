#pragma once
#include "AutoName.h"
#include "MeshRepresentation.h"
#include "glm/glm.hpp"

#include <vector>
#include <memory>

class Mesh : public AutoName<Mesh>
{
private:
	std::vector<Vertex> vertices;
	std::vector<Vertex> stridedVertices;
	MeshRepresentation representation;
	bool filtering = true;
	int maxVertices = 1'000'000;
	bool pickVerticesSequentially = false;
	bool cull = false;
	glm::mat4 cullMatrix;

public:
	Mesh(std::vector<Vertex>&& vertices);

private:
	void updateRepresentation();

public:
	std::string getNamePrefix() const override;
	static std::unique_ptr<Mesh> join(std::vector<std::unique_ptr<Mesh>>&& meshes);
	MeshRepresentation const& getRepresentation() const;
	void setCullMatrix(glm::mat4 mvp);
	void applyTransformation(glm::mat4 t);
	void drawUI();

};

