#pragma once
#include "Manager.h"
#include "Scene.h"

#include <filesystem>

class SceneManager : public Manager<Scene, true>
{
public:
	static inline bool importIntoActiveScene = false;
	static Scene* load(std::filesystem::path const& filename);
	static void drawUI(bool* open);

};

