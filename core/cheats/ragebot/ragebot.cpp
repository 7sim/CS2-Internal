#include "ragebot.h"
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
#include "ragebot.h"
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
#include <future>
#include <vector>
#include <algorithm> // For std::max
#include "..\animations\animations.h"
static std::vector<std::uint32_t> m_rage_aim_hitboxes;
int m_status = 0;
CCSPlayerPawn* m_rage_last_target;
ragebot::c_best_target m_rage_best_target;
std::vector<ragebot::c_best_target> m_rage_aim_rage_targets;
std::vector<ragebot::c_best_target> m_potential_targets;

std::vector<ragebot::c_target> m_rage_targets;
std::unordered_map<int, std::deque<ragebot::anim_record_t>> m_anim_records{};

#define MAX_STUDIO_BONES 1024
int rage_type = 0;
static constexpr int get_hitgroup(int group)
{
	switch (group)
	{
	case HITBOX_HEAD:
		return HEAD;
	case HITBOX_NECK:
		return NECK;
	case HITBOX_UPPER_CHEST:
	case HITBOX_CHEST:
	case HITBOX_THORAX:
		return CHEST;
	case HITBOX_PELVIS:
		return PELVIS;
	case HITBOX_STOMACH:
		return STOMACH;
	case HITBOX_LEFT_THIGH:
	case HITBOX_LEFT_CALF:
		return L_LEG;
	case HITBOX_RIGHT_THIGH:
	case HITBOX_RIGHT_CALF:
		return R_LEG;
	case HITBOX_LEFT_FOOT:
		return L_FEET;
	case HITBOX_RIGHT_FOOT:
		return R_FEET;
	default:
		return CENTER;
	}
}
enum e_rage_hitboxes : unsigned int
{
	none = 0U,
	head = 1 << 0,
	chest = 1 << 1,
	stomach = 1 << 2,
	legs = 1 << 3,
	feet = 1 << 4

};
enum e_type : int {
	SPREAD = 0,
	NO_SPREAD = 1
};
enum e_status : int {
	NORMAL = 0,
	SAFE_MODE = 1
};
enum e_stop_mode : int {
	QUICK = 0,
	ACCURATE = 1,
	SLOW = 2,
	BETWEEN_SHOTS = 3,
	PREDICTIVE = 4

};
inline int get_hitbox_from_menu(int hitbox) {
	switch (hitbox) {
	case HITBOX_HEAD:
		return 0;
	case HITBOX_CHEST:
	case HITBOX_THORAX:
	case HITBOX_UPPER_CHEST:
		return 1;
	case HITBOX_PELVIS:
	case HITBOX_STOMACH:
		return 2;
	case HITBOX_RIGHT_HAND:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_FOREARM:
		return 3;
	case HITBOX_RIGHT_THIGH:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_LEFT_CALF:
		return 4;
	case HITBOX_RIGHT_FOOT:
	case HITBOX_LEFT_FOOT:
		return 5;
	}

	return -1;
}

// setup config hitboxes
void ragebot::c_data::menu_hitboxes()
{
	/* emplace menu hitboxes which will be used for hitscan*/
	if (adaptive.hitboxes & head)
	{
		m_rage_aim_hitboxes.emplace_back(HITBOX_HEAD);
	}

	if (adaptive.hitboxes & chest)
	{
		m_rage_aim_hitboxes.emplace_back(HITBOX_CHEST);
		m_rage_aim_hitboxes.emplace_back(HITBOX_UPPER_CHEST);
		m_rage_aim_hitboxes.emplace_back(HITBOX_THORAX);
	}

	if (adaptive.hitboxes & stomach)
	{
		m_rage_aim_hitboxes.emplace_back(HITBOX_STOMACH);
		m_rage_aim_hitboxes.emplace_back(HITBOX_PELVIS);
	}

	if (adaptive.hitboxes & legs)
	{
		m_rage_aim_hitboxes.emplace_back(HITBOX_LEFT_THIGH);
		m_rage_aim_hitboxes.emplace_back(HITBOX_RIGHT_THIGH);
		m_rage_aim_hitboxes.emplace_back(HITBOX_LEFT_CALF);
		m_rage_aim_hitboxes.emplace_back(HITBOX_RIGHT_CALF);

	}
	if (adaptive.hitboxes & feet)
	{
		m_rage_aim_hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		m_rage_aim_hitboxes.emplace_back(HITBOX_LEFT_FOOT);
	}
}


static Vector_t m_angle_to_vector(CCSPlayerPawn* localplayer, Vector_t position)
{
	auto data = g_prediction->get_local_data();
	if (!data)
		return Vector_t();

	Vector_t eye_position = data->m_eye_pos;
	Vector_t angle = position;

	angle.x = position.x - eye_position.x;
	angle.y = position.y - eye_position.y;
	angle.z = position.z - eye_position.z;

	angle.Normalizes();
	math::vec_angles(angle, &angle);

	angle.clamp();
	return angle;
}

void clear() {
	command::m_shoot = false;
	command::m_rage_aim_target = nullptr;
	m_rage_best_target.clear();
	m_rage_aim_rage_targets.clear();
	m_rage_targets.clear();
	m_potential_targets.clear();
}

void ragebot::c_data::init()
{
	m_rage_targets.clear();
	m_rage_aim_rage_targets.clear();
	m_rage_aim_hitboxes.clear();
	m_rage_best_target.clear();
	m_potential_targets.clear();
}


bool ragebot::c_data::allowed() {

	if (!sdk::m_engine->is_valid())
		return false;

	//if (!g::m_local_player_pawn->valid_move())
	//	return false;

	if (!g::m_active_weapon)
		return false;

	if (g::m_active_weapon->clip1() <= 0
		|| g::m_weapon_data->m_WeaponType() == WEAPONTYPE_GRENADE)
		return false;

	return true;
}


inline float get_cs2_default_fov(const QAngle_t& viewAngle, const QAngle_t& aimAngle)
{
	QAngle_t delta = (aimAngle - viewAngle).Normalize();

	return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}
inline float get_cs2_default_fov_vec3(const QAngle_t& viewAngle, const Vector_t& aimAngle)
{
	QAngle_t delta = (QAngle_t(aimAngle.x, aimAngle.y, aimAngle.z) - viewAngle).Normalize();

	return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}


Vector_t aim_direction(Vector_t src, Vector_t dst)
{
	Vector_t result;

	Vector_t delta = dst - src;

	math::vector_angles(delta, result);

	return result;
}

inline float crosshair_fov(const Vector_t& hitboxPos)
{
	auto center = ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);
	ImVec2 out;
	auto screenPos = math::WorldToScreen(hitboxPos, out);

	if (screenPos) {
		ImVec2 crosshairPos = center;
		ImVec2 hitboxScreenPos = out;

		float deltaX = crosshairPos.x - hitboxScreenPos.x;
		float deltaY = crosshairPos.y - hitboxScreenPos.y;

		return std::sqrt(deltaX * deltaX + deltaY * deltaY);
	}

	return FLT_MAX;
}

enum rage_mode : int {
	SILENT = 0,
	NORMAL_S = 1
};



void ragebot::c_data::stop(CUserCmd* user_cmd, CBaseUserCmdPB* crc, int type) {

	if (!adaptive.auto_stop)
		return;

	// if sniper && not auto and not scoped why should we stop 
	if (g::m_active_weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE && !g::m_local_player_pawn->m_bIsScoped())
		return;


	if (type == PREDICTIVE) {
		if (g::m_local_player_pawn->valid_move() && (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND) && !(crc->pInButtonState->nValue & IN_JUMP)) {
			misc::movement->slow_walk(crc);
			misc::movement->quick_stop(crc, 1);
			misc::movement->quick_stop(crc);
		}
	}
	else if (type == QUICK) {
		if (g::m_local_player_pawn->valid_move() && (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND) && !(crc->pInButtonState->nValue & IN_JUMP)) {

			misc::movement->quick_stop(crc, 1);
			misc::movement->quick_stop(crc);
		}
	}
	else if (type == ACCURATE) {
		if (g::m_local_player_pawn->valid_move() && !(user_cmd->nButtons.nValue & IN_FORWARD) && !(user_cmd->nButtons.nValue & IN_BACK) && !(user_cmd->nButtons.nValue & IN_MOVELEFT) && !(user_cmd->nButtons.nValue & IN_MOVERIGHT))
			misc::movement->quick_stop(crc);
	}
	else if (type == SLOW) {
		if (g::m_local_player_pawn->valid_move() && (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND) && !(crc->pInButtonState->nValue & IN_JUMP)) {
			misc::movement->slow_walk(crc);
			misc::movement->quick_stop(crc);

		}
	}
	else if (type == BETWEEN_SHOTS) {
		if (g::m_local_player_pawn->valid_move() && (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND) && !(crc->pInButtonState->nValue & IN_JUMP)) {
			misc::movement->quick_stop(crc, 1);
			misc::movement->slow_walk(crc);
		}
	}
}
struct lag_compensation_data_t {
	std::uint64_t m_tick{};
	float         m_time{};
	pad(0x24);
}; // size: 0x40

/* pasted from weave which got pasted from neverlose*/
struct data_info
{
	std::uint64_t tick;
	float time;
	char pad[0x24];
};


float ragebot::anim_record_t::calculate_delta_time(int newest_tick) {

	int tick_count = sdk::m_global_vars->m_tickcount;
	float max_unlag = 0.2f;

	float unk_modifier = std::fmod(max_unlag, 0.015625);
	float unlag_diff = max_unlag - unk_modifier;
	float time_modifier = unk_modifier * 64.0;
	int unlag_ticks = (int)(float)((float)(unlag_diff * 64.0) + 0.5);

	lag_compensation_data_t v28{};
	v28.m_tick = newest_tick;
	v28.m_time = newest_tick * 0.015625;

	int v20{};
	float v19{};
	float v21{};
	int v22{};
	float v24{};
	int v37{};
	int tick{};
	float v30{};
	int v31{};
	int v32{};
	int v33{};

	if (time_modifier <= (float)(1.0 - 0.0099999998))
	{
		v20 = tick_count - unlag_ticks;
		if (time_modifier > 0.0099999998)
		{
			v19 = -time_modifier;
			v21 = v19;

			if (v19 <= 0.0)
				v19 = v19 + 1.0;
		}
		else
		{
			v19 = 0.0;
			v21 = 0.0;
		}
	}

	v22 = v20 - 1;

	lag_compensation_data_t v40{};
	v40.m_time = v19;

	if (v21 >= 0.0)
		v22 = v20;

	v40.m_tick = v22;

	if (v19 <= (float)(1.0 - 0.0099999998))
	{
		if (v19 <= 0.0099999998)
			v40.m_time = 0.0;
	}
	else
	{
		v40.m_tick = v22 + 1;
		v40.m_time = 0.0;
	}

	tick = v40.m_tick;
	v24 = (float)v28.m_tick;
	lag_compensation_data_t v38 = v40;

	lag_compensation_data_t v39{};

	if ((float)((float)(v24 + v28.m_time) * 0.015625) <= 0.0)
	{
		v40.m_tick = newest_tick;
		v40.m_time = 0.0;
		v37 = v40.m_tick;
		tick = v38.m_tick;
		v39 = v40;
	}
	else
	{
		v37 = v28.m_tick;
		v39 = v28;
	}


	if (tick <= v37 && (tick < v37 || v39.m_time > v38.m_time))
	{
		v33 = v37 - tick;
		v30 = v39.m_time - v38.m_time;
		if ((float)(v39.m_time - v38.m_time) <= 0.0)
			v30 = v30 + 1.0;
		v32 = v33 - 1;
		if ((float)(v39.m_time - v38.m_time) >= 0.0)
			v32 = v33;
		if (v30 >= (float)(1.0 - 0.0099999998))
		{
			++v32;
			goto LABEL_48;
		}
	LABEL_47:
		if (v30 > 0.0099999998)
			goto LABEL_49;
		goto LABEL_48;
	}

	v30 = v38.m_time - v39.m_time;
	v31 = tick - v37;
	if ((float)(v38.m_time - v39.m_time) <= 0.0)
		v30 = v30 + 1.0;
	v32 = v31 - 1;
	if ((float)(v38.m_time - v39.m_time) >= 0.0)
		v32 = v31;
	if (v30 <= (float)(1.0 - 0.0099999998))
		goto LABEL_47;
	++v32;
LABEL_48:
	v30 = 0.0;
LABEL_49:
	float delta_time = (float)((float)v32 + v30) * g::interval;

	return delta_time;
}
float __fastcall __fmax(float* a1, float* a2)
{
	return fmaxf(*a1, *a2);
}/*
 double __fastcall sub_18009DA90(__int64 a1)
{
  double v1; // xmm1_8
  double result; // xmm0_8

  *(_QWORD *)&v1 = *(unsigned int *)(a1 + 0x7390);
  if ( *(float *)&v1 < 0.0 )
	result = getnetwork(a1);
  else
	result = fmax(0.0, v1);
  return result;
}
double __fastcall get_network_latency(__int64 a1)
{
	__int64 v1; // rdx
	double result; // xmm0_8
	int v4; // ebx
	int v5; // eax
	int v6; // [rsp+40h] [rbp+8h] BYREF
	float v7; // [rsp+48h] [rbp+10h] BYREF

	v1 = *(unsigned int*)(a1 + 29856);
	if (!(DWORD)v1)
		return 0.0;

	HIDWORD(result) = HIDWORD(qword_180269268);
	if (*(double*)&qword_180269268 != *(double*)(a1 + 29896))
	{
		*(DWORD*)(a1 + 29896) = qword_180269268;
		v4 = *(DWORD*)(a1 + 29960);
		(*(void(__fastcall**)(__int64, __int64, __int64, DWORD, DWORD))(*(DWORD*)qword_180275D00 + 128i64))(
			qword_180275D00,
			v1,
			a1 + 29904,
			0i64,
			0i64);

		if (v4 <= *(DWORD*)(a1 + 0x7508))
			v4 = *(DWORD*)(a1 + 0x7508);
		*(DWORD*)(a1 + 29960) = v4;
	}
	v5 = *(DWORD*)(a1 + 29908);
	if (v5 < 0)
		return 0.0;
	v6 = 897988541;
	v7 = (float)v5 / 1000.0;
	*(float*)&result = __fmax(&v7, (float*)&v6);
	return result;
}

double __fastcall get_network_latency(__int64 a1)
{
	__int64 v1; // rdx
	double result; // xmm0_8
	int v4; // ebx
	int v5; // eax
	int v6; // [rsp+40h] [rbp+8h] BYREF
	float v7; // [rsp+48h] [rbp+10h] BYREF

	v1 = *(unsigned int*)(a1 + 29856);
	if (!(DWORD)v1)
		return 0.0;

	HIDWORD(result) = HIDWORD(qword_180269268);
	if (*(double*)&qword_180269268 != *(double*)(a1 + 29896))
	{
		*(DWORD*)(a1 + 29896) = qword_180269268;
		v4 = *(DWORD*)(a1 + 29960);
		(*(void(__fastcall**)(__int64, __int64, __int64, DWORD, DWORD))(*(DWORD*)qword_180275D00 + 128i64))(
			qword_180275D00,
			v1,
			a1 + 29904,
			0i64,
			0i64);

		if (v4 <= *(DWORD*)(a1 + 0x7508))
			v4 = *(DWORD*)(a1 + 0x7508);
		*(DWORD*)(a1 + 29960) = v4;
	}
	v5 = *(DWORD*)(a1 + 29908);
	if (v5 < 0)
		return 0.0;
	v6 = 897988541;
	v7 = (float)v5 / 1000.0;
	*(float*)&result = __fmax(&v7, (float*)&v6);
	return result;
}
*/

