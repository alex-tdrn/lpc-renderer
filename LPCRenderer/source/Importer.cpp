#include "Importer.h"
#include "SceneManager.h"
#include "PCManager.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"

#include <fstream>
#include <iostream>

struct MeshData
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::u8vec3> colors;
};

namespace Importer
{
	static std::vector<MeshData> importCONF(std::filesystem::path const& filename);
	static MeshData importPLY(std::filesystem::path const& filename);

	void import(std::vector<std::filesystem::path> const& filenames)
	{
		std::vector<MeshData> meshes;
		std::string name = filenames.front().filename().string();
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

		std::size_t verticesCount = 0;
		for (auto const& mesh : meshes)
			verticesCount += mesh.positions.size();
		std::vector<glm::vec3> allPositions;
		allPositions.reserve(verticesCount);
		
		bool useNormals = true;
		bool useColors = true;

		for (auto& mesh : meshes)
		{
			allPositions.insert(allPositions.end(),
				std::make_move_iterator(mesh.positions.begin()),
				std::make_move_iterator(mesh.positions.end())
			);
			useNormals = useNormals && !mesh.normals.empty();
			useColors = useColors && !mesh.colors.empty();
		}
		std::vector<glm::vec3> allNormals;
		if (useNormals)
		{
			allNormals.reserve(verticesCount);
			for (auto& mesh : meshes)
			{
				allNormals.insert(allNormals.end(),
					std::make_move_iterator(mesh.normals.begin()),
					std::make_move_iterator(mesh.normals.end())
				);
			}
		}

		std::vector<glm::u8vec3> allColors;
		if(useColors)
		{
			allColors.reserve(verticesCount);
			for(auto& mesh : meshes)
			{
				allColors.insert(allColors.end(),
					std::make_move_iterator(mesh.colors.begin()),
					std::make_move_iterator(mesh.colors.end())
				);
			}
		}

		bool makeDecimated = allPositions.size() > 1'000'000;
		auto cloud = PCManager::add(std::make_unique<PointCloud>(std::move(allPositions), std::move(allNormals), std::move(allColors)));
		SceneManager::add(std::make_unique<Scene>(cloud));
		if(makeDecimated)
		{
			auto decimatedCloud = PCManager::add(cloud->decimate(1'000'000));
			decimatedCloud->setName(cloud->getName() + "(decimated)");
			SceneManager::add(std::make_unique<Scene>(decimatedCloud));
		}
	}

	std::vector<MeshData> importCONF(std::filesystem::path const& filename)
	{
		std::vector<MeshData> meshes;
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
				for (auto& position : meshes.back().positions)
					position = translationMatrix * rotationMatrix * glm::vec4{ position, 1.0f };
			}
		}
		return meshes;
	}

	MeshData importPLY(std::filesystem::path const& filename)
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
		bool normalsAvailable = true;
		std::shared_ptr<tinyply::PlyData> plyNormals;
		try
		{
			plyNormals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"});
		}
		catch(...)
		{
			normalsAvailable = false;
		}

		bool colorsAvailable = true;
		std::shared_ptr<tinyply::PlyData> plyColors;
		try
		{
			plyColors = file.request_properties_from_element("vertex", {"red", "green", "blue"});
		}
		catch(...)
		{
			colorsAvailable = false;
		}
		file.read(fileStream);

		MeshData mesh;
		mesh.positions.resize(plyPositions->count);
		std::memcpy(mesh.positions.data(), plyPositions->buffer.get(), plyPositions->buffer.size_bytes());
		if(normalsAvailable)
		{
			mesh.normals.resize(plyNormals->count);
			std::memcpy(mesh.normals.data(), plyNormals->buffer.get(), plyNormals->buffer.size_bytes());
		}
		if(colorsAvailable)
		{
			mesh.colors.resize(plyColors->count);
			std::memcpy(mesh.colors.data(), plyColors->buffer.get(), plyColors->buffer.size_bytes());
		}
		return mesh;
	}

}
