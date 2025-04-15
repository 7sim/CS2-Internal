#include "legitbot.h"
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
#include "..\ragebot\ragebot.h"
#include "..\..\utils\hook\hooks.h"
#include "..\movement\setup.h"
#include "..\prediction\prediction.h"
CCSPlayerPawn* m_last_target;
legitbot::c_best_target m_best_target;
std::vector<legitbot::c_best_target> m_aim_targets;
std::vector<legitbot::c_target> m_targets;
static std::vector<std::uint32_t> m_aim_hitboxes;
#define MAX_STUDIO_BONES 1024
int kill_delay = 0;
KeyBind_t legit_key = KeyBind_t{ "", 0, 0 };

//animsync client.dll -> E8 ? ? ? ? 84 C0 75 ? 4C 8D 44 24 abs 0x1, 0x0

static Vector_t m_angle_to_vector(CCSPlayerPawn* localplayer, Vector_t position)
{
	Vector_t eye_position = localplayer->GetEyePosition();
	Vector_t angle = position;

	angle.x = position.x - eye_position.x;
	angle.y = position.y - eye_position.y;
	angle.z = position.z - eye_position.z;

	angle.Normalizes();
	math::vec_angles(angle, &angle);

	angle.clamp();
	return angle;
}
// setup config hitboxes
void legitbot::c_data::menu_hitboxes()
{
	/* emplace menu hitboxes which will be used for hitscan*/
	if (adaptive.hitboxes & head)
	{
		m_aim_hitboxes.emplace_back(HITBOX_HEAD);
	}

	if (adaptive.hitboxes & chest)
	{
		m_aim_hitboxes.emplace_back(HITBOX_CHEST);
		m_aim_hitboxes.emplace_back(HITBOX_UPPER_CHEST);
		m_aim_hitboxes.emplace_back(HITBOX_THORAX);
	}

	if (adaptive.hitboxes & stomach)
	{
		m_aim_hitboxes.emplace_back(HITBOX_STOMACH);
		m_aim_hitboxes.emplace_back(HITBOX_PELVIS);
	}

	if (adaptive.hitboxes & legs)
	{
		m_aim_hitboxes.emplace_back(HITBOX_LEFT_THIGH);
		m_aim_hitboxes.emplace_back(HITBOX_RIGHT_THIGH);
		m_aim_hitboxes.emplace_back(HITBOX_LEFT_CALF);
		m_aim_hitboxes.emplace_back(HITBOX_RIGHT_CALF);

	}
	if (adaptive.hitboxes & feet)
	{
		m_aim_hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		m_aim_hitboxes.emplace_back(HITBOX_LEFT_FOOT);
	}
}

void legitbot::c_data::init()
{
	command::m_aim_target = nullptr;

	if (!m_targets.empty())
		m_targets.clear();

	if (!m_aim_targets.empty())
		m_aim_targets.clear();

	if (!m_aim_hitboxes.empty())
		m_aim_hitboxes.clear();

	m_best_target.clear();
}

void clear_data() {
	command::m_aim_target = nullptr;
	m_best_target.clear();

	if (!m_aim_targets.empty())
		m_aim_targets.clear();

	if (!m_targets.empty())
		m_targets.clear();
}

inline QAngle_t clampAngles(QAngle_t angles) {
	if (angles.x > 89.0f && angles.x <= 180.0f) {
		angles.x = 89.0f;
	}

	if (angles.x > 180.0f) {
		angles.x -= 360.0f;
	}

	if (angles.x < -89.0f) {
		angles.x = -89.0f;
	}

	if (angles.y > 180.0f) {
		angles.y -= 360.0f;
	}

	if (angles.y < -180.0f) {
		angles.y += 360.0f;
	}
	angles.z = 0;

	return angles;
};
std::unordered_map<int, std::deque<legitbot::anim_record_t>> m_anim_records{};

void legitbot::c_data::store_records() {
	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return;

	auto controller = CCSPlayerController::Get();
	if (!controller)
		return;

	auto pawn = hooks::m_get_local_player(0);
	if (!pawn)
		return;
	int max_backtrack_ticks = 12;

	auto max_ticks = std::clamp(max_backtrack_ticks, 5, 12);

	for (int i = 1; i < sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); ++i) {
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

			if (!player_pawn->enemy(pawn)) {
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

			// is it added yet?
			if (m_anim_records.find(handle) == m_anim_records.end()) {
				m_anim_records.insert_or_assign(handle, std::deque<legitbot::anim_record_t>{});
				continue;
			}

			// clamp records etc 
			auto& records = m_anim_records[handle];

			if (records.size() != max_ticks) {
				records.clear();
				records.resize(max_ticks);
			}

			auto& record = records.emplace_front(player_pawn);


	
			//record.store(player_pawn);

		}
	}
}
enum record_type_t : int {
	 best = 0,
	 front = 1
};
legitbot::anim_record_t* legitbot::c_data::select_record(int handle) {
	if (!g::m_local_player_pawn->IsAlive())
		return nullptr;

	auto& records = m_anim_records[handle];

	if (records.empty())
		return nullptr;

	legitbot::anim_record_t* best_record = nullptr;
	legitbot::anim_record_t* last_record = &records.front();

	if (records.size() == 1)
		return last_record;

	int history_type = cfg_get(int, g_cfg.legit_history);

	// iterate records in reverse.
	for (auto it = records.rbegin(); it != records.rend(); ++it) {
		legitbot::anim_record_t* record = &(*it);

		if (!record)
			continue;

		if (!record->m_pawn)
			continue;

		if (!record->m_pawn->immune()) {
			best_record = record;
		}
	}

	if (!best_record)
		return nullptr;

	return best_record;
}