/*
typedef void(__fastcall* idk)(__int64, __int64, __int64, int, int);

double __fastcall get_network_info(INetChannelInfo* a1) {
	// Declare variables
	__int64 v1 = 0;
	double og = 0.0;
	int v4 = 0;
	int v5 = 0;
	int v6 = 897988541; // FLT_MAX
	float v7 = 0.0f;

	v1 = *reinterpret_cast<unsigned int*>(reinterpret_cast<uintptr_t>(a1) + 29856);
	if (!static_cast<DWORD>(v1))
		return 0.0;

	auto qword_180269268 = utils::FindPattern("client.dll", "F2 0F 10 05 ?? ?? ?? ?? 66 0F 2E 81 C8 74 00 00 7A 02 74 4C F2 0F 11 81 C8 74 00 00 4C 8D 87 D0 74 00 00 48 89 5C 24 50");
	og = *reinterpret_cast<double*>(&qword_180269268);
	_log(LOG_INFO) << "new qword value: " << og;

	auto call_idk_func  = *reinterpret_cast<idk>(utils::FindPattern("client.dll", "48 8B 0D ?? ?? ?? ?? 48 C7 44 24 20 00 00 00 00 48 8B 01 FF 90 ?? ?? ?? ?? 8B 87 08 75 00 00 3B D8 0F 4E D8 89 9F 08 75 00 00 48 8B 5C 24 50"));


	if (og != *reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(a1) + 29896)) {

		*reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(a1) + 29896) = *reinterpret_cast<double*>(&qword_180269268);

		// Access the value at (a1 + 29960)
		v4 = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 29960);

		call_idk_func(qword_180275D00, v1, reinterpret_cast<__int64>(a1) + 29904, 0, 0);

		if (v4 <= *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 29960))
			v4 = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 29960);

		*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 29960) = v4;
	}

	v5 = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(a1) + 29908);
	if (v5 < 0)
		return 0.0;

	v7 = static_cast<float>(v5) / 1000.0f;
	*(float*)&og = __fmax(&v7, (float*)&v6);

	return og;
}*/

double __fastcall get_engine_latency(INetChannelInfo* a1) {
	if (!a1)
		return 0.0;
	double v1 = 0.0;
	/*
	[20 - 12 - 2024 22:18 : 53] [ragebot.cpp:502] [info] unk_func: -5.64009e+34
		[20 - 12 - 2024 22:18 : 53][ragebot.cpp:555][warning] delta : 1.82394e+07 network_latency : 5.08782e-312 engine_latency : 5.08782e-315
		[20 - 12 - 2024 22:18 : 53][ragebot.cpp:502][info] unk_func : -5.64009e+34*/
	double result = 0.0;
	*(DWORD*)&v1 = *(unsigned int*)(*(uintptr_t*)a1 + 0x7390);
	auto unk = *(float*)&v1;
	_log(LOG_INFO) << "unk_func: " << unk;
	if (unk < 0.0f)
		return a1->get_network_latency();

	// cs2 FLT_MAX lmao
	float flt_max = 897988541;

	result = static_cast<double>(fmaxf(unk, flt_max));

	return result;
}/*
double __fastcall get_engine_latency(INetChannelInfo* a1)
{
	double result; // xmm0_8
	int v2; // [rsp+30h] [rbp+8h] BYREF

	result = 0.0;

	if (*(float*)(*(uintptr_t*)a1 + 0x7390) < 0.0)
		return a1->get_network_latency();

	// FLT_MAX
	v2 = 897988541;

	result = __fmax((float*)(*(uintptr_t*)a1 + 0x7390), (float*)&v2);

	return result;
}*/
bool ragebot::anim_record_t::is_valid()
{
	auto network = sdk::m_network->GetNetworkClient();
	if (!network)
		return false;

	auto net_channel2 = network->get_net_channel2();
	if (!net_channel2)
		return false;

	const auto nci = sdk::m_engine->get_net_channel_info(0);
	if (!nci)
		return false;

	float max_unlag = 0.2f;
	const double l1 = (utils::CallVFunc<double, 10U>(net_channel2));
	const double l2 = (utils::CallVFunc<double, 11U>(net_channel2));

	auto network_latency = l1 * 0.5f;
	auto engine_latency = l2;
	// dead time check..

	float server_time = (m_simulation_time * 64.0) + network_latency + engine_latency;

	float delta_time = this->calculate_delta_time(server_time);

	auto cur_delta_time = std::abs(delta_time);

	if (cur_delta_time > max_unlag)
		return false;

	auto dead_tick = (g_prediction->get_local_data()->m_shoot_tick * g::interval) - this->m_simulation_time ;
	//_log(LOG_ERROR) << ("dead_tick: " ) << dead_tick;
	return true;//dead_tick < 0.2f;
	/*
	const float flMaxUnlag = 0.2f;
	const float flLatency = nci->latency(FLOW_OUTGOING) + nci->latency(FLOW_INCOMING);
	const float flCorrectedValue = std::clamp(flLatency + sdk::m_network->GetNetworkClient()->lerp(), 0.0f, flMaxUnlag);
	float flMaxDelta = std::min(flMaxUnlag - flCorrectedValue, 0.2f);
	const float flDelta = flMaxDelta - g_prediction->get_local_data()->m_current_time;
	
	if (m_simulation_time <= fabsf(flDelta))
		_log(LOG_ERROR) << ("cant get valid");

	return m_simulation_time > fabsf(flDelta);*/
	// dead time check..
/* IsDeadTime:
if ( max_unlag >=  (tickbase - simtime + 0.5) )
	  break;
*/
//auto dead_tick = (g_prediction->get_local_data()->m_tick_base - this->m_simulation_time + 0.5);
//_log(LOG_INFO) << "dead_tick: " << dead_tick;
}
float get_finished_cycle_rate(CBaseAnimGraph* a1) {
	static auto some_poop_func = reinterpret_cast<bool(__fastcall*)(void*)>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", "E8 ? ? ? ? 84 C0 75 ? 4C 8D 44 24"), 0x1, 0x0));

	auto& v3 = a1->m_seq_fixed_cycle();
	auto& v5 = a1->m_playback_rate();
	auto v6 = 0;

	if (v5) {
		v3 = ((v3 - a1->m_seq_start_time()) * a1->m_cached_sequence_cycle_rate()) * v5;
		if (v5 < 0.0)
			v3 = v3 + 1.0;
	}

	if (some_poop_func(a1)) {
		if (v5 < 0.0f)
			return v3;
		else
			return v3 - v3;
	}
	else {
		if (v3 >= 0.0)
			return fminf(v3, 1.0);
		return v6;
	}
}
void on_sequence_update(CBaseAnimGraph* base_anim_graph) {
	if (!base_anim_graph)
		return;

	int new_sequence = 0;
	int sequence = base_anim_graph->m_sequence();

	float finished_cycle_rate = get_finished_cycle_rate(base_anim_graph);

	base_anim_graph->write_new_sequence(new_sequence);
	bool diff_btw_sequences = sequence != new_sequence;

	if (base_anim_graph->m_sequence() != sequence)
		base_anim_graph->m_sequence() = sequence;

	base_anim_graph->m_cached_sequence_cycle_rate() = finished_cycle_rate;
	base_anim_graph->m_sequence_finished() = false;

	if (base_anim_graph->m_playback_rate() == 0.0f) {
		if (base_anim_graph->m_seq_fixed_cycle() != sdk::m_global_vars->m_tickcount)
			base_anim_graph->m_seq_fixed_cycle() = sdk::m_global_vars->m_tickcount;
	}
	else {
		if (base_anim_graph->m_seq_fixed_cycle() != 0.0)
			base_anim_graph->m_seq_fixed_cycle() = 0.0;

		float seq_cycle_rate = base_anim_graph->m_cached_sequence_cycle_rate();
		float playback_rate = base_anim_graph->m_playback_rate();
		if (seq_cycle_rate != 0.0)
		{
			if (playback_rate < 0.0)
				sdk::m_global_vars->m_tickcount--;
			sequence = sequence - (sdk::m_global_vars->m_tickcount / playback_rate);
		}
		if (playback_rate != base_anim_graph->m_seq_start_time())
			base_anim_graph->m_seq_start_time() = playback_rate;
	}
}


void ragebot::c_data::store_records() {
	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return;

	auto controller = CCSPlayerController::Get();
	if (!controller)
		return;

	auto pawn = hooks::m_get_local_player(0);
	if (!pawn)
		return;

	const int max_ticks = 12;

	for (int i = 1; i < 64; ++i) {
		auto entity = sdk::m_game->pGameEntitySystem->Get(i);
		if (!entity)
			continue;


		SchemaClassInfoData_t* _class = nullptr;
		entity->dump_class_info(&_class);
		if (!_class)
			continue;

		const uint32_t hash = hash_32_fnv1a_const(_class->szName);

		/* perhaps i coudl use this logic instead for all entities and sort them based on class name ??? prob better*/
		if (hash == hash_32_fnv1a_const(xorstr_("CCSPlayerController")))
		{

			auto player_controller = reinterpret_cast<CCSPlayerController*>(entity);
			if (!player_controller || player_controller == controller)
				continue;

			auto handle = player_controller->handle().index();

			if (!player_controller->m_bPawnIsAlive()) {
				auto player_iterator = m_anim_records.find(handle);
				if (player_iterator != m_anim_records.end())
					m_anim_records.erase(player_iterator);

				continue;
			}

			auto player_pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(player_controller->PawnHandle());
			if (!player_pawn) {
				auto player_iterator = m_anim_records.find(handle);
				if (player_iterator != m_anim_records.end())
					m_anim_records.erase(player_iterator);

				continue;
			}

			if (!player_pawn->IsAlive()) {
				auto player_iterator = m_anim_records.find(handle);
				if (player_iterator != m_anim_records.end())
					m_anim_records.erase(player_iterator);

				continue;
			}

			if (!g::deathmatch && player_pawn->m_iTeamNum() == pawn->m_iTeamNum()) {
				auto player_iterator = m_anim_records.find(handle);
				if (player_iterator != m_anim_records.end())
					m_anim_records.erase(player_iterator);

				continue;
			}
			if (player_pawn == pawn) {
				auto player_iterator = m_anim_records.find(handle);
				if (player_iterator != m_anim_records.end())
					m_anim_records.erase(player_iterator);

				continue;
			}

			/*	auto body_component = player_pawn->m_CBodyComponent();
			if (!body_component) {
				auto player_iterator = m_lag_records.find(handle);
				if (player_iterator != m_lag_records.end())
					m_lag_records.erase(player_iterator);
				continue;
			}

			CBaseAnimGraph* base_anim = *reinterpret_cast<CBaseAnimGraph**>(*(uintptr_t*)(body_component)+0x490);
			if (!base_anim) {
				auto player_iterator = m_lag_records.find(handle);
				if (player_iterator != m_lag_records.end())
					m_lag_records.erase(player_iterator);

				continue;
			}
			*/
			// update animations on entity
			//if (!(player_pawn->m_fFlags() & FL_ONGROUND))
			//	 on_sequence_update(base_anim);

			// is it added yet?
			if (m_anim_records.find(handle) == m_anim_records.end()) {
				m_anim_records.insert_or_assign(handle, std::deque<ragebot::anim_record_t>{});
				continue;
			}

			// clamp records etc 
			auto& records = m_anim_records[handle];

			if (records.size() != max_ticks) {
				records.clear();
				records.resize(max_ticks);
			}

			auto& record = records.emplace_front(player_pawn);
			if (records.size() > max_ticks - 1)
				records.erase(records.end() - 1);

			//record.store(player_pawn);

		}
	}
}

