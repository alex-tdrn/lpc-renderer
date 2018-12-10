#include "Profiler.h"
#include "glad/glad.h"
#include "imgui.h"

#include <chrono>
#include <array>
#include <vector>
#include <numeric>

//TIME
static const int frameSamples = 200;
static unsigned int currentFrameIndex = 0;

static std::array<float, frameSamples> frametimes;
static float averageFrametime = 0.0f;
static float longestFrametime = 100.0f;
static std::array<float, frameSamples> fenceWaitDurations;
static std::chrono::system_clock::time_point fenceWaitStart;
static float currentFenceWaitDuration = 0.0f;
static float averageFenceWaitDuration = 0.0f;
static float longestFenceWaitDuration = 10.0f;

void Profiler::recordFrame()
{
	static auto lastFrame = std::chrono::system_clock::now();
	auto currentFrame = std::chrono::system_clock::now();

	currentFrameIndex = (currentFrameIndex + 1) % frameSamples;

	auto updateStats = [](float& current, float& average, float& longest, float longestMin, std::array<float, frameSamples>& lastSamples){
		average -= lastSamples[currentFrameIndex] / frameSamples;
		if(std::abs(longest - lastSamples[currentFrameIndex]) < 0.01f)
			longest = longestMin;
		lastSamples[currentFrameIndex] = current;
		average += lastSamples[currentFrameIndex] / frameSamples;
		longest = std::max(longest, current);
	};

	float currentFrametime = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrame - lastFrame).count();
	updateStats(currentFrametime, averageFrametime, longestFrametime, 100.0f, frametimes);
	lastFrame = currentFrame;

	updateStats(currentFenceWaitDuration, averageFenceWaitDuration, longestFenceWaitDuration, 10.0f, fenceWaitDurations);
	currentFenceWaitDuration = 0.0f;

}

void Profiler::beginFenceWait()
{
	fenceWaitStart = std::chrono::system_clock::now();
}

