#pragma once

#include "Manager.h"
#include "Shader.h"

class ShaderManager : public Manager<Shader>
{
public:
	static void initialize();
	static Shader* debug();
	static void drawUI(bool* open)
	{
		Manager<Shader>::drawUI(open);
	}
};