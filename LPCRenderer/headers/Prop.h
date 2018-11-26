#pragma once
#include "AutoName.h"
#include "glm/glm.hpp"

class Mesh;

class Prop : public AutoName<Prop>
{
private:
	Mesh* mesh = nullptr;
	glm::mat4 transform = glm::mat4(1.0f);
	float scale = 1.0f;
	bool _enabled = true;
public:
	Prop(Mesh* mesh, glm::mat4 transform = glm::mat4(1.0f));

protected:
	std::string getNamePrefix() const override;

public:
	bool enabled() const;
	void enable();
	void disable();
	glm::mat4 getModelMatrix() const;
	Mesh* getMesh() const;
	void drawUI();

};