#pragma once
#include "PCRenderer.h"
#include "GPUBuffer.h"

class PCRendererBrickIndirect : public PCRenderer
{
private:
	GPUBuffer VBO{GL_ARRAY_BUFFER};
	GPUBuffer DrawBuffer{GL_DRAW_INDIRECT_BUFFER};
	std::size_t indirectDrawCount = 0;

public:
	PCRendererBrickIndirect();
	PCRendererBrickIndirect(const PCRendererBrickIndirect&) = delete;
	PCRendererBrickIndirect(PCRendererBrickIndirect&&) = default;
	~PCRendererBrickIndirect() = default;
	PCRendererBrickIndirect& operator=(const PCRendererBrickIndirect&) = delete;
	PCRendererBrickIndirect& operator=(PCRendererBrickIndirect&&) = default;

private:
	void updatePositions16();
	void updatePositions32();

public:
	virtual void update() override;
	virtual void render(Scene const* scene) override;
	virtual void drawUI() override;
	virtual void reloadShaders() override;

};

