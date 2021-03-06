#pragma once
#include "PCRenderer.h"
#include "GPUBuffer.h"

class PCRendererBrickIndirect : public PCRenderer
{
private:
	GPUBuffer VBOPositions{GL_ARRAY_BUFFER};
	GPUBuffer VBONormals{GL_ARRAY_BUFFER};
	GPUBuffer VBOColors{GL_ARRAY_BUFFER};
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
	bool needNormals() const;
	bool needColors() const;
	void updatePositions32();
	void updatePositions16();
	void updateNormals16();
	void updateNormals8();

public:
	virtual void update() override;
	virtual void render(Scene const* scene) override;
	virtual void drawUI() override;
	virtual void reloadShaders() override;

};

