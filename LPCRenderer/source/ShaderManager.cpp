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
	box();
	pcDebugNormals();
	pcBarebones();
	pcBarebonesBrickGS();
	pcBarebonesBrickVS();
	pcLit();
	pcLitDisk();
}

Shader *ShaderManager::box()
{
	static auto ret = load("Box",
		"shaders/box.vert", "shaders/box.frag"
	);
	return ret;
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

Shader* ShaderManager::pcBarebonesBrickGS()
{
	static auto ret = load("Barebones BrickGS",
		"shaders/pcBarebonesBrickGS.vert", "shaders/pcBarebonesBrickGS.frag", "shaders/pcBarebonesBrickGS.geom"
	);
	return ret;
}

Shader* ShaderManager::pcBarebonesBrickVS()
{
	static auto ret = load("Barebones BrickVS",
		"shaders/pcBarebonesBrickVS.vert", "shaders/pcBarebonesBrickVS.frag"
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