void ragebot::c_data::render_lag_compensation_bones(CCSPlayerPawn* player) {

	if (!cfg_get(bool, g_cfg.bSkeletonLagComp))
		return;

	if (!g::m_local_player_pawn)
		return;

	auto& records = m_anim_records[player->handle().index()];

	if (records.empty())
		return;

	ragebot::anim_record_t* last_record = &records.front();
	if (!last_record)
		return;

	auto skeleton = last_record->m_skeleton;
	if (!skeleton)
		return;

	auto bone_array = last_record->m_bone_data;

	const Color_t skeleton_color{ cfg_get(ColorPickerVar_t, g_cfg.colSkeletonBacktrack).colValue };
	const Color_t outline_color{ 0,0,0, 55 };
	float thickness = 1.f;
	float outline_thickness = 1.5f;

	// Helper to draw lines with outlines
	auto draw_bone = [&](int from, int to) {
		ImVec2 screen_start, screen_end;
		if (math::WorldToScreen(bone_array[from].pos, screen_start) &&
			math::WorldToScreen(bone_array[to].pos, screen_end)) {
			ImGui::GetBackgroundDrawList()->AddLine(screen_start, screen_end, outline_color.GetU32(), outline_thickness);
			ImGui::GetBackgroundDrawList()->AddLine(screen_start, screen_end, skeleton_color.GetU32(), thickness);
		}
	};

	// Head and Spine
	draw_bone(6, 5); // Head to Neck
	draw_bone(5, 4); // Neck to Spine_3
	draw_bone(4, 3); // Spine_3 to Spine_2
	draw_bone(3, 2); // Spine_2 to Spine_1
	draw_bone(2, 0); // Spine_1 to Pelvis

	// Left Leg
	draw_bone(0, 22); // Pelvis to Left Upper Leg
	draw_bone(22, 23); // Left Upper Leg to Left Lower Leg
	draw_bone(23, 24); // Left Lower Leg to Left Ankle

	// Right Leg
	draw_bone(0, 25); // Pelvis to Right Upper Leg
	draw_bone(25, 26); // Right Upper Leg to Right Lower Leg
	draw_bone(26, 27); // Right Lower Leg to Right Ankle

	// Left Arm
	draw_bone(4, 8);  // Spine_3 to Left Upper Arm
	draw_bone(8, 9);  // Left Upper Arm to Left Lower Arm
	draw_bone(9, 10); // Left Lower Arm to Left Hand

	// Right Arm
	draw_bone(4, 13); // Spine_3 to Right Upper Arm
	draw_bone(13, 14); // Right Upper Arm to Right Lower Arm
	draw_bone(14, 15); // Right Lower Arm to Right Hand
}
ragebot::anim_record_t* ragebot::c_data::select_record(int handle) {
	if (!g::m_local_player_pawn->IsAlive())
		return nullptr;

	auto& records = m_anim_records[handle];

	if (records.empty())
		return nullptr;

	ragebot::anim_record_t* best_record = nullptr;
	ragebot::anim_record_t* last_record = &records.front();

	if (records.size() == 1u)
		return last_record;

	for (auto it = records.begin(); it != records.end(); it = std::next(it)) {
		auto record = &*it;
		if (!record)
			continue;

		if (!record->m_pawn || !record->is_valid())
			continue;


		if (!best_record) {
			best_record = record;
			continue;
		}

		if (record->m_throwing) {
			best_record = record;
			continue;
		}

		if ((record->m_pawn->m_fFlags() & FL_ONGROUND) != (best_record->m_pawn->m_fFlags() & FL_ONGROUND)) {
			if ((record->m_pawn->m_fFlags() & FL_ONGROUND))
				best_record = record;
		}
	}

	if (!best_record)
		return nullptr;

	return best_record;

}
// no spread
bool can_shoot(int tick)
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

	if (g_prediction->get_local_data()->m_next_primary_tick > tick)
		return false;

	if (rage_type == SPREAD) {
		if (g_prediction->get_local_data()->m_next_attack > TICKS_TO_TIME(tick))
			return false;
	}

	return true;
}


namespace spread_system {
	// subview > names > seed
	uint32_t compute_random_seed(QAngle_t* angViewAngles, int nPlayerTickCount) {
		using fn_t = uint32_t(__fastcall*)(void* a1, QAngle_t* angViewAngles, int nPlayerTickCount);
		static fn_t fn = (fn_t)utils::FindPattern("client.dll", "48 89 5C 24 08 57 48 81 EC F0 00");
		return fn(g::m_local_player_pawn, angViewAngles, nPlayerTickCount);
	}


	Vector2D_t calc_spread(int nRandomSeed, float flInAccuracy, float flSpread)
	{
		using fn_t = void(__fastcall*)(int16_t, int, int, std::uint32_t, float, float, float, float*, float*);
		static auto predict_spread = (fn_t)utils::FindPattern("client.dll", "48 8B C4 48 89 58 08 48 89 68 18 48 89 70 20 57 41 54 41 55 41 56 41 57 48 81 EC E0");

		Vector2D_t spread_predicted;
		predict_spread(g::m_active_weapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex(), 1, 0, nRandomSeed, flInAccuracy, flSpread, g::m_active_weapon->m_flRecoilIndex(), &spread_predicted.x, &spread_predicted.y);

		return Vector2D_t(spread_predicted.x, spread_predicted.y);
	}


	struct spread_info_t {
		bool found;
		QAngle_t adjusted;
		int foundAfter;
		int seed;
	};

	constexpr float XMxConvertToDegrees(float fRadians) noexcept { return fRadians * (180.0f / 3.141592654f); }
	inline QAngle_t calc_angles(Vector_t viewPos, Vector_t aimPos)
	{
		QAngle_t angle = { 0, 0, 0 };

		Vector_t delta = aimPos - viewPos;

		angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
		angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

		return angle;
	}
	spread_info_t listen(QAngle_t& angle) {
		spread_info_t res{};
		res.found = false;
		auto pCmd = CUserCmd::Get();
		if (!pCmd)
			return spread_info_t{};

		auto weapon = g::m_active_weapon;
		if (!weapon)
			return spread_info_t{};

		auto data = g_prediction->get_local_data();
		if (!data)
			return spread_info_t{};

		int nCurrentTick = command::m_player_tick_count;
		float flInAccuracy = data->m_inaccuracy;
		float flSpread = data->m_spread;

		for (int i = 0; i < 720; i++)
		{
			QAngle_t angTemp = QAngle_t((float)i / 2.0f, angle.y, 0.0f);

			int nTempSeed = compute_random_seed(&angTemp, nCurrentTick);

			Vector2D_t vecSpread = calc_spread(nTempSeed + 1, flInAccuracy, flSpread);

			QAngle_t angNoSpreadView = angle;
			angNoSpreadView.x += XMxConvertToDegrees(atan(sqrt((vecSpread.x * vecSpread.x) + (vecSpread.y * vecSpread.y))));
			angNoSpreadView.z = -XMxConvertToDegrees(atan2(vecSpread.x, vecSpread.y));

			if (compute_random_seed(&angNoSpreadView, nCurrentTick) == nTempSeed)
			{
				res.adjusted = angNoSpreadView;
				res.seed = nTempSeed;
				res.found = true;
				break;
			}

		}

		return res;
	}
}



