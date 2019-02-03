#pragma once
#include "AutoName.h"

#include <glm\gtc\matrix_transform.hpp>
#include <string>
#include <string_view>
#include <optional>

class Shader : public AutoName<Shader>
{
private:
	unsigned int ID = 0;
	bool initialized = false;
	bool isCompute = false;
	std::string const computePath;
	std::string const vertexPath;
	std::string const fragmentPath;
	std::optional<std::string const> const geometryPath;

public:
	Shader(std::string const vertexPath, std::string const fragmentPath, std::optional<std::string const> geometryPath = std::nullopt);
	Shader(std::string const computePath);

private:
	int getLocation(std::string_view const name) const;

public:
	std::string getNamePrefix() const override;
	void reload();
	void use();
	void set(std::string_view const name, int value) const;
	void set(std::string_view const name, float value) const;
	void set(std::string_view const name, glm::uvec3 const& value) const;
	void set(std::string_view const name, glm::vec2 const& value) const;
	void set(std::string_view const name, glm::vec3 const& value) const;
	void set(std::string_view const name, glm::vec4 const& value) const;
	void set(std::string_view const name, glm::mat4 const& value) const;
	void drawUI();
};