#pragma once

// used: callvfunc
#include "..\..\..\utils\utils.h"

struct ResourceBinding_t;

class IResourceSystem
{
public:
	void* QueryInterface(const char* szInterfaceName)
	{
		return utils::CallVFunc<void*, 2U>(this, szInterfaceName);
	}
};

class CResourceHandleUtils
{
public:
	void DeleteResource(const ResourceBinding_t* pBinding)
	{
		utils::CallVFunc<void, 2U>(this, pBinding);
	}
};