__forceinline Vector_t calc_manual_spread_sheet(CCSWeaponBase* weapon, int seed, float inaccuracy, float spread, bool revolver2 = false) {
	const char* item_def_index;
	float      recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

	if (!weapon)
		return { };
	// if we have no bullets, we have no spread.
	auto wep_info = weapon->GetWeaponData();
	if (!wep_info)
		return { };

	// get some data for later.
	item_def_index = wep_info->m_szName();
	recoil_index = weapon->m_flRecoilIndex();

	math::RandomSeed((seed & 0xff) + 1);

	// generate needed floats.
	r1 = math::RandomFloat(0.f, 1.f);
	r2 = math::RandomFloat(0.f, 3.14159265358979323846264338327950288f * 2);
	r3 = math::RandomFloat(0.f, 1.f);
	r4 = math::RandomFloat(0.f, 3.14159265358979323846264338327950288f * 2);

	// revolver secondary spread.
	if (item_def_index == ("weapon_revoler") && revolver2) {
		r1 = 1.f - (r1 * r1);
		r3 = 1.f - (r3 * r3);
	}

	// negev spread.
	else if (item_def_index == ("weapon_negev") && recoil_index < 3.f) {
		for (int i{ 3 }; i > recoil_index; --i) {
			r1 *= r1;
			r3 *= r3;
		}

		r1 = 1.f - r1;
		r3 = 1.f - r3;
	}

	// get needed sine / cosine values.
	c1 = std::cos(r2);
	c2 = std::cos(r4);
	s1 = std::sin(r2);
	s2 = std::sin(r4);

	// calculate spread vector.
	return {
		(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
		(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
		0.f
	};
}
bool  ragebot::c_data::weapon_is_at_max_accuracy(CUserCmd* cmd, CCSWeaponBaseVData* weapon_data, float inaccuracy) {
	auto local_data = g_prediction->get_local_data();

	if (!local_data || !cmd || !weapon_data)
		return false;

	constexpr auto round_accuracy = [](float accuracy) { return floorf(accuracy * 170.f) / 170.f; };
	constexpr auto round_duck_accuracy = [](float accuracy) { return floorf(accuracy * 300.f) / 300.f; };

	float speed = g::m_local_player_pawn->m_vecVelocity().Length();

	bool is_scoped = ((weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE)
		&& !(cmd->nButtons.nValue & IN_SCOPE)
		&& !g::m_local_player_pawn->m_bIsScoped());

	bool is_ducking = ((g::m_local_player_pawn->m_fFlags() & (1 << 1)) || (cmd->nButtons.nValue & IN_DUCK));

	float rounded_accuracy = round_accuracy(inaccuracy);
	float rounded_duck_accuracy = round_duck_accuracy(inaccuracy);

	if (is_ducking && is_scoped && (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND) && (rounded_duck_accuracy < local_data->m_inaccuracy))
		return true;

	if (speed <= 0 && is_scoped && (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND) && (rounded_accuracy < local_data->m_inaccuracy))
		return true;

	return false;
}

bool ragebot::c_data::hitchance(CUserCmd* cmd, CBaseEntity* entity, QAngle_t vAimpoint)
{
	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return false;

	if (!g::m_local_player_controller || !cmd)
		return false;

	if (!g::m_active_weapon || !g::m_weapon_data)
		return false;

	auto weapon = g::m_active_weapon;
	if (!weapon)
		return false;

	auto data = g::m_weapon_data;
	if (!data)
		return false;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return false;

	float HITCHANCE_MAX = 100.f;
	constexpr int SEED_MAX = 255;

	Vector_t start{ local_data->m_eye_pos }, end, fwd, right, up, dir, wep_spread;
	float inaccuracy, spread = {};
	float hitchance = adaptive.rage_hitchance;

	if (hitchance <= 0)
		return true;

	if (rage_type == NO_SPREAD)
		return true;

	// we cant reach it anymore too far
	if (local_data->m_eye_pos.DistTo(Vector_t(vAimpoint.x, vAimpoint.y, vAimpoint.z)) > data->range())
		return false;

	g::m_active_weapon->update_accuracy();

	if (this->weapon_is_at_max_accuracy(cmd, data, g::m_active_weapon->get_inaccuracy()))
		return true;

	size_t total_hits{}, needed_hits{ (size_t)std::ceil((hitchance * SEED_MAX) / HITCHANCE_MAX) };

	// get needed directional vectors.
	math::anglevectors(vAimpoint, &fwd, &right, &up);

	// store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
//	weapon->update_accuracy();
	spread = local_data->m_spread;
	inaccuracy = local_data->m_inaccuracy;

	// iterate all possible seeds.
	for (int i{}; i <= SEED_MAX; ++i)
	{
		// get spread.
		wep_spread = calc_manual_spread_sheet(weapon, i, inaccuracy, spread);
		//	hooks::m_get_spread_weapon_calculation(weapon, data, weapon->m_weaponMode(), i, inaccuracy, spread, weapon->m_flRecoilIndex(), &wep_spread.x, &wep_spread.y);

			// get spread direction.
		dir = (fwd + (right * wep_spread.x) + (up * wep_spread.y)).normalized();

		// get end of trace.
		end = start - (dir * data->range());

		trace_filter_t filter = {};
		sdk::m_trace->Init(filter, g::m_local_player_pawn, MASK_SHOT, 3, 7);
		game_trace_t trace = {};
		ray_t ray = {};

		sdk::m_trace->TraceShape(ray, &start, &end, filter, trace);
		sdk::m_trace->ClipTraceToPlayers(start, end, &filter, &trace, 0.F, 60.F, (1.F / (start - end).Length()) * (trace.m_end_pos - start).Length());

		if (trace.HitEntity) {
			if (trace.HitEntity->handle().index() == entity->handle().index())
				++total_hits;
		}

		// we made it.
		if (total_hits >= needed_hits) {

			return true;
		}


		// we cant make it anymore.
		if ((SEED_MAX - i + total_hits) < needed_hits)
			return false;
	}

	return false;

}
bool clamp_game_animations = false;


void ragebot::c_data::proccess(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base)
{
	command::m_shoot = false;
	command::m_rage_bot = false;
	clamp_game_animations = false;
	g::shoot_pos = g_prediction->get_local_data()->m_eye_pos;

	// update current rage status
	m_status = cfg_get(bool, g_cfg.safe_mode) ? e_status::SAFE_MODE : e_status::NORMAL;
	rage_type = adaptive.remove_weapon_accuracy_spread ? e_type::NO_SPREAD : e_type::SPREAD;

	if (!adaptive.rage_enable)
		return;

	if (!sdk::m_engine->is_valid() || sdk::m_game_rules->m_bFreezePeriod())
		return;

	if (!g::m_local_player_pawn || !g::m_local_player_controller)
		return;

	if (!g::m_local_player_pawn->IsAlive() || !g::m_local_player_pawn->GetActiveWeapon())
	{
		return;
	}

	if (!g::m_active_weapon || !g::m_weapon_data)
		return;

	if (!cmd)
		return;

	if (!base)
		return;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return;

	if (m_rage_last_target != nullptr)
		m_rage_last_target = nullptr;

	// init our routine & reset all necessary data 
	data->init();
	data->menu_adaptive();
	data->menu_hitboxes();

	// check if we allowed to use weapons
	if (!data->allowed())
	{
		clear(); return;
	}

	// Should i skip scan here and only scan best target? can delay rage in certain situations but improve performance nor just use multithreding
	// search for potential targets in game
	data->search_targets(cmd, base, e_target_mode::players);
	if (m_rage_aim_rage_targets.empty())
	{
		clear(); return;
	}

	if (!data->select_targets(cmd, base))
	{
		clear(); return;
	}

	if (!m_rage_best_target.m_player_pawn)
	{
		clear(); return;
	}

	if (!m_rage_best_target.m_player_pawn->IsAlive() || !m_rage_best_target.m_anim_record || !m_rage_best_target.m_data.record || !m_rage_best_target.m_data.valid())
	{
		clear(); return;
	}

	command::m_rage_bot = true;

	{
		// call RunCommand apply record/lag compensation data and call its command

		command::m_tick_count = m_rage_best_target.m_anim_record->m_simulation_time * 64.f + cfg_get(int, g_cfg.long_jump_key_style);
		command::m_tick_fraction = m_rage_best_target.m_data.m_tick.m_tickrange;
		command::m_aim_target = m_rage_best_target.m_player_pawn;
		command::m_rage_aim_target = m_rage_best_target.m_player_pawn;
		command::m_target_ent_index = m_rage_best_target.m_player_pawn->handle().index();

	

		// run nospread
		if (rage_type == NO_SPREAD) {
			command::m_player_tick_count = input->Message ? input->Message->m_nPlayerTickCount + cfg_get(int, g_cfg.semiragehc) : g_prediction->get_local_data()->m_shoot_tick;

			// can we fire this tick ?
			if (!can_shoot(g::m_local_player_controller->m_nTickBase()))
				return;

			QAngle_t aim_punch{ };
			hooks::m_get_aim_punch(g::m_local_player_pawn, &aim_punch, 0.f, true);
			auto spread_angle = spread_system::calc_angles(g::m_local_player_pawn->GetEyePosition(), m_rage_best_target.m_data.m_point.pos);
			auto spread_data = spread_system::listen(spread_angle);
			bool spread_seed_found = spread_data.found;
			if (!spread_seed_found)
				return;

			// use nospreaded angle
			auto best_point = spread_data.adjusted;
#ifdef _DEBUG
			//	L_PRINT(LOG_INFO) << "[ragebot] nospread valid seed found | angle: " << spread_data.adjusted << " seed:" << spread_data.seed;
#endif		

			command::m_best_point = best_point;
			auto crc = base;
			crc->bit(BASE_BITS_CLIENT_TICK);
			crc->nClientTick = command::m_tick_count;

			crc->view_angles->angValue = command::m_best_point;

			auto record_tick_count = command::m_tick_count;

			int current_player_tick_count = g_prediction->get_local_data()->m_player_tick;
			auto input_message = input->Message;
			if (input_message) {
				input_message->m_angViewAngles = command::m_best_point;

				input_message->m_nFrameTickCount = command::m_tick_count;
				//input_message->m_nPlayerTickCount = g_prediction->get_local_data()->m_shoot_tick;

				input_message->m_vecShootPosition = g::shoot_pos;
				input_message->m_nTargetIndex = command::m_target_ent_index;

				current_player_tick_count = input_message->m_nPlayerTickCount;
			}

			crc->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1); crc->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1); {
				if (!(crc->pInButtonState->nValue & IN_ATTACK)) {
					/* set attack button*/
					crc->pInButtonState->nValue |= IN_ATTACK;

					/* set attack start history index accordingly with our current start index from input history hook, cl_shot_stored_attack_idx: "-1 | 0" */
					cmd->bit(CSGOUSERCMD_BITS_PREDICTING_BODY_SHOT);
					cmd->m_bIsPredictingBodyShotFX = true;
					cmd->bit(CSGOUSERCMD_BITS_PREDICTING_HEAD_SHOT);
					cmd->m_bIsPredictingHeadShotFX = true;
					cmd->bit(CSGOUSERCMD_BITS_PREDICTING_KILL_RAGDOLLS);
					cmd->m_bIsPredictingKillRagdolls = true;

					cmd->bit(CSGOUSERCMD_BITS_ATTACK1START);
					cmd->nAttack1StartHistoryIndex = 0;

					cmd->bit(CSGOUSERCMD_BITS_ATTACK3START);
					cmd->nAttack1StartHistoryIndex = 0;
					//_log(LOG_INFO) << "[pre-shot] Old data player_tick_count: " << input_message->m_nPlayerTickCount << " fract: " << input_message->m_flPlayerTickFraction << " tickbase:" << g::m_local_player_controller->m_nTickBase() << " shoot_tick:" << g_prediction->get_local_data()->m_shoot_tick;

				}
			}

			auto user_cmd = cmd;
			auto input_history = user_cmd->get_input_history_tick(user_cmd->nAttack1StartHistoryIndex);
			if (input_history && user_cmd->nAttack1StartHistoryIndex != -1) {
				// proccess interp info
				struct csgo_interpolation_info_t {
					int src_tick = -1;
					int dst_tick = -1;
					float fraction = 0.f;
				};

				input_history->set_view_angles(command::m_best_point, true);
				input_history->set_render_tick_count(command::m_tick_count);
				input_history->set_player_tick_count(command::m_player_tick_count);
				// set target idx 
				input_history->set_shoot_pos(g_prediction->get_local_data()->m_eye_pos, true);
				_log(LOG_INFO) << "[lagcomp] set client shoot position data (non interpolated).";

				input_history->bit(INPUT_HISTORY_BITS_TARGETENTINDEX);
				input_history->nTargetEntIndex = command::m_target_ent_index;
			}

		}
		else if (rage_type == SPREAD) {
			// can we fire this tick ?
			if (!can_shoot(g::m_local_player_controller->m_nTickBase())) 
			{ _log(LOG_WARNING) << "[ragebot] set client delay invalid fire rate ( cant shoot)"; return; }
			
			// if safe mode check fov
			//auto fov = std::hypotf(g::angle.x - m_rage_best_target.m_data.m_point.pos.x, g::angle.y - m_rage_best_target.m_data.m_point.pos.y);

			/* calculate aim punch */
			QAngle_t aim_punch{ };
			hooks::m_get_aim_punch(g::m_local_player_pawn, &aim_punch, 0.f, true);

			/* calculate final angle direction */
			auto angle = aim_direction(local_data->m_eye_pos, m_rage_best_target.m_data.m_point.pos);
			auto best_point = QAngle_t(angle.x, angle.y, angle.z) - aim_punch;
			auto best_point_vec = angle - Vector_t(aim_punch.x, aim_punch.y, aim_punch.z);

			/* set best point */
			command::m_best_point = best_point;

			// run post auto stop ( to maintain accuracy not meant to make us shoot faster )
			command::m_stop = adaptive.auto_stop && g::m_local_player_pawn->valid_move();
			if (command::m_stop) {
				stop(cmd, base, QUICK);
				stop(cmd, base, ACCURATE);
				command::m_stop = false;
			}

			/* finish up everything and shoot */
			m_rage_best_target.m_anim_record->apply(m_rage_best_target.m_player_pawn);
			{
				// check for hitchance / accuracy perhaps run autostop if needed to
				if (adaptive.rage_hitchance > 0 && !hitchance(cmd, m_rage_best_target.m_player_pawn, m_rage_best_target.m_data.m_point.angle2d)) {
					command::m_stop = true;
					adaptive.failed_hitchance = true;
					return;
				}

				data->process_attack(input, cmd, base, SPREAD);
			}
			m_rage_best_target.m_anim_record->reset(m_rage_best_target.m_player_pawn);
		}
	}

	m_rage_last_target = m_rage_best_target.m_player_pawn;
}

