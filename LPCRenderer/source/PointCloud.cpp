#include "PointCloud.h"
#include "imgui.h"

PointCloud::PointCloud(std::vector<glm::vec3>&& positions)
	:positions(std::move(positions)), maxVertices(positions.size())
{

}

int PointCloud::getSize() const
{
	return positions.size();
}

std::vector<glm::vec3> const& PointCloud::getPositions() const
{
	return positions;
}

void PointCloud::transform(glm::mat4 t)
{
	for(auto& position : positions)
		position = t * glm::vec4{position, 1.0f};
	_decimated = nullptr;
	_culled = nullptr;
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
	for(auto& mesh : meshes)
	{
		allPositions.insert(allPositions.end(),
			std::make_move_iterator(mesh->positions.begin()),
			std::make_move_iterator(mesh->positions.end())
		);
	}
	return std::make_unique<PointCloud>(std::move(allPositions));
}

PointCloud* PointCloud::decimated(int maxVertices) const
{
	if(maxVertices != this->maxVertices || !_decimated)
	{
		this->maxVertices = maxVertices;
		if(!_decimated )
			_decimated = std::make_unique<PointCloud>();
		_decimated->positions.clear();
		_decimated->positions.reserve(maxVertices);
		int stride = std::ceil(double(positions.size()) / maxVertices);
		for(std::size_t i = 0; i < positions.size(); i += stride)
			_decimated->positions.push_back(positions[i]);
	}
	return _decimated.get();
}

PointCloud* PointCloud::culled(glm::mat4 mvp) const
{
	if(!_culled)
		_culled = std::make_unique<PointCloud>();
	_culled->positions.clear();
	for(auto const& position : positions)
	{
		glm::vec4 clippedPosition = mvp * glm::vec4{position, 1.0f};
		clippedPosition /= clippedPosition.w;
		if(std::abs(clippedPosition.x) <= 1.0f &&
		   std::abs(clippedPosition.y) <= 1.0f &&
		   std::abs(clippedPosition.z) <= 1.0f)
			_culled->positions.push_back(position);
	}
	return _culled.get();
}


void PointCloud::drawUI() const
{
	ImGui::PushID(this);
	ImGui::Text(getName().data());
	if(ImGui::CollapsingHeader("Vertices"))
	{
		ImGui::Text("%i Total Vertices", positions.size());
		ImGui::BeginChild("###TotalVertices");
		ImGuiListClipper vertexListClipper{static_cast<int>(positions.size())};
		while(vertexListClipper.Step())
			for (int i = vertexListClipper.DisplayStart; i < vertexListClipper.DisplayEnd; i++)
				ImGui::Text("%.3f, %.3f, %.3f", positions[i].x, positions[i].y, positions[i].z);
		ImGui::EndChild();
	}
	if(_decimated && ImGui::CollapsingHeader("Decimated Cloud"))
		_decimated->drawUI();
	if(_culled && ImGui::CollapsingHeader("Culled Cloud"))
		_culled->drawUI();
	ImGui::PopID();
}

