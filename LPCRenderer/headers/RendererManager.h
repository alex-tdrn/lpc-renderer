#pragma once
#include "Manager.h"
#include "Renderer.h"

class RendererManager : public Manager<Renderer, true>
{
public:
	static void initialize();
	static Renderer* main();
};

