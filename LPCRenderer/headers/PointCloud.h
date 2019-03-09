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
	glm::vec3 brickSize;
	std::vector<PointCloudBrick> bricks;
	std::size_t vertexCount = 0;
	mutable std::size_t emptyBrickCount = 0;
	mutable std::size_t redundantPointsIfCompressed = 0;
	mutable float pointsPerBrickAverage = 0;
	mutable std::size_t brickPrecision = 32;

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
	void setBrickPrecision(std::size_t precision) const;
	void updateStatistics() const;
	void setSubDivisions(glm::ivec3 subdivisions);
	bool hasNormals() const;
	std::pair<glm::vec3, glm::vec3> getBounds() const;
	glm::vec3 getSize() const;
	glm::ivec3 getSubdivisions() const;
	glm::vec3 getBrickSize() const;
	std::vector<PointCloudBrick> const& getAllBricks() const;
	PointCloudBrick& getBrickAt(glm::ivec3 indices);
	glm::vec3 convertToWorldPosition(glm::ivec3 indices, glm::vec3 localPosition) const;
	std::pair<glm::vec3, glm::vec3> getBoundsAt(glm::ivec3 indices) const;
	glm::vec3 getOffsetAt(glm::ivec3 indices) const;

	std::unique_ptr<PointCloud> decimate(std::size_t maxPoints) const; 
	void drawUI();

};

std::uint32_t packPosition1024(glm::vec3);
std::uint16_t packPosition32(glm::vec3);
std::uint16_t packPosition16(glm::vec3);
std::uint16_t packPosition8(glm::vec3);
std::uint16_t packPosition4(glm::vec3);