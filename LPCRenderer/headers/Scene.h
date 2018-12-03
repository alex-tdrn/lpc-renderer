#pragma once
#include "PointCloud.h"
#include "AutoName.h"
#include "Camera.h"

#include <vector>

class Scene : public AutoName<Scene>
{
private:
	glm::vec3 backgroundColor{0.6, 0.7, 0.3};
	Camera camera;
	glm::vec3 lightDirection{0.0f, 0.0f, 1.0f};
	glm::vec3 lightColor{1.0f};
	glm::vec3 diffuseColor{0.3f, 0.4f, 0.2f};
	glm::vec3 specularColor{1.0f};
	float shininess = 32.0f;
	float ambientStrength = 0.25f;
	PointCloud* cloud = nullptr;
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	float scaling = 1.0f;

public:
	Scene(PointCloud* cloud = nullptr);

public:
	std::string getNamePrefix() const override;
	Camera& getCamera();
	glm::vec3 getBackgroundColor() const;
	glm::vec3 getLightColor() const;
	glm::vec3 getLightDirection() const;
	glm::vec3 getDiffuseColor() const;
	glm::vec3 getSpecularColor() const;
	float getShininess() const;
	float getAmbientStrength() const;
	PointCloud const* getPointCloud() const;
	glm::mat4 getModelMatrix() const;
	void drawUI();

};