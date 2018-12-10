#include "Octree.h"
#include "PointCloud.h"
#include "glm/gtc/matrix_transform.hpp"

#include <array>

Octree::Octree(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals,
	std::pair<glm::vec3, glm::vec3> bounds, int depth,
	int maxDepth, std::size_t maxVerticesPerNode)
	:bounds(bounds), currentDepth(depth), maxDepth(maxDepth), 
	maxVerticesPerNode(maxVerticesPerNode), totalVerticesCount(positions.size())
{
	if(currentDepth < maxDepth && positions.size() > maxVerticesPerNode)
	{
		isLeaf = false;
		split(std::move(positions), std::move(normals));
	}
	else
	{
		isLeaf = true;
		cloud = new PointCloud(std::move(positions), std::move(normals));
	}
}

Octree::Octree(PointCloud const& cloud)
{
	isLeaf = true;
	this->cloud = new PointCloud(cloud);
	totalVerticesCount = cloud.getSize();
}

Octree::Octree(Octree&& other)
	:isLeaf(other.isLeaf), children(std::move(other.children)), cloud(other.cloud),
	bounds(std::move(other.bounds)), maxVerticesPerNode(other.maxVerticesPerNode), 
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
	/*positions.clear();
	positions.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();*/
	children = std::vector<Octree>{};
	for(auto i = 0; i < 8; i++)
	{
		children->push_back({std::move(childrenPositions[i]), std::move(childrenNormals[i]),
			childrenBounds[i], currentDepth + 1, maxDepth, maxVerticesPerNode});
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

	children = std::nullopt;
	isLeaf = true;
	cloud = new PointCloud(std::move(positions), std::move(normals));
}

void Octree::getAllLeafNodes(std::vector<Octree const*>& leafNodes) const
{
	if(isLeaf && cloud->getSize() > 0)
		leafNodes.push_back(this);
	else if(!isLeaf)
		for(auto const& childNode : *children)
			childNode.getAllLeafNodes(leafNodes);
}

void Octree::getAllPointClouds(std::vector<PointCloud const*>& pointClouds) const
{
	if(isLeaf && cloud->getSize() > 0)
		pointClouds.push_back(cloud);
	else if(!isLeaf)
		for(auto const& childNode : *children)
			childNode.getAllPointClouds(pointClouds);
}

void Octree::getPointCloudsInsideFrustum(std::vector<PointCloud const*>& pointClouds, glm::mat4 mvp) const
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
				if(testOverlap())//conservatively let the box pass
				{
					if(isLeaf)
						pointClouds.push_back(cloud);
					else
						for(auto const& childNode : *children)
							childNode.getPointCloudsInsideFrustum(pointClouds, mvp);
					return;
				}
			}
		}
	}
}

glm::mat4 Octree::getBoundsTransform() const
{
	auto bounds = this->bounds;
	glm::vec3 center = (bounds.first + bounds.second) / 2.0f;
	bounds.first -= center;
	glm::vec3 scale = -bounds.first;

	glm::mat4 t = glm::translate(glm::mat4{1.0f}, center);
	glm::mat4 s = glm::scale(glm::mat4{1.0f}, glm::vec3{scale});
	return t * s;
}

void Octree::update(int maxDepth, std::size_t maxVerticesPerNode)
{
	this->maxDepth = maxDepth;
	this->maxVerticesPerNode = maxVerticesPerNode;
	if(isLeaf)
	{
		if(currentDepth < maxDepth && cloud->getSize() > maxVerticesPerNode)
			split(std::move(cloud->getPositions()), std::move(cloud->getNormals()));
	}
	else
	{
		if(currentDepth >= maxDepth || getTotalVerticesCount() <= maxVerticesPerNode)
			join();
		else
			for(auto& childNode : *children)
				childNode.update(maxDepth, maxVerticesPerNode);
	}
}

std::size_t Octree::getMaxVerticesPerNode() const
{
	return maxVerticesPerNode;
}

std::size_t Octree::getTotalVerticesCount() const
{
	return totalVerticesCount;
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
	return float(getVerticesCount()) / maxVerticesPerNode;
}

int Octree::getMaxDepth() const
{
	return maxDepth;
}

std::pair<glm::vec3, glm::vec3> Octree::getBounds() const
{
	return bounds;
}

//PointCloud* PointCloud::culled(glm::mat4 mvp) const
//{
//	if(!_culled)
//		_culled = std::make_unique<PointCloud>();
//	_culled->positions.clear();
//	_culled->normals.clear();
//	for(std::size_t i = 0; i < positions.size(); i++)
//	{
//		glm::vec4 clippedPosition = mvp * glm::vec4{positions[i], 1.0f};
//		clippedPosition /= clippedPosition.w;
//		if(std::abs(clippedPosition.x) <= 1.0f &&
//		   std::abs(clippedPosition.y) <= 1.0f &&
//		   std::abs(clippedPosition.z) <= 1.0f)
//		{
//			_culled->positions.push_back(positions[i]);
//			if(hasNormals())
//				_culled->normals.push_back(normals[i]);
//		}
//	}
//	return _culled.get();
//}