#pragma once
#include "PCRenderer.h"
#include "GPUBuffer.h"

class PCRendererUncompressed : public PCRenderer
{
private:
	GPUBuffer VBO{GL_ARRAY_BUFFER};
	std::size_t vertexCount = 0;
	
public:
	PCRendererUncompressed();
	PCRendererUncompressed(const PCRendererUncompressed&) = delete;
	PCRendererUncompressed(PCRendererUncompressed&&) = default;
	~PCRendererUncompressed();
	PCRendererUncompressed& operator=(const PCRendererUncompressed&) = delete;
	PCRendererUncompressed& operator=(PCRendererUncompressed&&) = default;

private:
	bool needNormals() const;

public:
	virtual void update() override;
	virtual void render(Scene const* scene) override;
	virtual void drawUI() override;

};