QAngle_t m_prev_punch = QAngle_t();
bool legitbot::c_data::correct_punch(QAngle_t& correct_punch) {
	if (!g::m_local_player_pawn)
		return false;

	if (g::m_local_player_pawn->m_shots() < 2)
		return false;

	m_prev_punch = QAngle_t(0.f, 0.f, 0.f);
	auto& m_cached_punch = g::m_local_player_pawn->m_aimPunchCache();

	auto current_punch = m_cached_punch.m_Size > 0 ? m_cached_punch.m_Data[m_cached_punch.m_Size - 1] : QAngle_t(0, 0, 0);


	correct_punch = m_prev_punch - current_punch * 2.f;
	if (m_cached_punch.m_Size > 0 && m_cached_punch.m_Size <= 0xFFFF) {

		current_punch = m_cached_punch.m_Data[m_cached_punch.m_Size - 1];
		m_prev_punch = current_punch;
	}
	else
		m_prev_punch = QAngle_t();

	return true;
}
void legitbot::c_data::render_multi_points()
{
	if (!cfg_get(bool, g_cfg.render_multipoints))
		return;


	if (!m_best_target.m_player_pawn)
		return;

	if (!m_best_target.m_player_pawn->IsAlive())
		return;


	if (!m_best_target.m_lag_entity || !m_best_target.m_data.record )
		return;

	if (m_best_target.m_data.render_points.empty())
		return;

	for (const auto& point : m_best_target.m_data.render_points) {
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
bool legitbot::c_data::allowed() {

	if (!sdk::m_engine->is_valid())
		return false;

	if (!g::m_local_player_pawn->valid_move())
		return false;

	if (!g::m_active_weapon)
		return false;

	if (g::m_active_weapon->clip1() <= 0
		|| g::m_weapon_data->m_WeaponType() == WEAPONTYPE_GRENADE)
		return false;

	if (adaptive.disable_conditions & cond_smoke && g::m_local_player_pawn->locking_smoke())
		return false;

	if (adaptive.disable_conditions & cond_flashed && g::m_local_player_pawn->m_flFlashDuration() > 0.f) 
		return false;

	// target switch delay
	if (kill_delay >= (int)g::m_local_player_controller->m_nTickBase())
		return false;

	return true;
}




__forceinline Vector_t calc_trigger_spread(CCSWeaponBase* weapon, int seed, float inaccuracy, float spread, bool revolver2 = false) {
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
bool legitbot::c_data::hitchance(CBaseEntity* entity, QAngle_t vAimpoint)
{
	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return false;

	if (!g::m_local_player_controller)
		return false;

	if (!g::m_active_weapon || !g::m_weapon_data)
		return false;

	auto weapon = g::m_active_weapon;
	if (!weapon)
		return false;

	auto data = g::m_weapon_data;
	if (!data)
		return false;

	float HITCHANCE_MAX = 100.f;
	constexpr int SEED_MAX = 255;

	Vector_t start{ g::eye_pos }, end, fwd, right, up, dir, wep_spread;
	float inaccuracy, spread = {};
	float hitchance = adaptive.trigger_hitchance;
	Vector_t point = Vector_t(vAimpoint.x, vAimpoint.y, vAimpoint.z);

	if (hitchance <= 0)
		return true;

	// we cant reach it anymore too far
	if (g::eye_pos.DistTo(point) > data->range())
		return false;

	size_t total_hits{}, needed_hits{ (size_t)std::ceil((hitchance * SEED_MAX) / HITCHANCE_MAX) };

	// get needed directional vectors.
	math::anglevectors(vAimpoint, &fwd, &right, &up);

	// store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
	spread = weapon->get_spread();
	inaccuracy = weapon->get_inaccuracy();
	
	// iterate all possible seeds.
	for (int i{}; i <= SEED_MAX; ++i)
	{
		// get spread.
		wep_spread = calc_trigger_spread(weapon, i, inaccuracy, spread);
		//	hooks::m_get_spread_weapon_calculation(weapon, data, weapon->m_weaponMode(), i, inaccuracy, spread, weapon->m_flRecoilIndex(), &wep_spread.x, &wep_spread.y);

			// get spread direction.
		dir = (fwd + (right * wep_spread.x) + (up * wep_spread.y)).normalized();

		// get end of trace.
		end = start - (dir * data->range());

		trace_filter_t filter = {};
		sdk::m_trace->Init(filter, g::m_local_player_pawn, MASK_SHOT, 4, 7);
		game_trace_t trace = {};
		ray_t ray = {};

		sdk::m_trace->TraceShape(ray, &start, &end, filter, trace);
		sdk::m_trace->ClipTraceToPlayers(start, end, &filter, &trace, 0.F, 60.F, (1.F / (start - end).Length()) * (trace.m_end_pos - start).Length());

		if (trace.HitEntity ) {
		

				if (trace.HitEntity->handle().index() == entity->handle().index())
					++total_hits;
				

			
		}

		// we made it.
		if (total_hits >= needed_hits)
			return true;


		// we cant make it anymore.
		if ((SEED_MAX - i + total_hits) < needed_hits)
			return false;
	}
	return false;

}struct c_lag_compensation_data_t {
	std::uint64_t m_tick{};
	float         m_time{};
	pad(0x24);
}; // size: 0x40




float calculate_interpolated_delta_time(int newest_tick) {

	int tick_count = sdk::m_global_vars->m_tickcount;
	float max_unlag = 0.2f;

	float unk_modifier = std::fmod(max_unlag, 0.015625);
	float unlag_diff = max_unlag - unk_modifier;
	float time_modifier = unk_modifier * 64.0;
	int unlag_ticks = (int)(float)((float)(unlag_diff * 64.0) + 0.5);

	c_lag_compensation_data_t v28{};
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

	c_lag_compensation_data_t v40{};
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
	c_lag_compensation_data_t v38 = v40;

	c_lag_compensation_data_t v39{};

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

bool on_interpolation(CCSGOInput* input, CCSPlayerPawn* player)
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

	auto history = input->Message;
	if (!history)
		return false;

	/* proccess interp info */
	struct csgo_interpolation_info_t {
		int src_tick = -1;
		int dst_tick = -1;
		float fraction = 0.f;
	};

	int player_tick_count = history->m_nPlayerTickCount;
	float player_fraction = history->m_flPlayerTickFraction;

	if (player_fraction >= float(1.0 - 0.0099999998))
		player_tick_count += 1;

	float time = player_tick_count;
	time += player_fraction;

	/* make sure we grab correct interp info from game */
	csgo_interpolation_info_t cl_interpolated_data, sv0_interpolated_data, sv1_interpolated_data;
	/*hooks::m_get_lerp_tick(g::m_local_player_pawn->m_pGameSceneNode(), &cl_interpolated_data,
		&sv0_interpolated_data,
		&sv1_interpolated_data,
		&player_tick_count);*/

	float max_unlag = 0.3f;
	const double l1 = (utils::CallVFunc<double, 10U>(net_channel2));
	const double l2 = (utils::CallVFunc<double, 11U>(net_channel2));

	auto network_latency = l1 * 0.5f;
	auto engine_latency = l2;
	// dead time check..

	float server_time = (player->m_flSimulationTime() * 64.0) + network_latency + engine_latency;

	float delta_time = calculate_interpolated_delta_time(server_time);

	auto cur_delta_time = std::abs(delta_time);

	if (cur_delta_time > max_unlag) {
		_log(LOG_ERROR) << ("Invalid dead tick:") << max_unlag;
		return false;
	}

	return true;
}

bool attack(CUserCmd* user_cmd, CCSGOInput* input, CBaseUserCmdPB* crc) {
	if (!user_cmd || !input || !crc)
		return false;

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

			_log(LOG_INFO) << "[shot] attack command input sent";
		}
	}
}

bool legitbot::c_data::trigger_bot(CUserCmd* cmd, CCSGOInput* input, CBaseUserCmdPB* base) {
	if (!adaptive.trigger)
	{
		g::trigger_enable = false;
		return false;
	}

	if (!sdk::m_engine->is_valid())
		return false;

	if (!g::m_local_player_pawn || !g::m_local_player_controller)
		return false;

	if (!g::m_local_player_pawn->IsAlive() || !g::m_local_player_pawn->GetActiveWeapon())
		return false;

	if (!g::m_active_weapon || !g::m_weapon_data)
		return false;

	if (!cmd)
		return false;

	if (!cmd->pBaseCmd)
		return false;

	if (!cmd->pBaseCmd->view_angles)
		return false;

	if (g::m_active_weapon->clip1() <= 0 || g::m_weapon_data->m_WeaponType() == WEAPONTYPE_GRENADE)
		return false;

	if (g::m_local_player_pawn->m_flFlashDuration() > 0.f )
		return false;

	auto local_ent = sdk::m_game->pGameEntitySystem->Get(g::m_local_player_pawn->handle());
	if (!local_ent)
		return false;

	auto on_tg_key = Input::GetBindState(cfg_get(KeyBind_t, g_cfg.legit_trigger_bot_key));
	bool on_trigger = adaptive.trigger_on_key ? on_tg_key : true;
	// calculate aimpunch & compensate it
	QAngle_t aim_punch{ };
	hooks::m_get_aim_punch(g::m_local_player_pawn, &aim_punch, 0.f, true);

	auto m_angle = base->view_angles->angValue - aim_punch;
	bool fire = g::m_active_weapon->can_primary_attack(g::m_weapon_data->m_WeaponType(), g::m_local_player_controller->m_nTickBase());

	if (on_trigger && g::on_ground) {

		// sim time check


		trace_filter_t filter = {};
		sdk::m_trace->Init(filter, g::m_local_player_pawn, 0x1C300B, 4, 7);

		game_trace_t trace = {};
		ray_t ray = {};
		Vector_t vecForward = { };
		m_angle.ToDirections(&vecForward);
		vecForward *= g::m_weapon_data->range();
		Vector_t src = g::eye_pos;
		Vector_t dst = src + vecForward;
		sdk::m_trace->TraceShape(ray, &src, &dst, filter, trace);

		if (!trace.HitEntity || trace.HitEntity->handle().index() == 0)
			return false;

		if (!trace.HitEntity->IsBasePlayer())
			return false;

		if (trace.HitEntity->m_iHealth() <= 0)
			return false;

		auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(trace.HitEntity->handle());
		if (!player)
			return false;

		if (!player->enemy(g::m_local_player_pawn) || player->m_iHealth() <= 0)
			return false;

		const auto scene_node = player->m_pGameSceneNode();
		if (!scene_node)
			return false;

		const auto skel = scene_node->m_pSkeleton();
		if (!skel)
			return false;

		auto& mdl_state = skel->GetModel();
		auto mdl = mdl_state.m_hModel();
		if (!scene_node)
			return false;

		auto  num_hitboxes = mdl->hitboxes_num();
		if (num_hitboxes == 0)
			return false;

		auto hitbox_data = mdl_state.hitbox_data();
		if (!hitbox_data)
			return false;

		if (num_hitboxes > MAX_STUDIO_BONES)
			num_hitboxes = MAX_STUDIO_BONES;

		skel->setup_bones(FLAG_ALL_BONE_FLAGS);

		// create our ange based on our current point
		const auto& m_data = hitbox_data[stomach];
		QAngle_t target_angle;
		math::VectorAngless(g::eye_pos - m_data.pos, target_angle);
		// trace dmg
		float damage = 0.f;
		bool canHit = false;
		bool valid = true;

		// initialize autowall data
		autowall::c_auto_wall::data_t data;

		// do trace / penetration with updated pos
		autowall::g_auto_wall->pen(data, src, dst, trace.HitEntity, local_ent, g::m_local_player_pawn, player, g::m_weapon_data, damage, canHit);
		float dmg = data.m_can_hit ? data.m_dmg : 0.f;

		int hitgroup = trace.HitboxData->m_hitgroup;
		if (hitgroup == HITGROUP_HEAD && !(adaptive.trigger_hitbox & head))
			valid = false;
		else if (hitgroup == HITGROUP_CHEST && !(adaptive.trigger_hitbox & chest))
			valid = false;
		else if (hitgroup == HITGROUP_STOMACH && !(adaptive.trigger_hitbox & stomach))
			valid = false;
		// limbs
		else if (hitgroup > HITGROUP_STOMACH && !(adaptive.trigger_hitbox & (legs | feet)))
			valid = false;

		if (g::m_weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE && !g::m_local_player_pawn->m_bIsScoped())
			valid = false;

		auto view_angles = cmd->pBaseCmd->view_angles->angValue;

		auto end_pos = g::eye_pos + math::angle_vector(view_angles) * g::m_weapon_data->range();

		bool hitchance_ret = adaptive.trigger_hitchance <= 0 ? true : hitchance(trace.HitEntity, target_angle);
		if (!hitchance_ret) {
			if (adaptive.trigger_magnet) {
				if (g::m_weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE)
					misc::movement->quick_stop(base);

				misc::movement->change_speed(base, -1.f);
				misc::movement->quick_stop(base);
				misc::movement->slow_walk(base);
			}
		}


		if (fire && dmg >= adaptive.trigger_dmg && valid && hitchance_ret && on_interpolation(input, player))
		{
			// manually input lag comp data
			if (!m_best_target.m_player_pawn || !m_best_target.m_data.record) {
				command::m_tick_count = player->m_flSimulationTime() * 64.f;
				command::m_target_ent_index = player->handle().index();
			}

			return true;
		}
	}

	return false;
}
float legit_bot_fov = 0.f;
	

QAngle_t get_aim_punch() {
	QAngle_t aim_punch{};
	if (g::m_local_player_pawn->m_aimPunchCache().m_Size <= 0)
		return QAngle_t();

	return	g::m_local_player_pawn->m_aimPunchCache().m_Data[g::m_local_player_pawn->m_aimPunchCache().m_Size - 1];
}

QAngle_t get_prev_aim_punch() {
	QAngle_t aim_punch{};
	if (g::m_local_player_pawn->m_aimPunchCache().m_Size < 1)
		return QAngle_t();

	return	g::m_local_player_pawn->m_aimPunchCache().m_Data[g::m_local_player_pawn->m_aimPunchCache().m_Size - 2];
}

void normalize_angles(QAngle_t& angle)
{
    while (angle.x > 180.0f) angle.x -= 360.0f;
    while (angle.x < -180.0f) angle.x += 360.0f;

    while (angle.y > 180.0f) angle.y -= 360.0f;
    while (angle.y < -180.0f) angle.y += 360.0f;

    // Z is usually not needed for FOV calculations, but include if necessary
    while (angle.z > 180.0f) angle.z -= 360.0f;
    while (angle.z < -180.0f) angle.z += 360.0f;
}

float get_fov(const QAngle_t& view_angle, const QAngle_t& aim_angle)
{
	QAngle_t delta = aim_angle - view_angle;

	// Normalize the delta to ensure angles are between -180 and 180
	normalize_angles(delta);

	// Calculate the FOV as the 2D distance on the pitch (x) and yaw (y) axes
	float fov = sqrtf(delta.x * delta.x + delta.y * delta.y);

	// Clamp the FOV to a maximum of 180
	return std::min(fov, 180.0f);
}

Vector_t aim_direction_fov(Vector_t src, Vector_t dst)
{
	Vector_t result;

	Vector_t delta = dst - src;

	math::vector_angles(delta, result);

	return result;
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


float calculate_delta_times(int newest_tick) {

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
bool legitbot::anim_record_t::is_valid()
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

	float server_time = (m_simulation_time * 64.0) + network_latency + engine_latency;

	float delta_time = calculate_delta_times(server_time);

	auto cur_delta_time = std::abs(delta_time);

	if (cur_delta_time > max_unlag)
		return true;

	auto dead_tick = (g::m_local_player_controller->m_nTickBase()  *g::interval) - this->m_simulation_time;
	return true;
}
bool override_fov_color = false;
void legitbot::c_data::proccess(CUserCmd* cmd, CCSGOInput* input, CBaseUserCmdPB* base)
{
	// update adaptive settings 
	legit_bot_fov = (float)adaptive.fov;
	command::m_legit_bot = false;
	override_fov_color = false;
	bool on_trigger = adaptive.trigger_on_key ? Input::GetBindState(cfg_get(KeyBind_t, g_cfg.legit_trigger_bot_key)) : true;
	g::trigger_enable = (adaptive.trigger && on_trigger) ? true : false;
	g::trigger_delay = g::trigger_enable ? adaptive.trigger_delay : 0;
	command::m_legit_bot = adaptive.legit_enable;
	g::shoot_pos = g::eye_pos;

	if (!adaptive.legit_enable || m_anim_records.empty()) {
		clear_data();
		return;
	}


	command::m_legit_bot = true;

	if (!sdk::m_engine->is_valid()) {
		clear_data();
		return;
	}

	if (!g::m_local_player_pawn || !g::m_local_player_controller) {
		clear_data();
		return;
	}

	if (!g::m_local_player_pawn->IsAlive() || !g::m_local_player_pawn->GetActiveWeapon()) {
		clear_data();
		return;
	}

	if (!g::m_active_weapon || !g::m_weapon_data) {
		clear_data();
		return;
	}

	if (!cmd) {
		clear_data();
		return;
	}
	if (!base) {
		clear_data();
		return;
	}

	if (m_last_target != nullptr) {
			m_last_target = nullptr;
	}


	// init our routine & reset all necessary data 
	data->init();
	data->menu_adaptive();

	// standalone rcs
	{
		auto pressing_legit_aim_key = Input::GetBindState(legit_key);
		bool should_rcs_standlone = false;
		if (pressing_legit_aim_key)
			// if normal rcs enabled no point of standlone rcs 
			should_rcs_standlone = !adaptive.rcs_control;
		else
			should_rcs_standlone = true;

		if (adaptive.recoil_adjust && should_rcs_standlone) {

			if (base->view_angles) {

				auto old_view_angle = QAngle_t(input->vecViewAngle.x, input->vecViewAngle.y, input->vecViewAngle.z);

				static auto prev_punch = QAngle_t();

				auto aim_punch = g::m_local_player_pawn->m_aimPunchAngle();

				QAngle_t* current_punch = &aim_punch;

				if (g::m_local_player_pawn->m_shots() > 1) {
					QAngle_t compensation;

					aim_punch = g::m_local_player_pawn->m_aimPunchAngle();

					current_punch = &aim_punch;

					compensation.x = (current_punch->x - prev_punch.x);
					compensation.y = (current_punch->y - prev_punch.y);
					compensation.z = (current_punch->z - prev_punch.z);

					// Calculate target angles based on recoil compensation
					QAngle_t final_rcs = old_view_angle - QAngle_t(compensation.x, compensation.y, compensation.z);

					// Normalize and clamp the result
					final_rcs.Normalize();
					final_rcs.Clamp();

					// Apply the target view angle
					input->vecViewAngle = Vector_t(final_rcs.x, final_rcs.y, final_rcs.z);

					input->vecViewAngle.clamp();
					// Update the previous punch angle for the next calculation
					prev_punch = *current_punch;

				}
				else {
					// Reset the state when not shooting
					prev_punch = *current_punch;

				}
			}
		}
	}

	// setup hitboxes
	data->menu_hitboxes();

	if (!data->allowed()) {
		clear_data();
		return;
	}
	data->search_targets(cmd, e_target_mode::players);

	if (m_aim_targets.empty()) {
		clear_data();
		return;
	}


	if (!m_best_target.m_player_pawn || m_best_target.m_player_pawn->m_iHealth() <= 0 || !m_best_target.m_lag_entity || !m_best_target.m_data.m_point.pos.IsValid()  || !m_best_target.m_data.valid()) {
		clear_data();
		return;
	}

	if (!m_best_target.m_data.visible) {
		clear_data();
		return;
	}
	// apply lag compensation data
	command::m_aim_target = m_best_target.m_player_pawn;
	command::m_tick_fraction = m_best_target.m_data.m_tick.m_tickrange;
	command::m_target_ent_index = m_best_target.m_player_pawn->handle().index();
	command::m_tick_count = m_best_target.m_lag_entity->m_simulation_time * 64.f - 2;

	auto message = input->Message;


	m_best_target.m_lag_entity->apply(m_best_target.m_player_pawn);
	{
		// calculate aimpunch & compensate it
		QAngle_t aim_punch{ };
		QAngle_t final_aim_punch{};

		if (g::m_local_player_pawn->m_shots() > adaptive.rcs_shots) {
			hooks::m_get_aim_punch(g::m_local_player_pawn, &aim_punch, 0.f, true);
			final_aim_punch.x = aim_punch.x * 0.01f * (float)adaptive.rcs_x;
			final_aim_punch.y = aim_punch.y * 0.01f * (float)adaptive.rcs_y;
		}
		else { /* reset punch as we cant reach needed shots */
			aim_punch = QAngle_t(0, 0, 0);
			final_aim_punch = QAngle_t(0, 0, 0);
		}

		auto angle = aim_direction_fov(g::eye_pos, m_best_target.m_data.m_point.pos);
		auto best_point = QAngle_t(angle.x, angle.y, angle.z) - aim_punch;
		auto best_point_vec = angle - Vector_t(aim_punch.x, aim_punch.y, aim_punch.z);
		auto pressing_aim_key = Input::GetBindState(legit_key);
		if (pressing_aim_key) {

			if (base->view_angles) {
				auto point = data->smooth(base->view_angles->angValue, best_point);
				/* legit bot*/
				base->bit(CachedBits::BASE_BITS_VIEWANGLES); base->view_angles->bit(BASE_BITS_VIEWANGLES); {
					base->view_angles->angValue = point;
					base->view_angles->angValue.Clamp();
				}
			}

			// emulate mouse1
			/* {
				if (base->pInButtonState) {
					bool on_attack = (cmd->nAttack1StartHistoryIndex != -1 || (base->pInButtonState->nValue & IN_ATTACK));
					bool null_smooth = (!adaptive.smoothing || (adaptive.smooth_x <= 1 && adaptive.smooth_y <= 1));
					bool emulate_mouse_button = adaptive.perfect_silent && legit_key.uKey == VK_LBUTTON && null_smooth && message && on_attack;

					if (g::m_weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE && !g::m_local_player_pawn->m_bIsScoped())
						emulate_mouse_button = false;

					// basicly get prev history tick and set its viewangles so in the next tick ( shoot happens ) we got the psilent on bestpoint 
					if (emulate_mouse_button) {
						//message->m_angViewAngles = best_point;
					//	message->m_nFrameTickCount = command::m_tick_count;
					}
				}
			}*/
		}
	}
	m_best_target.m_lag_entity->reset(m_best_target.m_player_pawn);


	/*
	auto user_cmd = cmd;
	if (message) {
		message->m_nFrameTickCount = command::m_tick_count;
		message->m_nTargetIndex = m_best_target.m_player_pawn->handle().index();
	}*/
	/*
	
		/* make sure we grab correct interp info from game with fresh tick count */
		/*csgo_interpolation_info_t cl_interpolated_data, sv0_interpolated_data, sv1_interpolated_data;
		hooks::m_get_lerp_tick(m_best_target.m_player_pawn->m_pGameSceneNode(), &cl_interpolated_data,
			&sv0_interpolated_data,
			&sv1_interpolated_data,
			&input_history->nPlayerTickCount);

		// setup interpolation info Note: dst tick always game default now cuz was getting wrong dst tick values */
		//input_history->set_cl_interp(cl_interpolated_data.fraction, true);
		//input_history->set_sv_interp(sv0_interpolated_data.src_tick, sv0_interpolated_data.dst_tick, sv0_interpolated_data.fraction, false, true);
		//input_history->set_sv_interp1(sv1_interpolated_data.src_tick, sv1_interpolated_data.dst_tick, sv1_interpolated_data.fraction, false, true);

	m_last_target = m_best_target.m_player_pawn;
}

float cross_dist(const Vector_t& hitboxPos)
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

void legitbot::c_data::search_targets(CUserCmd* cmd, legitbot::e_target_mode mode) {

	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return;

	if (!g::m_local_player_pawn || !g::m_local_player_controller)
		return;

	if (!cmd)
		return;

	// Iterate targets optimally with distance sorting and target limiting
	if (mode == e_target_mode::players) {
		std::vector<c_target> m_targets; // Temporary list for all potential targets

		// First pass: Collect potential targets
		for (int i = 1; i < sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); ++i) {
			auto entity = sdk::m_game->pGameEntitySystem->Get(i);
			if (!entity)
				continue;

			SchemaClassInfoData_t* _class = nullptr;
			entity->dump_class_info(&_class);
			if (!_class)
				continue;

			const uint32_t hash = hash_32_fnv1a_const(_class->szName);

			// Filter only player controllers
			if (hash == hash_32_fnv1a_const(xorstr_("CCSPlayerController"))) {
				auto p_controller = reinterpret_cast<CCSPlayerController*>(entity);
				if (!p_controller)
					continue;

				if (!p_controller->handle().valid())
					continue;

				auto handle = p_controller->handle().index();

				// Retrieve player pawn
				auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle());
				if (!player)
					continue;

				auto player_scene_node = player->m_pGameSceneNode();
				if (!player_scene_node)
					continue;

				// Filter out invalid targets
				if (!player->IsAlive() || !g::m_local_player_pawn->enemy(player) || player_scene_node->dormant())
					continue;

				// Add valid target
				m_targets.emplace_back(c_target(player, entity, handle));
			}
		}

		// Return early if no targets were found
		if (m_targets.empty())
			return;

		// Reset aim targets for updated data
		m_aim_targets.clear();

		// Second pass: Scan and select the best target
		for (auto& target : m_targets) {
			auto m_entity = target.entity;
			auto m_player = target.player;

			if (!m_entity || !m_player)
				continue;

			// Get lag records for the player
			auto lag_record = this->select_record(target.handle);
			if (!lag_record) {
				continue;
			}

			c_aim_data aim_data = {};
			/* compete closest target to crosshair */
			// Calculate crosshair distance for prioritization
			float crosshair_distance = crosshair_dist(m_player->GetEyePosition());

			// Add target with the processed aim data
			m_aim_targets.emplace_back(c_best_target(m_entity, m_player, lag_record,  crosshair_distance, aim_data));
		}

		if (m_aim_targets.empty())
			return;

		data->search_best_target(cmd->get_view_angle());

		if (m_best_target.m_player_pawn && m_best_target.m_lag_entity) {
			// scan best target data for the aimbot calculation
			m_best_target.m_lag_entity->apply(m_best_target.m_player_pawn);
			{
				data->get_best_data(cmd, m_best_target.m_player_pawn, m_best_target.m_lag_entity, m_best_target.m_data);
			}
			m_best_target.m_lag_entity->reset(m_best_target.m_player_pawn);
		}
	}
}



