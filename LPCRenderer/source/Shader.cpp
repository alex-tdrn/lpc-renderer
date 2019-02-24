#include "Shader.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "glad/glad.h"

#include <fstream>
#include <sstream>
#include <iostream>

struct ShaderSource
{
	unsigned int ID;
	ShaderSource(std::string_view const path, GLenum type)
	{
		std::ifstream file(path.data());
		std::stringstream stream;
		stream << file.rdbuf();
		ID = glCreateShader(type);
		std::string source = stream.str();
		char const* aux = source.data();
		glShaderSource(ID, 1, &aux, nullptr);
		glCompileShader(ID);

		int logLength;
		glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) 
		{
			std::string log;
			log.resize(logLength + 1);
			glGetShaderInfoLog(ID, logLength, NULL, log.data());
			std::cout << "ERROR COMPILING " << path << ":\n" << log;
		}

	}
	~ShaderSource()
	{
		glDeleteShader(ID);
	}
};

Shader::Shader(std::string const vertexPath, std::string const fragmentPath, std::optional<std::string const> geometryPath)
	:vertexPath(vertexPath), fragmentPath(fragmentPath), geometryPath(geometryPath)
{
}

Shader::Shader(std::string const computePath)
	:computePath(computePath), isCompute(true)
{
}

int Shader::getLocation(std::string_view const name) const
{
	int res = glGetUniformLocation(ID, name.data());
#ifndef NDEBUG
	assert(res != -1);
#endif
	return res;
}

void Shader::reload()
{
	if(initialized)
		glDeleteProgram(ID);
	initialized = true;
	ID = glCreateProgram();

	if (isCompute)
	{
		ShaderSource compute(computePath, GL_COMPUTE_SHADER);
		glAttachShader(ID, compute.ID);
		glLinkProgram(ID);
	}
	else
	{
		ShaderSource vertex(vertexPath, GL_VERTEX_SHADER);
		glAttachShader(ID, vertex.ID);

		ShaderSource fragment(fragmentPath, GL_FRAGMENT_SHADER);
		glAttachShader(ID, fragment.ID);

		if (geometryPath)
		{
			ShaderSource geometry(*geometryPath, GL_GEOMETRY_SHADER);
			glAttachShader(ID, geometry.ID);
			glLinkProgram(ID);
		}
		else
		{
			glLinkProgram(ID);
		}
	}

	int logLength;
	glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		std::string log;
		log.resize(logLength + 1);
		glGetProgramInfoLog(ID, logLength, NULL, log.data());
		std::cout << "ERROR LINKING: \n" << log;
	}
}

void Shader::use()
{
	if(!initialized)
		reload();
	glUseProgram(ID);
}

void Shader::set(std::string_view const name, int value) const
{
	glUniform1i(getLocation(name), value);
}
void Shader::set(std::string_view const name, unsigned int value) const
{
	glUniform1ui(getLocation(name), value);
}
void Shader::set(std::string_view const name, std::size_t value) const
{
	glUniform1ui(getLocation(name), value);
}
void Shader::set(std::string_view const name, float value) const
{
	glUniform1f(getLocation(name), value);
}
void Shader::set(std::string_view const name, glm::uvec3 const & value) const
{
	glUniform3ui(getLocation(name), value.x, value.y, value.z);
}
void Shader::set(std::string_view const name, glm::vec2 const& value) const
{
	glUniform2f(getLocation(name), value.x, value.y);
}
void Shader::set(std::string_view const name, glm::vec3 const& value) const
{
	glUniform3f(getLocation(name), value.x, value.y, value.z);
}
void Shader::set(std::string_view const name, glm::vec4 const& value) const
{
	glUniform4f(getLocation(name), value.x, value.y, value.z, value.w);
}
void Shader::set(std::string_view const name, glm::mat4 const& value) const
{
	glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::drawUI()
{
	if(ImGui::Button("Reload"))
		reload();
}
