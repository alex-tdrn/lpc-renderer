#pragma once
#include "AutoName.h"
#include "glm/glm.hpp"
#include "Octree.h"

#include <vector>
#include <memory>
#include <optional>

struct PointCloudBlock
{
	glm::vec3 origin;
	glm::vec3 size;
	std::vector<glm::u8vec3> offsets;
};

class PointCloud : public AutoName<PointCloud>
{
	friend class std::unique_ptr<PointCloud>;

private:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	mutable std::unique_ptr<PointCloud> _decimated = nullptr;
	mutable int maxVerticesDecimated = 0;
	mutable std::unique_ptr<Octree> _octree = nullptr;
	mutable std::pair<glm::vec3, glm::vec3> bounds;
	mutable bool boundsOutOfDate = true;
	mutable std::unique_ptr<PointCloudBlock> _block = nullptr;
public:
	PointCloud(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals = {});
	PointCloud() = default;
	PointCloud(PointCloud const& other);
	PointCloud(PointCloud&& other) = default;
	~PointCloud() = default;
	PointCloud& operator=(PointCloud const& other);
	PointCloud& operator=(PointCloud&& other) = default;

protected:
	std::string getNamePrefix() const;

public:
	std::size_t getSize() const;
	bool hasNormals() const;
	std::vector<glm::vec3> const& getPositions() const
	{
		return positions;
	}
	std::vector<glm::vec3> const& getNormals() const
	{
		return normals;
	}
	std::vector<glm::vec3>& getPositions()
	{
		return positions;
	}
	std::vector<glm::vec3>& getNormals()
	{
		return normals;
	}
	void addPoint(glm::vec3 position, std::optional<glm::vec3> normal = std::nullopt);
	void transform(glm::mat4 t);
	static std::unique_ptr<PointCloud> join(std::vector<std::unique_ptr<PointCloud>>&& meshes);
	PointCloud* decimated(int maxVertices) const;
	Octree* octree(std::size_t preferredVerticesPerNode, int maxDepth) const;
	PointCloudBlock* asBlock() const;
	std::pair<glm::vec3, glm::vec3> const& getBounds() const;
	void drawUI() const;

};
