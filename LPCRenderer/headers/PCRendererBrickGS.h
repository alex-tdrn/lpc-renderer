#pragma once
#include "PCRenderer.h"
#include "GPUBuffer.h"

class PCRendererBrickGS : public PCRenderer
{
private:
	GPUBuffer VBO{GL_ARRAY_BUFFER};
	GPUBuffer SSBO{GL_SHADER_STORAGE_BUFFER};
	std::size_t brickCount = 0;
public:
	PCRendererBrickGS();
	PCRendererBrickGS(const PCRendererBrickGS&) = delete;
	PCRendererBrickGS(PCRendererBrickGS&&) = default;
	~PCRendererBrickGS() = default;
	PCRendererBrickGS& operator=(const PCRendererBrickGS&) = delete;
	PCRendererBrickGS& operator=(PCRendererBrickGS&&) = default;

public:
	virtual void update() override;
	virtual void render(Scene const* scene) override;
	virtual void drawUI() override;

};

