#include "RendererManager.h"
#include "DebugRenderer.h"

std::string const Manager<Renderer, true>::name = "Renderers";

void RendererManager::initialize()
{
	debug();
}

Renderer* RendererManager::debug()
{
	static auto ret = add(std::make_unique<DebugRenderer>());
	return ret;
}
