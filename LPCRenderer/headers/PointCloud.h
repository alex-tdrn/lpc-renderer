#pragma once
#include "AutoName.h"
#include "glm/glm.hpp"

#include <vector>
#include <memory>

class PointCloud : public AutoName<PointCloud>
{
	friend class std::unique_ptr<PointCloud>;

private:
	std::vector<glm::vec3> positions;
	mutable int maxVertices = 0;
	mutable std::unique_ptr<PointCloud> _decimated = nullptr;
	mutable std::unique_ptr<PointCloud> _culled = nullptr;

public:
	PointCloud() = default;
	PointCloud(std::vector<glm::vec3>&& positions);

protected:
	std::string getNamePrefix() const;

public:
	int getSize() const;
	std::vector<glm::vec3> const& getPositions() const;
	void transform(glm::mat4 t);
	static std::unique_ptr<PointCloud> join(std::vector<std::unique_ptr<PointCloud>>&& meshes);
	PointCloud* decimated(int maxVertices) const;
	PointCloud* culled(glm::mat4 mvp) const;
	void drawUI() const;

};
