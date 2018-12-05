#include "ShaderManager.h"

std::string const Manager<Shader>::name = "Shaders";

template <typename ...Sources>
Shader* load(std::string&& name, Sources&&... sources)
{
	auto shader = std::make_unique<Shader>(sources...);
	shader->setName(std::move(name));
	return ShaderManager::add(std::move(shader));
}

void ShaderManager::initialize()
{
	pcDebugNormals();
	pcBarebones();
	pcLit();
	pcLitDisk();
}

Shader* ShaderManager::pcDebugNormals()
{
	static auto ret = load("Debug Normals",
		"shaders/pcDebugNormals.vert", "shaders/pcDebugNormals.frag", "shaders/pcDebugNormals.geom"
	);
	return ret;
}

Shader* ShaderManager::pcBarebones()
{
	static auto ret = load("Barebones",
		"shaders/pcBarebones.vert", "shaders/pcBarebones.frag"
	);
	return ret;
}

Shader* ShaderManager::pcLit()
{
	static auto ret = load("Lit",
		"shaders/pcLit.vert", "shaders/pcLit.frag"
	);
	return ret;
}

Shader* ShaderManager::pcLitDisk()
{
	static auto ret = load("Lit Disk",
		"shaders/pcLitDisk.vert", "shaders/pcLitDisk.frag", "shaders/pcLitDisk.geom"
	);
	return ret;
}
