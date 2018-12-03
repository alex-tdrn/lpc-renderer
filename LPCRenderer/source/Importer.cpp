#include "Importer.h"
#include "SceneManager.h"
#include "PointCloudManager.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"

#include <fstream>
#include <iostream>

namespace Importer
{
	static std::vector<std::unique_ptr<PointCloud>> importCONF(std::filesystem::path const& filename);
	static std::unique_ptr<PointCloud> importPLY(std::filesystem::path const& filename);

	void import(std::vector<std::filesystem::path> const& filenames)
	{
		std::vector<std::unique_ptr<PointCloud>> meshes;
		for(auto& filename : filenames)
		{
			if(filename.extension().string() == ".ply")
			{
				meshes.push_back(importPLY(filename));
			}
			else if(filename.extension().string() == ".conf")
			{
				auto m = importCONF(filename);
				meshes.insert(meshes.end(),
					std::make_move_iterator(m.begin()),
					std::make_move_iterator(m.end())
				);
			}
		}
		auto cloud = PointCloudManager::add(PointCloud::join(std::move(meshes)));
		SceneManager::add(std::make_unique<Scene>(cloud));
	}

	std::vector<std::unique_ptr<PointCloud>> importCONF(std::filesystem::path const& filename)
	{
		std::vector<std::unique_ptr<PointCloud>> meshes;
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
				glm::vec3 translation;
				filestream >> translation.x;
				filestream >> translation.y;
				filestream >> translation.z;
				glm::quat rotation;
				filestream >> rotation.x;
				filestream >> rotation.y;
				filestream >> rotation.z;
				filestream >> rotation.w;
				glm::mat4 rotationMatrix = glm::transpose(glm::mat4_cast(rotation));
				glm::mat4 translationMatrix = glm::translate(glm::mat4{1.0f}, translation);
				meshes.push_back(importPLY(filename.parent_path() / meshName));
				meshes.back()->transform(translationMatrix * rotationMatrix);
			}
		}
		return meshes;
	}

	std::unique_ptr<PointCloud> importPLY(std::filesystem::path const& filename)
	{
		std::ifstream fileStream{filename, std::ios::binary};
		if(fileStream.fail())
		{
			std::cerr << "Failed to open" + filename.string();
			std::terminate();
		}

		tinyply::PlyFile file;
		file.parse_header(fileStream);
		std::shared_ptr<tinyply::PlyData> plyPositions =
			file.request_properties_from_element("vertex", {"x", "y", "z"});
		file.read(fileStream);
		std::vector<glm::vec3> vertices(plyPositions->count);
		std::memcpy(vertices.data(), plyPositions->buffer.get(), plyPositions->buffer.size_bytes());
		auto mesh = std::make_unique<PointCloud>(std::move(vertices));
		mesh->setName(filename.stem().string());
		return mesh;
	}

}
