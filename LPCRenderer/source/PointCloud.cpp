#include "PointCloud.h"
#include <algorithm>

PointCloud::PointCloud(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals)
{
	for (int i = 0; i < 3; i++)
	{
		bounds.first[i] = +std::numeric_limits<float>::max();
		bounds.second[i] = -std::numeric_limits<float>::max();
	}
	for (auto const& position : positions)
	{
		for (int i = 0; i < 3; i++)
		{
			bounds.first[i] = std::min(bounds.first[i], position[i]);
			bounds.second[i] = std::max(bounds.second[i], position[i]);
		}
	}
	_hasNormals = !normals.empty();
	bricks.push_back(PointCloudBrick{ { 0, 0, 0 }, std::move(positions), std::move(normals) });
}

std::string PointCloud::getNamePrefix() const
{
	return "PointCloud";
}


void PointCloud::setSubDivisions(glm::ivec3 subdivisions)
{
	this->subdivisions = subdivisions;
	//TODO
}

bool PointCloud::hasNormals() const
{
	return _hasNormals;
}

std::pair<glm::vec3, glm::vec3> PointCloud::getBounds() const
{
	return bounds;
}

glm::vec3 PointCloud::getSize() const
{
	return bounds.second - bounds.first;
}

glm::vec3 PointCloud::getBrickSize() const
{
	return getSize() / glm::vec3(subdivisions + 1);
}

std::vector<PointCloudBrick> const& PointCloud::getAllBricks() const
{
	return bricks;
}

PointCloudBrick const& PointCloud::getBrickAt(glm::ivec3 indices) const
{
	int idx = 0;
	for (int i = 0; i < 3; i++)
		idx += indices[i] * subdivisions[i];
	return bricks[idx];
}

std::pair<glm::vec3, glm::vec3> PointCloud::getBoundsAt(glm::ivec3 indices) const
{
	auto brickSize = getBrickSize();
	auto first = brickSize * glm::vec3(indices) + bounds.first;
	return { first, first + brickSize };
}

glm::vec3 PointCloud::getOffsetAt(glm::ivec3 indices) const
{
	return getBrickSize() * glm::vec3(indices);
}

void PointCloud::drawUI()
{
}

