#include "Profiler.h"
#include "glad/glad.h"
#include "imgui.h"

#include <chrono>
#include <array>
#include <vector>
#include <numeric>

//TIME
static float frametime = 0.0f;
static float averageFrametime = 0.0f;
static float fps = 0.0f;
static float averageFPS = 0.0f;
static const int frameSamples = 200;
static std::array<float, frameSamples> frametimePlot;
static unsigned int currentFrameIndex = 0;
static float longestFrame = 100.0f;

void Profiler::recordFrame()
{
	static auto lastFrame = std::chrono::system_clock::now();
	auto currentFrame = std::chrono::system_clock::now();
	currentFrameIndex = (currentFrameIndex + 1) % frameSamples;
	frametime = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrame - lastFrame).count();
	averageFrametime -= frametimePlot[currentFrameIndex] / frameSamples;
	if(std::abs(longestFrame - frametimePlot[currentFrameIndex]) < 0.01f)
		longestFrame = 100.0f;
	frametimePlot[currentFrameIndex] = frametime;
	averageFrametime += frametimePlot[currentFrameIndex] / frameSamples;
	longestFrame = std::max(longestFrame, frametime);
	fps = 1000.0f / frametime;
	averageFPS = 1000.0f / averageFrametime;
	lastFrame = currentFrame;
}

//MEMORY
static std::size_t allocatedBytes = 0;
static std::size_t allocatedKiloBytes = 0;
static std::size_t allocatedMegaBytes = 0;
static std::size_t allocatedGigaBytes = 0;

static void updateAllocatedSizes()
{
	allocatedKiloBytes = allocatedBytes >> 10;
	allocatedMegaBytes = allocatedKiloBytes >> 10;
	allocatedGigaBytes = allocatedMegaBytes >> 10;
}

void Profiler::recordGPUAllocation(std::size_t size)
{
	allocatedBytes += size;
	updateAllocatedSizes();
}

void Profiler::recordGPUDeallocation(std::size_t size)
{
	allocatedBytes -= size;
	updateAllocatedSizes();
}

void Profiler::drawUI()
{
	if(ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_DefaultOpen)) 
	{
		static float const plotHeight = 100;
		ImGui::Text("Plot Range (%.1fms) - (%.1fms)", 0.0f, longestFrame);
		ImGui::Text("Total Frametime: %.1f ms", frametime);
		ImGui::Text("Average Total Frametime: %.1f ms", averageFrametime);
		ImGui::PlotLines("###Frametimes", frametimePlot.data(), frameSamples, currentFrameIndex, nullptr, 0.0f, longestFrame, {ImGui::GetContentRegionAvailWidth(), plotHeight});
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Average FPS: %.1f", averageFPS);
	}

	ImGui::NewLine();
	if(ImGui::CollapsingHeader("Memory"))
	{
		if(allocatedGigaBytes)
		{
			ImGui::Text("%lu GB ", allocatedGigaBytes);
			ImGui::Text("%lu MB ", allocatedMegaBytes - (allocatedGigaBytes << 10));
		}
		else if(allocatedMegaBytes)
		{
			ImGui::Text("%lu MB ", allocatedMegaBytes);
			ImGui::Text("%lu KB ", allocatedKiloBytes - (allocatedMegaBytes << 10));
		}
		else if(allocatedKiloBytes)
		{
			ImGui::Text("%lu KB ", allocatedKiloBytes);
			ImGui::Text("%lu B ", allocatedBytes - (allocatedKiloBytes << 10));
		}
		else
		{
			ImGui::Text("%lu B ", allocatedBytes);
		}
	}

	ImGui::NewLine();
	if(ImGui::CollapsingHeader("GL Context"))
	{
		struct GLContext
		{
			int majorVersion;
			int minorVersion;
			std::string vendor;
			std::string device;
			std::vector<std::string> supportedExtensions;
		};
		static GLContext const context = []() -> GLContext{
			GLContext ret;
			glGetIntegerv(GL_MAJOR_VERSION, &ret.majorVersion);
			glGetIntegerv(GL_MINOR_VERSION, &ret.minorVersion);
			ret.vendor = reinterpret_cast<char const*>(glGetString(GL_VENDOR));
			ret.device = reinterpret_cast<char const*>(glGetString(GL_RENDERER));
			int nSupportedExtensions;
			glGetIntegerv(GL_NUM_EXTENSIONS, &nSupportedExtensions);
			for(int i = 0; i < nSupportedExtensions; i++)
				ret.supportedExtensions.push_back(reinterpret_cast<char const*>(glGetStringi(GL_EXTENSIONS, i)));
			return ret;
		}();
		ImGui::Text("API: OpenGL %i.%i", context.majorVersion, context.minorVersion);
		ImGui::Text("Vendor: %s", context.vendor.data());
		ImGui::Text("Device: %s", context.device.data());
		if(ImGui::TreeNode("Supported Extensions:"))
		{
			ImGui::Indent();
			ImGui::BeginChild("Supported Extensions", {0, 100});
			for(auto extension : context.supportedExtensions)
				ImGui::Text(extension.data());
			ImGui::EndChild();
			ImGui::Unindent();
			ImGui::TreePop();
		}
	}
}