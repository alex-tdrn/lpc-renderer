#pragma once
#include "AutoName.h"
#include "PointCloudBrick.h"
#include <vector>

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
	std::vector<PointCloudBrick> const& getBricks() const;
	std::pair<glm::vec3, glm::vec3> getBounds() const;
	void drawUI();

};