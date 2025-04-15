
#include "antiaim.h"
#include "..\..\utils\utils.h"
#include "..\..\sdk\cache\cache_entity.h"
#include "..\..\sdk\interfaces\engineclient.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include "../../sdk/interfaces/trace.h"
#include "..\..\helpers\log.h"
#include "..\autowall\wall.h"
#include "../../utils/hook/hooks.h"
#include "..\..\sdk\sdk.h"
#include "..\..\sdk\interfaces\cvar.h"
#include "..\..\utils\utils.h"
#include "..\backtrack\record.h"
#include "..\..\sdk\cache\cache_entity.h"
#include "..\..\sdk\interfaces\engineclient.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include "..\autowall\wall.h"
#include "../../sdk/interfaces/trace.h"
#include "..\..\helpers\log.h"
#include "..\autowall\wall.h"
#include "..\prediction\prediction.h"
#include "..\ragebot\ragebot.h"
#include "..\..\utils\hook\hooks.h"
#include "..\movement\setup.h"
#include "..\subtick\subtick.h"
float angle_normalize(float angle)
{
	angle = fmodf(angle, 360.0f);
	if (angle > 180)
	{
		angle -= 360;
	}
	if (angle < -180)
	{
		angle += 360;
	}
	return angle;
}
bool can_shoot_this_tick(int tick)
{
	if (!g::m_local_player_controller)
		return false;

	if (!g::m_local_player_pawn)
		return false;

	if (!g::m_active_weapon)
		return false;

	if (g::m_active_weapon->clip1() <= 0)
		return false;

	// get weapon services
	auto services = static_cast<CCSPlayer_WeaponServices*>(g::m_local_player_pawn->m_pWeaponServices());
	if (!services)
		return false;

	// check for nospread
	if (g::m_local_player_pawn->m_bWaitForNoAttack())
		return false;

	if (g::m_active_weapon->GetNextPrimaryAttackTick() > tick)
		return false;

	return true;
}
void c_antiaim::run(CUserCmd* cmd, CBaseUserCmdPB* base)
{
	if (!cfg_get(bool, g_cfg.anti_aim))
		return;

	if (!g::m_local_player_pawn || !g::m_active_weapon)
		return;

	if (!g::m_local_player_pawn->valid_move())
		return;

	if (!base )
		return;

	if (!base->view_angles)
		return;

	if (g::history_on_tick)
		return;

	if ( base->pInButtonState->nValue & IN_USE || base->pInButtonState->nValue & IN_ATTACK)
		return;

	if ( g::m_active_weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_GRENADE)
	{
		auto grenade = (C_BaseCSGrenade*)g::m_active_weapon;
		if (grenade->m_fThrowTime() > 0)
			return;
	}
	else
	{
		if (base->pInButtonState) {
			if ((base->pInButtonState->nValue & IN_ATTACK || g::history_on_tick) && can_shoot_this_tick(g::m_local_player_controller->m_nTickBase()) && (cfg_get(int, g_cfg.rage_silent_mode) != 0 || cfg_get(bool, g_cfg.safe_mode)))
				return;
		}
	}

	pitch(cmd, base);

	yaw(cmd, base);
}
bool _switch = false;
void c_antiaim::pitch(CUserCmd* cmd, CBaseUserCmdPB* base)
{
	if (cfg_get(int , g_cfg.aa_pitch) == 0 || !base->view_angles)
		return;
	_switch = !_switch;
	base->bit(BASE_BITS_VIEWANGLES);
	base->view_angles->bit(BASE_BITS_VIEWANGLES);
	base->view_angles->angValue.x = 89.f;
}

float get_fov(const Vector_t& view_angles, const Vector_t& start, const Vector_t& end) {
	Vector_t dir, fw;

	// get direction and normalize.
	dir = (end - start).normalized();

	// get the forward direction Vector_t of the view angles.
	math::angle_vectors(view_angles, fw);

	// get the angle between the view angles forward directional Vector_t and the target location.
	return std::max(M_RAD2DEG(std::acos(fw.Dot(dir))), 0.f);
}