void ragebot::c_data::process_attack(CCSGOInput* input, CUserCmd* user_cmd, CBaseUserCmdPB* crc, int type) {
	if (!user_cmd || !crc || !cfg_get(bool, g_cfg.rage_auto_fire))
		return;

	if (crc->pInButtonState) {
		{						
			command::m_player_tick_count = input->Message ? input->Message->m_nPlayerTickCount + cfg_get(int, g_cfg.semiragehc) : g_prediction->get_local_data()->m_shoot_tick;


			command::m_shoot = true;

			// between shots
			if (cfg_get(bool, g_cfg.between_shots)) 
				stop(user_cmd, crc, BETWEEN_SHOTS);
			
			bool ang_rotation_lag_comp_limit = std::abs(crc->view_angles->angValue.y - command::m_best_point.y) > 45.f || std::abs(crc->view_angles->angValue.x - command::m_best_point.x) > 45.f;

			/* patch WantsLagCompOnEntity function */
			if (crc->view_angles) {
				bool wants_force_lag_comp_on_entity = std::abs(crc->view_angles->angValue.y - command::m_best_point.y) > 45.f || std::abs(crc->view_angles->angValue.x - command::m_best_point.x) > 45.f && adaptive.rage_silent_mode == SILENT && m_status != SAFE_MODE;
				if (wants_force_lag_comp_on_entity && m_status != SAFE_MODE) {
					clamp_game_animations = adaptive.rage_silent_mode == SILENT;
					crc->bit(BASE_BITS_VIEWANGLES); {
						if (user_cmd->pBaseCmd) {
							if (user_cmd->pBaseCmd->view_angles)
								user_cmd->pBaseCmd->view_angles->angValue.y = crc->view_angles->angValue.y = std::remainder(command::m_best_point.y + 180.0f, 360.0f);
							else
								crc->view_angles->angValue.y = std::remainder(command::m_best_point.y + 180.0f, 360.0f);
						}
						else
							crc->view_angles->angValue.y = std::remainder(command::m_best_point.y + 180.0f, 360.0f);
					}
				}

				if (clamp_game_animations && m_status != SAFE_MODE) {
					crc->bit(BASE_BITS_VIEWANGLES);
					crc->view_angles->bit(BASE_BITS_VIEWANGLES);

					if (user_cmd->pBaseCmd) {
						user_cmd->pBaseCmd->bit(BASE_BITS_VIEWANGLES);
						if (user_cmd->pBaseCmd->view_angles) {
							user_cmd->pBaseCmd->view_angles->bit(BASE_BITS_VIEWANGLES);
							user_cmd->pBaseCmd->view_angles->angValue.x = crc->view_angles->angValue.x = 179.f;
						}
						else
							crc->view_angles->angValue.x = 179.f;
					}
					else
						crc->view_angles->angValue.x = 179.f;
				}
			}


			int current_player_tick_count = g_prediction->get_local_data()->m_player_tick;
		
			auto input_message = input->Message;
			if (input_message && m_status != SAFE_MODE) {
				input_message->m_angViewAngles = command::m_best_point;
				input_message->m_nFrameTickCount = command::m_tick_count;
				input_message->m_vecShootPosition = g::shoot_pos;
				input_message->m_nTargetIndex = command::m_target_ent_index;
				current_player_tick_count = input_message->m_nPlayerTickCount;
			}

			/* set current client tick */
			crc->set_tick_count(command::m_tick_count);

			/* on shot stuff */
			if (crc->view_angles) {
				// detected
				if (adaptive.rage_silent_mode != SILENT)
					crc->set_view_angles(command::m_best_point);
			}

			bool should_fire = true;
			if (ang_rotation_lag_comp_limit && m_status == SAFE_MODE)
				should_fire = false;

			if (should_fire) {
				crc->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1); crc->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1); {
					if (!(crc->pInButtonState->nValue & IN_ATTACK)) {
						crc->pInButtonState->nValue |= IN_ATTACK;


						/* add csgoinput submovefields button */
						/* set attack start history index accordingly with our current start index from input history hook, cl_shot_stored_attack_idx: "-1 | 0" */
						user_cmd->bit(CSGOUSERCMD_BITS_PREDICTING_BODY_SHOT);
						user_cmd->m_bIsPredictingBodyShotFX = true;
						user_cmd->bit(CSGOUSERCMD_BITS_PREDICTING_HEAD_SHOT);
						user_cmd->m_bIsPredictingHeadShotFX = true;
						user_cmd->bit(CSGOUSERCMD_BITS_PREDICTING_KILL_RAGDOLLS);
						user_cmd->m_bIsPredictingKillRagdolls = true;

						user_cmd->bit(CSGOUSERCMD_BITS_ATTACK1START);
						user_cmd->nAttack1StartHistoryIndex = 0;

						user_cmd->bit(CSGOUSERCMD_BITS_ATTACK3START);
						user_cmd->nAttack3StartHistoryIndex = 0;
						_log(LOG_INFO) << "[shot] data player_tick_count: " << input_message->m_nPlayerTickCount << " fract: " << input_message->m_flPlayerTickFraction << " tickbase:" << g::m_local_player_controller->m_nTickBase() << " shoot_tick:" << g_prediction->get_local_data()->m_shoot_tick;
					}
				}
			}

			auto input_history = user_cmd->get_input_history_tick(user_cmd->nAttack1StartHistoryIndex);
			if (input_history && user_cmd->nAttack1StartHistoryIndex != -1) {

				// proccess interp info
				struct csgo_interpolation_info_t {
					int src_tick = -1;
					int dst_tick = -1;
					float fraction = 0.f;
				};

				input_history->set_view_angles(command::m_best_point, true);
				//input_history->set_render_tick_fraction(sv0_interpolated_data.fraction);

				// make sure we grab correct interp info from game with fresh tick count
				/*csgo_interpolation_info_t cl_interpolated_data, sv0_interpolated_data, sv1_interpolated_data;
				if (hooks::m_get_lerp_tick(m_rage_best_target.m_player_pawn->m_pGameSceneNode(), &cl_interpolated_data,
					&sv0_interpolated_data,
					&sv1_interpolated_data,
					&input_history->nPlayerTickCount)) {

					// setup interpolation info Note: dst tick always game default now cuz was getting wrong dst tick values
					input_history->set_cl_interp(cl_interpolated_data.fraction, true);
					input_history->set_sv_interp(sv0_interpolated_data.src_tick, sv0_interpolated_data.dst_tick, sv0_interpolated_data.fraction, false, true);
					input_history->set_sv_interp1(sv1_interpolated_data.src_tick, sv1_interpolated_data.dst_tick, sv1_interpolated_data.fraction, false, true);
					_log(LOG_INFO) << "[lagcomp] set client interpolation information.";
				}
				*/
				//hooks::m_interpolate_shoot_position(player, eyepos_to_be_interpolated, playeritckcountmby??? );
				input_history->set_render_tick_count(command::m_tick_count);

				input_history->set_player_tick_count(command::m_player_tick_count);

				// set target idx 
				input_history->set_shoot_pos(g_prediction->get_local_data()->m_eye_pos, true);

				input_history->bit(INPUT_HISTORY_BITS_TARGETENTINDEX);
				input_history->nTargetEntIndex = command::m_target_ent_index;
			}				
			
				/* setup input message info */
/*#ifdef _DEBUG
				_log(LOG_WARNING) << "[Interpolation] cl_interp src:" << cl_interpolated_data2.src_tick << " dst: " << cl_interpolated_data2.src_tick << " fract:" << cl_interpolated_data2.fraction;
				_log(LOG_WARNING) << "[Interpolation] sv_interp0 src:" << sv0_interpolated_data.src_tick << " dst: " << sv0_interpolated_data.src_tick << " fract:" << sv0_interpolated_data.fraction;
				_log(LOG_WARNING) << "[Interpolation] sv_interp1 src:" << sv1_interpolated_data.src_tick << " dst: " << sv1_interpolated_data.src_tick << " fract:" << sv1_interpolated_data.fraction;
				_log(LOG_INFO) << "[backtrack] renderTickCount:" << input_history->nRenderTickCount << " commandTick_count: " << command::m_tick_count << " lerp : " << m_rage_best_target.m_anim_record->m_simulation_time + sdk::m_network->GetNetworkClient()->lerp() * 64.f << " fract : " << cl_interpolated_data.fraction;
				_log(LOG_WARNING) << "[post-shot] localPlayer > player_tick_count: " << input_history->nPlayerTickCount << " fract: " << input_history->flPlayerTickFraction << " tickbase:" << g::m_local_player_controller->m_nTickBase() << " shoot_tick:" << g_prediction->get_local_data()->m_shoot_tick;
#endif*/
		}
	}

	// reset prev attack state
	if (g::prev_button_attack)
		g::prev_button_attack = false;

}

// todo: rebuilt game movement 
void ragebot::c_data::early_stop(CUserCmd* cmd, CBaseUserCmdPB* crc, int damage)
{
	auto override_damage = Input::GetBindState(cfg_get(KeyBind_t, g_cfg.min_dmg_key));

	int min_damage = adaptive.minimum_damage;

	if (override_damage)
		min_damage = adaptive.minimum_damage_override;


	if (adaptive.auto_stop) {
		if (damage >= 1) {
			auto cmd = CUserCmd::Get();
			if (cmd && crc && can_shoot(g_prediction->get_local_data()->m_shoot_tick)) {
				if (g_prediction->get_local_data()->m_velocity.Length2D() >= g_prediction->get_local_data()->m_max_accurate_velocity) {
					if (g::m_active_weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE) {
						if (damage >= (min_damage) && g::m_local_player_pawn->m_bIsScoped())
							stop(cmd, crc, PREDICTIVE);
					}
					else { // do not be too agressive 
						if (damage >= (min_damage))
							stop(cmd, crc, SLOW);
					}
				}
			}
		}
	}
}

void ragebot::c_data::search_targets(CUserCmd* cmd, CBaseUserCmdPB* base, ragebot::e_target_mode mode) {

	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return;

	if (!g::m_local_player_pawn || !g::m_local_player_controller)
		return;

	if (!cmd)
		return;

	// basic logic to iterate targets in a optimized way however way more improvements will be done by tmrw
	if (mode == e_target_mode::players) {
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

			/* perhaps i coudl use this logic instead for all entities and sort them based on class name ??? prob better*/
			if (hash == hash_32_fnv1a_const(xorstr_("CCSPlayerController")))
			{
				CCSPlayerController* p_controller = reinterpret_cast<CCSPlayerController*>(entity);
				/* make sure to avoid caching local */
				if (!p_controller)
					continue;

				auto handle = p_controller->handle().index();

				CCSPlayerPawn* player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle());

				if (!player)
					continue;

				auto player_scene_node = player->m_pGameSceneNode();
				if (!player_scene_node)
					continue;

				if (!player->IsAlive() || !g::m_local_player_pawn->enemy(player) || player_scene_node->dormant() || player->immune())
					continue;

				// add target and keep searching for new ones
				m_rage_targets.push_back(c_target(player, entity, handle, p_controller->GetPlayerName()));
			}
		}

		// we found targets proceed to scan them
		if (m_rage_targets.empty())
			return;

		// rset old data
		m_rage_aim_rage_targets.clear();

		// iterate thru sorted targets 
		for (auto& target : m_rage_targets)
		{
			auto m_entity = target.entity;
			if (!m_entity)
				continue;

			auto m_player = target.player;
			if (!m_player)
				continue;

			// get current best player lag records etc
			auto best_record = this->select_record(target.handle);
			if (!best_record) {

				continue;
			}

			// compete best records & store data for them  
			// get best record, point, store best, dmg, hitbox etc
			// once we retrived all info write it into aim_data buffer
			c_aim_data aim_data = {};
			//if (!data->scan_target(base, m_player, best_record, aim_data))		
			//		continue;

			m_rage_aim_rage_targets.emplace_back(c_best_target(best_record, m_entity, m_player, g::eye_pos.DistTo(m_player->GetEyePosition()), aim_data, target.name));
		}
	}


}

ragebot::c_best_target* ragebot::c_data::get_best_target(CUserCmd* cmd, CBaseUserCmdPB* base) {
	if (!g::m_local_player_pawn->IsAlive())
		return nullptr;

	ragebot::c_best_target* best_target = nullptr;

	auto local_entity = sdk::m_game->pGameEntitySystem->Get(g::m_local_player_pawn->handle());
	if (!local_entity)
		return nullptr;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return nullptr;

	auto m_predicted_shoot_position = local_data->m_eye_pos;

	float best_damage = 0.f;
	float best_distance = FLT_MAX;

	// get network client 
	auto m_network_game_client = sdk::m_network->GetNetworkClient();

	if (m_network_game_client) {
		auto networked_net_channel = m_network_game_client->get_net_channel2();
		if (networked_net_channel) {
			auto m_tick_rate = 64.f;
			auto m_max_unlag = 1000.f;
			// ghetto but should do its job
			auto m_ticks_to_unlag = local_data->m_velocity.Length2D() > g_prediction->get_local_data()->m_max_accurate_velocity ? 2.f : 1.f;
			const auto m_interval_per_tick = g::interval;//1.f / 64.f;
			auto predicted_unlag = static_cast<float>((networked_net_channel->get_network_latency() * m_tick_rate) / m_max_unlag);
			m_predicted_shoot_position = (local_data->m_eye_pos) + (local_data->m_velocity * m_interval_per_tick * static_cast<float>(m_ticks_to_unlag)) + (local_data->m_velocity * predicted_unlag);
		}
	}
	else  // should never happen
		m_predicted_shoot_position = (local_data->m_eye_pos);

	auto stop = false;
	auto stop_damage = 0.f;
	float best_crosshair = 9999;

	// doesn't define best target later one we sort by damage 
	for (auto& target : m_rage_aim_rage_targets)
	{
		if (!target.m_anim_record || !target.m_player_pawn)
			continue;

		if (!target.m_player_pawn->IsAlive() || !target.m_player_pawn->m_pGameSceneNode() || target.m_player_pawn->immune())
			continue;

		if (!target.m_player_pawn->m_pGameSceneNode()->m_pSkeleton() || !target.m_player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().m_hModel())
			continue;
		float m_predicted_damage = 0.f;
		bool m_predicted_can_hit = false;
		float dist = g::m_local_player_pawn->GetEyePosition().DistTo(target.m_player_pawn->m_pGameSceneNode()->m_vecAbsOrigin());

		autowall::c_auto_wall::data_t pen_data;

		if (!best_target)
			best_target = &target;

		auto m_predicted_bone_data = target.m_player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().hitbox_data()[head];
		// if we cannot peform pre penetration on entity ignore damage factor and sort by dist/fov etc
		if (!autowall::g_auto_wall->penetrate(
			pen_data,
			m_predicted_shoot_position,
			m_predicted_bone_data.pos,
			target.m_entity,
			local_entity,
			g::m_local_player_pawn,
			target.m_anim_record->m_pawn,
			g::m_weapon_data,
			m_predicted_damage,
			m_predicted_can_hit)) {

			// we aint reach here yet just set this fresh one
			float cross = crosshair_fov(target.m_player_pawn->GetEyePosition());
			if (cross < best_crosshair) {
				best_target = &target;
				best_distance = dist;
				best_crosshair = cross;
				continue;
			}


			continue;
		}

		m_predicted_damage = pen_data.m_can_hit ? pen_data.m_dmg : 0.f;

		// we can deal damage & found valid target? select him
		if (!best_target && m_predicted_damage > 0)
			best_target = &target;

		// set predicted stop data
		stop_damage = m_predicted_damage;
		stop = stop_damage > 0;

		// no damage can be deal remove the target
		if (m_predicted_damage <= 0.f) {

			// we aint reach here yet just set this fresh one
			if (!best_target) {
				float cross = crosshair_fov(target.m_player_pawn->GetEyePosition());
				if (cross < best_crosshair) {
					best_crosshair = cross;
					best_target = &target;
				}
			}
		}

		if (m_predicted_damage > best_damage) {
			stop = true;
			best_target = &target;
			best_distance = dist;
			stop_damage = m_predicted_damage;
			best_damage = m_predicted_damage;
			break; // get out of the loop we got our best target rn 
		}

		continue;
	}

	// idk how whatever 
	if (!best_target) {
		m_rage_best_target.clear();
		return nullptr;
	}

	// auto scope
	if (best_damage > 0 && best_target && can_shoot(g_prediction->get_local_data()->m_shoot_tick) && adaptive.auto_scope && g::m_weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE && !g::m_local_player_pawn->m_bIsScoped()) {
		// force scope/zoom
		if (base->pInButtonState) {
			cmd->bit(CSGOUSERCMD_BITS_ATTACK2START);
			cmd->nAttack2StartHistoryIndex = 0;
			base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1); {
				base->pInButtonState->nValue |= IN_ATTACK2;
			}
		}
	}


	// run predicted stop
	if (stop)
		early_stop(cmd, base, stop_damage);

	return best_target;
}

