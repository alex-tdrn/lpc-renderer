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
	std::size_t maxVerticesPerNode = 100'000;
	std::size_t totalVerticesCount = 0;
	int currentDepth = 0;
	int maxDepth = 1;

private:
	Octree(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals, 
		std::pair<glm::vec3, glm::vec3> bounds, int depth, 
		int maxDepth,std::size_t maxVerticesPerNode);

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
	void getAllLeafNodes(std::vector<Octree*>&);
	void update(int maxDepth, std::size_t maxVerticesPerNode);
	std::size_t getMaxVerticesPerNode() const;
	std::size_t getTotalVerticesCount() const;
	int getMaxDepth() const;
	std::pair<glm::vec3, glm::vec3> getBounds() const;

};

