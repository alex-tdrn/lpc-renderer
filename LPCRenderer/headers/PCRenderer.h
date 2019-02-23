#pragma once
#include "GPUBuffer.h"

enum class PCRenderType
{
	basic,
	N
};

class Shader;
class PointCloud;

class PCRenderer
{
private:
	unsigned int VAO = 0;

protected:
	PointCloud const* cloud = nullptr;
	Shader* mainShader = nullptr;

public:
	PCRenderer(Shader* mainShader);
	PCRenderer(PCRenderer const&) = delete;
	PCRenderer(PCRenderer&&);
	virtual ~PCRenderer();
	PCRenderer& operator=(PCRenderer const&) = delete;
	PCRenderer& operator=(PCRenderer&&);

private:
	void freeVAO();

protected:
	void bindVAO() const;

public:
	Shader* getMainShader() const;
	void setPointCloud(PointCloud const* cloud);
	virtual void update() = 0;
	virtual void render() const = 0;
	virtual void drawUI() = 0;
};
