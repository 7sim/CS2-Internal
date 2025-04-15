#include "entitysystem.h"
#include "..\..\utils\hook\hooks.h"

void* CGameEntitySystem::GetEntityByIndex(int nIndex)
{
	return hooks::m_get_base_entity(this, nIndex);
}
unsigned int CGameEntitySystem::get_handle_entity(CCSPlayerPawn* entity)
{
	if (!entity)
		return -1;

	return hooks::m_get_handle_entity(entity);
}
CCSPlayerPawn* CGameEntitySystem::get_local_player() {
	debug(hooks::m_get_local_player != nullptr);

	return hooks::m_get_local_player(0);
}

CCSPlayerController* CGameEntitySystem::get_controller(CCSPlayerPawn* local_player)
{
	if (!local_player)
		return nullptr;

	return nullptr;
}
