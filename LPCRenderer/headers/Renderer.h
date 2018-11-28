#pragma once
#include "AutoName.h"

#include <glm/glm.hpp>
#include <string_view>

class Scene;

class Renderer : public AutoName<Renderer>
{
private:
	glm::vec3 backgroundColor{0.6, 0.7, 0.3};
	glm::vec3 meshColor{0.3f, 0.4f, 0.2f};
	glm::vec3 highlightColor{1.0f};
	bool highlightProps = true;
	float pointSize = 2.0f;
	mutable bool frustumCulling = false;

protected:
	std::string getNamePrefix() const override;

public:
	void render(Scene* = nullptr) const;
	void drawUI();

};
