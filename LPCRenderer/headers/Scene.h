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

protected:
	std::string getNamePrefix() const override;

public:
	Scene(Prop&& prop);
	Scene(std::vector<Prop>&& props);

public:
	float getGlobalScaling() const;
	void addProp(Prop&& prop);
	std::vector<Prop> const& getProps() const;
	Camera& getCamera();
	void drawUI();

};