void at_target(CUserCmd* cmd, CBaseUserCmdPB* base) {
	if (cfg_get(bool, g_cfg.at_targets))
		return;

	float  best_dist{ std::numeric_limits< float >::max() };
	float  dist;
	CCSPlayerPawn* target, * best_target{ nullptr };

	for (int i = 1; i < sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); i++)
	{
		auto entity = sdk::m_game->pGameEntitySystem->Get(i);
		if (!entity)
			continue;

		SchemaClassInfoData_t* _class = nullptr;
		entity->dump_class_info(&_class);
		if (!_class)
			continue;

		const uint32_t hash = hash_32_fnv1a_const(_class->szName);

		if (hash == hash_32_fnv1a_const(xorstr_("CCSPlayerController")))
		{
			auto p_controller = reinterpret_cast<CCSPlayerController*>(entity);
			if (!p_controller)
				continue;

			auto handle = p_controller->handle().index();

			auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle());
			if (!player || player->m_iTeamNum() == g::m_local_player_pawn->m_iTeamNum())
				continue;

			auto player_scene_node = player->m_pGameSceneNode();
			if (!player_scene_node)
				continue;

			// check if a player was closer to us.
			dist = get_fov(g::angle, g::eye_pos, player_scene_node->m_vecAbsOrigin());
			if (dist < best_dist) {
				best_dist = dist;
				best_target = player;
			}
		}
	}

	if (best_target) {
		// todo - dex; calculate only the yaw needed for this (if we're not going to use the x component that is).
		Vector_t angle;
		math::vector_angles(best_target->m_pGameSceneNode()->m_vecOrigin() - g::m_local_player_pawn->m_pGameSceneNode()->m_vecOrigin(), angle);
		base->view_angles->angValue.y = angle.y + 180.f; // Backwards
	}
	else
		base->view_angles->angValue.y = g::angle.y + 180.f; // Backwards
}

void c_antiaim::yaw(CUserCmd* cmd, CBaseUserCmdPB* base) {
	// Check for valid pointers
	if (!cmd || !base)
		return;
	if (!base->view_angles)
		return;
	int side = 0; // Default to backwards

	// Check the bind states for left and right keys
	bool left_key = Input::GetBindState(cfg_get(KeyBind_t, g_cfg.left_key));
	bool right_key = Input::GetBindState(cfg_get(KeyBind_t, g_cfg.right_key));

	// Determine the direction based on key presses
	if (left_key ) {
		side = -1; // Left
		cfg_get(KeyBind_t, g_cfg.right_key).bEnable = false;
	}
	else if (right_key) {
		side = 1; // Right
		cfg_get(KeyBind_t, g_cfg.left_key).bEnable = false;

	}
	else {
		side = 0; // Backwards if no key or both keys are pressed
	}

	// Set the yaw angle based on the direction
	if (side == -1) {
		base->bit(BASE_BITS_VIEWANGLES);
		base->view_angles->bit(BASE_BITS_VIEWANGLES);	

		base->view_angles->angValue.y = sdk::m_csgo_input->vecViewAngle.y + 90.f; // Left
	}
	else if (side == 1) {

		base->bit(BASE_BITS_VIEWANGLES);
		base->view_angles->bit(BASE_BITS_VIEWANGLES);	
		base->view_angles->angValue.y = sdk::m_csgo_input->vecViewAngle.y - 90.f; // Right
	}
	else {
		if (cfg_get(bool,g_cfg.at_targets)) 
			at_target(cmd, base);		
		else {
			base->bit(BASE_BITS_VIEWANGLES);
			base->view_angles->bit(BASE_BITS_VIEWANGLES);
			base->view_angles->angValue.y = sdk::m_csgo_input->vecViewAngle.y + 180.f; // Backwards
		}
	}

	// Normalize the angle

	base->bit(BASE_BITS_VIEWANGLES);
	base->view_angles->bit(BASE_BITS_VIEWANGLES);
	base->view_angles->angValue.y = angle_normalize(base->view_angles->angValue.y);
}