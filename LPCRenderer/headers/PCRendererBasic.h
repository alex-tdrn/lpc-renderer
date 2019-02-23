#pragma once
#include "PCRenderer.h"
#include "GPUBuffer.h"

class PCRendererBasic : public PCRenderer
{
private:
	GPUBuffer VBO{GL_ARRAY_BUFFER};
	std::size_t vertexCount = 0;
	int pointSize = 1;

public:
	PCRendererBasic();
	PCRendererBasic(const PCRendererBasic&) = delete;
	PCRendererBasic(PCRendererBasic&&) = default;
	~PCRendererBasic();
	PCRendererBasic& operator=(const PCRendererBasic&) = delete;
	PCRendererBasic& operator=(PCRendererBasic&&) = default;

public:
	virtual void update() override;
	virtual void render() const override;
	virtual void drawUI() override;

};

