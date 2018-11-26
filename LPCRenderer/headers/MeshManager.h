#pragma once
#include "Manager.h"
#include "Mesh.h"

#include <filesystem>

class MeshManager : public Manager<Mesh>
{
public:
	static std::vector<Mesh*> load(std::filesystem::path const& filename);

};

