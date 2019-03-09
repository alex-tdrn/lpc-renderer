#include "PCRendererBrickIndirect.h"
#include "Shader.h"
#include "PointCloud.h"
#include "Scene.h"
#include "imgui.h"

enum class RenderMode
{
	basic,
	lit,
	litColoured
};

namespace
{
	Shader basicShader{"shaders/pcBrickIndirect.vert", "shaders/pcBrickIndirect.frag"};
	Shader litShader{"shaders/pcBrickIndirectLit.vert", "shaders/pcLitDisk.frag", "shaders/pcLitDisk.geom"};
	Shader litColouredShader{"shaders/pcBrickIndirectLit.vert", "shaders/pcBrickIndirect.frag"};
	RenderMode renderMode = RenderMode::basic;
	
	bool backFaceCulling = true;
	int pointSize = 2;
	float diskRadius = 0.0005f;
	int positionSize = 16;
	int normalSize = 16;
	glm::vec2 toSpherical(glm::vec3 n)
	{
		float thetaNormalized = glm::acos(n.y) / glm::pi<float>();
		float phiNormalized = (glm::atan(n.x, n.z) / glm::pi<float>()) * 0.5 + 0.5;
		return glm::vec2(phiNormalized, thetaNormalized);
	}

	std::uint32_t toSpherical16(glm::vec3 n)
	{
		return glm::packUnorm2x16(toSpherical(n));
	}

	std::uint16_t toSpherical8(glm::vec3 n)
	{
		glm::vec2 s = toSpherical(n);
		std::uint16_t packed = 256 * s.x;
		packed |= std::uint16_t(256 * s.y) << 8;
		return packed;
	}
}

PCRendererBrickIndirect::PCRendererBrickIndirect()
	:PCRenderer(&basicShader)
{
}

bool PCRendererBrickIndirect::needNormals() const
{
	return renderMode != RenderMode::basic;
}

bool PCRendererBrickIndirect::needColours() const
{
	return renderMode == RenderMode::litColoured;
}

void PCRendererBrickIndirect::updatePositions32()
{
	static std::vector<DrawCommand> indirectDraws;
	static std::vector<std::uint32_t> compressedPositions;

	int brickIndex = -1;
	for(auto brick : cloud->getAllBricks())
	{
		brickIndex++;
		if(brick.positions.empty())
			continue;
		DrawCommand brickDrawCommand{};
		brickDrawCommand.count = brick.positions.size();
		brickDrawCommand.first = compressedPositions.size();
		brickDrawCommand.baseInstance = brickIndex;
		indirectDraws.push_back(std::move(brickDrawCommand));
		for(auto const& position : brick.positions)
			compressedPositions.push_back(packPosition1024(position));
	}
	indirectDrawCount = indirectDraws.size();

	bindVAO();
	DrawBuffer.write({{(std::byte const*)indirectDraws.data(), indirectDraws.size() * sizeof(indirectDraws.front())}});
	DrawBuffer.bind();
	VBOPositions.write({{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front())}});
	VBOPositions.bind();
	glEnableVertexAttribArray(0);//Compressed Positions
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)(0));

	compressedPositions.clear();
	indirectDraws.clear();
}

void PCRendererBrickIndirect::updatePositions16()
{
	static std::vector<DrawCommand> indirectDraws;
	static std::vector<std::uint16_t> compressedPositions;

	int brickIndex = -1;
	for(auto brick : cloud->getAllBricks())
	{
		brickIndex++;
		if(brick.positions.empty())
			continue;
		DrawCommand brickDrawCommand{};
		brickDrawCommand.count = brick.positions.size();
		brickDrawCommand.first = compressedPositions.size();
		brickDrawCommand.baseInstance = brickIndex;
		indirectDraws.push_back(std::move(brickDrawCommand));
		for(auto const& position : brick.positions)
			compressedPositions.push_back(packPosition32(position));
	}
	indirectDrawCount = indirectDraws.size();

	bindVAO();
	DrawBuffer.write({{(std::byte const*)indirectDraws.data(), indirectDraws.size() * sizeof(indirectDraws.front())}});
	DrawBuffer.bind();
	VBOPositions.write({{(std::byte const*)compressedPositions.data(), compressedPositions.size() * sizeof(compressedPositions.front())}});
	VBOPositions.bind();
	glEnableVertexAttribArray(0);//Compressed Positions
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, 0, (void*)(0));

	compressedPositions.clear();
	indirectDraws.clear();
}


void PCRendererBrickIndirect::updateNormals16()
{
	static std::vector<std::uint32_t> normals;

	for(auto const& brick : cloud->getAllBricks())
	{
		if(brick.positions.empty())
			continue;
		for(glm::vec3 normal : brick.normals)
		{
			normals.push_back(toSpherical16(normal));
		}
	}

	VBONormals.write({{(std::byte const*)normals.data(), normals.size() * sizeof(normals.front())}});
	VBONormals.bind();

	glEnableVertexAttribArray(1);//Normals
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, (void*)(0));

	normals.clear();
}

