#include "Profiler.h"
#include "glad/glad.h"
#include "imgui.h"
#include "glm/glm.hpp"

#include <chrono>
#include <array>
#include <vector>
#include <numeric>
#include <string>

using namespace std::literals::chrono_literals;

//TIME
static const int frameSamples = 200;
static unsigned int currentFrameIndex = 0;

static std::array<std::chrono::nanoseconds, frameSamples> frametimes;
static std::chrono::nanoseconds averageFrametime = 0ns;
static std::chrono::nanoseconds longestFrametime = 100ms;
static std::array<std::chrono::nanoseconds, frameSamples> fenceWaitDurations;
static std::chrono::steady_clock::time_point fenceWaitStart;
static std::chrono::nanoseconds currentFenceWaitDuration = 0ns;
static std::chrono::nanoseconds averageFenceWaitDuration = 0ns;
static std::chrono::nanoseconds longestFenceWaitDuration = 5ms;

void Profiler::recordFrame()
{
	static auto lastFrame = std::chrono::steady_clock::now();
	auto currentFrame = std::chrono::steady_clock::now();

	currentFrameIndex = (currentFrameIndex + 1) % frameSamples;

	auto updateStats = [](auto& current, auto& average, auto& longest, auto longestMin, std::array<std::chrono::nanoseconds, frameSamples>& lastSamples){
		average -= lastSamples[currentFrameIndex] / frameSamples;
		if(longest == lastSamples[currentFrameIndex])
			longest = longestMin;
		lastSamples[currentFrameIndex] = current;
		average += lastSamples[currentFrameIndex] / frameSamples;
		if(current > longest)
			longest = current;
	};

	std::chrono::nanoseconds currentFrametime = currentFrame - lastFrame;
	updateStats(currentFrametime, averageFrametime, longestFrametime, 100ms, frametimes);
	lastFrame = currentFrame;

	updateStats(currentFenceWaitDuration, averageFenceWaitDuration, longestFenceWaitDuration, 5ms, fenceWaitDurations);
	currentFenceWaitDuration = 0ns;

}

void Profiler::beginFenceWait()
{
	fenceWaitStart = std::chrono::steady_clock::now();
}

