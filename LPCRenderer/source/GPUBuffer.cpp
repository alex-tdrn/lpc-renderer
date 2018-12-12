#include "GPUBuffer.h"
#include "Profiler.h"

GPUBuffer::GPUBuffer(GLenum target)
	:target(target)
{
}

GPUBuffer::GPUBuffer(GPUBuffer&& other)
	:ID(other.ID), target(other.target), 
	currentSize(other.currentSize),
	fence(other.fence), data(other.data)
{
	other.ID = 0;
	other.fence = nullptr;
	other.currentSize = 0;
}

GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other)
{
	free();
	ID = other.ID;
	target = other.target;
	currentSize = other.currentSize;
	fence = other.fence;
	data = other.data;
	other.ID = 0;
	other.fence = nullptr;
	other.currentSize = 0;
	return *this;
}

GPUBuffer::~GPUBuffer()
{
	free();
}

void GPUBuffer::resize(std::size_t newSize)
{
	if(newSize > currentSize)
		Profiler::recordGPUAllocation(newSize - currentSize);
	else
		Profiler::recordGPUDeallocation(currentSize - newSize);

	if(ID != 0)
	{
		unlock();
		glBindBuffer(target, ID);
		glUnmapBuffer(target);
		glDeleteBuffers(1, &ID);
	}

	glGenBuffers(1, &ID);
	glBindBuffer(target, ID);
	glBufferStorage(target, newSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT);
	data = (std::byte*)(glMapBufferRange(target, 0, newSize, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT));

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) (0));

	currentSize = newSize;
}

void GPUBuffer::unlock()
{
	if(fence)
	{
		Profiler::beginFenceWait();
		auto fenceState = GL_UNSIGNALED;
		while(fenceState != GL_ALREADY_SIGNALED && fenceState != GL_CONDITION_SATISFIED)
			fenceState = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		Profiler::endFenceWait();
		glDeleteSync(fence);
		fence = nullptr;
	}
}

void GPUBuffer::lock()
{
	fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void GPUBuffer::write(bool shrinkToFit, std::vector<std::pair<std::byte const*, std::size_t>>&& data)
{
	std::size_t totalSize = 0;
	for(auto const& buffer : data)
		totalSize += buffer.second;

	if(totalSize != currentSize && (shrinkToFit || totalSize > currentSize))
		resize(totalSize);

	std::size_t offset = 0;
	for(auto const& buffer : data)
	{
		std::memcpy(this->data + offset, buffer.first, buffer.second);
		offset += buffer.second;
	}
}

void GPUBuffer::bind()
{
	glBindBuffer(target, ID);
}

void GPUBuffer::free()
{
	glDeleteBuffers(1, &ID);
	Profiler::recordGPUDeallocation(currentSize);
	if(fence)
		glDeleteSync(fence);
}
