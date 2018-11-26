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

protected:
	std::string getNamePrefix() const override;

public:
	Scene(std::vector<Prop>&& props);

public:
	void addProp(Prop&& prop);
	std::vector<Prop> const& getProps() const;
	Camera& getCamera();
	void drawUI();

};