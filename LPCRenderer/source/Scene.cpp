#include "Scene.h"
#include "imgui.h"

Scene::Scene(std::vector<Prop>&& props)
	:props(std::move(props))
{

}

std::string Scene::getNamePrefix() const
{
	return "Scene";
}

void Scene::addProp(Prop&& prop)
{
	props.push_back(prop);
}

std::vector<Prop> const& Scene::getProps() const
{
	return props;
}

Camera& Scene::getCamera()
{
	return camera;
}

void Scene::drawUI()
{
	ImGui::PushID(this);
	static std::size_t currentProp = 0;
	static bool noneSelected = false;
	if(currentProp >= props.size())
		currentProp = 0;
	if(props.empty())
		noneSelected = true;
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Prop");
	std::string currentName = "None";
	if(!noneSelected)
	{
		auto& prop = props[currentProp];
		currentName = prop.getName();
		if(!prop.enabled())
			currentName += "(*)";
	}
	if(ImGui::BeginCombo("###Prop", currentName.data()))
	{
		for(int i = 0; i < props.size(); i++)
		{
			ImGui::PushID(i);
			bool isSelected = i == currentProp;
			ImGui::Selectable(props[i].getName().data(), &isSelected);
			if(isSelected)
			{
				ImGui::SetItemDefaultFocus();
				currentProp = i;
				noneSelected = false;
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	if(!noneSelected)
	{
		props[currentProp].drawUI();
		ImGui::Separator();
	}
	ImGui::NewLine();
	ImGui::Text("Camera");
	camera.drawUI();
	ImGui::PopID();
}
