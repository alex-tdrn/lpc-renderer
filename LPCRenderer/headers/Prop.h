#pragma once
#include "AutoName.h"
#include "glm/glm.hpp"

class Mesh;

class Prop : public AutoName<Prop>
{
private:
	friend class Scene;
	Scene* scene = nullptr;
	Mesh* mesh = nullptr;
	glm::mat4 transform = glm::mat4(1.0f);
	float scaling = 1.0f;
	bool _enabled = true;
	bool _highlighted = false;

public:
	Prop(Mesh* mesh, glm::mat4 transform = glm::mat4(1.0f));

protected:
	std::string getNamePrefix() const override;

public:
	bool enabled() const;
	void enable();
	void disable();
	bool highlighted() const;
	void setHighlighted(bool flag);
	glm::mat4 getModelMatrix() const;
	void setTransformation(glm::mat4 t);
	Mesh* getMesh() const;
	void drawUI();

};