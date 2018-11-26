#include "MeshRepresentation.h"
#include "glad/glad.h"

MeshRepresentation::MeshRepresentation(std::vector<Vertex> const& vertices)
	: vertexCount(vertices.size())
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	glBindVertexArray(0);
}

MeshRepresentation::MeshRepresentation(MeshRepresentation&& other)
	:VAO(other.VAO), VBO(other.VBO)
{
	other.VAO = 0;
	other.VBO = 0;
}

MeshRepresentation::~MeshRepresentation()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void MeshRepresentation::use() const
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, vertexCount);
	glBindVertexArray(0);
}
