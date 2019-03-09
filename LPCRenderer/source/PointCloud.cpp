#include "PointCloud.h"
#include <algorithm>
#include <imgui.h>
#include <unordered_map>
#include <PCManager.h>
#include <Scene.h>
#include <SceneManager.h>
#include "GPUBuffer.h"

PointCloud::PointCloud(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals, std::vector<glm::u8vec3>&& colors)
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
	_hasColors = !colors.empty();
	brickSize = getSize() / glm::vec3(subdivisions + 1);
	for (auto& position : positions)
		position = glm::fract((position - bounds.first) / brickSize);
	bricks.push_back(PointCloudBrick{ { 0, 0, 0 }, std::move(positions), std::move(normals), std::move(colors)});
}

std::string PointCloud::getNamePrefix() const
{
	return "PointCloud";
}

void PointCloud::setBrickPrecision(std::size_t precision) const
{
	this->brickPrecision = precision;
	updateStatistics();
}

void PointCloud::updateStatistics() const
{
	emptyBrickCount = 0;
	redundantPointsIfCompressed = 0;
	for(auto const& brick : bricks)
	{
		if(brick.positions.empty())
		{
			emptyBrickCount++;
			continue;
		}
		std::unordered_map<std::uint32_t, std::size_t> occurences;
		for(auto const& position : brick.positions)
		{
			switch(brickPrecision)
			{
				case 1024:
					occurences[packPosition1024(position)]++;
					break;
				case 32:
					occurences[packPosition32(position)]++;
					break;
				case 16:
					occurences[packPosition16(position)]++;
					break;
				case 8:
					occurences[packPosition8(position)]++;
					break;
				case 4:
					occurences[packPosition4(position)]++;
					break;
			}
		}
		for(auto n : occurences)
			redundantPointsIfCompressed += n.second - 1;
	}
	pointsPerBrickAverage = 0;
	for(auto const& brick : bricks)
	{
		if(brick.positions.empty())
			continue;
		pointsPerBrickAverage += static_cast<float>(brick.positions.size()) / (bricks.size() - emptyBrickCount);
	}
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
			if(_hasColors)
				getBrickAt(indices).colors.push_back(brick.colors[i]);
		}
	}

	updateStatistics();
}

bool PointCloud::hasNormals() const
{
	return _hasNormals;
}

bool PointCloud::hasColors() const
{
	return _hasColors;
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

std::unique_ptr<PointCloud> PointCloud::decimate(std::size_t maxPoints) const
{
	int stride = std::max(1ULL, vertexCount / maxPoints);
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::u8vec3> colors;

	positions.reserve(vertexCount);
	if(hasNormals())
		normals.reserve(vertexCount);
	if(hasColors())
		colors.reserve(vertexCount);

	for(auto const& brick : bricks)
	{
		for(auto const& position : brick.positions)
			positions.push_back(convertToWorldPosition(brick.indices, position));
		for(auto const& normal : brick.normals)
			normals.push_back(normal);
		for(auto const& color : brick.colors)
			colors.push_back(color);
	}

	std::vector<glm::vec3> decimatedPositions;
	std::vector<glm::vec3> decimatedNormals;
	std::vector<glm::u8vec3> decimatedColors;
	decimatedPositions.reserve(maxPoints);
	
	if(hasNormals())
		decimatedNormals.reserve(maxPoints);
	if(hasColors())
		decimatedColors.reserve(maxPoints);

	for(int i = 0; i < positions.size(); i += stride)
	{
		decimatedPositions.push_back(positions[i]);
		if(hasNormals())
			decimatedNormals.push_back(normals[i]);
		if(hasColors())
			decimatedColors.push_back(colors[i]);
	}
	return std::make_unique<PointCloud>(std::move(decimatedPositions), std::move(decimatedNormals), std::move(decimatedColors));
}

void PointCloud::drawUI()
{
	glm::ivec3 tmpSubdivisions = subdivisions;
	static int maxSubdivisions = 7;
	ImGui::Text("Total Point Count: %i", vertexCount);
	if(hasNormals())
		ImGui::Text("Has Normals");
	if(hasColors())
		ImGui::Text("Has Colors");

	ImGui::Text("Memory Consumption: ");
	ImGui::Text("    -Positions ");
	ImGui::SameLine();
	drawMemoryConsumption(vertexCount * sizeof(glm::vec3));
	if(hasNormals())
	{
		ImGui::Text("    -Normals ");
		ImGui::SameLine();
		drawMemoryConsumption(vertexCount * sizeof(glm::vec3));
	}
	if(hasColors())
	{
		ImGui::Text("    -Colors ");
		ImGui::SameLine();
		drawMemoryConsumption(vertexCount * sizeof(glm::u8vec3));
	}

	switch(brickPrecision)
	{
		case 32:
			ImGui::Text("Average Point Count Per Brick: %.2f (need 2048)", pointsPerBrickAverage);
			break;
		case 16:
			ImGui::Text("Average Point Count Per Brick: %.2f (need 256)", pointsPerBrickAverage);
			break;
		case 8:
			ImGui::Text("Average Point Count Per Brick: %.2f (need 32)", pointsPerBrickAverage);
			break;
		case 4:
			ImGui::Text("Average Point Count Per Brick: %.2f (need 8)", pointsPerBrickAverage);
			break;
	}
	ImGui::Text("Redundant Points if compressed: %i, (%.2f%%)", redundantPointsIfCompressed, 100 * static_cast<float>(redundantPointsIfCompressed) / vertexCount);
	ImGui::SliderInt("Max Subdivisions: ", &maxSubdivisions, 1, 255);
	ImGui::SliderInt3("Subdivisions", &tmpSubdivisions.x, 0, maxSubdivisions);
	glm::clamp(tmpSubdivisions, glm::ivec3{ 0 }, tmpSubdivisions);
	if (tmpSubdivisions != subdivisions)
		setSubDivisions(tmpSubdivisions);
	ImGui::Text("Total Brick Count: %i", bricks.size());
	ImGui::Text("Empty Brick Count: %i, (%.2f%%)", emptyBrickCount, 100 * static_cast<float>(emptyBrickCount) / bricks.size());

	static int decimatePointCount = 100'000;
	ImGui::InputInt("Decimate Max Points: ", &decimatePointCount);
	if(ImGui::Button("Decimate"))
	{
		auto cloud = PCManager::add(decimate(decimatePointCount));
		cloud->setName(getName() + "(decimated)");
		SceneManager::add(std::make_unique<Scene>(cloud));
	}
}

std::uint32_t packPosition1024(glm::vec3 p)
{
	std::uint32_t packed = 1024 * p.x;
	packed |= std::uint32_t(1024 * p.y) << 10;
	packed |= std::uint32_t(1024 * p.z) << 20;
	return packed;
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
