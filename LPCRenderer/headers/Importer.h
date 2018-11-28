#pragma once

#include <filesystem>

class Scene;
class Mesh;

namespace Importer
{
	void import(std::vector<std::filesystem::path> const& filenames);
	void drawUI();

};
