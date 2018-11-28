#pragma once
#include "Prop.h"
#include "AutoName.h"
#include "Camera.h"

#include <vector>

class Scene : public AutoName<Scene>
{
private:
	std::vector<Prop> props;
	Camera camera;
	float globalScaling = 1.0f;

public:
	Scene(Prop&& prop);
	Scene(std::vector<Prop>&& props);

public:
	std::string getNamePrefix() const override;
	float getGlobalScaling() const;
	void addProp(Prop&& prop);
	void addProps(std::vector<Prop> && props);
	std::vector<Prop> const& getProps() const;
	Camera& getCamera();
	void drawUI();

};