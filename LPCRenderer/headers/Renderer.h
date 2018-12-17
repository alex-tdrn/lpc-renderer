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
	mutable enum class RenderMode
	{
		barebones,
		debugNormals,
		lit,
		litDisk
	} renderMode = RenderMode::barebones;
	mutable PointCloud const* currentPointCloud = nullptr;
	mutable std::size_t renderedVertices = 0;
	static inline std::vector<PointCloudRepresentation> pointCloudBufffers{0};
	static inline int currentPointCloudBuffer = 0;
	static inline int nBuffers = 1;
	static inline bool shrinkBuffersToFit = false;
	static inline bool useOctree = true;
	static inline bool drawOctreeBoundingBoxes = true;
	static inline bool drawPointCloudBoundingBoxes = false;
	static inline int octreeMaxDepth = 8;
	static inline int octreePreferredVerticesPerNode = 1'000;
	static inline bool drawAllOctreeDepths = true;
	static inline int drawOnlyOctreeDepth = 0;
	static inline bool frustumCulling = true;
	static inline bool decimation = true;
	static inline int decimationMaxVertices = 100'000;
	static inline bool LOD = false;
	static inline int LODVertices = 200;
	static inline int LODPixelArea = 4'096;
	bool compressPointClouds = false;
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