// check closest to crosshair maybe usefull for mm prime / safe mode 
ragebot::c_best_target* ragebot::c_data::get_closest_fov_target() {
	if (!g::m_local_player_pawn->IsAlive())
		return nullptr;

	ragebot::c_best_target* best_target{};

	auto local_data = g_prediction->get_local_data();

	if (!local_data)
		return nullptr;

	auto shoot_pos = local_data->m_eye_pos;

	float best_distance = 9999;

	for (auto it = m_rage_aim_rage_targets.begin(); it != m_rage_aim_rage_targets.end(); ++it) {
		auto& target = *it;

		if (!target.m_anim_record
			|| !target.m_player_pawn
			|| !target.m_player_pawn->IsAlive()
			|| !target.m_player_pawn->m_pGameSceneNode()) {
			continue;
		}

		float distance = crosshair_fov(target.m_player_pawn->GetEyePosition());
		if (distance < best_distance) {
			best_distance = distance;
			best_target = &target;
		}
	}

	return best_target;
}

bool ragebot::c_data::select_targets(CUserCmd* cmd, CBaseUserCmdPB* crc)
{
	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return false;

	if (!cmd || !crc) {
		m_rage_best_target.clear();
		return false;
	}

	if (!g::m_local_player_controller || !g::m_local_player_pawn || !g::m_active_weapon || !g::m_weapon_data) {
		m_rage_best_target.clear();
		return false;
	}

	if (m_rage_targets.empty() || m_rage_aim_rage_targets.empty()) {
		m_rage_best_target.clear();
		return false;
	}

	c_best_target* best_potential_target = this->get_best_target(cmd, crc);
	if (!best_potential_target) {
		m_rage_best_target.clear();
		return false;
	}


	auto anim_record = best_potential_target->m_anim_record;
	if (!anim_record) {
		m_rage_best_target.clear();
		return false;
	}

	auto player = best_potential_target->m_player_pawn;
	if (!player) {
		m_rage_best_target.clear();
		return false;
	}

	if (!player->IsAlive()) {
		m_rage_best_target.clear();
		return false;
	}

	bool is_taser = g::m_weapon_data->m_WeaponType() == WEAPONTYPE_TASER;

	auto override_damage = Input::GetBindState(cfg_get(KeyBind_t, g_cfg.min_dmg_key));

	int min_damage = std::min(adaptive.minimum_damage, anim_record->m_pawn->m_iHealth());

	if (override_damage)
		min_damage = std::min(adaptive.minimum_damage_override, anim_record->m_pawn->m_iHealth());
	
	if (min_damage > anim_record->m_pawn->m_iHealth() + 5)
		min_damage = anim_record->m_pawn->m_iHealth() + 5; /* should never happen */

	if (is_taser)
		min_damage = anim_record->m_pawn->m_iHealth();

	anim_record->apply(player);

	float best_damage = 0.0f;
	c_aim_point best_point = this->scan_points(crc, anim_record, best_damage, min_damage);

	anim_record->reset(player);

	// yeah we gon delay 1 tick but whatever at least we dont stick to a target 
	if (best_point.idx == -1 || best_damage < min_damage)
		return false;


	// set new best target 
	m_rage_best_target.m_data.record = anim_record;
	m_rage_best_target.m_data.m_point = best_point;
	m_rage_best_target.m_data.m_hitbox.m_idx = best_point.idx;
	m_rage_best_target.m_data.m_hitbox.m_damage = best_damage;
	m_rage_best_target.m_player_pawn = player;
	m_rage_best_target.m_anim_record = anim_record;
	m_rage_best_target.m_entity = best_potential_target->m_entity;
	m_rage_best_target.m_distance = best_potential_target->m_distance;

	// update last target 
	m_rage_last_target = m_rage_best_target.m_player_pawn;

	return true;
}


ragebot::hitbox_data_t ragebot::c_data::get_hitbox_data(CCSPlayerPawn* player_pawn, const int hitbox_id) {
	if (!player_pawn)
		return ragebot::hitbox_data_t();

	auto game_scene = player_pawn->m_pGameSceneNode();
	if (!game_scene)
		return ragebot::hitbox_data_t();

	auto skeleton = game_scene->m_pSkeleton();
	if (!skeleton)
		return ragebot::hitbox_data_t();

	auto& model_state = skeleton->GetModel();
	auto& model = model_state.m_hModel();
	if (!model)
		return ragebot::hitbox_data_t();

	CHitBoxSets* m_hitbox_set = hooks::m_get_hitbox_set(player_pawn, 0);
	if (!m_hitbox_set)
		return ragebot::hitbox_data_t();

	CHitBox* hitbox = &m_hitbox_set->m_HitBoxes()[hitbox_id];
	if (!hitbox)
		return ragebot::hitbox_data_t();

	CTransform m_hitbox_transform[64];
	int m_count = hooks::m_get_hitbox_to_world_transform(player_pawn, m_hitbox_set, m_hitbox_transform, 1024);
	if (!m_count)
		return ragebot::hitbox_data_t();

	const auto transformed_matrix = m_hitbox_transform[hitbox_id].ToMatrix();
	Vector_t w_mins, w_maxs;

	math::TransformAABB(transformed_matrix, hitbox->m_vMinBounds(), hitbox->m_vMaxBounds(), w_mins, w_maxs);

	const auto bone_index = static_cast<int>(get_hitgroup(static_cast<int>(hitbox_id)));

	const auto bones = model_state.hitbox_data();

	ragebot::hitbox_data_t data;
	data.m_num_hitbox = hitbox_id;
	data.m_num_bone = bone_index;
	data.m_hitbox_data = hitbox;
	data.m_radius = hitbox->m_flShapeRadius();
	data.m_num_hitgroup = hitbox->m_hitbox_group_id();
	math::transform_to_matrix(bones[bone_index], data.m_matrix);
	data.m_mins = hitbox->m_vMaxBounds();
	data.m_maxs = hitbox->m_vMaxBounds();
	data.m_transformed_mins = w_mins;
	data.m_transformed_maxs = w_maxs;

	// we set data for this hitbox set as valid
	data.m_invalid_data = false;

	return data;
}
Vector_t transform_point(Matrix3x4_t matrix, Vector_t point)
{
	Vector_t result{ };
	math::vector_transform(point, matrix, result);
	return result;
}
std::vector<Vector_t> calculate_sphere_points(float radius, int num_points)
{
	std::vector<Vector_t> points;
	points.reserve(num_points);

	const float phi = math::_PI * (3.0f - std::sqrt(5.0f));

	for (int i = 0; i < num_points; ++i)
	{
		float y = 1 - (i / float(num_points - 1)) * 2;
		float radius_at_y = std::sqrt(1 - y * y);

		float theta = phi * i;

		float x = std::cos(theta) * radius_at_y;
		float z = std::sin(theta) * radius_at_y;

		Vector_t vec_point{};
		points.push_back({ x * radius, y * radius, z * radius });
	}

	return points;
}

std::vector<Vector_t> calculate_points(int num_points, float radius, float height, Matrix3x4_t matrix, ragebot::hitbox_data_t hitbox)
{
	std::vector<Vector_t> points;
	points.reserve(num_points);

	for (int i = 0; i < num_points; ++i)
	{
		float theta = 2.f * math::_PI * i / num_points;
		float y = radius * std::cos(theta);
		float z = radius * std::sin(theta);

		Vector_t in_point = { hitbox.m_mins.x + hitbox.m_radius * height, y, z };

		Vector_t vec_point{};
		math::vector_transform(in_point, matrix, vec_point);
		points.push_back(vec_point);
	}

	return points;
}

bool ragebot::c_data::add_point(CBaseUserCmdPB* crc, anim_record_t* record, int hitbox_index, std::vector<c_aim_point>& points) {
	if (!record)
		return false;

	auto pawn = record->m_pawn;

	if (!pawn || !pawn->IsAlive())
		return false;

	hitbox_data_t hitbox_data = data->get_hitbox_data(pawn, hitbox_index);

	if (hitbox_data.m_invalid_data)
		return false;

	auto local_data = g_prediction->get_local_data();

	if (!local_data)
		return false;

	Vector_t center = (hitbox_data.m_mins + hitbox_data.m_maxs) * 0.5f;

	Matrix3x4_t matrix;
	math::transform_to_matrix(record->m_bone_data[hitbox_data.m_num_bone], matrix);


	auto bone_pos = record->m_bone_data[hitbox_data.m_num_bone].pos;
	QAngle_t hitchance_pos;
	math::VectorAngless(local_data->m_eye_pos - bone_pos, hitchance_pos);

	Vector_t point = transform_point(matrix, center);

	int hitbox_from_menu = (hitbox_index);
	if (hitbox_from_menu == -1)
		return false;

	// add center point on current hitbox
	c_aim_point temp_point;
	temp_point.pos = point;
	temp_point.idx = hitbox_index;
	temp_point.angle2d = hitchance_pos;
	points.emplace_back(temp_point);


	auto cfg_scale = hitbox_index == HITBOX_HEAD || hitbox_index == HITBOX_NECK ? adaptive.head_scale : adaptive.point_scale;
	float scale = cfg_scale >= 10.f ? cfg_scale : 20.f;

	float radius = hitbox_data.m_radius * (scale / 100.f);

	if (radius <= 0.f)
		return false;

	// skip mp here
	if (hitbox_index == HITBOX_UPPER_CHEST)
		return false;

	// Limbs
	if (hitbox_index > HITBOX_UPPER_CHEST) {
		auto limb_points = calculate_sphere_points(radius, 3);
		if (limb_points.empty())
			return false;
		for (const auto& point : limb_points) {
			auto point_pos = transform_point(matrix, { hitbox_data.m_maxs.x + point.x, hitbox_data.m_maxs.y + point.y, hitbox_data.m_maxs.z + point.z });
			c_aim_point temp_limb_point;
			temp_limb_point.pos = point_pos;
			temp_limb_point.idx = hitbox_index;
			temp_limb_point.angle2d = hitchance_pos;
			points.emplace_back(temp_limb_point);
		}
		return true;
	}

	if (hitbox_index == HITBOX_HEAD || hitbox_index == HITBOX_NECK) {
		auto sphere_points = calculate_sphere_points(radius, 10);

		if (sphere_points.empty())
			return false;

		for (const auto& point : sphere_points) {
			auto point_pos = transform_point(matrix, { hitbox_data.m_maxs.x + point.x, hitbox_data.m_maxs.y + point.y, hitbox_data.m_maxs.z + point.z });
			c_aim_point temp_head_point;
			temp_head_point.pos = point_pos;
			temp_head_point.idx = hitbox_index;
			temp_head_point.angle2d = hitchance_pos;

			// add current head point
			points.emplace_back(temp_head_point);
		}

		return true;
	}

	auto capsule_points = calculate_points(4, radius, 0.5f, matrix, hitbox_data);

	if (capsule_points.empty())
		return false;

	for (const auto& point : capsule_points) {
		c_aim_point temp_default_point;
		temp_default_point.pos = point;
		temp_default_point.idx = hitbox_index;
		temp_default_point.angle2d = hitchance_pos;
		points.emplace_back(temp_default_point);
	}

	return true;
}
// test
ragebot::c_aim_point ragebot::c_data::multi_thread_scan_points(CBaseUserCmdPB* crc, anim_record_t* record, float& damage, float min_damage)
{
	std::vector<c_aim_point> points;
	c_aim_point best_point{};

	if (!g::m_local_player_pawn->IsAlive() || !record->m_pawn || !record->m_pawn->IsAlive() || !g::m_active_weapon)
		return best_point;

	// Prepare local and target entities
	auto local_entity = sdk::m_game->pGameEntitySystem->Get(g::m_local_player_pawn->handle());
	auto entity = sdk::m_game->pGameEntitySystem->Get(record->m_pawn->handle());
	if (!local_entity || !entity)
		return best_point;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return best_point;

	Vector_t shoot_pos = local_data->m_eye_pos;

	// Storage for all multipoints
	std::vector<ragebot::c_aim_point> stored_multipoints;

	// Split workload into batches
	std::vector<std::future<std::pair<c_aim_point, float>>> futures;

	for (auto& hitbox_idx : m_rage_aim_hitboxes) {
		std::vector<ragebot::c_aim_point> multi_points;

		if (!add_point(crc, record, hitbox_idx, multi_points))
			continue;

		// never happen
		if (multi_points.empty())
			continue;

		stored_multipoints.insert(stored_multipoints.end(), multi_points.begin(), multi_points.end());

		// Launch async task to process this batch
		futures.push_back(std::async(std::launch::async, [this, shoot_pos, entity, local_entity, crc, record, multi_points]() {
			float best_damage = 0.0f;
			bool can_hit = false;
			c_aim_point best_point;

			for (const auto& point : multi_points) {
				autowall::c_auto_wall::data_t pen_data;

				if (!autowall::g_auto_wall->penetrate(pen_data, shoot_pos, point.pos, entity, local_entity, g::m_local_player_pawn, record->m_pawn, g::m_weapon_data, best_damage, can_hit))
					continue;

				float point_damage = pen_data.m_can_hit ? pen_data.m_dmg : 0.0f;

				if (point_damage > best_damage) {
					best_damage = point_damage;
					best_point = point;
				}
			}

			return std::make_pair(best_point, best_damage);
			}));
	}

	// Aggregate results from all threads
	float best_damage = 0.0f;
	for (auto& future : futures) {
		auto [current_best_point, current_damage] = future.get();
		if (current_damage > best_damage) {
			best_damage = current_damage;
			best_point = current_best_point;
		}
	}

	// Store the render points for visualization
	best_point.render_points.clear();
	best_point.render_points.insert(best_point.render_points.end(), stored_multipoints.begin(), stored_multipoints.end());

	damage = best_damage;
	return best_point;
}

