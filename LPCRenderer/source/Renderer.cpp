#include "Renderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "imgui.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "PointCloud.h"

Renderer::Renderer()
{
	if(pointCloudBufffers.empty())
		pointCloudBufffers.resize(nBuffers);
}

void drawBricks(PointCloud const* cloud, glm::mat4 mvp, bool drawEmptyBricks);

void Renderer::render(Scene* scene) const
{
	if(!scene)
		return;

	glm::vec3 backgroundColor = scene->getBackgroundColor();
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);

	if(!scene->getPointCloud())
		return;
	auto activeShader = [&](){
		switch(renderMode)
		{
			case Renderer::RenderMode::debugNormals:
				return ShaderManager::pcDebugNormals();
			case Renderer::RenderMode::lit:
				return ShaderManager::pcLit();
			case Renderer::RenderMode::litDisk:
				return ShaderManager::pcLitDisk();
			default:
				switch (compression)
				{
					case Compression::none:
						return ShaderManager::pcBarebones();
					case Compression::brickGS:
						return ShaderManager::pcBarebonesBrickGS();
					case Compression::brickVS:
						return ShaderManager::pcBarebones();
				}
		}
	}();

	if(renderMode == RenderMode::barebones || renderMode == RenderMode::lit)
		glPointSize(pointSize);
	else
		glPointSize(1.0f);

	glm::mat4 m = scene->getModelMatrix();
	glm::mat4 v = scene->getCamera().getViewMatrix();
	glm::mat4 p = scene->getCamera().getProjectionMatrix();
	currentPointCloud = scene->getPointCloud();
	switch (drawBricksMode)
	{
		case DrawBricksMode::all:
			drawBricks(currentPointCloud, p * v * m, true);
			break;
		case DrawBricksMode::nonEmpty:
			drawBricks(currentPointCloud, p * v * m, false);
			break;
	}

	/*if(decimation)
		currentPointCloud = currentPointCloud->decimated(decimationMaxVertices);*/

	activeShader->use();
	activeShader->set("model", m);
	activeShader->set("view", v);
	activeShader->set("projection", p);

	activeShader->set("diffuseColor", scene->getDiffuseColor());
	if (compression == Compression::brickGS)
	{
		activeShader->set("cloudOrigin", currentPointCloud->getBounds().first);
		activeShader->set("brickSize", currentPointCloud->getBrickSize());

	}
	if(renderMode == RenderMode::lit || renderMode == RenderMode::litDisk)
	{
		activeShader->set("backFaceCulling", backFaceCulling);
		activeShader->set("specularColor", scene->getSpecularColor());
		activeShader->set("shininess", scene->getShininess());
		activeShader->set("ambientStrength", scene->getAmbientStrength());
		activeShader->set("ambientColor", backgroundColor);
		activeShader->set("light.color", scene->getLightColor());
		glm::vec3 viewSpaceLightDirection = v * glm::vec4(scene->getLightDirection(), 0.0f);
		activeShader->set("light.direction", viewSpaceLightDirection);
		if(renderMode == RenderMode::litDisk)
		{
			activeShader->set("diskRadius", diskRadius);
		}
	}
	else if(renderMode == RenderMode::debugNormals)
	{
		glLineWidth(debugNormalsLineThickness);
		activeShader->set("lineLength", debugNormalsLineLength);
	}

	currentPointCloudBuffer %= pointCloudBufffers.size();

	if (refreshBuffers)
	{
		pointCloudBufffers[currentPointCloudBuffer]
			.update(shrinkBuffersToFit, useNormalsIfAvailable, compression, currentPointCloud);
	}
	pointCloudBufffers[currentPointCloudBuffer].render();
	currentPointCloudBuffer++;
}

std::string Renderer::getNamePrefix() const
{
	return "Renderer";
}

void Renderer::drawUI()
{
	ImGui::PushID(this);
	ImGui::Checkbox("Refresh Buffers", &refreshBuffers);
	ImGui::Checkbox("Shrink Buffers To Fit", &shrinkBuffersToFit);
	nBuffers = pointCloudBufffers.size();
	ImGui::InputInt("# of Buffers", &nBuffers, 1);
	if(nBuffers <= 0)
		nBuffers = 1;
	if(nBuffers != pointCloudBufffers.size())
	{
		pointCloudBufffers.resize(nBuffers);
	}
	if(currentPointCloud)
	{
		ImGui::Text("Rendering %lu Vertices", renderedVertices);
		ImGui::SameLine();
		if(currentPointCloud->hasNormals() && useNormalsIfAvailable)
			ImGui::Text("With Normals");
		else
			ImGui::Text("Without Normals");
	}
	ImGui::Checkbox("Decimation", &decimation);
	if(decimation)
	{
		ImGui::SameLine();
		ImGui::Text("Max Vertices");
		ImGui::DragInt("###InputMaxVertices", &decimationMaxVertices, 10'000, 1, std::numeric_limits<int>::max());
		if(decimationMaxVertices < 1)
			decimationMaxVertices = 1;
	}
	ImGui::Separator();
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
	ImGui::Text("Compression");
	if (ImGui::RadioButton("None", compression == Compression::none))
		compression = Compression::none;
	ImGui::SameLine();
	if (ImGui::RadioButton("BrickGS", compression == Compression::brickGS))
		compression = Compression::brickGS;
	ImGui::SameLine();
	if (ImGui::RadioButton("BrickVS", compression == Compression::brickVS))
		compression = Compression::brickVS;

	ImGui::Text("Rendering Mode");
	if(ImGui::RadioButton("Barebones", renderMode == RenderMode::barebones))
	{
		renderMode = RenderMode::barebones;
		useNormalsIfAvailable = false;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Debug Normals", renderMode == RenderMode::debugNormals))
	{
		renderMode = RenderMode::debugNormals;
		useNormalsIfAvailable = true;
	}
	if(ImGui::RadioButton("Lit", renderMode == RenderMode::lit))
	{
		renderMode = RenderMode::lit;
		useNormalsIfAvailable = true;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit Disk", renderMode == RenderMode::litDisk))
	{
		renderMode = RenderMode::litDisk;
		useNormalsIfAvailable = true;
	}
	if(renderMode == RenderMode::barebones || renderMode == RenderMode::lit)
		ImGui::SliderInt("Point Size", &pointSize, 1, 16);

	if(renderMode == RenderMode::lit || renderMode == RenderMode::litDisk)
		ImGui::Checkbox("Backface Culling", &backFaceCulling);

	if(renderMode == RenderMode::litDisk)
		ImGui::DragFloat("Disk Radius", &diskRadius, 0.00001f, 0.00001f, 0.005f, "%.5f");

	if(renderMode == RenderMode::debugNormals)
	{
		ImGui::DragFloat("Line Length", &debugNormalsLineLength, 0.0001f, 0.0001f, 0.01f, "%.4f");
		ImGui::SliderInt("Line Thickness", &debugNormalsLineThickness, 1, 16);
	}

	ImGui::PopID();
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

	ShaderManager::box()->use();
	ShaderManager::box()->set("mvp", mvp);
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
