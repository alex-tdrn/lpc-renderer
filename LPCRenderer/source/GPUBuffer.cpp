#include "GPUBuffer.h"
#include "Profiler.h"

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

void GPUBuffer::free()
{
	if (glIsBuffer(ID))
	{
		glDeleteBuffers(1, &ID);
		Profiler::recordGPUDeallocation(currentSize);
	}
}

void GPUBuffer::write(bool shrinkToFit, std::vector<std::pair<std::byte const*, std::size_t>>&& data)
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

void GPUBuffer::bind()
{
	bind(target);
}

void GPUBuffer::bind(GLenum target)
{
	glBindBuffer(target, ID);
}

void GPUBuffer::bindBase(unsigned int base)
{
	if (target != GL_SHADER_STORAGE_BUFFER)
		throw "Illegal bindbufferbase!";
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, base, ID);
}
