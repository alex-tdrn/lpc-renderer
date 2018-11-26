#pragma once
#include "AutoName.h"
#include "MeshRepresentation.h"
#include <vector>
#include <memory>

class Mesh : public AutoName<Mesh>
{
private:
	std::vector<Vertex> const vertices;
	mutable std::unique_ptr<MeshRepresentation> representation = nullptr;

public:
	Mesh(std::vector<Vertex>&& vertices);
	~Mesh();

protected:
	std::string getNamePrefix() const override;

public:
	MeshRepresentation* getRepresentation() const;
	void drawUI() const;

};

