#include "Profiler.h"
#include "glad/glad.h"
#include "imgui.h"

#include <chrono>
#include <array>
#include <vector>
#include <numeric>

float frametime = 0.0f;
float averageFrametime = 0.0f;
float fps = 0.0f;
float averageFPS = 0.0f;
const int frameSamples = 200;
std::array<float, frameSamples> frametimePlot;
unsigned int currentFrameIndex = 0;
float longestFrame = 100.0f;

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

void Profiler::drawUI(bool* open)
{
	if(!*open)
		return;
	float const plotHeight = 100;
	ImGui::Begin("Profiler", open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Plot Range (%.1fms) - (%.1fms)", 0.0f, longestFrame);
	ImGui::Text("Frametime: %.1f ms", frametime);
	ImGui::Text("Average Frametime: %.1f ms", averageFrametime);
	ImGui::PlotLines("###Frametimes", frametimePlot.data(), frameSamples, currentFrameIndex, nullptr, 0.0f, longestFrame, {ImGui::GetContentRegionAvailWidth(), plotHeight});
	ImGui::Text("FPS: %.1f", fps);
	ImGui::Text("Average FPS: %.1f", averageFPS);

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
	ImGui::NewLine();
	ImGui::Text("Context Info:");
	ImGui::Text("API: OpenGL %i.%i", context.majorVersion, context.minorVersion);
	ImGui::Text("Vendor: %s", context.vendor.data());
	ImGui::Text("Device: %s", context.device.data());
	if(ImGui::CollapsingHeader("Supported Extensions:"))
	{
		ImGui::Indent();
		ImGui::BeginChild("Supported Extensions", {0, 100});
		for(auto extension : context.supportedExtensions)
			ImGui::Text(extension.data());
		ImGui::EndChild();
		ImGui::Unindent();
	}
	ImGui::End();
}