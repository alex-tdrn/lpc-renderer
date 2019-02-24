#include "PointCloud.h"
#include <algorithm>
#include <imgui.h>
#include <unordered_map>

PointCloud::PointCloud(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals)
	:vertexCount(positions.size())
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

	emptyBrickCount = 0;
	redundantPointsIfCompressed = 0;
	for (auto const& brick : bricks)
	{
		if (brick.positions.empty())
		{
			emptyBrickCount++;
			continue;
		}
		std::unordered_map<std::uint16_t, std::size_t> occurences;
		for (auto const& position : brick.positions)
			occurences[packPosition4(position)]++;
		for (auto n : occurences)
			redundantPointsIfCompressed += n.second - 1;
	}
	pointsPerBrickAverage = 0;
	for (auto const& brick : bricks)
	{
		if (brick.positions.empty())
			continue;
		pointsPerBrickAverage += static_cast<float>(brick.positions.size()) / (bricks.size() - emptyBrickCount);
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
	static int maxSubdivisions = 7;
	ImGui::Text("Total Point Count: %i", vertexCount);
	ImGui::Text("Average Point Count Per Brick: %.2f (need 8)", pointsPerBrickAverage);
	ImGui::Text("Redundant Points if compressed: %i, (%.2f%%)", redundantPointsIfCompressed, 100 * static_cast<float>(redundantPointsIfCompressed) / vertexCount);
	ImGui::SliderInt("Max Subdivisions: ", &maxSubdivisions, 1, 255);
	ImGui::SliderInt3("Subdivisions", &tmpSubdivisions.x, 0, maxSubdivisions);
	glm::clamp(tmpSubdivisions, glm::ivec3{ 0 }, tmpSubdivisions);
	if (tmpSubdivisions != subdivisions)
		setSubDivisions(tmpSubdivisions);
	ImGui::Text("Total Brick Count: %i", bricks.size());
	ImGui::Text("Empty Brick Count: %i, (%.2f%%)", emptyBrickCount, 100 * static_cast<float>(emptyBrickCount) / bricks.size());

}

std::uint16_t packPosition32(glm::vec3 p)
{
	std::uint16_t packed = 32 * p.x;
	packed |= std::uint16_t(32 * p.y) << 5;
	packed |= std::uint16_t(32 * p.z) << 10;
	return packed;
}

std::uint16_t packPosition16(glm::vec3 p)
{
	std::uint16_t packed = 16 * p.x;
	packed |= std::uint16_t(16 * p.y) << 4;
	packed |= std::uint16_t(16 * p.z) << 8;
	return packed;
}

std::uint16_t packPosition8(glm::vec3 p)
{
	std::uint16_t packed = 8 * p.x;
	packed |= std::uint16_t(8 * p.y) << 3;
	packed |= std::uint16_t(8 * p.z) << 6;
	return packed;
}

std::uint16_t packPosition4(glm::vec3 p)
{
	std::uint16_t packed = 4 * p.x;
	packed |= std::uint16_t(4 * p.y) << 2;
	packed |= std::uint16_t(4 * p.z) << 4;
	return packed;
}
