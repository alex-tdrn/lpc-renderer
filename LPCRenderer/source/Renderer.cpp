#include "Renderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "imgui.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::render(Scene*) const
{
	glClearColor(background.r, background.g, background.b, 1.0);
}

void Renderer::drawUI()
{
	ImGui::ColorEdit3("Background", &background.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
}