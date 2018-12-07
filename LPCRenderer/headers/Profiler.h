#pragma once
#include <cstddef>

namespace Profiler
{
	void recordFrame();
	void beginFenceWait();
	void endFenceWait();
	void recordGPUAllocation(std::size_t size);
	void recordGPUDeallocation(std::size_t size);
	void drawUI();
}