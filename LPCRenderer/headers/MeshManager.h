#pragma once
#include "Manager.h"
#include "Mesh.h"

#include <filesystem>

class MeshManager : public Manager<Mesh>
{
public:
	static Mesh* load(std::filesystem::path const& filename);
	static void drawUI(bool* open)
	{
		Manager<Mesh>::drawUI(open);
	}
};

