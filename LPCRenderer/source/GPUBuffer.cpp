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
	bind();
	if (glIsBuffer(ID))
	{
		glDeleteBuffers(1, &ID);
		Profiler::recordGPUDeallocation(currentSize);
		currentSize = 0;
	}
}

void GPUBuffer::clear()
{
	bind();
	glInvalidateBufferData(ID);
	glClearBufferSubData(target, GL_R8, 0, currentSize, GL_RED, GL_UNSIGNED_BYTE, nullptr);
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
