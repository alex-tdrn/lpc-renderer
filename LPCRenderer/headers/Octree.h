#pragma once
#include <optional>
#include <glm/glm.hpp>
#include <vector>
#include <tuple>

class PointCloud;

class Octree
{
private:
	bool isLeaf;
	std::optional<std::vector<Octree>> children = std::nullopt;
	PointCloud* cloud = nullptr;
	std::pair<glm::vec3, glm::vec3> bounds{glm::vec3{-1.0f}, glm::vec3{1.0f}};
	std::size_t preferredVerticesPerNode = 100'000;
	std::size_t totalVerticesCount = 0;
	std::size_t totalLeafNodesCount = 0;
	int currentDepth = 0;
	int maxDepth = 1;

private:
	Octree(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals, 
		std::pair<glm::vec3, glm::vec3> bounds, int depth, 
		int maxDepth,std::size_t preferredVerticesPerNode);

public:
	Octree(PointCloud const& cloud);
	Octree() = delete;
	Octree(Octree const& other) = delete;
	Octree(Octree&& other);
	~Octree();
	Octree& operator=(Octree const& other) = delete;
	Octree& operator=(Octree&& other) = delete;

private:
	void split(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals);
	void join();

public:
	void getAllLeafNodes(std::vector<Octree const*>&) const;
	void getAllPointClouds(std::vector<PointCloud const*>&) const;
	void getPointCloudsInsideFrustum(std::vector<PointCloud const*>&, glm::mat4, int LODPixelArea = 0, std::size_t LODVertices = 0) const;
	glm::mat4 getBoundsTransform() const;
	void update(int maxDepth, std::size_t preferredVerticesPerNode);
	std::size_t getPrefferedVerticesPerNode() const;
	std::size_t getTotalVerticesCount() const;
	std::size_t getTotalLeafNodesCount() const;
	std::size_t getVerticesCount() const;
	float getOccupancy() const;
	int getMaxDepth() const;
	int getDepth() const;
	std::pair<glm::vec3, glm::vec3> getBounds() const;

};

