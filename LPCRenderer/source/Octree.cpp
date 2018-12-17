#include "Octree.h"
#include "PointCloud.h"
#include "glm/gtc/matrix_transform.hpp"
#include "OSWindow.h"

#include <array>

Octree::Octree(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals,
	std::pair<glm::vec3, glm::vec3> bounds, int depth,
	int maxDepth, std::size_t preferredVerticesPerNode)
	:bounds(bounds), currentDepth(depth), maxDepth(maxDepth), 
	preferredVerticesPerNode(preferredVerticesPerNode), totalVerticesCount(positions.size())
{
	if(currentDepth < maxDepth && positions.size() > preferredVerticesPerNode)
	{
		isLeaf = false;
		split(std::move(positions), std::move(normals));
	}
	else
	{
		isLeaf = true;
		cloud = new PointCloud(std::move(positions), std::move(normals));
	}
	std::vector<Octree const*> leafNodes;
	getAllLeafNodes(leafNodes);
	totalLeafNodesCount = leafNodes.size();
}

Octree::Octree(PointCloud const& cloud)
{
	isLeaf = true;
	this->cloud = new PointCloud(cloud);
	totalVerticesCount = cloud.getSize();
}

Octree::Octree(Octree&& other)
	:isLeaf(other.isLeaf), children(std::move(other.children)), cloud(other.cloud),
	bounds(std::move(other.bounds)), preferredVerticesPerNode(other.preferredVerticesPerNode), 
	currentDepth(other.currentDepth), maxDepth(other.maxDepth), totalVerticesCount(other.totalVerticesCount)
{
	other.cloud = nullptr;
}

Octree::~Octree()
{
	if(cloud)
		delete cloud;
}

void Octree::split(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals)
{
	glm::vec3 center = (bounds.first + bounds.second) / 2.0f;
	glm::vec3 childDiagonal = (bounds.second - bounds.first) / 2.0f;
	float sx = childDiagonal.x;
	float sy = childDiagonal.y;
	float sz = childDiagonal.z;
	auto childrenBounds = std::array<std::pair<glm::vec3, glm::vec3>, 8>{};

	int idx = 0;
	for(float x : {center.x - sx, center.x})
		for(float y : {center.y - sy, center.y})
			for(float z : {center.z - sz, center.z})
				childrenBounds[idx++] = {glm::vec3{x, y, z}, glm::vec3{x, y, z} +childDiagonal};

	auto childrenPositions = std::array<std::vector<glm::vec3>, 8>{};
	auto childrenNormals = std::array<std::vector<glm::vec3>, 8>{};
	for(auto i = 0; i < positions.size(); i++)
	{
		auto p = positions[i];
		for(auto j = 0; j < 8; j++)
		{
			bool inside = true;
			for(int coord = 0; coord < 3; coord++)
			{
				if(p[coord] < childrenBounds[j].first[coord] || p[coord] > childrenBounds[j].second[coord])
				{
					inside = false;
					break;
				}
			}
			if(inside)
			{
				childrenPositions[j].push_back(p);
				if(!normals.empty())
					childrenNormals[j].push_back(normals[i]);
				break;
			}
		}
	}
	isLeaf = false;
	if(cloud)
	{
		delete cloud;
		cloud = nullptr;
	}
	for(auto i = 0; i < 8; i++)
	{
		children.push_back({std::move(childrenPositions[i]), std::move(childrenNormals[i]),
			childrenBounds[i], currentDepth + 1, maxDepth, preferredVerticesPerNode});
	}
}

void Octree::join()
{
	assert(!isLeaf);

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::size_t nVertices = 0;
	std::vector<Octree const*> leafNodes;
	getAllLeafNodes(leafNodes);
	bool hasNormals = false;
	for(auto &leafNode : leafNodes)
	{
		nVertices += leafNode->cloud->getSize();
		if(leafNode->cloud->hasNormals())
			hasNormals = true;
	}
	positions.reserve(nVertices);

	for(auto &leafNode : leafNodes)
	{
		for(auto p : leafNode->cloud->getPositions())
			positions.push_back(p);
	}

	if(hasNormals)
	{
		normals.reserve(nVertices);
		for(auto &leafNode : leafNodes)
		{
			for(auto p : leafNode->cloud->getNormals())
				normals.push_back(p);
		}
	}

	children.clear();
	isLeaf = true;
	cloud = new PointCloud(std::move(positions), std::move(normals));
}

void Octree::getAllLeafNodes(std::vector<Octree const*>& leafNodes) const
{
	if(isLeaf && cloud->getSize() > 0)
		leafNodes.push_back(this);
	else if(!isLeaf)
		for(auto const& childNode : children)
			childNode.getAllLeafNodes(leafNodes);
}

