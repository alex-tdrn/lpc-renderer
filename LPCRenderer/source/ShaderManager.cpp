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
	debug();
}

Shader* ShaderManager::debug()
{
	static auto ret = load("Debug",
		"shaders/debug.vert", "shaders/debug.frag"
	);
	return ret;
}
