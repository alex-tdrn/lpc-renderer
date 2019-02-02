#pragma once
#include <vector>
#include <tuple>
#include <glad/glad.h>

class GPUBuffer
{
private:
	unsigned int ID = 0;
	GLenum target;
	GLsync fence = nullptr;
	std::byte* data = nullptr;
	std::size_t currentSize = 0;

public:
	GPUBuffer(GLenum target);
	GPUBuffer(GPUBuffer const&) = delete;
	GPUBuffer(GPUBuffer&&);
	~GPUBuffer();
	GPUBuffer& operator=(GPUBuffer const&) = delete;
	GPUBuffer& operator=(GPUBuffer&&);

public:
	void unlock();
	void lock();
	void free();
	void write(bool shrinkToFit, std::vector<std::pair<std::byte const*, std::size_t>>&& data);
	void bind();
	void bindBase(unsigned int base);
};

