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
	pcBarebones();
	pcLit();
}

Shader* ShaderManager::pcBarebones()
{
	static auto ret = load("Point Cloud Barebones",
		"shaders/pcBarebones.vert", "shaders/pcBarebones.frag"
	);
	return ret;
}

Shader* ShaderManager::pcLit()
{
	static auto ret = load("Point Cloud Lit",
		"shaders/pcLit.vert", "shaders/pcLit.frag"
	);
	return ret;
}
