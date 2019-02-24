#include "MainRenderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "imgui.h"
#include "Scene.h"
#include "Shader.h"
#include "PointCloud.h"
#include "PCRenderer.h"
#include "PCRendererUncompressed.h"
#include "PCRendererBrickGS.h"
#include "PCRendererBrickIndirect.h"
#include "PCRendererBitmap.h"

#include <array>

enum class DrawBricksMode
{
	disabled,
	all,
	nonEmpty
};

enum class CompressionMode
{
	none,
	brickGS,
	brickIndirect,
	bitmap
};

namespace
{
	std::unique_ptr<PCRenderer> pointCloudRenderer = nullptr;
	DrawBricksMode drawBricksMode = DrawBricksMode::all;
	CompressionMode compressionMode = CompressionMode::none;
}

void drawBricks(PointCloud const* cloud, glm::mat4 mvp, bool drawEmptyBricks);

void MainRenderer::render(Scene* scene)
{
	if(!pointCloudRenderer)
		pointCloudRenderer = std::make_unique<PCRendererUncompressed>();
	if(!scene)
		return;
	

	glm::vec3 backgroundColor = scene->getBackgroundColor();
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);

	if(!scene->getPointCloud())
		return;

	static glm::ivec3 previousSubdivions = scene->getPointCloud()->getSubdivisions();
	if(previousSubdivions != scene->getPointCloud()->getSubdivisions())
	{
		previousSubdivions = scene->getPointCloud()->getSubdivisions();
		pointCloudRenderer->update();
	}

	glm::mat4 m = scene->getModelMatrix();
	glm::mat4 v = scene->getCamera().getViewMatrix();
	glm::mat4 p = scene->getCamera().getProjectionMatrix();
	if(drawBricksMode != DrawBricksMode::disabled)
		glPointSize(1.0f);

	switch (drawBricksMode)
	{
		case DrawBricksMode::all:
			drawBricks(scene->getPointCloud(), p * v * m, true);
			break;
		case DrawBricksMode::nonEmpty:
			drawBricks(scene->getPointCloud(), p * v * m, false);
			break;
	}

	Shader* mainShader = pointCloudRenderer->getMainShader();
	mainShader->use();
	mainShader->set("model", m);
	mainShader->set("view", v);
	mainShader->set("projection", p);
	mainShader->set("diffuseColor", scene->getDiffuseColor());

	pointCloudRenderer->render(scene);
}

