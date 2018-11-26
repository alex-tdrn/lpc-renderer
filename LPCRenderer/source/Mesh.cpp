#include "Mesh.h"
#include "imgui.h"

Mesh::Mesh(std::vector<Vertex>&& vertices)
	:vertices(std::move(vertices))
{
}

Mesh::~Mesh()
{
}

std::string Mesh::getNamePrefix() const
{
	return "Mesh";
}

MeshRepresentation* Mesh::getRepresentation() const
{
	if(representation.get() == nullptr)
		representation = std::make_unique<MeshRepresentation>(vertices);
	return representation.get();
}

void Mesh::drawUI() const
{
	ImGui::Text(getName().data());
	ImGui::Text("%i Vertices", vertices.size());
	ImGuiListClipper vertexListClipper{static_cast<int>(vertices.size())};
	while(vertexListClipper.Step())
	{
		for (int i = vertexListClipper.DisplayStart; i < vertexListClipper.DisplayEnd; i++)
			ImGui::Text("%.3f, %.3f, %.3f", vertices[i].x, vertices[i].y, vertices[i].z);
	}
}