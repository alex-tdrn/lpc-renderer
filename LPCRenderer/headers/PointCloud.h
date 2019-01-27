#pragma once
#include "AutoName.h"
#include "glm/glm.hpp"
#include <vector>

struct PointCloudBrick
{
	glm::ivec3 indices;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
};

class PointCloud : public AutoName<PointCloud>
{
private:
	std::pair<glm::vec3, glm::vec3> bounds;
	bool _hasNormals = false;
	glm::ivec3 subdivisions{0};
	std::vector<PointCloudBrick> bricks;

public:
	PointCloud(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals = {});
	PointCloud() = delete;
	PointCloud(PointCloud const& other) = default;
	PointCloud(PointCloud&& other) = default;
	~PointCloud() = default;
	PointCloud& operator=(PointCloud const& other) = default;
	PointCloud& operator=(PointCloud&& other) = default;

protected:
	std::string getNamePrefix() const;

public:
	void setSubDivisions(glm::ivec3 subdivisions);
	bool hasNormals() const;
	std::pair<glm::vec3, glm::vec3> getBounds() const;
	glm::vec3 getSize() const;
	glm::vec3 getBrickSize() const;
	std::vector<PointCloudBrick> const& getAllBricks() const;
	PointCloudBrick const& getBrickAt(glm::ivec3 indices) const;
	std::pair<glm::vec3, glm::vec3> getBoundsAt(glm::ivec3 indices) const;
	glm::vec3 getOffsetAt(glm::ivec3 indices) const;
	void drawUI();

};