#include "PointCloud.h"
#include <algorithm>
#include <imgui.h>

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
	brickSize = getSize() / glm::vec3(subdivisions + 1);
	for (auto& position : positions)
		position = glm::fract((position - bounds.first) / brickSize);
	bricks.push_back(PointCloudBrick{ { 0, 0, 0 }, std::move(positions), std::move(normals) });
}

std::string PointCloud::getNamePrefix() const
{
	return "PointCloud";
}

void PointCloud::setSubDivisions(glm::ivec3 subdivisions)
{
	this->subdivisions = subdivisions;
	std::vector<PointCloudBrick> oldBricks;
	for (int k = 0; k <= subdivisions.z; k++)
		for (int j = 0; j <= subdivisions.y; j++)
		for (int i = 0; i <= subdivisions.x; i++)
				oldBricks.push_back(PointCloudBrick{ { i, j, k } });
	std::swap(oldBricks, bricks);
	glm::vec3 oldBrickSize = brickSize;
	brickSize = getSize() / glm::vec3(subdivisions + 1);

	for (auto const& brick : oldBricks)
	{
		for (int i = 0; i < brick.positions.size(); i++)
		{
			glm::vec3 globalPosition = oldBrickSize * (brick.positions[i] + glm::vec3(brick.indices));
			glm::vec3 relativePosition = globalPosition / brickSize;
			glm::ivec3 indices = glm::floor(relativePosition);
			relativePosition = glm::fract(relativePosition);
			getBrickAt(indices).positions.push_back(relativePosition);
			if (_hasNormals)
				getBrickAt(indices).normals.push_back(brick.normals[i]);
		}
	}
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

glm::ivec3 PointCloud::getSubdivisions() const
{
	return subdivisions;
}

glm::vec3 PointCloud::getBrickSize() const
{
	return brickSize;
}

std::vector<PointCloudBrick> const& PointCloud::getAllBricks() const
{
	return bricks;
}

PointCloudBrick& PointCloud::getBrickAt(glm::ivec3 indices)
{
	int idx = 0;
	idx += indices.x;//jump points
	idx += indices.y * (subdivisions.x + 1);//jump lines
	idx += indices.z * (subdivisions.x + 1) * (subdivisions.y + 1);//jump surfaces

	return bricks[idx];
}

glm::vec3 PointCloud::convertToWorldPosition(glm::ivec3 indices, glm::vec3 localPosition) const
{
	return bounds.first + getOffsetAt(indices) + localPosition * brickSize;
}

std::pair<glm::vec3, glm::vec3> PointCloud::getBoundsAt(glm::ivec3 indices) const
{
	auto first = bounds.first + getOffsetAt(indices);
	return { first, first + brickSize };
}

glm::vec3 PointCloud::getOffsetAt(glm::ivec3 indices) const
{
	return brickSize * glm::vec3(indices);
}

void PointCloud::drawUI()
{
	glm::ivec3 tmpSubdivisions = subdivisions;
	static int maxSubdivisions = 8;
	ImGui::SliderInt("Max Subdivisions: ", &maxSubdivisions, 1, 255);
	ImGui::SliderInt3("Subdivisions", &tmpSubdivisions.x, 0, maxSubdivisions);
	glm::clamp(tmpSubdivisions, glm::ivec3{ 0 }, tmpSubdivisions);
	if (tmpSubdivisions != subdivisions)
		setSubDivisions(tmpSubdivisions);
}

