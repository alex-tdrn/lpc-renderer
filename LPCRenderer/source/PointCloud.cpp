#include "PointCloud.h"
#include "imgui.h"

PointCloud::PointCloud(std::vector<glm::vec3>&& positions, std::vector<glm::vec3>&& normals)
	:positions(std::move(positions)), maxVertices(positions.size()), normals(normals)
{

}

int PointCloud::getSize() const
{
	return positions.size();
}

bool PointCloud::hasNormals() const
{
	return !normals.empty();
}

std::vector<glm::vec3> const& PointCloud::getPositions() const
{
	return positions;
}

std::vector<glm::vec3> const& PointCloud::getNormals() const
{
	return normals;
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
	if(maxVertices != this->maxVertices || !_decimated)
	{
		this->maxVertices = maxVertices;
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
	}
	return _decimated.get();
}

PointCloud* PointCloud::culled(glm::mat4 mvp) const
{
	if(!_culled)
		_culled = std::make_unique<PointCloud>();
	_culled->positions.clear();
	_culled->normals.clear();
	for(std::size_t i = 0; i < positions.size(); i++)
	{
		glm::vec4 clippedPosition = mvp * glm::vec4{positions[i], 1.0f};
		clippedPosition /= clippedPosition.w;
		if(std::abs(clippedPosition.x) <= 1.0f &&
		   std::abs(clippedPosition.y) <= 1.0f &&
		   std::abs(clippedPosition.z) <= 1.0f)
		{
			_culled->positions.push_back(positions[i]);
			if(hasNormals())
				_decimated->normals.push_back(normals[i]);
		}
	}
	return _culled.get();
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
	if(_culled && ImGui::CollapsingHeader("Culled Cloud"))
	{
		ImGui::Indent();
		_culled->drawUI();
		ImGui::Unindent();
	}
	ImGui::PopID();
}