void MainRenderer::drawUI()
{
	ImGui::Text("Brick Rendering");
	if (ImGui::RadioButton("Disabled", drawBricksMode == DrawBricksMode::disabled))
		drawBricksMode = DrawBricksMode::disabled;

	ImGui::SameLine();
	if (ImGui::RadioButton("All", drawBricksMode == DrawBricksMode::all))
		drawBricksMode = DrawBricksMode::all;

	ImGui::SameLine();
	if (ImGui::RadioButton("Non-Empty", drawBricksMode == DrawBricksMode::nonEmpty))
		drawBricksMode = DrawBricksMode::nonEmpty;
	ImGui::Separator();
	
	ImGui::Text("Compression Mode");
	if(ImGui::RadioButton("None", compressionMode == CompressionMode::none))
	{
		compressionMode = CompressionMode::none;
		pointCloudRenderer = std::make_unique<PCRendererUncompressed>();
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Brick Geometry Shader", compressionMode == CompressionMode::brickGS))
	{
		compressionMode = CompressionMode::brickGS;
		pointCloudRenderer = std::make_unique<PCRendererBrickGS>();
	}
	if(ImGui::RadioButton("Brick Indirect Draw", compressionMode == CompressionMode::brickIndirect))
	{
		compressionMode = CompressionMode::brickIndirect;
		pointCloudRenderer = std::make_unique<PCRendererBrickIndirect>();
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Bitmap", compressionMode == CompressionMode::bitmap))
	{
		compressionMode = CompressionMode::bitmap;
		pointCloudRenderer = std::make_unique<PCRendererBitmap>();
	}

	ImGui::Separator();

	pointCloudRenderer->drawUI();
	
}


void drawBoxes(glm::mat4 mvp, std::optional<std::vector<glm::mat4>> newBoxes = std::nullopt)
{
	static unsigned int VAO = []() -> unsigned int {
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		std::array<glm::vec3, 8> vertices = {
			//front face
			//top left
			glm::vec3{ -1.0f, +1.0f, +1.0f },
			//bottom left
			glm::vec3{ -1.0f, -1.0f, +1.0f },
			//bottom right
			glm::vec3{ +1.0f, -1.0f, +1.0f },
			//top right
			glm::vec3{ +1.0f, +1.0f, +1.0f },

			//back face
			//top left
			glm::vec3{ +1.0f, +1.0f, -1.0f },
			//bottom left
			glm::vec3{ +1.0f, -1.0f, -1.0f },
			//bottom right
			glm::vec3{ -1.0f, -1.0f, -1.0f },
			//top right
			glm::vec3{ -1.0f, +1.0f, -1.0f }
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 3, vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		unsigned int EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		std::array<uint8_t, 24> indices = {
			0, 1, 1, 2, 2, 3, 3, 0,
			0, 7, 7, 6, 6, 1, 2, 5,
			5, 4, 4, 3, 7, 4, 5, 6
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 24, indices.data(), GL_STATIC_DRAW);

		return VAO;
	}();
	glBindVertexArray(VAO);

	static unsigned int instanceVBO = []() {
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(0));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(4 * sizeof(float)));
		glVertexAttribDivisor(2, 1);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(8 * sizeof(float)));
		glVertexAttribDivisor(3, 1);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(12 * sizeof(float)));
		glVertexAttribDivisor(4, 1);
		return VBO;

	}();
	static int boxCount = 0;
	if (newBoxes)
	{
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * boxCount, nullptr, GL_STATIC_DRAW);
		boxCount = newBoxes->size();
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * boxCount, newBoxes->data(), GL_STATIC_DRAW);
	}
	static Shader boxShader{"shaders/box.vert", "shaders/box.frag"};
	boxShader.use();
	boxShader.set("mvp", mvp);
	glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_BYTE, 0, boxCount);

}

glm::mat4 getBoundsTransform(std::pair<glm::vec3, glm::vec3> bounds)
{
	glm::vec3 center = (bounds.first + bounds.second) / 2.0f;
	bounds.first -= center;
	glm::vec3 scale = -bounds.first;

	glm::mat4 t = glm::translate(glm::mat4{ 1.0f }, center);
	glm::mat4 s = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ scale });
	return t * s;
}

void drawBricks(PointCloud const* cloud, glm::mat4 mvp, bool drawEmptyBricks)
{
	auto isDrawDataOutdated = [&]() {
		static PointCloud const* lastCloud = nullptr;
		static bool lastDrawEmptyBricks = false;
		static glm::ivec3 lastSubdivsions{-1};
		if(lastCloud != cloud || lastDrawEmptyBricks != drawEmptyBricks || lastSubdivsions != cloud->getSubdivisions())
		{
			lastCloud = cloud;
			lastDrawEmptyBricks = drawEmptyBricks;
			lastSubdivsions = cloud->getSubdivisions();
			return true;
		}
		return false;
	};
	
	if(isDrawDataOutdated())
	{
		std::vector<glm::mat4> boxes;

		for (auto const& brick : cloud->getAllBricks())
		{
			if (!drawEmptyBricks && brick.positions.empty())
				continue;
			boxes.push_back(getBoundsTransform(cloud->getBoundsAt(brick.indices)));
		}
		drawBoxes(mvp, std::move(boxes));
	}
	else
	{
		drawBoxes(mvp);
	}
}
