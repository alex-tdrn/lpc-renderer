#pragma once
#include <vector>
#include <memory>

#include "imgui.h"

namespace impl
{
	class EmptyBaseClass
	{
	};

	template<typename T>
	class BaseClassWithActiveInstance
	{
	protected:
		static inline T* activeResource = nullptr;
	};
}

template<typename T, bool ActiveInstance = false>
class Manager : public std::conditional_t<ActiveInstance, impl::EmptyBaseClass, impl::BaseClassWithActiveInstance<T>>
{
private:
	static inline std::vector<std::unique_ptr<T>> store{};

public:
	static std::string const name;

public:
	static auto const& getAll()
	{
		return store;
	}

	static T* add(std::unique_ptr<T>&& resource)
	{
		T* ret = resource.get();
		store.push_back(std::move(resource));
		if constexpr(ActiveInstance)
			impl::BaseClassWithActiveInstance<T>::activeResource = ret;
		return ret;
	}

	template<typename Dummy = T*>
	static std::enable_if_t<ActiveInstance, Dummy> getActive()
	{
		return impl::BaseClassWithActiveInstance<T>::activeResource;
	}

	static void drawUI(bool* open)
	{
		static T* selected = [&](){
			if constexpr(ActiveInstance)
				return impl::BaseClassWithActiveInstance<T>::activeResource;
			else
				return nullptr;
		}();
		ImGui::PushID(open);
		ImGui::Begin(name.data(), open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		ImGui::Columns(2);

		ImGui::SetColumnWidth(-1, ImGui::GetTextLineHeightWithSpacing() * 15);
		ImGui::BeginChild(("###SelectionArea" + name).data());
		int id = 0;
		for(auto& resource : store)
		{
			if(ImGui::Selectable(resource->getName().data(), selected == resource.get()))
			{
				selected = resource.get();
				if constexpr(ActiveInstance)
				{
					impl::BaseClassWithActiveInstance<T>::activeResource = selected;
				}
			}
		}
		ImGui::EndChild();

		ImGui::NextColumn();
		ImGui::BeginChild(("###Selected" + name).data());
		if(selected)
			selected->drawUI();
		ImGui::EndChild();

		ImGui::Columns();
		ImGui::End();
		ImGui::PopID();
	}
};