float legitbot::c_data::get_damage_crosshair(CBaseEntity* target, Vector_t start, Vector_t end)
{


	auto pLocal = g::m_local_player_pawn;
	if (!pLocal || !target)
		return 0.f;

	if (pLocal->m_iHealth() <= 0)
		return 0.f;

	auto weapon = g::m_active_weapon;
	if (!weapon)
		return 0.f;

	auto vdata = g::m_weapon_data;
	if (!vdata)
		return 0.f;

	auto localent = sdk::m_game->pGameEntitySystem->Get(pLocal->handle());
	if (!localent)
		return 0.f;

	if (!target)
		return 0.f;

	auto controller = sdk::m_game->pGameEntitySystem->Get<CCSPlayerController>(target->handle());

	if (!controller)
		return 0.f;


	auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(controller->PawnHandle());

	if (!player)
		return 0.f;

	float damage = 0.f;
	bool canHit = false;

	// initialize autowall data
	autowall::c_auto_wall::data_t data;

	// do trace / penetration with updated pos
	autowall::g_auto_wall->pen(data, start, end, target, localent, pLocal, player, vdata, damage, canHit);


	// return autowall damage if found a valid one
	return data.m_can_hit ? data.m_dmg : 0.f;
}

static constexpr int get_bone_hitgroup(int group)
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

