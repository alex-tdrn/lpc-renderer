#pragma once
#include "AutoName.h"

#include <glm/glm.hpp>
#include <string_view>

class Scene;

class Renderer : public AutoName<Renderer>
{
private:
	glm::vec3 background{0.6, 0.7, 0.3};

public:
	Renderer();
	virtual ~Renderer();

public:
	virtual void render(Scene* = nullptr) const;
	virtual void drawUI();

};