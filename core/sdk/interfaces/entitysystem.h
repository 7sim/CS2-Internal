#pragma once
#include "../../utils/utils.h"
#include "..\handle\handle.hpp"
class CBaseEntity;
class CCSPlayerController;
class CCSPlayerPawn;
class CGameEntitySystem
{
public:
	template <typename T = CBaseEntity>
	T* Get(int nIndex)
	{
		return reinterpret_cast<T*>(this->GetEntityByIndex(nIndex));
	}

	/// GetClientEntityFromHandle
	template <typename T = CBaseEntity>
	T* Get(const CBaseHandle hHandle)
	{
		if (!hHandle.valid())
			return nullptr;

		return reinterpret_cast<T*>(this->GetEntityByIndex(hHandle.index()));
	}

	int GetHighestEntityIndex()
	{
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x20F0);
	}
	unsigned int get_handle_entity(CCSPlayerPawn* entity);
	CCSPlayerPawn* get_entity(int index)
	{
		__int64 v2; // rcx
		__int64 v3; // r8
		__int64 result{}; // rax

		if ((unsigned int)index <= 0x7FFE
			&& (unsigned int)(index >> 9) <= 0x3F
			&& (v2 = *(std::uintptr_t*)(std::uintptr_t(this) + 8 * (index >> 9) + 16)) != 0
			&& (v3 = 120 * (index & 0x1FF), v3 + v2)
			&& (*(std::uintptr_t*)(v3 + v2 + 16) & 0x7FFF) == index)
		{
			result = *(std::uintptr_t*)(v3 + v2);
		}
		return reinterpret_cast<CCSPlayerPawn*>(result);
	}
	CCSPlayerPawn* get_local_player();
	CCSPlayerController* get_controller(CCSPlayerPawn* local_player);

private:
	void* GetEntityByIndex(int nIndex);

};

class IGameResourceService
{
public:
	pad(0x58);
	CGameEntitySystem* pGameEntitySystem;
};