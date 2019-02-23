#include "MainRenderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "imgui.h"
#include "Scene.h"
#include "Shader.h"
#include "PointCloud.h"
#include "PCRenderer.h"
#include "PCRendererBasic.h"

#include <array>

enum class DrawBricksMode
{
	disabled,
	all,
	nonEmpty
};

namespace
{
	std::unique_ptr<PCRenderer> pointCloudRenderer = nullptr;
	DrawBricksMode drawBricksMode = DrawBricksMode::all;
}

void drawBricks(PointCloud const* cloud, glm::mat4 mvp, bool drawEmptyBricks);

void MainRenderer::render(Scene* scene)
{
	if(!pointCloudRenderer)
		pointCloudRenderer = std::make_unique<PCRendererBasic>();
	if(!scene)
		return;
	static PointCloud const* currentPointCloud = nullptr;
	if(currentPointCloud != scene->getPointCloud())
	{
		currentPointCloud = scene->getPointCloud();
		pointCloudRenderer->setPointCloud(currentPointCloud);
	}

	glm::vec3 backgroundColor = scene->getBackgroundColor();
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);

	if(!scene->getPointCloud())
		return;

	glPointSize(1.0f);

	glm::mat4 m = scene->getModelMatrix();
	glm::mat4 v = scene->getCamera().getViewMatrix();
	glm::mat4 p = scene->getCamera().getProjectionMatrix();
	switch (drawBricksMode)
	{
		case DrawBricksMode::all:
			drawBricks(currentPointCloud, p * v * m, true);
			break;
		case DrawBricksMode::nonEmpty:
			drawBricks(currentPointCloud, p * v * m, false);
			break;
	}

	Shader* mainShader = pointCloudRenderer->getMainShader();
	mainShader->use();
	mainShader->set("model", m);
	mainShader->set("view", v);
	mainShader->set("projection", p);
	mainShader->set("diffuseColor", scene->getDiffuseColor());

	/*if (compression == Compression::brickGS || compression == Compression::brickIndirect || compression == Compression::bitmap)
	{
		mainShader->set("cloudOrigin", currentPointCloud->getBounds().first);
		mainShader->set("brickSize", currentPointCloud->getBrickSize());
	}
	if (compression == Compression::brickIndirect || compression == Compression::bitmap)
	{
		mainShader->set("subdivisions", glm::uvec3(currentPointCloud->getSubdivisions()));
	}

	if(renderMode == RenderMode::lit || renderMode == RenderMode::litDisk)
	{
		mainShader->set("backFaceCulling", backFaceCulling);
		mainShader->set("specularColor", scene->getSpecularColor());
		mainShader->set("shininess", scene->getShininess());
		mainShader->set("ambientStrength", scene->getAmbientStrength());
		mainShader->set("ambientColor", backgroundColor);
		mainShader->set("light.color", scene->getLightColor());
		glm::vec3 viewSpaceLightDirection = v * glm::vec4(scene->getLightDirection(), 0.0f);
		mainShader->set("light.direction", viewSpaceLightDirection);
		if(renderMode == RenderMode::litDisk)
		{
			mainShader->set("diskRadius", diskRadius);
		}
	}
	else if(renderMode == RenderMode::debugNormals)
	{
		glLineWidth(debugNormalsLineThickness);
		mainShader->set("lineLength", debugNormalsLineLength);
	}*/

	pointCloudRenderer->render();
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

	pointCloudRenderer->drawUI();
	/*if(renderMode == RenderMode::lit || renderMode == RenderMode::litDisk)
		ImGui::Checkbox("Backface Culling", &backFaceCulling);

	if(renderMode == RenderMode::litDisk)
		ImGui::DragFloat("Disk Radius", &diskRadius, 0.00001f, 0.00001f, 0.005f, "%.5f");

	if(renderMode == RenderMode::debugNormals)
	{
		ImGui::DragFloat("Line Length", &debugNormalsLineLength, 0.0001f, 0.0001f, 0.01f, "%.4f");
		ImGui::SliderInt("Line Thickness", &debugNormalsLineThickness, 1, 16);
	}*/
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
