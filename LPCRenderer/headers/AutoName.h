#pragma once

#include <string>

template <typename T>
class AutoName
{
private:
	mutable bool initialized = false;
	inline static int counter{1};
	mutable std::string name;

protected:
	virtual std::string getNamePrefix() const = 0;

public:
	std::string const& getName() const
	{
		if(!initialized)
		{
			initialized = true;
			name = getNamePrefix() + "(#" + std::to_string(counter++) + ")";
		}
		return name;
	}

	void setName(std::string name)
	{
		initialized = true;
		this->name = std::move(name);
	}

};
