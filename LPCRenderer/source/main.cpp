#include "OSWindow.h"
#include "UIWindow.h"
#include "Profiler.h"
#include "MainRenderer.h"
#include "imgui_impl_glfw_gl3.h"
#include "PCManager.h"
#include "SceneManager.h"
#include "Importer.h"

#include <vector>
#include <memory>
#include <array>
//#include <vld.h>

void drawUI();

int main(int argc, char** argv)
{
	OSWindow::init();
	while(!OSWindow::shouldClose())
	{
		OSWindow::beginFrame();

		Profiler::recordFrame();
		MainRenderer::render(SceneManager::getActive());
		drawUI();

		OSWindow::endFrame();
	}
	OSWindow::destroy();
	return 0;
}

void drawUI()
{
	ImGui_ImplGlfwGL3_NewFrame();

	static std::array uiWindows = {
		UIWindow{"Profiler", Profiler::drawUI, true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize},
		UIWindow{"Renderer", MainRenderer::drawUI, true},
		UIWindow{SceneManager::name, SceneManager::drawUI},
		UIWindow{PCManager::name, PCManager::drawUI}
	};

	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("View"))
		{
			for(auto& window : uiWindows)
				window.drawMenuItem();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	for(auto& window : uiWindows)
		window.drawUI();

	ImGui::Render();
	ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
}
