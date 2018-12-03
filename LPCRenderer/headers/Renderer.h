#pragma once
#include "AutoName.h"
#include "ShaderManager.h"
#include "PointCloudRepresentation.h"

#include <glm/glm.hpp>
#include <string_view>

class Scene;
class Shader;

class Renderer : public AutoName<Renderer>
{
private:
	Shader* activeShader = ShaderManager::pcBarebones();
	mutable PointCloudRepresentation pointCloudRepresentation;
	float pointSize = 1.0f;
	bool decimation = true;
	int maxVertices = 1'000'000;
	bool frustumCulling = false;

public:
	std::string getNamePrefix() const override;
	void render(Scene* = nullptr) const;
	void drawUI();

};