void Profiler::endFenceWait()
{
	currentFenceWaitDuration += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - fenceWaitStart).count();
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
		ImGui::Text("Total Frametime: %.1f ms", frametimes[currentFrameIndex]);
		ImGui::Text("Average: %.1f ms", averageFrametime);
		ImGui::Text("FPS: %.1f", 1000.0f / frametimes[currentFrameIndex]);
		ImGui::Text("Average FPS: %.1f", 1000.0f / averageFrametime);
		ImGui::Text("Plot Range (%.1fms) - (%.1fms)", 0.0f, longestFrametime);
		ImGui::PlotLines("###Frametimes", frametimes.data(), frameSamples, currentFrameIndex, nullptr, 0.0f, longestFrametime, {ImGui::GetContentRegionAvailWidth(), plotHeight});
		
		ImGui::NewLine();
		ImGui::Text("Time Waiting On Fences: %.1f ms", fenceWaitDurations[currentFrameIndex]);
		ImGui::Text("Average: %.1f ms", averageFenceWaitDuration);
		ImGui::Text("Plot Range (%.1fms) - (%.1fms)", 0.0f, longestFenceWaitDuration);
		ImGui::PlotLines("###FenceWaitDuration", fenceWaitDurations.data(), frameSamples, currentFrameIndex, nullptr, 0.0f, longestFenceWaitDuration, {ImGui::GetContentRegionAvailWidth(), plotHeight});
	}

	ImGui::NewLine();
	if(ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen))
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
	static bool showContextInfo = false;
	if(ImGui::Button("GL Context"))
		showContextInfo = true;
	if(showContextInfo)
	{
		ImGui::Begin("GL Context", &showContextInfo);
		struct GLContext
		{
			int majorVersion;
			int minorVersion;
			std::string vendor;
			std::string device;
			int maxShaderStorageBufferBytes;
			int maxShaderStorageBufferKiloBytes;
			int maxShaderStorageBufferMegaBytes;
			int maxShaderStorageBufferGigaBytes;
			int maxUniformBufferBytes;
			int maxUniformBufferKiloBytes;
			int maxUniformBufferMegaBytes;
			int maxUniformBufferGigaBytes;
			int maxVertexUniforms;
			int maxGeometryUniforms;
			int maxFragmentUniforms;
			int maxVertexUniformComponents;
			int maxFragmentUniformComponents;
			int maxGeometryUniformComponents;
			int maxGeometryInputComponents;
			int maxGeometryOutputComponents;
			int maxGeometryTotalOutputComponents;
			int maxGeometryOutputVertices;
			int maxGeometryShaderInvocations;
			int maxVertices;
			int maxVertexAttributes;
			int maxVertexTextureUnits;
			std::vector<std::string> supportedExtensions;
		};
		static GLContext const context = []() -> GLContext{
			GLContext ret;
			glGetIntegerv(GL_MAJOR_VERSION, &ret.majorVersion);
			glGetIntegerv(GL_MINOR_VERSION, &ret.minorVersion);
			ret.vendor = reinterpret_cast<char const*>(glGetString(GL_VENDOR));
			ret.device = reinterpret_cast<char const*>(glGetString(GL_RENDERER));

			glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &ret.maxShaderStorageBufferBytes);
			ret.maxShaderStorageBufferKiloBytes = ret.maxShaderStorageBufferBytes >> 10;
			ret.maxShaderStorageBufferMegaBytes = ret.maxShaderStorageBufferKiloBytes >> 10;
			ret.maxShaderStorageBufferGigaBytes = ret.maxShaderStorageBufferMegaBytes >> 10;

			glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &ret.maxUniformBufferBytes);
			ret.maxUniformBufferKiloBytes = ret.maxUniformBufferBytes >> 10;
			ret.maxUniformBufferMegaBytes = ret.maxUniformBufferKiloBytes >> 10;
			ret.maxUniformBufferGigaBytes = ret.maxUniformBufferMegaBytes >> 10;

			glGetIntegerv(GL_MAX_GEOMETRY_INPUT_COMPONENTS, &ret.maxGeometryInputComponents);
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &ret.maxGeometryOutputComponents);
			glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &ret.maxGeometryTotalOutputComponents);
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &ret.maxGeometryOutputVertices);
			glGetIntegerv(GL_MAX_GEOMETRY_SHADER_INVOCATIONS, &ret.maxGeometryShaderInvocations);

			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &ret.maxVertexUniforms);
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &ret.maxGeometryUniforms);
			glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &ret.maxFragmentUniforms);

			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &ret.maxVertexUniformComponents);
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &ret.maxGeometryUniformComponents);
			glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &ret.maxFragmentUniformComponents);

			glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &ret.maxVertices);
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &ret.maxVertexAttributes);
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &ret.maxVertexTextureUnits);

			int nSupportedExtensions;
			glGetIntegerv(GL_NUM_EXTENSIONS, &nSupportedExtensions);
			for(int i = 0; i < nSupportedExtensions; i++)
				ret.supportedExtensions.push_back(reinterpret_cast<char const*>(glGetStringi(GL_EXTENSIONS, i)));
			return ret;
		}();
		ImGui::Text("API: OpenGL %i.%i", context.majorVersion, context.minorVersion);
		ImGui::Text("Vendor: %s", context.vendor.data());
		ImGui::Text("Device: %s", context.device.data());
		ImGui::Separator();
		ImGui::Text("Max Shader Storage Buffer Size: ");
		ImGui::SameLine();
		if(context.maxShaderStorageBufferGigaBytes)
		{
			ImGui::Text("%lu GB ", context.maxShaderStorageBufferGigaBytes);
			ImGui::SameLine();
			ImGui::Text("%lu MB ", context.maxShaderStorageBufferMegaBytes - (context.maxShaderStorageBufferGigaBytes << 10));
		}
		else if(context.maxShaderStorageBufferMegaBytes)
		{
			ImGui::Text("%lu MB ", context.maxShaderStorageBufferMegaBytes);
			ImGui::SameLine();
			ImGui::Text("%lu KB ", context.maxShaderStorageBufferKiloBytes - (context.maxShaderStorageBufferMegaBytes << 10));
		}
		else if(context.maxShaderStorageBufferKiloBytes)
		{
			ImGui::Text("%lu KB ", context.maxShaderStorageBufferKiloBytes);
			ImGui::SameLine();
			ImGui::Text("%lu B ", context.maxShaderStorageBufferBytes - (context.maxShaderStorageBufferKiloBytes << 10));
		}
		else
		{
			ImGui::Text("%lu B ", context.maxShaderStorageBufferBytes);
		}

		ImGui::Text("Max Uniform Buffer Size: ");
		ImGui::SameLine();
		if(context.maxUniformBufferGigaBytes)
		{
			ImGui::Text("%lu GB ", context.maxUniformBufferGigaBytes);
			ImGui::SameLine();
			ImGui::Text("%lu MB ", context.maxUniformBufferMegaBytes - (context.maxUniformBufferGigaBytes << 10));
		}
		else if(context.maxUniformBufferMegaBytes)
		{
			ImGui::Text("%lu MB ", context.maxUniformBufferMegaBytes);
			ImGui::SameLine();
			ImGui::Text("%lu KB ", context.maxUniformBufferKiloBytes - (context.maxUniformBufferMegaBytes << 10));
		}
		else if(context.maxUniformBufferKiloBytes)
		{
			ImGui::Text("%lu KB ", context.maxUniformBufferKiloBytes);
			ImGui::SameLine();
			ImGui::Text("%lu B ", context.maxUniformBufferBytes - (context.maxUniformBufferKiloBytes << 10));
		}
		else
		{
			ImGui::Text("%lu B ", context.maxUniformBufferBytes);
		}
		ImGui::Text("Max Uniforms: %i, %i, %i", context.maxVertexUniforms, context.maxGeometryUniforms, context.maxFragmentUniforms);
		ImGui::Text("Max Uniform Components: %i, %i, %i", context.maxVertexUniformComponents, context.maxGeometryUniformComponents, context.maxFragmentUniformComponents);
		ImGui::NewLine();
		ImGui::Text("Max Geometry Input Components: %i", context.maxGeometryInputComponents);
		ImGui::Text("Max Geometry Output Components: %i", context.maxGeometryOutputComponents);
		ImGui::Text("Max Geometry Total Output Components: %i", context.maxGeometryTotalOutputComponents);
		ImGui::Text("Max Geometry Output Vertices: %i", context.maxGeometryOutputVertices);
		ImGui::Text("Max Geometry Shader Invocations: %i", context.maxGeometryShaderInvocations);
		ImGui::NewLine();
		ImGui::Text("Max VAO Vertices: %i", context.maxVertices);
		ImGui::Text("Max Vertex Attributes: %i", context.maxVertexAttributes);
		ImGui::Text("Max Vertex Texture Units: %i", context.maxVertexTextureUnits);
		ImGui::Separator();
		ImGui::Text("Supported Extensions");
		ImGui::Indent();
		for(auto extension : context.supportedExtensions)
			ImGui::Text(extension.data());
		ImGui::Unindent();
		ImGui::End();
	}
}