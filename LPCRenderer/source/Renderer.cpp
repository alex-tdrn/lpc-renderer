#include "Renderer.h"
#include "OSWindow.h"
#include "glad/glad.h"
#include "imgui.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "PointCloud.h"

static void drawOctree(Octree const* octree, glm::mat4 mvp);

Renderer::Renderer()
{
	if(pointCloudBufffers.empty())
		pointCloudBufffers.resize(nBuffers);
}

void Renderer::render(Scene* scene) const
{
	if(!scene)
		return;

	glm::vec3 backgroundColor = scene->getBackgroundColor();
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);

	if(!scene->getPointCloud())
		return;

	if(activeShader == ShaderManager::pcBarebones() ||
		activeShader == ShaderManager::pcLit())
		glPointSize(pointSize);
	else
		glPointSize(1.0f);

	glm::mat4 m = scene->getModelMatrix();
	glm::mat4 v = scene->getCamera().getViewMatrix();
	glm::mat4 p = scene->getCamera().getProjectionMatrix();
	currentPointCloud = scene->getPointCloud();
	if(decimation)
		currentPointCloud = currentPointCloud->decimated(decimationMaxVertices);

	if(drawOctreeBoundingBoxes)
		drawOctree(currentPointCloud->octree(octreeMaxVerticesPerNode, octreeMaxDepth), p * v * m);

	activeShader->use();
	activeShader->set("model", m);
	activeShader->set("view", v);
	activeShader->set("projection", p);

	activeShader->set("diffuseColor", scene->getDiffuseColor());
	if(activeShader == ShaderManager::pcLit() || 
		activeShader == ShaderManager::pcLitDisk())
	{
		activeShader->set("backFaceCulling", backFaceCulling);
		activeShader->set("specularColor", scene->getSpecularColor());
		activeShader->set("shininess", scene->getShininess());
		activeShader->set("ambientStrength", scene->getAmbientStrength());
		activeShader->set("ambientColor", backgroundColor);
		activeShader->set("light.color", scene->getLightColor());
		glm::vec3 viewSpaceLightDirection = v * glm::vec4(scene->getLightDirection(), 0.0f);
		activeShader->set("light.direction", viewSpaceLightDirection);
		if(activeShader == ShaderManager::pcLitDisk())
		{
			activeShader->set("diskRadius", diskRadius);
		}
	}
	else if(activeShader == ShaderManager::pcDebugNormals())
	{
		glLineWidth(debugNormalsLineThickness);
		activeShader->set("lineLength", debugNormalsLineLength);
	}
	
	currentPointCloudBuffer %= pointCloudBufffers.size();
	pointCloudBufffers[currentPointCloudBuffer++].updateAndUse(currentPointCloud, useNormalsIfAvailable, bufferOrphaning);
}

std::string Renderer::getNamePrefix() const
{
	return "Renderer";
}

