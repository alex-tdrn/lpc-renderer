#include "GPUBuffer.h"
#include "Profiler.h"
#include "imgui.h"

GPUBuffer::GPUBuffer(GLenum target)
	:target(target)
{
}

GPUBuffer::GPUBuffer(GPUBuffer&& other)
	:ID(other.ID), target(other.target), 
	currentSize(other.currentSize)
{
	other.ID = 0;
	other.currentSize = 0;
}

GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other)
{
	free();
	ID = other.ID;
	target = other.target;
	currentSize = other.currentSize;
	other.ID = 0;
	other.currentSize = 0;
	return *this;
}

GPUBuffer::~GPUBuffer()
{
	free();
}

std::size_t GPUBuffer::size() const
{
	return currentSize;
}

void GPUBuffer::free()
{
	bind();
	if (glIsBuffer(ID))
	{
		glDeleteBuffers(1, &ID);
		Profiler::recordGPUDeallocation(currentSize);
		currentSize = 0;
		glBindBuffer(target, 0);
	}
}

void GPUBuffer::clear()
{
	bind();
	glInvalidateBufferData(ID);
	glClearBufferSubData(target, GL_R8, 0, currentSize, GL_RED, GL_UNSIGNED_BYTE, nullptr);
}

void GPUBuffer::write(std::vector<std::pair<std::byte const*, std::size_t>>&& data)
{
	std::size_t newSize = 0;      
	for(auto const& buffer : data)
		newSize += buffer.second;

	free();
	Profiler::recordGPUAllocation(newSize);
	currentSize = newSize;

	std::vector<std::byte> joinedData;
	joinedData.resize(newSize);
	std::size_t offset = 0;
	for(auto const& buffer : data)
	{
		std::memcpy(joinedData.data() + offset, buffer.first, buffer.second);
		offset += buffer.second;
	}

	glGenBuffers(1, &ID);
	bind();
	glBufferStorage(target, joinedData.size(), joinedData.data(), 0);
}

void GPUBuffer::reserve(std::size_t size)
{
	free();
	Profiler::recordGPUAllocation(size);
	currentSize = size;

	glGenBuffers(1, &ID);
	bind();
	glBufferStorage(target, size, nullptr, 0);
}

void GPUBuffer::bind() const
{
	bind(target);
}

void GPUBuffer::bind(GLenum target) const
{
	glBindBuffer(target, ID);
}

void GPUBuffer::bindBase(unsigned int base) const
{
	if (target != GL_SHADER_STORAGE_BUFFER && target != GL_ATOMIC_COUNTER_BUFFER)
		throw "Illegal bindbufferbase!";
	glBindBufferBase(target, base, ID);
}

void drawMemoryConsumption(std::size_t amountInBytes)
{
	std::size_t amountInKiloBytes = amountInBytes >> 10;
	std::size_t amountInMegaBytes = amountInKiloBytes >> 10;
	std::size_t amountInGigaBytes = amountInMegaBytes >> 10;

	if(amountInGigaBytes)
	{
		ImGui::Text("%lu GB ", amountInGigaBytes);
		ImGui::SameLine();
		ImGui::Text("%lu MB ", amountInMegaBytes - (amountInGigaBytes << 10));
	}
	else if(amountInMegaBytes)
	{
		ImGui::Text("%lu MB ", amountInMegaBytes);
		ImGui::SameLine();
		ImGui::Text("%lu KB ", amountInKiloBytes - (amountInMegaBytes << 10));
	}
	else if(amountInKiloBytes)
	{
		ImGui::Text("%lu KB ", amountInKiloBytes);
		ImGui::SameLine();
		ImGui::Text("%lu B ", amountInBytes - (amountInKiloBytes << 10));
	}
	else
	{
		ImGui::Text("%lu B ", amountInBytes);
	}
}