void PCRendererBrickIndirect::updateNormals8()
{
	static std::vector<std::uint16_t> normals;

	for(auto const& brick : cloud->getAllBricks())
	{
		if(brick.positions.empty())
			continue;
		for(glm::vec3 normal : brick.normals)
		{
			normals.push_back(toSpherical8(normal));
		}
	}

	VBONormals.write({{(std::byte const*)normals.data(), normals.size() * sizeof(normals.front())}});
	VBONormals.bind();

	glEnableVertexAttribArray(1);//Normals
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_SHORT, 0, (void*)(0));

	normals.clear();
}

void PCRendererBrickIndirect::update()
{
	switch(renderMode)
	{
		case RenderMode::basic:
			mainShader = &basicShader;
			break;
		case RenderMode::lit:
			mainShader = &litShader;
			break;
		case RenderMode::litColoured:
			mainShader = &litColouredShader;
			break;
	}

	if(positionSize == 32)
	{
		updatePositions32();
		cloud->setBrickPrecision(1024);
	}
	else
	{
		updatePositions16();
		cloud->setBrickPrecision(32);
	}
	mainShader->use();
	mainShader->set("positionSize", positionSize);
	
	if(needNormals())
	{
		if(normalSize == 16)
			updateNormals16();
		else
			updateNormals8();

		mainShader->set("normalSize", normalSize);
	}
	else
	{
		VBONormals.free();
		glDisableVertexAttribArray(1);
	}
}

void PCRendererBrickIndirect::render(Scene const* scene)
{
	PCRenderer::render(scene);

	mainShader->set("cloudOrigin", cloud->getBounds().first);
	mainShader->set("brickSize", cloud->getBrickSize());
	mainShader->set("subdivisions", glm::uvec3(cloud->getSubdivisions()));

	if(renderMode == RenderMode::lit)
	{
		mainShader->set("diskRadius", diskRadius * scene->getScaling());
		mainShader->set("backFaceCulling", backFaceCulling);
		mainShader->set("specularColor", scene->getSpecularColor());
		mainShader->set("shininess", scene->getShininess());
		mainShader->set("ambientStrength", scene->getAmbientStrength());
		mainShader->set("ambientColor", scene->getBackgroundColor());
		mainShader->set("light.color", scene->getLightColor());
		mainShader->set("light.direction", glm::vec3(scene->getCamera().getViewMatrix() * glm::vec4(scene->getLightDirection(), 0.0f)));
	}
	glPointSize(pointSize);

	bindVAO();
	DrawBuffer.bind();
	glMultiDrawArraysIndirect(GL_POINTS, nullptr, indirectDrawCount, 0);

}

void PCRendererBrickIndirect::drawUI()
{
	PCRenderer::drawUI();
	ImGui::SliderInt("Point Size", &pointSize, 1, 16);
	ImGui::Text("Position Size");
	if(ImGui::RadioButton("32", positionSize == 32))
	{
		positionSize = 32;
		update();
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("16", positionSize == 16))
	{
		positionSize = 16;
		update();
	}

	ImGui::Text("Render Mode");

	if(ImGui::RadioButton("Basic", renderMode == RenderMode::basic))
	{
		renderMode = RenderMode::basic;
		update();
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit", renderMode == RenderMode::lit))
	{
		renderMode = RenderMode::lit;
		update();
	}
	ImGui::SameLine();
	if(ImGui::RadioButton("Lit Coloured", renderMode == RenderMode::litColoured))
	{
		renderMode = RenderMode::litColoured;
		update();
	}
	
	if(needNormals() && cloud && !cloud->hasNormals())
	{
		ImGui::Text("Current render mode needs normals");
		ImGui::Text("but none are present in the dataset!");
	}
	else if(needNormals())
	{
		ImGui::Text("Normal Size");
		ImGui::PushID("NormalSize");
		if(ImGui::RadioButton("16", normalSize == 16))
		{
			normalSize = 16;
			update();
		}
		ImGui::SameLine();
		if(ImGui::RadioButton("8", normalSize == 8))
		{
			normalSize = 8;
			update();
		}
		ImGui::PopID();
	}

	if(needColours() && false)
	{
		ImGui::Text("Current render mode needs colours");
		ImGui::Text("but none are present in the dataset!");
	}

	if(renderMode != RenderMode::basic)
	{
		ImGui::DragFloat("Disk Radius", &diskRadius, 0.00001f, 0.00001f, 0.005f, "%.5f");
		ImGui::Checkbox("Backface Culling", &backFaceCulling);
	}
}

void PCRendererBrickIndirect::reloadShaders()
{
	basicShader.reload();
	litShader.reload();
	litColouredShader.reload();
}
