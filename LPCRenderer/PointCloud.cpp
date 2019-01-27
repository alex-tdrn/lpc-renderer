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
	bricks.push_back(PointCloudBrick(std::move(positions), std::move(normals)));
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

std::vector<PointCloudBrick> const& PointCloud::getBricks() const
{
	return bricks;
}

std::pair<glm::vec3, glm::vec3> PointCloud::getBounds() const
{
	return bounds;
}

void PointCloud::drawUI()
{
}

