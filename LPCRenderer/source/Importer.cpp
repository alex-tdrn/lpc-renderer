#include "Importer.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"

#include <fstream>
#include <iostream>

namespace Importer
{
	namespace
	{
		bool importIntoActiveScene = false;
		bool joinMeshesOnImport = false;
	}
	
	static std::vector<std::unique_ptr<Mesh>> importCONF(std::filesystem::path const& filename);
	static std::unique_ptr<Mesh> importPLY(std::filesystem::path const& filename);

	void import(std::vector<std::filesystem::path> const& filenames)
	{
		std::vector<std::unique_ptr<Mesh>> meshes;
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
		std::vector<Prop> props;
		if(joinMeshesOnImport)
		{
			props.emplace_back(MeshManager::add(Mesh::join(std::move(meshes))));
		}
		else
		{
			for(auto& mesh : meshes)
				props.emplace_back(MeshManager::add(std::move(mesh)));
		}

		if(importIntoActiveScene && SceneManager::getActive())
			SceneManager::getActive()->addProps(std::move(props));
		else
			SceneManager::add(std::make_unique<Scene>(std::move(props)));
	}

	std::vector<std::unique_ptr<Mesh>> importCONF(std::filesystem::path const& filename)
	{
		std::vector<std::unique_ptr<Mesh>> meshes;
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
				meshes.back()->applyTransformation(translationMatrix * rotationMatrix);
			}
		}
		return meshes;
	}

	std::unique_ptr<Mesh> importPLY(std::filesystem::path const& filename)
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
		return mesh;
	}

	void drawUI()
	{
		ImGui::Checkbox("Import Into Active Scene", &importIntoActiveScene);
		ImGui::Checkbox("Join Meshes on Import", &joinMeshesOnImport);
	}
}
