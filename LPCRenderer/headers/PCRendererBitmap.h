#pragma once
#include "PCRenderer.h"
#include "GPUBuffer.h"

class PCRendererBitmap : public PCRenderer
{
private:
	GPUBuffer SSBOBitmap{GL_SHADER_STORAGE_BUFFER};
	GPUBuffer SSBOPackedPositions{GL_SHADER_STORAGE_BUFFER};
	GPUBuffer SSBODrawCommands{GL_SHADER_STORAGE_BUFFER};
	GPUBuffer Counter{GL_ATOMIC_COUNTER_BUFFER};
	std::size_t indirectDrawCount = 0;

public:
	PCRendererBitmap();
	PCRendererBitmap(const PCRendererBitmap&) = delete;
	PCRendererBitmap(PCRendererBitmap&&) = default;
	~PCRendererBitmap() = default;
	PCRendererBitmap& operator=(const PCRendererBitmap&) = delete;
	PCRendererBitmap& operator=(PCRendererBitmap&&) = default;

public:
	virtual void update() override;
	virtual void render(Scene const* scene) override;
	virtual void drawUI() override;

};