legitbot::hitbox_data_t legitbot::c_data::get_hitbox_data(CCSPlayerPawn* player_pawn, const int hitbox_id) {
	if (!player_pawn)
		return legitbot::hitbox_data_t();

	auto game_scene = player_pawn->m_pGameSceneNode();
	if (!game_scene)
		return legitbot::hitbox_data_t();

	auto skeleton = game_scene->m_pSkeleton();
	if (!skeleton)
		return legitbot::hitbox_data_t();

	auto& model_state = skeleton->GetModel();
	auto& model = model_state.m_hModel();
	if (!model)
		return legitbot::hitbox_data_t();

	CHitBoxSets* m_hitbox_set = hooks::m_get_hitbox_set(player_pawn, 0);
	if (!m_hitbox_set)
		return legitbot::hitbox_data_t();

	CHitBox* hitbox = &m_hitbox_set->m_HitBoxes()[hitbox_id];
	if (!hitbox)
		return legitbot::hitbox_data_t();

	CTransform m_hitbox_transform[64];
	int m_count = hooks::m_get_hitbox_to_world_transform(player_pawn, m_hitbox_set, m_hitbox_transform, 1024);
	if (!m_count)
		return legitbot::hitbox_data_t();

	const auto transformed_matrix = m_hitbox_transform[hitbox_id].ToMatrix();
	Vector_t w_mins, w_maxs;

	math::TransformAABB(transformed_matrix, hitbox->m_vMinBounds(), hitbox->m_vMaxBounds(), w_mins, w_maxs);

	const auto bone_index = static_cast<int>(get_bone_hitgroup(static_cast<int>(hitbox_id)));

	const auto bones = model_state.hitbox_data();

	legitbot::hitbox_data_t data;
	data.m_bone_cache = model_state.hitbox_data();
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
Vector_t transform_point2(Matrix3x4_t matrix, Vector_t point)
{
	Vector_t result{ };
	math::vector_transform(point, matrix, result);
	return result;
}
std::vector<Vector_t> calculate_sphere_points2(float radius, int num_points)
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

std::vector<Vector_t> calculate_points(int num_points, float radius, float height, Matrix3x4_t matrix, legitbot::hitbox_data_t hitbox)
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

bool legitbot::c_data::add_point(CBaseUserCmdPB* crc, CCSPlayerPawn* player, legitbot::anim_record_t* record, int hitbox_index, std::vector<c_aim_point>& points) {
	if (!record || !player)
		return false;

	int history_type = cfg_get(int, g_cfg.legit_history);
	auto pawn = history_type == 1 ? player : record->m_pawn;

	if (!pawn || !pawn->IsAlive())
		return false;

	hitbox_data_t hitbox_data = data->get_hitbox_data(pawn, hitbox_index);

	if (hitbox_data.m_invalid_data)
		return false;

	auto bone_cache = history_type == 1 ? hitbox_data.m_bone_cache[hitbox_data.m_num_bone] : record->m_bone_data[hitbox_data.m_num_bone];
	
	Vector_t center = (hitbox_data.m_mins + hitbox_data.m_maxs) * 0.5f;
	Matrix3x4_t matrix;
	math::transform_to_matrix(bone_cache, matrix);

	auto bone_pos = bone_cache.pos;
	QAngle_t hitchance_pos;
	math::VectorAngless(g::eye_pos - bone_pos, hitchance_pos);

	Vector_t point = transform_point2(matrix, center);

	int hitbox_from_menu = (hitbox_index);
	if (hitbox_from_menu == -1 || hitbox_index == HITBOX_UPPER_CHEST || hitbox_index == HITBOX_NECK)
		return false;

	// add center point on current hitbox
	c_aim_point temp_point;
	temp_point.pos = point;
	temp_point.idx = hitbox_index;
	temp_point.angle2d = hitchance_pos;
	points.emplace_back(temp_point);

	auto cfg_scale = hitbox_index == HITBOX_HEAD  ? adaptive.head_scale : adaptive.body_scale;
	float scale = cfg_scale >= 10.f ? cfg_scale : 10.f;

	float radius = hitbox_data.m_radius * (scale / 100.f);

	if (radius <= 0.f)
		return false;

	if (hitbox_index == HITBOX_HEAD || hitbox_index == HITBOX_NECK) {
		auto sphere_points = calculate_sphere_points2(radius, 5);

		if (sphere_points.empty())
			return false;

		for (const auto& point : sphere_points) {
			auto point_pos = transform_point2(matrix, { hitbox_data.m_maxs.x + point.x, hitbox_data.m_maxs.y + point.y, hitbox_data.m_maxs.z + point.z });
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
bool can_see_player_position(CCSPlayerPawn* player) {
	if (!player || !g::m_local_player_pawn)
		return false;

	// trace in order to see if player's visible 
	trace_filter_t filter = {};
	sdk::m_trace->Init(filter, g::m_local_player_pawn, 0x1C300Bu, 3, 7);

	ray_t ray = {};
	game_trace_t trace = {};
	Vector_t src = g::eye_pos;
	Vector_t end = player->GetEyePosition();
	sdk::m_trace->TraceShape(ray, &src, &end, filter, trace);
	sdk::m_trace->ClipTraceToPlayers(src, end, &filter, &trace, 0.F, 60.F, (1.F / (src - end).Length()) * (trace.m_end_pos - src).Length());

	if (trace.HitEntity) {
		if (trace.HitEntity->handle().valid()) {
			if (trace.HitEntity->handle().index() == player->handle().index() || trace.Fraction > 0.97f)
				return true;
			else
				return false;
		}
		else
			return false;
	}

	return false;
}
void legitbot::c_data::get_best_data(CUserCmd* cmd, CCSPlayerPawn* player, legitbot::anim_record_t* record, c_aim_data& aim_data)
{
	if (!sdk::m_engine->in_game() || !sdk::m_engine->connected())
		return;

	if (!g::m_local_player_controller || !g::m_local_player_pawn || !g::m_active_weapon || !g::m_weapon_data)
		return;

	if (!record)
		return;

	if (!record->m_pawn)
		return;
	if (!record->m_pawn->handle().valid())
		return;
	if (!cmd)
		return;

	if (!cmd->pBaseCmd)
		return;

	auto local_entity = sdk::m_game->pGameEntitySystem->Get(g::m_local_player_pawn->handle());
	if (!local_entity)
		return;

	auto entity = sdk::m_game->pGameEntitySystem->Get(record->m_pawn->handle());
	if (!entity)
		return;

	const auto scene_node = player->m_pGameSceneNode();
	if (!scene_node)
		return;


	legitbot::c_aim_point* m_best_point = nullptr;
	std::vector<legitbot::c_aim_point> scanned_points;
	std::vector<legitbot::c_aim_point> stored_multipoints;
	std::vector<legitbot::c_multi_aim_point> m_multi_points;

	bool m_hit = true;
	float m_best_dist = FLT_MAX;
	float m_pen_damage, m_dist_to_crosshair, m_point_dist = 0.f;

	for (auto& i : m_aim_hitboxes)
	{
		std::vector<legitbot::c_aim_point> added_points{};

		if (!add_point(cmd->pBaseCmd, player, record, i, added_points))
			continue;

		for (auto& point : added_points) {

			autowall::c_auto_wall::data_t pen_data;

			if (!autowall::g_auto_wall->penetrate(pen_data, g::eye_pos, point.pos, entity, local_entity, g::m_local_player_pawn, record->m_pawn, g::m_weapon_data, m_pen_damage, point.canHit))
				continue;

			float point_damage = pen_data.m_can_hit ? pen_data.m_dmg : 0.f;
			if (point_damage <= 0)
				continue;

			m_point_dist = crosshair_dist(point.pos);

			scanned_points.emplace_back(c_aim_point(point.pos, point.angle, point.angle2d, i, point_damage, m_hit, m_point_dist));
		}

		stored_multipoints.insert(stored_multipoints.end(), scanned_points.begin(), scanned_points.end());
	}

	if (scanned_points.empty())
		return;

	if (!stored_multipoints.empty())
		aim_data.render_points.insert(aim_data.render_points.end(), stored_multipoints.begin(), stored_multipoints.end());

	for (auto& potential_point : scanned_points) {
		if (potential_point.dist < m_best_dist) {
			m_best_point = &potential_point;
			m_best_dist = potential_point.dist;
		}
	}

	if (!m_best_point)
		return;

	if (m_best_point->idx == -1 || m_best_point->dmg <= 0)
		return;

	// set aim data
	aim_data.record = record;
	aim_data.m_point = *m_best_point;
	aim_data.m_hitbox.m_idx = m_best_point->idx;
	aim_data.m_hitbox.m_damage = (int)m_best_point->dmg;
	aim_data.visible = can_see_player_position(player);
}


void legitbot::c_data::render_fov_circle()
{
	if (g::ammo <= 0)
		return;

	// Get the screen position of the player (or camera)
	auto static_fov = legit_bot_fov;
	float dynamic_fov = legit_bot_fov;// make fov smaller if we more distant from m_best_target if m_best_target == null use static fov 

	if (m_best_target.m_player_pawn) {
		float distance_to_target = m_best_target.m_distance;//(g::eye_pos - m_best_target.eye_pos).Length(); // Get distance to target
		
		const float max_distance = 1000.0f;
		const float min_fov = 10.0f;

		// Calculate the dynamic FOV based on the distance
		if (distance_to_target < max_distance) {
			dynamic_fov = min_fov + ((static_fov - min_fov) * (1.0f - (distance_to_target / max_distance)));
		}
		else {
			dynamic_fov = min_fov; // If too far, use min FOV
		}
	}

	float legit_fov = cfg_get(int, g_cfg.legit_fov_type) <= 1 ? legit_bot_fov : dynamic_fov;
	if (cfg_get(bool, g_cfg.renderize_fov) && legit_fov > 0.f) {
		//draw circle
		const auto [width, height] = ImGui::GetIO().DisplaySize;
		ImVec2 center = ImVec2(width / 2, height / 2);

		// Calculate the screen-space radius based on the FOV
		float maxScreenDist = std::sqrt(center.x * center.x + center.y * center.y);
		float radius = (legit_fov / 180.0f) * maxScreenDist;

		// Get the color for the circle
		Color_t clr = override_fov_color ? cfg_get(ColorPickerVar_t, g_cfg.renderize_fov_clr_hit).colValue : cfg_get(ColorPickerVar_t, g_cfg.renderize_fov_clr).colValue ;

		// Render the FOV circle
		ImGui::GetBackgroundDrawList()->AddCircle(center, radius, clr.GetU32(), 64);
	}
}

float calc_fov_distance(const Vector_t& playerPos, const Vector_t& targetPos, float baseFOV, float scalingFactor) {
	float distance = playerPos.DistTo(targetPos);
	float dynamicFOV = baseFOV / (1.0f + distance * scalingFactor);

	return dynamicFOV;
}

 float fov_crosshair_dist(const Vector_t& hitboxPos)
{
	// Screen center
	ImVec2 center = ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);
	ImVec2 out;

	// Convert world position to screen position
	auto screenPos = math::WorldToScreen(hitboxPos, out);

	if (screenPos) {
		ImVec2 crosshairPos = center;
		ImVec2 hitboxScreenPos = out;

		// Calculate the 2D distance between the crosshair and the hitbox on screen
		float deltaX = crosshairPos.x - hitboxScreenPos.x;
		float deltaY = crosshairPos.y - hitboxScreenPos.y;
		float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

		// Normalize the distance to the range [0, 1] based on screen dimensions
		float maxScreenDist = std::sqrt(center.x * center.x + center.y * center.y);
		float normalizedDist = distance / maxScreenDist;

		// Map normalized distance to FOV range [0, 180]
		return normalizedDist * 180.0f;
	}

	// Return max FOV if hitbox isn't on screen
	return 180.0f;
}
// here we just wanna sort targets based on crosshair distance etc
void legitbot::c_data::search_best_target(QAngle_t view_angles) {
	auto local = g::m_local_player_pawn;
	if (!local)
		return;

	float min_allowed = std::numeric_limits<float>::max();

	auto static_fov = (float)adaptive.fov;
	auto best_fov = (float)adaptive.fov;
	legit_bot_fov = best_fov > 1.f ? (float)adaptive.fov : (float)best_fov;

	for (auto& data : m_aim_targets) {
		if (!data.m_player_pawn)
			continue;

		if (data.m_distance < min_allowed) {
			auto fov = fov_crosshair_dist(data.m_player_pawn->GetEyePosition());
			if (cfg_get(int, g_cfg.legit_fov_type) > 1)
			{
				auto distance_to_target = (local->m_pGameSceneNode()->m_vecAbsOrigin() - data.m_player_pawn->m_pGameSceneNode()->m_vecAbsOrigin()).Length(); // Get distance to target

				const auto max_distance = 1000.0f;
				const auto min_fov = 10.0f;

				if (distance_to_target < max_distance) {
					best_fov = min_fov + ((static_fov - min_fov) * (1.0f - (distance_to_target / max_distance)));
				}
				else {
					best_fov = min_fov;
				}
			}
			else
				best_fov = (float)adaptive.fov;

			if (fov > best_fov)
				continue;

			override_fov_color = true;
			// set best target and keep searching
			m_best_target = data;
			min_allowed = data.m_distance;
			continue;
		}
		continue;
	}
}
float compute_factor(float distance, float maxDistance, float minFactor, float maxFactor)
{
	if (distance > maxDistance)
		distance = maxDistance;

	float factor = minFactor + (maxFactor - minFactor) * (1 - distance / maxDistance);

	return factor;
}

float normalize_yaw(float yaw) {
	while (yaw > 180.0f) yaw -= 360.0f;
	while (yaw < -180.0f) yaw += 360.0f;
	return yaw;
}
QAngle_t legitbot::c_data::smooth(QAngle_t currentAngles, const QAngle_t& final_rcs)
{
	QAngle_t smooth_effect = QAngle_t();

	// the difference between the current angles and the final target angles
	auto m_diff = final_rcs - currentAngles;
	m_diff.y = normalize_yaw(m_diff.y);

	auto x_smooth = adaptive.smooth_x;
	auto y_smooth = adaptive.smooth_y;

	auto smooth_x = std::clamp(adaptive.smooth_x, 1.f, 40.f);
	auto smooth_y = std::clamp(adaptive.smooth_y, 1.f, 40.f);

	// interpolation if no curve is applied
	smooth_effect.x = currentAngles.x + (m_diff.x / smooth_x);
	smooth_effect.y = currentAngles.y + (m_diff.y / smooth_y);

	return smooth_effect;
}
const char* get_weapon_name_as_string_(CCSWeaponBase* weapon)
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

void legitbot::c_data::menu_adaptive() {


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

	const char* extractedWeaponName = get_weapon_name_as_string_(ActiveWeapon);
	bool has_awp = strcmp(extractedWeaponName, xorstr_("awp")) == 0;
	bool has_pistols = strcmp(extractedWeaponName, xorstr_("hkp2000")) == 0 || strcmp(extractedWeaponName, xorstr_("usp")) == 0 || strcmp(extractedWeaponName, xorstr_("glock")) == 0;
	bool has_heavy_pistols = false;
	if (strcmp(extractedWeaponName, xorstr_("revolver")) == 0 || strcmp(extractedWeaponName, xorstr_("deagle")) == 0)
		has_heavy_pistols = true;
	else
		has_heavy_pistols = false;

	bool has_scout = strcmp(extractedWeaponName, xorstr_("ssg08")) == 0;
	adaptive.legit_enable = cfg_get(bool, g_cfg.legit_enable);
	adaptive.disable_conditions = cfg_get(unsigned int, g_cfg.legit_ignore);
	int adaptive_index = 0;

	// if default 

	if (has_awp) {
		adaptive_index = 6;
		legit_key = cfg_get(KeyBind_t, g_cfg.legit_key6);
	}
	else if (has_scout) {
		adaptive_index = 5;
		legit_key = cfg_get(KeyBind_t, g_cfg.legit_key5);
	}
	else if (has_heavy_pistols) {
		adaptive_index = 2;
		legit_key = cfg_get(KeyBind_t, g_cfg.legit_key2);
	}
	else if (data->m_WeaponType() == WEAPONTYPE_PISTOL && !has_heavy_pistols) {
		adaptive_index = 1;
		legit_key = cfg_get(KeyBind_t, g_cfg.legit_key1);
	}
	else if (data->m_WeaponType() == WEAPONTYPE_MACHINEGUN || data->m_WeaponType() == WEAPONTYPE_SUBMACHINEGUN || data->m_WeaponType() == WEAPONTYPE_SHOTGUN || data->m_WeaponType() == WEAPONTYPE_RIFLE) {
		adaptive_index = 3;
		legit_key = cfg_get(KeyBind_t, g_cfg.legit_key3);
	}
	else if (data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE) {
		adaptive_index = 4;
		legit_key = cfg_get(KeyBind_t, g_cfg.legit_key4);
	}


	adaptive.accelaration = cfg_get_adaptive(float, 7, g_cfg.legit_smooth_accel, adaptive_index);
	adaptive.rcs_control = cfg_get_adaptive(bool, 7, g_cfg.legit_rcs, adaptive_index);
	adaptive.trigger_on_key = cfg_get_adaptive(bool, 7, g_cfg.trigger_on_key, adaptive_index);
	adaptive.recoil_adjust = cfg_get_adaptive(bool, 7, g_cfg.rcs_human, adaptive_index);
	adaptive.rcs_shots = cfg_get_adaptive(int, 7, g_cfg.recoil_shots, adaptive_index);
	adaptive.trigger_dmg = cfg_get_adaptive(int, 7, g_cfg.trigger_dmg, adaptive_index);
	adaptive.trigger_delay = cfg_get_adaptive(int, 7, g_cfg.trigger_delay, adaptive_index);
	adaptive.hitboxes = cfg_get_adaptive(unsigned int, 7, g_cfg.legit_hitboxes, adaptive_index);
	adaptive.rcs_x = cfg_get_adaptive(float, 7, g_cfg.legit_rcs_x, adaptive_index);
	adaptive.rcs_y = cfg_get_adaptive(float, 7, g_cfg.legit_rcs_y, adaptive_index);
	adaptive.perfect_silent = cfg_get_adaptive(bool, 7, g_cfg.perfect_silent, adaptive_index);
	adaptive.trigger = cfg_get_adaptive(bool, 7, g_cfg.legit_triggerbot, adaptive_index);
	adaptive.curve = cfg_get_adaptive(bool, 7, g_cfg.legit_curve, adaptive_index);
	adaptive.smooth_y = cfg_get_adaptive(float, 7, g_cfg.legit_smooth_x, adaptive_index);
	adaptive.smooth_x = cfg_get_adaptive(float, 7, g_cfg.legit_smooth_y, adaptive_index);

	adaptive.smoothing = cfg_get_adaptive(bool, 7, g_cfg.legit_smoothing, adaptive_index);
	adaptive.hitbox_head = cfg_get_adaptive(bool, 7, g_cfg.hitbox_head, adaptive_index);
	adaptive.hitbox_stomach = cfg_get_adaptive(bool, 7, g_cfg.hitbox_stomach, adaptive_index);
	adaptive.hitbox_chest = cfg_get_adaptive(bool, 7, g_cfg.hitbox_chest, adaptive_index);
	adaptive.hitbox_legs = cfg_get_adaptive(bool, 7, g_cfg.hitbox_legs, adaptive_index);
	adaptive.fov = cfg_get_adaptive(float, 7, g_cfg.legit_fov, adaptive_index);
	adaptive.trigger_magnet = cfg_get_adaptive(bool, 7, g_cfg.trigger_magnet, adaptive_index);
	adaptive.trigger_rcs = cfg_get_adaptive(bool, 7, g_cfg.trigger_rcs, adaptive_index);
	adaptive.trigger_hitbox = cfg_get_adaptive(unsigned int, 7, g_cfg.trigger_hitbox, adaptive_index);
	adaptive.trigger_hitchance = cfg_get_adaptive(float, 7, g_cfg.trigger_hitchance, adaptive_index);
	adaptive.head_scale = cfg_get_adaptive(int, 7, g_cfg.legit_head_scale, adaptive_index);
	adaptive.body_scale = cfg_get_adaptive(int, 7, g_cfg.legit_body_scale, adaptive_index);


}
void legitbot::c_data::renderize_fov() {
	if (!cfg_get(bool, g_cfg.renderize_fov))
		return;

	// Base FOV calcu
}
void legitbot::c_events::on_event(IGameEvent* ev, e_game_events type)
{
	if (!cfg_get(bool, g_cfg.legit_enable))
		return;

	switch (type) 
	{
	case death: {	
		m_last_target = nullptr;
		// we died quit
		data->init();
	}
	break;
	case round_start: {
		m_last_target = nullptr;
		data->init();
		kill_delay = 0;
	}
	break;
	case kill: {
		kill_delay = g::tickbase + cfg_get(int, g_cfg.legit_target_delay);
	}
	break;
	}
}
