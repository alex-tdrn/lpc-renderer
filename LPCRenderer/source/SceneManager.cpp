#include "SceneManager.h"
#include "MeshManager.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <fstream>
#include <iostream>

std::string const Manager<Scene, true>::name = "Scenes";

Scene* SceneManager::load(std::filesystem::path const& filename)
{

	std::vector<Prop> props;
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
			props.push_back(MeshManager::load(filename.parent_path() / meshName));
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
			props.back().setTransformation(translationMatrix * rotationMatrix);
		}
	}
	return add(std::make_unique<Scene>(std::move(props)));
}