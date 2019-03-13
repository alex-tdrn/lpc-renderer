#include "OSWindow.h"
#include "UIWindow.h"
#include "Profiler.h"
#include "MainRenderer.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
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
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	static std::array uiWindows = {
		UIWindow{"Profiler", Profiler::drawUI, true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize},
		UIWindow{"Renderer", MainRenderer::drawUI, true},
		UIWindow{SceneManager::name, SceneManager::drawUI, true},
		UIWindow{PCManager::name, PCManager::drawUI, true}
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
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
