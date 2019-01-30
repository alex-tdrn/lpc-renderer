#pragma once

#include "Manager.h"
#include "Shader.h"

class ShaderManager : public Manager<Shader>
{
public:
	static void initialize();
	static Shader* box();
	static Shader* pcDebugNormals();
	static Shader* pcBarebones();
	static Shader* pcBarebonesBrickGS();
	static Shader* pcLit();
	static Shader* pcLitDisk();
};