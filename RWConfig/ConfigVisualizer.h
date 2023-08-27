
#pragma once

class IConfigVisualizer
{
public:
	virtual void ConfigSet(IConfig* a_pConfig) = 0;
	virtual void ItemsChanged() = 0;
};