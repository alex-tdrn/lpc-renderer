#include "PCRenderer.h"
#include "Scene.h"
#include "imgui.h"
#include "Shader.h"

PCRenderer::PCRenderer(Shader* mainShader)
	: mainShader(mainShader)
{
	glGenVertexArrays(1, &VAO);
}

PCRenderer::PCRenderer(PCRenderer&& other)
	: VAO(other.VAO), mainShader(mainShader)
{
	other.VAO = 0;
}

PCRenderer::~PCRenderer()
{
	freeVAO();
}

PCRenderer& PCRenderer::operator=(PCRenderer &&other)
{
	freeVAO();
	VAO = other.VAO;
	other.VAO = 0;
	mainShader = other.mainShader;
	return *this;
}

void PCRenderer::freeVAO()
{
	glDeleteVertexArrays(1, &VAO);
	glBindVertexArray(0);
}

void PCRenderer::bindVAO() const
{
	glBindVertexArray(VAO);
}

Shader* PCRenderer::getMainShader() const
{
	return mainShader;
}

void PCRenderer::render(Scene const* scene)
{
	if(cloud != scene->getPointCloud())
	{
		cloud = scene->getPointCloud();
		mainShader->use();
		update();
	}
}

void PCRenderer::drawUI()
{
	if(ImGui::Button("Reload Shaders"))
		reloadShaders();
}