ragebot::c_aim_point ragebot::c_data::scan_points(CBaseUserCmdPB* crc, ragebot::anim_record_t* record, float& damage, float min_damage) {
	std::vector<c_aim_point> points{};
	c_aim_point best_point{};

	if (!g::m_local_player_pawn->IsAlive())
		return best_point;

	if (!record->m_pawn || !record->m_pawn->IsAlive())
		return best_point;

	if (!g::m_active_weapon)
		return best_point;

	// trace bs
	auto local_entity = sdk::m_game->pGameEntitySystem->Get(g::m_local_player_pawn->handle());
	if (!local_entity)
		return best_point;

	auto entity = sdk::m_game->pGameEntitySystem->Get(record->m_pawn->handle());
	if (!entity)
		return best_point;

	auto weapon_data = g::m_weapon_data;
	if (!weapon_data)
		return best_point;

	auto local_data = g_prediction->get_local_data();

	if (!local_data)
		return best_point;

	Vector_t shoot_pos = local_data->m_eye_pos;

	bool is_taser = weapon_data->m_WeaponType() == WEAPONTYPE_TASER;

	float best_damage = 0.f;
	int prev_hitbox = -1;
	std::vector<ragebot::c_aim_point> stored_multipoints;

	for (auto& hitbox_idx : m_rage_aim_hitboxes) {
		std::vector<ragebot::c_aim_point> multi_points{};

		if (!add_point(crc, record, hitbox_idx, multi_points))
			continue;

		// store new multipoints into the global vector       
		//stored_multipoints.insert(stored_multipoints.end(), multi_points.begin(), multi_points.end());

		for (auto& point : multi_points) {

			// initialize autowall data
			autowall::c_auto_wall::data_t pen_data;

			// Run the trace/penetration with the updated position
			if (!autowall::g_auto_wall->penetrate(pen_data, shoot_pos, point.pos, entity, local_entity, g::m_local_player_pawn, record->m_pawn, g::m_weapon_data, damage, point.canHit))
				continue;

			Vector_t point_angle;
			float point_damage = pen_data.m_can_hit ? pen_data.m_dmg : 0.f;
				
			/* ignore limbs on air */
			if (!(record->m_pawn->m_fFlags() & FL_ONGROUND)  && hitbox_idx >= 6 && hitbox_idx <= 18)
				continue;

			/* first point scanned set as best point and keep searching */
			if (best_damage <= 0.f) {
				best_point = point;
				stored_multipoints.push_back(point);
				prev_hitbox = hitbox_idx;
				best_damage = point_damage;
			}

			/* more damage previous point */
			if (point_damage  > best_damage) {
				best_point = point;
				stored_multipoints.push_back(point);
				best_damage = point_damage;
				prev_hitbox = hitbox_idx;
			}

			/* body lethal */ 
			if (point_damage >= record->m_pawn->m_iHealth() + 5 && hitbox_idx != hitbox_head) {
				best_point = point;
				stored_multipoints.push_back(point);
				best_damage = point_damage;
				prev_hitbox = hitbox_idx;
				continue;
			}

			/* head & lethal break */
			if (hitbox_idx == hitbox_head && point_damage >= record->m_pawn->m_iHealth()) {
				best_point = point;
				stored_multipoints.push_back(point);
				best_damage = point_damage;
				prev_hitbox = hitbox_idx;
			}

			/* head & lethal break */
			if (point_damage > best_damage && hitbox_idx == hitbox_head && point_damage >= record->m_pawn->m_iHealth()) {
				best_point = point;
				stored_multipoints.push_back(point);
				best_damage = point_damage;
				prev_hitbox = hitbox_idx;
				break;
			}
		}
	}

	//best_point.render_points.insert(best_point.render_points.end(), stored_multipoints.begin(), stored_multipoints.end());	
	stored_multipoints.push_back(best_point);
	best_point.render_points.insert(best_point.render_points.end(), stored_multipoints.begin(), stored_multipoints.end());

	damage = best_damage;
	// predictive damage stop 
	{
		// should be auto scaled
		auto fov = std::hypotf(g::angle.x - best_point.pos.x, g::angle.y - best_point.pos.y);
		bool is_safe = m_status == e_status::SAFE_MODE ? fov <= cfg_get(int, g_cfg.rage_fov) : true;
		bool should_predict = m_status == e_status::SAFE_MODE ? rage_type == SPREAD && fov <= cfg_get(int, g_cfg.rage_fov) : rage_type == SPREAD;
		if (should_predict && adaptive.auto_stop && can_shoot(g::m_local_player_controller->m_nTickBase())) {
			if (damage >= 1) {
				auto cmd = CUserCmd::Get();
				if (cmd && crc && can_shoot(g_prediction->get_local_data()->m_shoot_tick)) {
					if (g_prediction->get_local_data()->m_velocity.Length2D() >= g_prediction->get_local_data()->m_max_accurate_velocity) {

						if (g::m_active_weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE) {
							bool should_hight_acc = false;
							if (min_damage > 55)
								should_hight_acc = damage >= (min_damage);
							else {
								auto new_dmg = (min_damage) / 2;
								should_hight_acc = new_dmg > 0 && damage >= (min_damage) / 2;
							}

							if (should_hight_acc && g::m_local_player_pawn->m_bIsScoped())
							{
								stop(cmd, crc, PREDICTIVE);
							}
						}
						else {
							stop(cmd, crc, PREDICTIVE);
						}
					}
				}
			}
		}
	}

	return best_point;
}

void ragebot::c_data::render_multi_points()
{
	if (!cfg_get(bool, g_cfg.off_screen))
		return;


	if (!m_rage_best_target.m_player_pawn)
		return;

	if (!m_rage_best_target.m_player_pawn->IsAlive())
		return;



	if (!m_rage_best_target.m_data.record)
		return;

	if (m_rage_best_target.m_data.m_point.render_points.empty())
		return;

	for (const auto& point : m_rage_best_target.m_data.m_point.render_points) {
		ImVec2 screen_pos;

		float distance = (g::eye_pos - point.pos).Length();
		float size = std::max(2.0f / (distance / 100.0f), 1.0f);
		if (math::WorldToScreen(point.pos, screen_pos)) {
			ImGui::GetBackgroundDrawList()->AddCircleFilled(
				screen_pos,
				2.f,
				IM_COL32(222, 0, 0, 222)
			);
		}
	}

}

/*
// Old hitscan logic
float ragebot::c_data::get_damage(anim_record_t* best_record, CCSPlayerPawn* target, CCSPlayerPawn* pLocal, Vector_t start, CCSWeaponBase* weapon, int id)
{
	if (!pLocal || !weapon || !target || !best_record)
		return 0.f;

	if (pLocal->m_iHealth() <= 0 || !best_record->m_pawn || best_record->m_pawn->m_iHealth() <= 0 || target->m_iHealth() <= 0)
		return 0.f;

	auto vdata = g::m_weapon_data;
	if (!vdata)
		return 0.f;

	auto localent = sdk::m_game->pGameEntitySystem->Get(pLocal->handle());
	if (!localent)
		return 0.f;

	auto entity = sdk::m_game->pGameEntitySystem->Get(best_record->m_pawn->handle());
	if (!entity)
		return 0.f;

	float damage = 0.f;
	bool canHit = false;

	// initialize autowall data
	autowall::c_auto_wall::data_t data;

	// Store the new fixed position
	auto pos = best_record->m_bone_data[id].pos;

	// Run the trace/penetration with the updated position
	autowall::g_auto_wall->pen(data, start, pos, entity, localent, pLocal, best_record->m_pawn, vdata, damage, canHit);

	// return autowall damage if found a valid one
	return data.m_can_hit ? data.m_dmg : 0.f;
}
void ragebot::c_data::get_best_data(CBaseUserCmdPB* crc, CCSPlayerPawn* player, anim_record_t* anim_record, c_aim_data& aim_data)
{
	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return;

	if (!g::m_local_player_controller || !g::m_local_player_pawn || !g::m_active_weapon || !g::m_weapon_data)
		return;

	if (!anim_record)
		return;

	if (!anim_record->m_pawn)
		return;

	if (!anim_record->m_pawn->IsAlive() || !anim_record->is_valid())
		return;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return;

	float m_best_dmg = 0.f;
	float m_dmg = 0.f;
	float m_dist = 0.f;
	bool m_hit, m_out_of_sim_time = false;
	ragebot::c_aim_point* m_best_point = nullptr;

	std::vector<ragebot::c_aim_point> m_points;

	for (auto& i : m_rage_aim_hitboxes)
	{
		// store hitbox vars
		Vector_t m_pos = {}; QAngle_t m_ang = {}; QAngle_t m_point_ang = {}; Vector4D_t rot;

		// get record data
		const auto& m_data = anim_record->m_bone_data[i];
		m_pos = m_data.pos;

		// create our ange based on our current point
		math::VectorAngless(local_data->m_eye_pos - m_pos, m_point_ang);

		// shit got in vec convert to a quarention angle
		auto vec = m_angle_to_vector(g::m_local_player_pawn, m_pos);
		m_ang.ToVec(vec);

		// scale penetration damage
		m_dmg = data->get_damage(anim_record, player, g::m_local_player_pawn, local_data->m_eye_pos, g::m_active_weapon, i);		// store this point data as ( position, angle, hitbox id, damage etc )

		m_points.emplace_back(c_aim_point(m_pos, m_point_ang, m_ang, i, m_dmg, m_hit, m_dist));
	}

	if (m_points.empty())
		return;

	int scale_damage = 10; // should be based on weapon
	bool is_taser = g::m_weapon_data->m_WeaponType() == WEAPONTYPE_TASER;
	auto override_damage = Input::GetBindState(cfg_get(KeyBind_t, g_cfg.min_dmg_key));

	int min_damage = std::min(adaptive.minimum_damage, anim_record->m_pawn->m_iHealth());

	if (override_damage)
		min_damage = std::min(adaptive.minimum_damage_override, anim_record->m_pawn->m_iHealth());

	if (is_taser)
		min_damage = anim_record->m_pawn->m_iHealth();

	// not needed due to prior code
	if (min_damage > anim_record->m_pawn->m_iHealth() + scale_damage)
		min_damage = anim_record->m_pawn->m_iHealth() + scale_damage;

	for (auto& point : m_points)
	{
		if (point.dmg < 1.f)
			continue;

		if (point.dmg > m_best_dmg) {
			m_best_point = &point;
			m_best_dmg = point.dmg;
		}

		continue;
	}


	// should be auto scaled
	float max_accurate_vel = g::m_active_weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE ? 130 : 100;
	const float speed{ (g_prediction->get_local_data()->m_velocity.Length2D()) };
	const float max_weapon_speed{ g::m_local_player_pawn->m_bIsScoped() ? g::m_weapon_data->m_flMaxSpeed().flValue[1]
														: g::m_weapon_data->m_flMaxSpeed().flValue[0] };
	const float max_accurate_speed{ 0.34f * max_weapon_speed };

	if (m_best_point) {
		// predictive stop
		auto fov = std::hypotf(g::angle.x - m_best_point->pos.x, g::angle.y - m_best_point->pos.y);

		// check safe mode fov?
		bool is_safe = m_status == e_status::SAFE_MODE ? fov <= cfg_get(int, g_cfg.rage_fov) : true;

		bool should_predict = m_status == e_status::SAFE_MODE ? rage_type == SPREAD && fov <= cfg_get(int, g_cfg.rage_fov) : rage_type == SPREAD;
		if (should_predict && adaptive.auto_stop && can_shoot(g::m_local_player_controller->m_nTickBase())) {
			if (m_best_point->dmg >= 1) {
				auto cmd = CUserCmd::Get();
				if (cmd && crc && can_shoot(g_prediction->get_local_data()->m_shoot_tick)) {
					if (g_prediction->get_local_data()->m_velocity.Length2D() >= max_accurate_speed) {

						if (g::m_active_weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE) {
							bool should_hight_acc = false;
							if (min_damage > 55)
								should_hight_acc = m_best_point->dmg >= (min_damage);
							else {
								auto new_dmg = (min_damage) / 2;
								should_hight_acc = new_dmg > 0 && m_best_point->dmg >= (min_damage) / 2;
							}

							if (should_hight_acc && g::m_local_player_pawn->m_bIsScoped())
							{
								stop(cmd, crc, PREDICTIVE);
							}
						}
						else
						{
							stop(cmd, crc, SLOW);
							stop(cmd, crc, ACCURATE);
							stop(cmd, crc, QUICK);
						}
					}
				}
			}
		}

		if (m_best_point->dmg >= min_damage && is_safe) {
			aim_data.record = anim_record;
			aim_data.m_point = *m_best_point;
			aim_data.m_hitbox.m_idx = m_best_point->idx;
			aim_data.m_hitbox.m_damage = (int)m_best_point->dmg;
			if (adaptive.rage_silent_mode == 0 && rage_type == SPREAD) {
				QAngle_t aim_punch{ };
				hooks::m_get_aim_punch(g::m_local_player_pawn, &aim_punch, 0.f, true);
				command::m_best_point = m_best_point->angle - aim_punch;
			}
		}
	}
}
*/
void ragebot::c_events::on_event(IGameEvent* ev, e_game_events type)
{
	if (!cfg_get(bool, g_cfg.rage_enable))
		return;

	switch (type) {
	case death: {
		auto controller = g::m_local_player_controller;
		if (!controller)
			break;

		const auto event_controller = ev->get_player_controller(xorstr_("attacker"));
		if (!event_controller)
			return;

		const auto user_id = ev->get_player_controller(xorstr_("userid"));
		if (user_id == nullptr)
			return;

		auto user_id_idx = user_id->m_pEntityIdentity()->get_index();
		auto controller_idx = controller->m_pEntityIdentity()->get_index();
		auto event_controller_idx = event_controller->m_pEntityIdentity()->get_index();

		if (event_controller->m_pEntityIdentity()->get_index() == controller->m_pEntityIdentity()->get_index()) {
			// we killed somone
		}
		else {
			m_rage_last_target = nullptr;
			// we died quit
			data->init();
		}
	}
			  break;
	case round_start: {
		m_rage_last_target = nullptr;
		data->init();
	}
					break;
	}
}

