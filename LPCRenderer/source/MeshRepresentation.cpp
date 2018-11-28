#include "MeshRepresentation.h"
#include "glad/glad.h"

MeshRepresentation::MeshRepresentation(std::vector<Vertex> const& vertices, std::size_t n)
	: vertexCount(n)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, n * sizeof(Vertex), vertices.data(), GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	glBindVertexArray(0);
}

MeshRepresentation::MeshRepresentation(std::vector<Vertex> const& vertices)
	: MeshRepresentation(vertices, vertices.size())
{

}

MeshRepresentation::MeshRepresentation(MeshRepresentation&& other)
	:VAO(other.VAO), VBO(other.VBO)
{
	other.VAO = 0;
	other.VBO = 0;
}

MeshRepresentation::~MeshRepresentation()
{
	freeBuffers();
}

MeshRepresentation& MeshRepresentation::operator=(MeshRepresentation &&other)
{
	freeBuffers();
	VAO = other.VAO;
	VBO = other.VBO;
	vertexCount = other.vertexCount;
	other.VAO = 0;
	other.VBO = 0;
	return *this;
}

void MeshRepresentation::freeBuffers()
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
