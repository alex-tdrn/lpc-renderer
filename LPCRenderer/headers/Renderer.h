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
	static inline bool useOctree = false;
	static inline bool drawOctreeBoundingBoxes = true;
	static inline int octreeMaxDepth = 1;
	static inline int octreeMaxVerticesPerNode = 100'000;
	static inline bool decimation = true;
	static inline int decimationMaxVertices = 100'000;
	static inline bool frustumCulling = false;
	bool useNormalsIfAvailable = false;
	bool backFaceCulling = true;
	int pointSize = 1;
	float diskRadius = 0.0005f;
	float debugNormalsLineLength = 0.001f;
	int debugNormalsLineThickness = 2;

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
