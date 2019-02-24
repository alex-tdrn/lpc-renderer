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
	std::size_t size() const;
	void free();
	void clear();
	void write(std::vector<std::pair<std::byte const*, std::size_t>>&& data);
	void reserve(std::size_t size);
	void bind() const;
	void bind(GLenum target) const;
	void bindBase(unsigned int base) const;
};

template<typename T>
std::size_t sizeInBytes(std::vector<T> const& v)
{
	return v.size() * sizeof(v.front());
}

struct DrawCommand
{
	unsigned int count = 0;
	unsigned int instanceCount = 1;
	unsigned int first = 0;
	unsigned int baseInstance = 0;
};

void drawMemoryConsumption(std::size_t amountInBytes);