void Octree::getAllPointClouds(std::vector<PointCloud const*>& pointClouds) const
{
	if(isLeaf && cloud->getSize() > 0)
		pointClouds.push_back(cloud);
	else if(!isLeaf)
		for(auto const& childNode : children)
			childNode.getAllPointClouds(pointClouds);
}

void Octree::getPointCloudsInsideFrustum(std::vector<PointCloud const*>& pointClouds, glm::mat4 mvp, int LODPixelArea, std::size_t LODVertices) const
{
	if(isLeaf && cloud->getSize() == 0)
		return;
	glm::vec3 center = (bounds.first + bounds.second) / 2.0f;
	float s = (bounds.second.x - bounds.first.x) / 2.0f;

	glm::vec3 minAxisProjection;
	glm::vec3 maxAxisProjection;
	auto testOverlap = [&minAxisProjection, &maxAxisProjection]() -> bool{
		for(int i = 0; i < 3; i++)
		{
			float min = minAxisProjection[i];
			float max = maxAxisProjection[i];
			if(min >= -1.0f && min <= +1.0f
				|| max >= -1.0f && max <= +1.0f
				|| min <= +1.0f && max >= -1.0f)
				continue;
			else
				return false;
		}
		return true;

	};
	bool firstCorner = true;
	for(float x : {center.x - s, center.x + s})
	{
		for(float y : {center.y - s, center.y + s})
		{
			for(float z : {center.z - s, center.z + s})
			{
				glm::vec4 clippedCorner = mvp * glm::vec4{x, y, z, 1.0f};
				clippedCorner /= clippedCorner.w;
				if(firstCorner)
				{
					minAxisProjection = clippedCorner;
					maxAxisProjection = clippedCorner;
					firstCorner = false;
				}
				else
				{
					for(int i = 0; i < 3; i++)
					{
						minAxisProjection[i] = std::min(minAxisProjection[i], clippedCorner[i]);
						maxAxisProjection[i] = std::max(maxAxisProjection[i], clippedCorner[i]);
					}
				}
				if((LODPixelArea == 0 || !isLeaf) && testOverlap())//conservatively let the box pass, LOD disabled
				{
					if(isLeaf && cloud->getSize() != 0)
						pointClouds.push_back(cloud);
					else
						for(auto const& childNode : children)
							childNode.getPointCloudsInsideFrustum(pointClouds, mvp, LODPixelArea, LODVertices);
					return;
				}
			}
		}
	}
	if(testOverlap())
	{
		if(LODVertices == 0)
			return;
		glm::vec2 pixels = glm::vec2(OSWindow::getSize()) * glm::vec2(maxAxisProjection - minAxisProjection);
		PointCloud const* ret;
		if(pixels.x * pixels.y <= LODPixelArea)
			ret = cloud->decimated(LODVertices);
		else
			ret = cloud;
		if(ret->getSize() != 0)
			pointClouds.push_back(ret);
	}
}



void Octree::update(int maxDepth, std::size_t preferredVerticesPerNode)
{
	this->maxDepth = maxDepth;
	this->preferredVerticesPerNode = preferredVerticesPerNode;
	if(isLeaf)
	{
		if(currentDepth < maxDepth && cloud->getSize() > preferredVerticesPerNode)
			split(std::move(cloud->getPositions()), std::move(cloud->getNormals()));
	}
	else
	{
		if(currentDepth >= maxDepth || getTotalVerticesCount() <= preferredVerticesPerNode)
			join();
		else
			for(auto& childNode : children)
				childNode.update(maxDepth, preferredVerticesPerNode);
	}
	std::vector<Octree const*> leafNodes;
	getAllLeafNodes(leafNodes);
	totalLeafNodesCount = leafNodes.size();
}

std::size_t Octree::getPrefferedVerticesPerNode() const
{
	return preferredVerticesPerNode;
}

std::size_t Octree::getTotalVerticesCount() const
{
	return totalVerticesCount;
}

std::size_t Octree::getTotalLeafNodesCount() const
{
	return totalLeafNodesCount;
}

std::size_t Octree::getVerticesCount() const
{
	if(cloud)
		return cloud->getSize();
	else
		return 0;
}

float Octree::getOccupancy() const
{
	return float(getVerticesCount()) / preferredVerticesPerNode;
}

int Octree::getMaxDepth() const
{
	return maxDepth;
}

int Octree::getDepth() const
{
	return currentDepth;
}

std::pair<glm::vec3, glm::vec3> Octree::getBounds() const
{
	return bounds;
}

PointCloud const * Octree::getPointCloud() const
{
	return cloud;
}
