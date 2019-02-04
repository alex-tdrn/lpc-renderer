#pragma once
#include <vector>
#include <tuple>
#include <glad/glad.h>

class GPUBuffer
{
private:
	unsigned int ID = 0;
	GLenum target;
	std::size_t currentSize = 0;

public:
	GPUBuffer(GLenum target);
	GPUBuffer(GPUBuffer const&) = delete;
	GPUBuffer(GPUBuffer&&);
	~GPUBuffer();
	GPUBuffer& operator=(GPUBuffer const&) = delete;
	GPUBuffer& operator=(GPUBuffer&&);

public:
	void free();
	void clear();
	void write(bool shrinkToFit, std::vector<std::pair<std::byte const*, std::size_t>>&& data);
	void reserve(std::size_t size);
	void bind();
	void bind(GLenum target);
	void bindBase(unsigned int base);
};

