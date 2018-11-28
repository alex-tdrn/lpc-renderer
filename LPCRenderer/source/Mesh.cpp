#include "Mesh.h"
#include "imgui.h"

Mesh::Mesh(std::vector<Vertex>&& vertices)
	:vertices(std::move(vertices)), representation(this->vertices)
{
	updateRepresentation();
}

std::vector<Vertex> cullAgainst(glm::mat4 mvp, std::vector<Vertex> const& vertices, std::size_t n)
{
	std::vector<Vertex> ret;
	ret.reserve(1'000);
	for(int i = 0; i < n; i++)
	{
		glm::vec4 aux = mvp * glm::vec4{vertices[i].position, 1.0f};
		Vertex v{glm::vec3{aux.x, aux.y, aux.z} / aux.w};
		if(std::abs(v.position.x) <= 1.0f && std::abs(v.position.y) <= 1.0f && std::abs(v.position.z) <= 1.0f)
			ret.push_back(vertices[i]);
	}
	return ret;
}

void Mesh::updateRepresentation()
{
	if(maxVertices < 1)
		maxVertices = 1;
	if(filtering && maxVertices < vertices.size())
	{
		if(pickVerticesSequentially)
		{
			if(cull)
				representation = MeshRepresentation(cullAgainst(cullMatrix, vertices, maxVertices));
			else
				representation = MeshRepresentation(vertices, maxVertices);
			return;
		}
		else
		{
			int stride = std::ceil(double(vertices.size()) / maxVertices);
			stridedVertices.clear();
			stridedVertices.reserve(maxVertices);
			for(std::size_t i = 0; i < vertices.size(); i += stride)
				stridedVertices.push_back(vertices[i]);
			if(cull)
				representation = MeshRepresentation(cullAgainst(cullMatrix, stridedVertices, stridedVertices.size()));
			else
				representation = MeshRepresentation(stridedVertices);
			return;
		}
	}
	else
	{
		if(cull)
			representation = MeshRepresentation(cullAgainst(cullMatrix, vertices, vertices.size()));
		else
			representation = MeshRepresentation(vertices);
	}
}

std::string Mesh::getNamePrefix() const
{
	return "Mesh";
}

std::unique_ptr<Mesh> Mesh::join(std::vector<std::unique_ptr<Mesh>>&& meshes)
{
	std::size_t verticesCount = 0;
	for(auto const& mesh : meshes)
		verticesCount += mesh->vertices.size();
	std::vector<Vertex> allVertices;
	allVertices.reserve(verticesCount);
	for(auto& mesh : meshes)
	{
		allVertices.insert(allVertices.end(),
			std::make_move_iterator(mesh->vertices.begin()),
			std::make_move_iterator(mesh->vertices.end())
		);
	}
	return std::make_unique<Mesh>(std::move(allVertices));
}

MeshRepresentation const& Mesh::getRepresentation() const
{
	return representation;
}

void Mesh::setCullMatrix(glm::mat4 mvp)
{
	cull = true;
	cullMatrix = mvp;
	updateRepresentation();
}

void Mesh::applyTransformation(glm::mat4 t)
{
	for(auto& vertex : vertices)
		vertex.position = t * glm::vec4(vertex.position, 1.0f);
	updateRepresentation();
}

void Mesh::drawUI()
{
	ImGui::Text(getName().data());
	if(ImGui::Checkbox("Filtering", &filtering))
		updateRepresentation();
	if(filtering)
	{
		ImGui::Text("Max Vertices");
		ImGui::Separator();
		if(ImGui::InputInt("###MaxVerticesInput", &maxVertices, 1000, 100'000))
			updateRepresentation();
		if(ImGui::SliderInt("###MaxVerticesSlider", &maxVertices, 1, vertices.size()))
			updateRepresentation();
		ImGui::Separator();
		ImGui::Text("Pick Vertices");
		ImGui::SameLine();
		if(ImGui::RadioButton("Sequentually", pickVerticesSequentially))
		{
			pickVerticesSequentially = true;
			updateRepresentation();
		}
		ImGui::SameLine();
		if(ImGui::RadioButton("Strided", !pickVerticesSequentially))
		{
			pickVerticesSequentially = false;
			updateRepresentation();
		}
	}
	bool showStridedVertices = filtering && maxVertices < vertices.size() && !pickVerticesSequentially;
	if(showStridedVertices)
		ImGui::Columns(2);
	{
		ImGui::Text("%i Total Vertices", vertices.size());
		ImGui::BeginChild("###TotalVertices");
		ImGuiListClipper vertexListClipper{static_cast<int>(vertices.size())};
		while(vertexListClipper.Step())
		{
			for (int i = vertexListClipper.DisplayStart; i < vertexListClipper.DisplayEnd; i++)
				ImGui::Text("%.3f, %.3f, %.3f", vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
		}
		ImGui::EndChild();
	}

	if(showStridedVertices)
	{
		ImGui::NextColumn();
		ImGui::Text("%i Strided Vertices", stridedVertices.size());
		ImGui::BeginChild("###StridedVertices");
		ImGuiListClipper vertexListClipper{static_cast<int>(stridedVertices.size())};
		while(vertexListClipper.Step())
		{
			for(int i = vertexListClipper.DisplayStart; i < vertexListClipper.DisplayEnd; i++)
				ImGui::Text("%.3f, %.3f, %.3f", stridedVertices[i].position.x, stridedVertices[i].position.y, stridedVertices[i].position.z);
		}
		ImGui::EndChild();
		ImGui::Columns(1);
	}
}