const char* get_weapon_name_as_string(CCSWeaponBase* weapon)
{
	if (!weapon)
		return "";

	auto weapon_data = g::m_weapon_data;
	if (!weapon_data)
		return "";

	const char* szWeaponName = weapon_data->m_szName();
	const char* weaponPrefix = ("weapon_");
	const char* weaponNameStart = strstr(szWeaponName, weaponPrefix);
	const char* extractedWeaponName = weaponNameStart ? weaponNameStart + strlen(weaponPrefix) : szWeaponName;

	return extractedWeaponName;
}

void ragebot::c_data::menu_adaptive() {

	if (!g::m_local_player_pawn)
		return;

	auto pLocal = g::m_local_player_pawn;
	if (!pLocal)
		return;

	if (pLocal->m_iHealth() <= 0)
		return;

	auto ActiveWeapon = g::m_active_weapon;
	if (!ActiveWeapon)
		return;

	auto data = g::m_weapon_data;
	if (!data)
		return;

	if (!ActiveWeapon->IsWeapon())
		return;

	const char* extractedWeaponName = get_weapon_name_as_string(ActiveWeapon);
	bool has_awp = strcmp(extractedWeaponName, xorstr_("awp")) == 0;
	bool hasmath_PIstols = strcmp(extractedWeaponName, xorstr_("hkp2000")) == 0 || strcmp(extractedWeaponName, xorstr_("usp")) == 0 || strcmp(extractedWeaponName, xorstr_("glock")) == 0;
	bool has_heavymath_PIstols = false;
	if (strcmp(extractedWeaponName, xorstr_("revolver")) == 0 || strcmp(extractedWeaponName, xorstr_("deagle")) == 0)
		has_heavymath_PIstols = true;
	else
		has_heavymath_PIstols = false;

	bool has_scout = strcmp(extractedWeaponName, xorstr_("ssg08")) == 0;
	adaptive.rage_enable = cfg_get(bool, g_cfg.rage_enable);
	adaptive.rage_silent_mode = cfg_get(int, g_cfg.rage_silent_mode);
	adaptive.remove_weapon_accuracy_spread = cfg_get(bool, g_cfg.remove_weapon_accuracy_spread);

	if (has_awp)
	{
		adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, 6);

		adaptive.minimum_damage = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, 6);
		adaptive.rage_hitchance = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, 6);
		adaptive.auto_stop = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, 6);
		adaptive.auto_scope = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_scope, 6);
		adaptive.minimum_damage_override = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage_override, 6);
		adaptive.penetration = cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, 6);
		adaptive.rage_target_select = cfg_get_adaptive(int, 7, g_cfg.rage_target_select, 6);
		adaptive.point_scale = cfg_get_adaptive(int, 7, g_cfg.point_scale, 6);
		adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.head_scale, 6);

		adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, 6);
		adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, 6);
		adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, 6);
		adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, 6);
		adaptive.hitbox_feets = cfg_get_adaptive(bool, 7, g_cfg.hitbox_feet, 6);
		adaptive.rapid_fire = cfg_get_adaptive(bool, 7, g_cfg.rapid_fire, 6);
		return;
	}
	else if (has_scout)
	{
		adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, 5);
		adaptive.minimum_damage_override = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage_override, 5);
		adaptive.point_scale = cfg_get_adaptive(int, 7, g_cfg.point_scale, 5);
		adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.head_scale, 5);

		adaptive.auto_scope = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_scope, 5);

		adaptive.minimum_damage = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, 5);
		adaptive.rage_hitchance = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, 5);
		adaptive.auto_stop = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, 5);
		adaptive.penetration = cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, 5);
		adaptive.rage_target_select = cfg_get_adaptive(int, 7, g_cfg.rage_target_select, 5);
		adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, 5);
		adaptive.hitbox_neck = cfg_get_adaptive(bool, 7, g_cfg.hitbox_neck, 5);
		adaptive.hitbox_uppeer_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_uppeer_chest, 5);
		adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, 5);
		adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, 5);
		adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, 5);
		adaptive.hitbox_feets = cfg_get_adaptive(bool, 7, g_cfg.hitbox_feet, 5);
		adaptive.rapid_fire = cfg_get_adaptive(bool, 7, g_cfg.rapid_fire, 5);

		return;
	}
	else if (data->m_WeaponType() == WEAPONTYPE_PISTOL && !has_heavymath_PIstols)
	{
		adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, 1);
		adaptive.minimum_damage_override = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage_override, 1);
		adaptive.point_scale = cfg_get_adaptive(int, 7, g_cfg.point_scale, 1);
		adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.head_scale, 1);

		adaptive.minimum_damage = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, 1);
		adaptive.rage_hitchance = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, 1);
		adaptive.auto_stop = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, 1);
		adaptive.penetration = cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, 1);
		adaptive.rage_target_select = cfg_get_adaptive(int, 7, g_cfg.rage_target_select, 1);
		adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, 1);
		adaptive.hitbox_neck = cfg_get_adaptive(bool, 7, g_cfg.hitbox_neck, 1);
		adaptive.hitbox_uppeer_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_uppeer_chest, 1);
		adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, 1);
		adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, 1);
		adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, 1);
		adaptive.hitbox_feets = cfg_get_adaptive(bool, 7, g_cfg.hitbox_feet, 1);
		adaptive.rapid_fire = cfg_get_adaptive(bool, 7, g_cfg.rapid_fire, 1);

		return;
	}
	else if (has_heavymath_PIstols)
	{
		adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, 2);
		adaptive.minimum_damage_override = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage_override, 2);
		adaptive.point_scale = cfg_get_adaptive(int, 7, g_cfg.point_scale, 2);
		adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.head_scale, 2);

		adaptive.minimum_damage = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, 2);
		adaptive.rage_hitchance = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, 2);
		adaptive.auto_stop = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, 2);
		adaptive.penetration = cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, 2);
		adaptive.rage_target_select = cfg_get_adaptive(int, 7, g_cfg.rage_target_select, 2);
		adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, 2);
		adaptive.hitbox_neck = cfg_get_adaptive(bool, 7, g_cfg.hitbox_neck, 2);
		adaptive.hitbox_uppeer_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_uppeer_chest, 2);
		adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, 2);
		adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, 2);
		adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, 2);
		adaptive.hitbox_feets = cfg_get_adaptive(bool, 7, g_cfg.hitbox_feet, 2);

		adaptive.rapid_fire = cfg_get_adaptive(bool, 7, g_cfg.rapid_fire, 2);
		return;
	}

	else if (data->m_WeaponType() == WEAPONTYPE_MACHINEGUN)
	{
		adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, 3);
		adaptive.minimum_damage_override = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage_override, 3);
		adaptive.point_scale = cfg_get_adaptive(int, 7, g_cfg.point_scale, 3);
		adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.head_scale, 3);

		adaptive.minimum_damage = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, 3);
		adaptive.rage_hitchance = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, 3);
		adaptive.auto_stop = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, 3);
		adaptive.penetration = cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, 3);
		adaptive.rage_target_select = cfg_get_adaptive(int, 7, g_cfg.rage_target_select, 3);
		adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, 3);
		adaptive.hitbox_neck = cfg_get_adaptive(bool, 7, g_cfg.hitbox_neck, 3);
		adaptive.hitbox_uppeer_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_uppeer_chest, 3);
		adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, 3);
		adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, 3);
		adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, 3);
		adaptive.hitbox_feets = cfg_get_adaptive(bool, 7, g_cfg.hitbox_feet, 3);
		adaptive.rapid_fire = cfg_get_adaptive(bool, 7, g_cfg.rapid_fire, 3);
		return;
	}
	else if (data->m_WeaponType() == WEAPONTYPE_RIFLE)
	{
		adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, 3);
		adaptive.minimum_damage_override = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage_override, 3);
		adaptive.point_scale = cfg_get_adaptive(int, 7, g_cfg.point_scale, 3);
		adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.head_scale, 3);

		adaptive.minimum_damage = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, 3);
		adaptive.rage_hitchance = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, 3);
		adaptive.auto_stop = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, 3);
		adaptive.penetration = cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, 3);
		adaptive.rage_target_select = cfg_get_adaptive(int, 7, g_cfg.rage_target_select, 3);
		adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, 3);
		adaptive.hitbox_neck = cfg_get_adaptive(bool, 7, g_cfg.hitbox_neck, 3);
		adaptive.hitbox_uppeer_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_uppeer_chest, 3);
		adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, 3);
		adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, 3);
		adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, 3);
		adaptive.hitbox_feets = cfg_get_adaptive(bool, 7, g_cfg.hitbox_feet, 3);
		adaptive.rapid_fire = cfg_get_adaptive(bool, 7, g_cfg.rapid_fire, 3);
		return;
	}
	else if (data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE)
	{
		adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, 4);
		adaptive.minimum_damage_override = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage_override, 4);
		adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.head_scale, 4);

		adaptive.auto_scope = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_scope, 4);
		adaptive.point_scale = cfg_get_adaptive(int, 7, g_cfg.point_scale, 4);

		adaptive.minimum_damage = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, 4);
		adaptive.rage_hitchance = cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, 4);
		adaptive.auto_stop = cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, 4);
		adaptive.penetration = cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, 4);
		adaptive.rage_target_select = cfg_get_adaptive(int, 7, g_cfg.rage_target_select, 4);
		adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, 4);
		adaptive.hitbox_neck = cfg_get_adaptive(bool, 7, g_cfg.hitbox_neck, 4);
		adaptive.hitbox_uppeer_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_uppeer_chest, 4);
		adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, 4);
		adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, 4);
		adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, 4);
		adaptive.hitbox_feets = cfg_get_adaptive(bool, 7, g_cfg.hitbox_feet, 4);
		adaptive.rapid_fire = cfg_get_adaptive(bool, 7, g_cfg.rapid_fire, 4);
		return;
	}
}