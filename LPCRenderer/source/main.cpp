#include "OSWindow.h"
#include "UIWindow.h"
#include "Profiler.h"
#include "Renderer.h"
#include "imgui_impl_glfw_gl3.h"
#include "RendererManager.h"
#include "MeshManager.h"
#include "ShaderManager.h"
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
	RendererManager::initialize();
	ShaderManager::initialize();
	while(!OSWindow::shouldClose())
	{
		OSWindow::beginFrame();

		Profiler::recordFrame();
		if(RendererManager::getActive())
		{
			if(SceneManager::getActive())
				RendererManager::getActive()->render(SceneManager::getActive());
		}
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
		UIWindow{SceneManager::name, SceneManager::drawUI, true},
		UIWindow{MeshManager::name, MeshManager::drawUI, true},
		UIWindow{RendererManager::name, RendererManager::drawUI, true},
		UIWindow{ShaderManager::name, ShaderManager::drawUI},
		UIWindow{"Importer", Importer::drawUI, true}
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
