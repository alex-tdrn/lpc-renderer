#pragma once
#include "glm/glm.hpp"

#include <vector>

class PointCloudBrick
{
private:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;

public:
	PointCloudBrick(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals = {})
		: positions(std::move(positions)), normals(std::move(normals))
	{
	};
	PointCloudBrick() = delete;
	PointCloudBrick(PointCloudBrick const&) = default;
	PointCloudBrick(PointCloudBrick&& other) = default;
	~PointCloudBrick() = default;
	PointCloudBrick& operator=(PointCloudBrick const&) = default;
	PointCloudBrick& operator=(PointCloudBrick&& other) = default;

public:
	std::vector<glm::vec3> const& getPositions() const
	{
		return positions;
	}
	std::vector<glm::vec3> const& getNormals() const
	{
		return normals;
	}

};
