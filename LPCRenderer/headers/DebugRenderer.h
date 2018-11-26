#pragma once

#include "Renderer.h"

class DebugRenderer final : public Renderer
{
private:
	glm::vec3 meshColor{0.3f, 0.4f, 0.2f};
	glm::vec3 highlightColor{1.0f};
	bool highlightProps = true;
	float pointSize = 2.0f;

protected:
	std::string getNamePrefix() const override;

public:
	void render(Scene* scene) const override;
	void drawUI() override;

};