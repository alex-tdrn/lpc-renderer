#pragma once

#include <filesystem>

namespace Importer
{
	void import(std::vector<std::filesystem::path> const& filenames);
};
