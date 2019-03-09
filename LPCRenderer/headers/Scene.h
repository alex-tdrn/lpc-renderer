#pragma once
#include "AutoName.h"
#include "Camera.h"

#include <vector>

class PointCloud;

class Scene : public AutoName<Scene>
{
private:
	glm::vec3 backgroundColor{0.065f, 0.0f, 0.125f};
	Camera camera;
	glm::vec3 lightDirection = glm::normalize(glm::vec3{0.0f, -1.0f, -1.0f});
	glm::vec3 lightColor{0.5f, 0.5f, 0.25f};
	glm::vec3 diffuseColor{0.0f, 1.0f, 0.5f};
	glm::vec3 specularColor{0.25f};
	float shininess = 64.0f;
	float ambientStrength = 0.05f;
	PointCloud* cloud = nullptr;
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	float scaling = 1.0f;
	float normalizationScale = 1.0f;

public:
	Scene(PointCloud* cloud);

public:
	std::string getNamePrefix() const override;
	Camera& getCamera();
	Camera const& getCamera() const;
	glm::vec3 getBackgroundColor() const;
	glm::vec3 getLightColor() const;
	glm::vec3 getLightDirection() const;
	glm::vec3 getDiffuseColor() const;
	glm::vec3 getSpecularColor() const;
	float getShininess() const;
	float getAmbientStrength() const;
	PointCloud const* getPointCloud() const;
	glm::mat4 getModelMatrix() const;
	float getScaling() const;
	void drawUI();

};