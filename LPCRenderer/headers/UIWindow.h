#pragma once
#include "imgui.h"
#include <string>

class UIWindow
{
private:
	bool _open;
	std::string name;
	void(*drawFunction)();
	ImGuiWindowFlags flags;
public:
	UIWindow(std::string name, void(*drawFunction)(), bool open = false, ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)
		:name(std::move(name)), drawFunction(drawFunction), _open(open), flags(flags)
	{

	}

public:
	void drawMenuItem()
	{
		if(ImGui::MenuItem(name.data()))
			_open = true;
	}

	void drawUI()
	{
		if(!_open)
			return;
		ImGui::Begin(name.data(), &_open, flags);
		ImGui::PushID(this);
		drawFunction();
		ImGui::PopID();
		ImGui::End();
	}
};