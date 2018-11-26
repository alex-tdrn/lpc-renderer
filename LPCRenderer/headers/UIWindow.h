#pragma once
#include "imgui.h"
#include <string>

class UIWindow
{
private:
	bool _open = false;
	std::string name;
	void(*drawFunction)(bool*);
public:
	UIWindow(std::string name, void(*drawFunction)(bool*))
		:name(std::move(name)), drawFunction(drawFunction)
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
		if(_open)
			drawFunction(&_open);
	}
};