#include "PointCloud.h"
#include "imgui.h"

#include <algorithm>

PointCloud::PointCloud(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals)
	:positions(std::move(positions)), maxVerticesDecimated(positions.size()), normals(normals)
{

}

PointCloud::PointCloud(PointCloud const& other)
	: positions(other.positions), normals(other.normals)
{
}

PointCloud& PointCloud::operator=(PointCloud const& other)
{
	positions = other.positions;
	normals = other.normals;
	maxVerticesDecimated = 0;
	_decimated = nullptr;
	boundsOutOfDate = true;
	return *this;
}

int PointCloud::getSize() const
{
	return positions.size();
}

bool PointCloud::hasNormals() const
{
	return !normals.empty();
}

void PointCloud::addPoint(glm::vec3 position, std::optional<glm::vec3> normal)
{
	if(hasNormals())
		assert(normal.has_value());
	positions.push_back(position);
	if(hasNormals())
		normals.push_back(*normal);
	maxVerticesDecimated = 0;
	_decimated = nullptr;
	boundsOutOfDate = true;
}

void PointCloud::transform(glm::mat4 t)
{
	for(auto& position : positions)
		position = t * glm::vec4{position, 1.0f};
	_decimated = nullptr;
	boundsOutOfDate = true;
}

std::string PointCloud::getNamePrefix() const
{
	return "PointCloud";
}

std::unique_ptr<PointCloud> PointCloud::join(std::vector<std::unique_ptr<PointCloud>>&& meshes)
{
	std::size_t verticesCount = 0;
	for(auto const& mesh : meshes)
		verticesCount += mesh->positions.size();
	std::vector<glm::vec3> allPositions;
	allPositions.reserve(verticesCount);
	bool useNormals = true;
	for(auto& mesh : meshes)
	{
		allPositions.insert(allPositions.end(),
			std::make_move_iterator(mesh->positions.begin()),
			std::make_move_iterator(mesh->positions.end())
		);
		useNormals = useNormals && mesh->hasNormals();
	}
	std::vector<glm::vec3> allNormals;
	if(useNormals)
	{
		allNormals.reserve(verticesCount);
		for(auto& mesh : meshes)
		{
			allNormals.insert(allNormals.end(),
				std::make_move_iterator(mesh->normals.begin()),
				std::make_move_iterator(mesh->normals.end())
			);
		}
	}
	return std::make_unique<PointCloud>(std::move(allPositions), std::move(allNormals));
}

PointCloud* PointCloud::decimated(int maxVertices) const
{
	if(maxVertices != maxVerticesDecimated || !_decimated)
	{
		maxVerticesDecimated = maxVertices;
		if(!_decimated )
			_decimated = std::make_unique<PointCloud>();
		_decimated->positions.clear();
		_decimated->positions.reserve(maxVertices);
		_decimated->normals.clear();
		if(hasNormals())
			_decimated->normals.reserve(maxVertices);
		int stride = std::ceil(double(positions.size()) / maxVertices);
		for(std::size_t i = 0; i < positions.size(); i += stride)
		{
			_decimated->positions.push_back(positions[i]);
			if(hasNormals())
				_decimated->normals.push_back(normals[i]);
		}
		_decimated->_octree = nullptr;
	}
	return _decimated.get();
}

Octree* PointCloud::octree(std::size_t maxVerticesPerNode, int maxDepth) const
{
	if(!_octree || maxVerticesPerNode != _octree->getMaxVerticesPerNode() || maxDepth != _octree->getMaxDepth())
	{
		if(!_octree)
			_octree = std::make_unique<Octree>(*this);
		_octree->update(maxDepth, maxVerticesPerNode);
	}
	return _octree.get();
}


std::pair<glm::vec3, glm::vec3> const& PointCloud::getBounds() const
{
	if(boundsOutOfDate)
	{
		bounds.first = {std::numeric_limits<float>::max()};
		bounds.second = -bounds.first;
		for(auto const& p : positions)
		{
			for(int i = 0; i < 3; i++)
			{
				bounds.first[i] = std::min(bounds.first[i], p[i]);
				bounds.second[i] = std::max(bounds.second[i], p[i]);
			}
		}
		boundsOutOfDate = false;
	}
	return bounds;
}


void PointCloud::drawUI() const
{
	ImGui::PushID(this);
	ImGui::Text(getName().data());
	ImGui::Text("%i Total Vertices", positions.size());
	if(ImGui::CollapsingHeader("Positions"))
	{
		ImGui::BeginChild("###Positions");
		ImGuiListClipper vertexListClipper{static_cast<int>(positions.size())};
		while(vertexListClipper.Step())
			for (int i = vertexListClipper.DisplayStart; i < vertexListClipper.DisplayEnd; i++)
				ImGui::Text("%.3f, %.3f, %.3f", positions[i].x, positions[i].y, positions[i].z);
		ImGui::EndChild();
		if(hasNormals())
		{
			ImGui::BeginChild("###Normals");
			ImGuiListClipper vertexListClipper{static_cast<int>(normals.size())};
			while(vertexListClipper.Step())
				for(int i = vertexListClipper.DisplayStart; i < vertexListClipper.DisplayEnd; i++)
					ImGui::Text("%.3f, %.3f, %.3f", normals[i].x, normals[i].y, normals[i].z);
			ImGui::EndChild();
		}
	}
	if(_decimated && ImGui::CollapsingHeader("Decimated Cloud"))
	{
		ImGui::Indent();
		_decimated->drawUI();
		ImGui::Unindent();
	}
	ImGui::PopID();
}

