#pragma once

#include <glm/glm.hpp>

namespace OSWindow
{
	void init();
	glm::ivec2 getSize();
	float getAspectRatio();
	void resize(glm::ivec2);
	void beginFrame();
	void endFrame();
	bool shouldClose();
	void destroy();
}


