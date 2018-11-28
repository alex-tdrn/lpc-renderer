#include "RendererManager.h"

std::string const Manager<Renderer, true>::name = "Renderers";

void RendererManager::initialize()
{
	main();
}

Renderer* RendererManager::main()
{
	static auto ret = add(std::make_unique<Renderer>());
	return ret;
}
