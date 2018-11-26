#include "MeshManager.h"

#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"

#include <fstream>

std::string const Manager<Mesh>::name = "Meshes";

static Mesh* loadPLY(std::filesystem::path const& filename)
{
	std::ifstream fileStream{filename, std::ios::binary};
	if(fileStream.fail())
	{
		std::cerr << "Failed to open" + filename.string();
		std::terminate();
	}

	tinyply::PlyFile file;
	file.parse_header(fileStream);
	std::shared_ptr<tinyply::PlyData> plyVertices =
		file.request_properties_from_element("vertex", {"x", "y", "z"});
	file.read(fileStream);
	std::vector<Vertex> vertices(plyVertices->count);
	std::memcpy(vertices.data(), plyVertices->buffer.get(), plyVertices->buffer.size_bytes());
	auto mesh = std::make_unique<Mesh>(std::move(vertices));
	mesh->setName(filename.stem().string());
	return MeshManager::add(std::move(mesh));
}

std::vector<Mesh*> MeshManager::load(std::filesystem::path const& filename)
{
	if(filename.extension().string() == ".ply")
	{
		return {loadPLY(filename)};
	}
	else if(filename.extension().string() == ".conf")
	{
		std::vector<Mesh*> ret;
		std::ifstream filestream{filename};
		if(filestream.fail())
		{
			std::cerr << "Failed to open" + filename.string();
			std::terminate();
		}
		std::string token;
		while(filestream >> token)
		{
			if(token == "bmesh")
			{
				std::string meshName;
				filestream >> meshName;
				ret.push_back(loadPLY(filename.parent_path() / meshName));
			}
		}
		return ret;
	}
	else
	{
		return {};
	}
}