void Profiler::endFenceWait()
{
	currentFenceWaitDuration += std::chrono::steady_clock::now() - fenceWaitStart;
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

template<typename T, typename Ratio>
std::string printDuration(std::chrono::duration<T, Ratio> duration)
{
	static int const threshold = 1'000;
	if constexpr(std::is_same_v<Ratio, std::nano>)
	{
		if(duration.count() < threshold)
			return std::to_string(duration.count()) + "ns";
		else
			return printDuration(std::chrono::duration_cast<std::chrono::microseconds>(duration));
	}
	else if constexpr(std::is_same_v<Ratio, std::micro>)
	{
		if(duration.count() < threshold)
			return std::to_string(duration.count()) + "us";
		else
			return printDuration(std::chrono::duration_cast<std::chrono::milliseconds>(duration));
	}
	else if constexpr(std::is_same_v<Ratio, std::milli>)
	{
		if(duration.count() < threshold)
			return std::to_string(duration.count()) + "ms";
		else
			return printDuration(std::chrono::duration_cast<std::chrono::seconds>(duration));
	}
	else if constexpr(std::is_same_v<Ratio, std::ratio<1, 1>>)
	{
		return std::to_string(duration.count()) + "s";
	}
}

void Profiler::drawUI()
{
	if(ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_DefaultOpen)) 
	{
		static float const plotHeight = 100;
		ImGui::Text("Total Frametime: %s", printDuration(frametimes[currentFrameIndex]).data());
		ImGui::Text("Average: %s", printDuration(averageFrametime).data());
		ImGui::Text("Longest: %s", printDuration(longestFrametime).data());
		ImGui::PlotLines("###Frametimes", [](void* data, int idx) -> float{
				return reinterpret_cast<std::chrono::nanoseconds*>(data)[idx].count();
		},frametimes.data(), frameSamples, currentFrameIndex, nullptr, 0.0f, longestFrametime.count(), {ImGui::GetContentRegionAvailWidth(), plotHeight});

		ImGui::NewLine();
		ImGui::Text("Time Waiting On Fences: %s", printDuration(fenceWaitDurations[currentFrameIndex]).data());
		ImGui::Text("Average: %s", printDuration(averageFenceWaitDuration).data());
		ImGui::Text("Longest: %s", printDuration(longestFenceWaitDuration).data());
		ImGui::PlotLines("###FenceWaitDuration", [](void* data, int idx) -> float{
			return reinterpret_cast<std::chrono::nanoseconds*>(data)[idx].count();
		}, fenceWaitDurations.data(), frameSamples, currentFrameIndex, nullptr, 0.0f, longestFenceWaitDuration.count(), {ImGui::GetContentRegionAvailWidth(), plotHeight});
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
			std::string glslVersion;
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
			int maxGeometryVertexStreams;
			int maxVertices;
			int maxVertexAttributes;
			int maxVertexTextureUnits;
			glm::ivec3 maxComputeWorkGroupCount;
			glm::ivec3 maxComputeWorkGroupSize;
			int maxComputeWorkGroupInvocations;
			int maxComputeSharedMemoryBytes;
			int maxComputeSharedMemoryKiloBytes;
			std::vector<std::string> supportedExtensions;
		};
		static GLContext const context = []() -> GLContext{
			GLContext ret;
			glGetIntegerv(GL_MAJOR_VERSION, &ret.majorVersion);
			glGetIntegerv(GL_MINOR_VERSION, &ret.minorVersion);
			ret.glslVersion = reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
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
			glGetIntegerv(GL_MAX_VERTEX_STREAMS, &ret.maxGeometryVertexStreams);

			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &ret.maxVertexUniforms);
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &ret.maxGeometryUniforms);
			glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &ret.maxFragmentUniforms);

			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &ret.maxVertexUniformComponents);
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &ret.maxGeometryUniformComponents);
			glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &ret.maxFragmentUniformComponents);

			glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &ret.maxVertices);
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &ret.maxVertexAttributes);
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &ret.maxVertexTextureUnits);

			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &ret.maxComputeWorkGroupCount[0]);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &ret.maxComputeWorkGroupCount[1]);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &ret.maxComputeWorkGroupCount[2]);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &ret.maxComputeWorkGroupSize[0]);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &ret.maxComputeWorkGroupSize[1]);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &ret.maxComputeWorkGroupSize[2]);
			glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &ret.maxComputeWorkGroupInvocations);
			glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &ret.maxComputeSharedMemoryBytes);
			ret.maxComputeSharedMemoryKiloBytes = ret.maxComputeSharedMemoryBytes >> 10;

			int nSupportedExtensions;
			glGetIntegerv(GL_NUM_EXTENSIONS, &nSupportedExtensions);
			for(int i = 0; i < nSupportedExtensions; i++)
				ret.supportedExtensions.push_back(reinterpret_cast<char const*>(glGetStringi(GL_EXTENSIONS, i)));
			return ret;
		}();
		ImGui::Text("API: OpenGL %i.%i, glsl %s", context.majorVersion, context.minorVersion, context.glslVersion.data());
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
		ImGui::Text("Max Geometry Shader Vertex Streams: %i", context.maxGeometryVertexStreams);
		ImGui::NewLine();
		ImGui::Text("Max VAO Vertices: %i", context.maxVertices);
		ImGui::Text("Max Vertex Attributes: %i", context.maxVertexAttributes);
		ImGui::Text("Max Vertex Texture Units: %i", context.maxVertexTextureUnits);
		ImGui::NewLine();
		ImGui::Text("Max Compute Work Group Count: %i, %i, %i", context.maxComputeWorkGroupCount[0], context.maxComputeWorkGroupCount[1], context.maxComputeWorkGroupCount[2]);
		ImGui::Text("Max Compute Work Group Size: %i, %i, %i", context.maxComputeWorkGroupSize[0], context.maxComputeWorkGroupSize[1], context.maxComputeWorkGroupSize[2]);
		ImGui::Text("Max Compute Work Group Invocations: %i", context.maxComputeWorkGroupInvocations);
		ImGui::Text("Max Compute Shared Memory: ");
		ImGui::SameLine();
		if(context.maxShaderStorageBufferKiloBytes)
		{
			ImGui::Text("%lu KB ", context.maxComputeSharedMemoryKiloBytes);
			ImGui::SameLine();
			ImGui::Text("%lu B ", context.maxComputeSharedMemoryBytes - (context.maxComputeSharedMemoryKiloBytes << 10));
		}
		else
		{
			ImGui::Text("%lu B ", context.maxComputeSharedMemoryBytes);
		}
		ImGui::Separator();
		ImGui::Text("Supported Extensions");
		ImGui::Indent();
		for(auto extension : context.supportedExtensions)
			ImGui::Text(extension.data());
		ImGui::Unindent();
		ImGui::End();
	}
}