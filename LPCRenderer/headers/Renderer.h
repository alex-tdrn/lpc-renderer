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
	mutable PointCloud const* currentPointCloud = nullptr;
	static inline std::vector<PointCloudRepresentation> pointCloudBufffers{0};
	static inline int currentPointCloudBuffer = 0;
	static inline int nBuffers = 1;
	static inline bool bufferOrphaning = false;
	float pointSize = 1.0f;
	bool decimation = true;
	int maxVertices = 100'000;
	bool frustumCulling = false;
	bool useNormalsIfAvailable = false;
	float debugNormalsLineLength = 0.015f;

public:
	Renderer();
	Renderer(Renderer const&) = delete;
	Renderer(Renderer&&) = default;
	~Renderer() = default;
	Renderer& operator=(Renderer const&) = delete;
	Renderer& operator=(Renderer&&) = default;

public:
	std::string getNamePrefix() const override;
	void render(Scene* = nullptr) const;
	void drawUI();

};