void Renderer::drawUI()
{
	ImGui::PushID(this);
	ImGui::Checkbox("Buffer Orphaning", &bufferOrphaning);
	ImGui::SameLine();
	if(ImGui::Button("Shrink Buffers"))
		for(auto& buffer : pointCloudBufffers)
			buffer.shrink();
	nBuffers = pointCloudBufffers.size();
	ImGui::InputInt("# of Buffers", &nBuffers, 1);
	if(nBuffers <= 0)
		nBuffers = 1;
	if(nBuffers != pointCloudBufffers.size())
	{
		pointCloudBufffers.resize(nBuffers);
	}
	ImGui::Checkbox("Draw Octree", &drawOctreeBoundingBoxes);
	if(drawOctreeBoundingBoxes)
	{
		ImGui::Text("Max Vertices Per Node");
		ImGui::DragInt("###InputMaxVerticesPerNode", &octreeMaxVerticesPerNode, 1'000, 1, std::numeric_limits<int>::max());
		ImGui::InputInt("Max Depth", &octreeMaxDepth, 1);
		if(octreeMaxDepth <= 0)
			octreeMaxDepth = 1;
	}
	if(currentPointCloud)
	{
		ImGui::Text("Rendering %lu Vertices", currentPointCloud->getSize());
		ImGui::SameLine();
		if(currentPointCloud->hasNormals() && useNormalsIfAvailable)
			ImGui::Text("With Normals");
		else
			ImGui::Text("Without Normals");
	}
	ImGui::Checkbox("Decimation", &decimation);
	ImGui::SameLine();
	ImGui::Checkbox("Frustum Culling", &frustumCulling);
	if(decimation)
	{
		ImGui::Text("Max Vertices");
		ImGui::SameLine();
		ImGui::DragInt("###InputMaxVertices", &decimationMaxVertices, 10'000, 1, std::numeric_limits<int>::max());
		if(decimationMaxVertices < 1)
			decimationMaxVertices = 1;
	}

	ImGui::Separator();
	ImGui::Text("Rendering Method");
	if(ImGui::RadioButton("Barebones", activeShader == ShaderManager::pcBarebones()))
	{
		activeShader = ShaderManager::pcBarebones();
		useNormalsIfAvailable = false;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Debug Normals", activeShader == ShaderManager::pcDebugNormals()))
	{
		activeShader = ShaderManager::pcDebugNormals();
		useNormalsIfAvailable = true;
	}
	if(ImGui::RadioButton("Lit", activeShader == ShaderManager::pcLit()))
	{
		activeShader = ShaderManager::pcLit();
		useNormalsIfAvailable = true;
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit Disk", activeShader == ShaderManager::pcLitDisk()))
	{
		activeShader = ShaderManager::pcLitDisk();
		useNormalsIfAvailable = true;
	}
	if(activeShader == ShaderManager::pcBarebones() || 
		activeShader == ShaderManager::pcLit())
		ImGui::SliderInt("Point Size", &pointSize, 1, 16);

	if(activeShader == ShaderManager::pcLit() ||
		activeShader == ShaderManager::pcLitDisk())
		ImGui::Checkbox("Backface Culling", &backFaceCulling);

	if(activeShader == ShaderManager::pcLitDisk())
		ImGui::DragFloat("Disk Radius", &diskRadius, 0.00001f, 0.00001f, 0.005f, "%.5f");

	if(activeShader == ShaderManager::pcDebugNormals())
	{
		ImGui::DragFloat("Line Length", &debugNormalsLineLength, 0.0001f, 0.0001f, 0.01f, "%.4f");
		ImGui::SliderInt("Line Thickness", &debugNormalsLineThickness, 1, 16);
	}

	ImGui::PopID();
}

void drawOctree(glm::mat4 mvp, std::optional<std::vector<glm::mat4>> nodeAttributes = std::nullopt)
{
	static unsigned int VAO = []() -> unsigned int{
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		std::array<glm::vec3, 8> vertices = {
			//front face
			//top left
			glm::vec3{-1.0f, +1.0f, +1.0f},
			//bottom left
			glm::vec3{-1.0f, -1.0f, +1.0f},
			//bottom right
			glm::vec3{+1.0f, -1.0f, +1.0f},
			//top right
			glm::vec3{+1.0f, +1.0f, +1.0f},

			//back face
			//top left
			glm::vec3{+1.0f, +1.0f, -1.0f},
			//bottom left
			glm::vec3{+1.0f, -1.0f, -1.0f},
			//bottom right
			glm::vec3{-1.0f, -1.0f, -1.0f},
			//top right
			glm::vec3{-1.0f, +1.0f, -1.0f}
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

	static unsigned int instanceVBO = [](){
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(0));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*) (4 * sizeof(float)));
		glVertexAttribDivisor(2, 1);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*) (8 * sizeof(float)));
		glVertexAttribDivisor(3, 1);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*) (12 * sizeof(float)));
		glVertexAttribDivisor(4, 1);
		return VBO;

	}();
	static int nodeCount = 0;
	if(nodeAttributes)
	{
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * nodeCount, nullptr, GL_STATIC_DRAW);
		nodeCount = nodeAttributes->size();
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * nodeCount, nodeAttributes->data(), GL_STATIC_DRAW);
	}
	ShaderManager::box()->use();
	ShaderManager::box()->set("mvp", mvp);
	glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_BYTE, 0, nodeCount);

}

glm::mat4 calculateBoundsTransfom(std::pair<glm::vec3, glm::vec3> bounds)
{
	glm::vec3 center = (bounds.first + bounds.second) / 2.0f;
	bounds.first -= center;
	glm::vec3 scale = -bounds.first;

	glm::mat4 t = glm::translate(glm::mat4{1.0f}, center);
	glm::mat4 s = glm::scale(glm::mat4{1.0f}, glm::vec3{scale});
	return t * s;
}

void drawOctree(Octree const* octree, glm::mat4 mvp)
{
	static Octree const* cachedOctree = nullptr;
	static int cachedMaxDepth = 1;
	static std::size_t cachedMaxVerticesPerNode = 100'000;
	static std::size_t cachedTotalVerticesCount = 0;
	if(octree != cachedOctree ||
		octree->getMaxVerticesPerNode() != cachedMaxVerticesPerNode ||
		octree->getTotalVerticesCount() != cachedTotalVerticesCount ||
		octree->getMaxDepth() != cachedMaxDepth)
	{
		cachedOctree = octree;
		cachedMaxVerticesPerNode = octree->getMaxVerticesPerNode();
		cachedTotalVerticesCount = octree->getTotalVerticesCount();
		cachedMaxDepth = octree->getMaxDepth();

		std::vector<glm::mat4> nodeAttributes;

		std::vector<Octree const*> leafNodes;
		octree->getAllLeafNodes(leafNodes);
		for(auto const& leafNode : leafNodes)
		{
			std::size_t totalVertices = leafNode->getTotalVerticesCount();
			if(totalVertices > 0)
			{
				glm::mat4 attribute = calculateBoundsTransfom(leafNode->getBounds());

				if(totalVertices < leafNode->getMaxVerticesPerNode())
					attribute[0][3] = 1.0f;

				nodeAttributes.push_back(attribute);
			}
		}
		drawOctree(mvp, nodeAttributes);
	}
	else
	{
		drawOctree(mvp);
	}
}
