
#include "..\..\utils\utils.h"
#include "..\..\sdk\cache\cache_entity.h"
#include "..\..\sdk\interfaces\engineclient.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include <vector>
#include "..\..\utils\hook\hooks.h"
#include "record.h"
#include "..\animations\animations.h"

std::unordered_map<int, std::unique_ptr<record::c_lag_entity>> m_lag_records;

void record::c_lag_record::init(CCSPlayerPawn* player) {
	if (!player)
		return;

	auto collision = player->m_pCollision();
	if (!collision)
		return;

	auto scene_node = player->m_pGameSceneNode();
	if (!scene_node)
		return;

	auto skeleton = player->m_pGameSceneNode()->m_pSkeleton();
	if (!skeleton)
		return;

	// Initialize the current state of the player into the instance variables
	this->mins = collision->m_vecMins();
	this->maxs = collision->m_vecMaxs();
	this->m_sim_time = player->m_flSimulationTime();
	this->m_flags = player->m_fFlags();
	this->ang_eye = player->m_angEyeAngles();
	this->m_rot = scene_node->m_angRotation();
	this->m_abs_rot = scene_node->m_angAbsRotation();
	this->m_velocity = player->m_vecVelocity();
	this->m_abs_velocity = player->m_vecAbsVelocity();
	this->m_origin = scene_node->m_vecOrigin();
	this->m_abs_origin = scene_node->m_vecAbsOrigin();
	this->m_eye_pos = player->GetEyePosition();
}

void record::c_lag_record::setup(CCSPlayerPawn* player) {
	if (!player)
		return;
	auto collision = player->m_pCollision();
	if (!collision)
		return;

	auto scene_node = player->m_pGameSceneNode();
	if (!scene_node)
		return;

	auto skeleton = player->m_pGameSceneNode()->m_pSkeleton();
	if (!skeleton)
		return;

	auto bone_count = skeleton->GetModel().m_hModel()->hitboxes_num();

	/* create a backup record instance */
	backup->m_player = player;
	backup->m_sim_time = player->m_flSimulationTime();
	backup->m_flags = player->m_fFlags();
	backup->ang_eye = player->m_angEyeAngles();
	backup->m_rot = scene_node->m_angRotation();
	backup->m_abs_rot = scene_node->m_angAbsRotation();
	backup->m_velocity = player->m_vecVelocity();
	backup->m_abs_velocity = player->m_vecAbsVelocity();
	backup->m_origin = scene_node->m_vecOrigin();
	backup->m_abs_origin = scene_node->m_vecAbsOrigin();
	backup->m_eye_pos = player->GetEyePosition();
	backup->mins = collision->m_vecMins();
	backup->maxs = collision->m_vecMaxs();

	// calc / transform bones 
	skeleton->setup_bones( FLAG_HITBOX);
	
	if (bone_count > 0) 
		memcpy(backup->m_data.data(), skeleton->GetModel().hitbox_data(), sizeof(bone_data) * bone_count);

	/* setup current record */
	this->mins = collision->m_vecMins();
	this->maxs = collision->m_vecMaxs();
	this->m_sim_time = player->m_flSimulationTime();
	this->m_flags = player->m_fFlags();
	this->ang_eye = player->m_angEyeAngles();
	this->m_rot = scene_node->m_angRotation();
	this->m_abs_rot = scene_node->m_angAbsRotation();
	this->m_velocity = player->m_vecVelocity();
	this->m_abs_velocity = player->m_vecAbsVelocity();
	this->m_origin = scene_node->m_vecOrigin();
	this->m_abs_origin = scene_node->m_vecAbsOrigin();
	this->m_eye_pos = player->GetEyePosition();

	if (bone_count > 0) {
		memcpy(this->m_data.data(), skeleton->GetModel().hitbox_data(), sizeof(bone_data) * bone_count);
		this->m_valid = true;
	}
}


void record::c_lag_record::apply(CCSPlayerPawn* player) {
	if (!player)
		return;
	CGameSceneNode* gameSceneNode = player->m_pGameSceneNode();
	if (!gameSceneNode) return;

	CSkeletonInstance* skeleton = gameSceneNode->m_pSkeleton();
	if (!skeleton) return;

	CCollisionProperty* collision = player->m_pCollision();
	if (!collision) return;

	collision->m_vecMins() = backup->mins;
	collision->m_vecMins() = backup->maxs;

	player->m_flSimulationTime() = backup->m_sim_time;
	player->m_fFlags() = backup->m_flags;

	player->m_angEyeAngles() = backup->ang_eye;
	gameSceneNode->m_angRotation() = backup->m_rot;
	gameSceneNode->m_angAbsRotation() = backup->m_abs_rot;

	player->m_vecVelocity() = backup->m_velocity;
	player->m_vecAbsVelocity() = backup->m_abs_velocity;

	gameSceneNode->m_vecOrigin() = backup->m_origin;
	player->set_origin(backup->m_abs_origin);

	const uint32_t boneCount = skeleton->GetModel().m_hModel()->hitboxes_num();
	if (boneCount > 0 && backup->m_data.size() >= boneCount)
		memcpy(skeleton->GetModel().hitbox_data(), backup->m_data.data(), sizeof(bone_data) * boneCount);
}

record::c_lag_entity::c_lag_entity(CBaseHandle handle) noexcept : handle(handle) {
	m_records.resize(6);
}
static bool inited = false;
// cache all records
void record::c_data::cache() {

	if (!g::m_local_player_pawn)
		return;

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
			CCSPlayerController* p_controller = reinterpret_cast<CCSPlayerController*>(entity);
			/* make sure to avoid caching local */
			if (!p_controller)
				continue;

			auto handle = sdk::m_game->pGameEntitySystem->get_handle_entity((CCSPlayerPawn*)entity);

			if (!p_controller->m_bPawnIsAlive()) {
				auto player_iterator = m_lag_records.find(handle);
				if (player_iterator != m_lag_records.end())
					m_lag_records.erase(player_iterator);

				continue;
			}

			CCSPlayerPawn* player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle());

			if (!player) {
				auto player_iterator = m_lag_records.find(handle);
				if (player_iterator != m_lag_records.end())
					m_lag_records.erase(player_iterator);

				continue;
			}

			if (player->handle().index() == g::m_local_player_pawn->handle().index()) {
				auto player_iterator = m_lag_records.find(handle);
				if (player_iterator != m_lag_records.end())
					m_lag_records.erase(player_iterator);

				continue;
			}


			auto hdl = player->handle();

			// check if current player is valid to cache records
			if ( !player->IsAlive() || !player->enemy(g::m_local_player_pawn)) {
				auto player_iterator = m_lag_records.find(handle);
				if (player_iterator != m_lag_records.end())
					m_lag_records.erase(player_iterator);

				continue;
			}


			// lag record has not been found, add a new one on current entity
			if (m_lag_records.find(handle) == m_lag_records.end())
				m_lag_records.insert_or_assign(handle, std::make_unique<record::c_lag_entity>(hdl));
		

			continue;
		}
		continue;
	}
}

void record::c_data::clear() {
	if (m_lag_records.empty())
		return;


	m_lag_records.clear();
}

void record::c_data::proccess(CUserCmd* cmd, CBaseUserCmdPB* base_cmd)
{
	if (!sdk::m_global_vars || !base_cmd || !cmd || !sdk::m_engine->connected() || !sdk::m_engine->in_game())
		return;

	if (m_lag_records.empty())
		return;

	auto local = CCSPlayerController::Get();
	if (!local)
		return;

	for (auto& [index, entity] : m_lag_records)
		entity->store(base_cmd->nClientTick);
}

void record::c_lag_entity::invalid() noexcept {
	if (m_last_valid) {
		for (auto& rec : this->m_records)
			rec.m_valid = false;

		m_last_valid = false;
	}
}

void record::c_data::render_skeleton() noexcept
{
	if (!cfg_get(bool, g_cfg.bSkeletonLagComp))
		return;

	auto local_controller = CCSPlayerController::Get();
	if (!local_controller)
		return;

	auto local = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(local_controller->PawnHandle());
	if (!local)
		return;

	if (m_lag_records.empty() || !local->IsAlive())
		return;

	for (const auto& [index, record] : m_lag_records) {
		if (!record)
			continue;

		if (!record->m_player)
			continue;

		if (!record->is_valid())
			continue;

		const auto& records = record->get_last_sim_record();
		if (!records)
			continue;


		auto delta_penisov = records->m_data[4].pos - records->m_data[5].pos;
		auto bone_penis_fix = records->m_data[5].pos + delta_penisov * 0.5f;

		for (uint32_t i = 0; i < record->count; i++) {
			if (!(record->model->hitbox_flags(i) & bone_flags::FLAG_HITBOX))
				continue;

			auto parent_index = record->model->hitbox_parent(i);
			if (parent_index == -1)
				continue;

			auto child = records->m_data[i].pos;
			auto parent = records->m_data[parent_index].pos;

			ImVec2 screen_pos, screen_parent_pos;

			if (!math::w2s(child, screen_pos))
				continue;
			if (!math::w2s(parent, screen_parent_pos))
				continue;

			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screen_pos.x, screen_pos.y), ImVec2(screen_parent_pos.x, screen_parent_pos.y), ImGui::VGetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f), 1.f), 2.f);
		}
	}
}

void record::c_lag_entity::store(unsigned int index)
{
	if (!sdk::m_engine->is_valid())
		return;

	auto local_c = CCSPlayerController::Get();
	if (!local_c)
		return;

	auto local_player = hooks::m_get_local_player(0);
	if (!local_player)
		return;

	if (m_lag_records.empty())
		return;

	if (!this) {
		invalid();
		return;
	}

	if (!this->handle.valid()) {
		invalid();
		return;
	}

	if (this->m_records.empty()) {
		invalid();
		return;
	}

	auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(this->handle.index());
	if (!player) {
		invalid();
		return;
	}

	if (!player->IsAlive() || !player->GetActiveWeapon()) {
		invalid();
		return;
	}

	if (player->handle().index() == local_player->handle().index()) {
		invalid();
		return;
	}

	if (!local_player->enemy(player)) {
		invalid();
		return;
	}

	auto game_scene_node = player->m_pGameSceneNode();
	if (!game_scene_node) {
		invalid();
		return;
	}

	// get current record 
	auto& m_record = this->m_records[index % this->m_records.size()];

	// setup record
	m_record.m_sim_time = player->m_flSimulationTime();
	m_record.m_eye_pos = player->GetEyePosition();
	m_record.m_origin = game_scene_node->m_vecOrigin();
	m_record.m_velocity = player->m_vecVelocity();
	m_record.m_flags = player->m_fFlags();
	m_record.m_player = player;

	this->m_player = player;
	if (!this->m_player) {
		invalid();
		return;
	}

	// set our lag entity boundaries to current player
	this->skeleton = player->m_pGameSceneNode()->m_pSkeleton();
	if (!this->skeleton) {
		invalid();
		return;
	}

	skeleton->setup_bones( bone_flags::FLAG_HITBOX);

	auto& model_state = skeleton->GetModel();
	this->model = model_state.m_hModel();
	if (!this->model) {
		invalid();
		return;
	}

	this->count = this->model->hitboxes_num();
	if (this->count == 0) {
		invalid();
		return;
	}

	if (!this->skeleton->GetModel().hitbox_data()) {
		invalid();
		return;
	}

	auto bone_data = this->skeleton->GetModel().hitbox_data();

	if (!bone_data) {
		invalid();
		return;
	}

	// apply bones for record
	memcpy(m_record.m_data.data(), bone_data, sizeof(bone_data) * this->count);
	skeleton->setup_bones(bone_flags::FLAG_ALL_BONE_FLAGS);
	skeleton->setup_bones(bone_flags::FLAG_HITBOX);


	// set our records as valid and able for its usage
	if (!m_record.m_data.empty()) {
		this->validate();
		m_record.validate();
	}

}

float record::c_lag_entity::get_distance(Vector_t p, Vector_t a, Vector_t b)
{

	Vector_t ap = p - a;
	Vector_t ab = b - a;

	float ab2 = ab.DotProduct(ab);
	float ap_ab = ap.DotProduct(ab);
	float t = ap_ab / ab2;

	// ignore if player is behind ur or too far away
	if (t < 0.0f || t > 1.0f)
		return std::numeric_limits<float>::max();

	Vector_t nearest = a + ab * t;
	return (p - nearest).Length();

}

const record::c_lag_record* record::c_lag_entity::get_last_record() noexcept
{
	if (!this)
		return nullptr;

	if (this->m_records.empty() || m_records.size() <= 2)
		return nullptr;

	if (!this->m_player)
		return nullptr;

	if (!this->m_last_valid)
		return nullptr;

	// get closest record
	const auto& anim_record = this->get_best_record(g::eye_pos, g::view_origin);

	record::c_lag_record* record = anim_record.m_last_record;

	if (!record)
		return nullptr;

	if (!record->is_valid())
		return nullptr;

	return record;
}
record::c_lag_record* record::c_lag_entity::get_last_sim_record() noexcept
{
	if (!this->m_last_valid)
		return nullptr;


	c_lag_record* last_record = nullptr;
	float last_record_time = std::numeric_limits<float>::max();

	for (auto& record : m_records)
	{
		if (!record.m_valid || !record.is_valid())
			continue;

		if (record.m_sim_time < last_record_time)
		{
			last_record_time = record.m_sim_time;
			last_record = &record;
		}
	}

	if (!last_record)
		return nullptr;

	return last_record;
}
const record::c_lag_record* record::c_lag_entity::get_first_record() noexcept
{
	if (!this->m_last_valid)
		return nullptr;

	const c_lag_record* first_record = nullptr;
	float last_record_time = std::numeric_limits<float>::max();

	for (auto& record : m_records)
	{
		if (!record.m_valid || !record.is_valid())
			continue;

		if (record.m_sim_time > last_record_time)
		{
			last_record_time = record.m_sim_time;
			first_record = &record;
		}
	}

	if (!first_record)
		return nullptr;

	return first_record;
}

float record::c_lag_entity::get_closest_distance(Vector_t start, Vector_t end, float las_valid_sim_time)
{
	float closest = std::numeric_limits<float>::max();

	for (auto& closest_rec : this->m_records)
	{
		if (!closest_rec.m_valid)
			continue;

		if (!closest_rec.is_valid())
			continue;

		float dist = get_distance(closest_rec.m_eye_pos, start, end);
		if (dist < closest)
			closest = dist;
	}

	return closest;
}

record::record_list record::c_lag_entity::get_best_record(Vector_t start, Vector_t end)
{
	int best_bone = -1;
	int best_record = -1;
	float best_bone_dist = std::numeric_limits<float>::max();

	for (int i = 0; i < (int)this->count; i++) {
		if (!(model->hitbox_flags(i) & bone_flags::FLAG_HITBOX))
			continue;

		auto parent_bone_index = model->hitbox_parent(i);
		if (parent_bone_index == -1)
			continue;

		for (int idx = 0; idx < std::size(m_records); idx++)
		{
			auto& record = m_records[idx];
			if (!record.m_valid)
				continue;

			if (!record.is_valid())
				continue;

			const float bone_dist = math::segment_dist(start, end, record.m_data[i].pos, record.m_data[parent_bone_index].pos);
			if (bone_dist < best_bone_dist) {
				best_bone_dist = bone_dist;
				best_bone = i;
				best_record = idx;
			}
		}
	}

	if (best_bone == -1 && best_record == -1)
		return record::record_list{ };
	c_tick tick_information = { 0.f, 0.f };

	int prev_record = best_record - 1;
	if (prev_record < 0)
		prev_record += m_records.size();
	if (!m_records[prev_record].m_valid)
		prev_record = best_record;

	int next_record = (best_record + 1) % m_records.size();
	if (!m_records[next_record].m_valid)
		next_record = best_record;

	const auto& best_rec = m_records[best_record];
	uint32_t best_bone_parent = model->hitbox_parent(best_bone);

	float prev_dist = math::segment_dist(
		start, end,
		m_records[prev_record].m_data[best_bone].pos,
		m_records[prev_record].m_data[best_bone_parent].pos);
	float next_dist = math::segment_dist(
		start, end,
		m_records[next_record].m_data[best_bone].pos,
		m_records[next_record].m_data[best_bone_parent].pos);

	bool m_prev_record_closer = prev_dist < next_dist;
	int first_record_idx = m_prev_record_closer ? prev_record : best_record;
	int last_record_idx = m_prev_record_closer ? best_record : next_record;


	tick_information.m_tickcount = 0.f;
	tick_information.m_tick_range = 0.f;
	const float best_full_dist = (m_prev_record_closer ? prev_dist : next_dist) + best_bone_dist;
	float fract = m_prev_record_closer ? prev_dist / best_full_dist : best_bone_dist / best_full_dist;

	return record::record_list{ &tick_information, &m_records[first_record_idx], &m_records[last_record_idx] ,fract };
}


/* pasted from neverlose*/



struct data_info
{
	std::uint64_t tick;
	float time;
	char pad[0x24];
};

bool restore = false;
CCSInputMessage* backup_message = nullptr;

void record::c_data::pre(CCSInputMessage* input_message, CCSGOInputHistoryEntryPB* current_history_command, int tickcount, int fraction, bool aim, bool record) {

	if (!input_message) 
		return;

	backup_message = input_message;
	restore = true;

	if (record) {
		//_log(LOG_INFO) << "pre tick: " << tickcount << " point:" << command::m_best_point;

		input_message->m_nFrameTickCount = tickcount;
		//input_message->m_flFrameTickFraction = fraction;
		//input_message->m_nPlayerTickCount = g::m_local_player_controller->m_nTickBase() - 1;
		//input_message->m_flPlayerTickFraction = 0.f;
	}
}
#include "..\prediction\prediction.h"
void record::c_data::post(CCSInputMessage* input_message, CCSGOInputHistoryEntryPB* current_history_command, int tickcount, int fraction, bool aim, bool record) {
	if (!current_history_command )
		return;

	auto cmd = CUserCmd::Get();
	if (!cmd)
		return;

	if (!sdk::m_engine)
		return;

	if (!command::m_aim_target && !command::m_rage_aim_target)
		return;

	if (restore) {
		input_message = backup_message;

		// get best target tickcount ( compensated simtime )		
		current_history_command->nRenderTickCount = tickcount;
		current_history_command->nCachedBits |= INPUT_HISTORY_BITS_RENDERTICKCOUNT;

		if (command::m_legit_bot) {
			_log(LOG_INFO) << " legit input";
			current_history_command->nPlayerTickCount = input_message->m_nPlayerTickCount + 2;
			current_history_command->nCachedBits |= INPUT_HISTORY_BITS_PLAYERTICKCOUNT;
		}
		else {
			current_history_command->nPlayerTickCount = input_message->m_nPlayerTickCount + cfg_get(int, g_cfg.semiragehc);
			current_history_command->nCachedBits |= INPUT_HISTORY_BITS_PLAYERTICKCOUNT;
		}
			

		restore = false;
	}

}

bool record::c_lag_record::is_valid() {

	if (!g::m_local_player_controller || !g::m_local_player_controller->m_bPawnIsAlive())
		return false;
	
	float dead_time = (g::m_local_player_controller->m_nTickBase() * g::interval) - m_sim_time;
	return dead_time < 0.2f;

}
float record::c_lag_record::get_delta_time()
{
	auto tick_entity = this->m_sim_time * 64.f;
	int tick_count = sdk::m_global_vars->m_tickcount;
	float maxunlag_val = std::min(g::max_unlag, 0.2f);

	float mod_value = fmodf(maxunlag_val, 0.015625);
	float unlag_difference = maxunlag_val - mod_value;
	float mod_ticks = mod_value * 64.0;
	int unlag_ticks = (int)((unlag_difference * 64.0) + 0.5);

	data_info info{};
	info.tick = tick_entity;
	info.time = tick_entity * 0.015625f;

	int v20 = 0;
	float v19 = 0.0f, v21 = 0.0f;

	if (mod_ticks < (1.0 - 0.0099999998))
	{
		v20 = tick_count - unlag_ticks;
		if (mod_ticks > 0.0099999998)
		{
			v19 = -mod_ticks;
			v21 = v19;
			if (v19 < 0.0f)
				v19 = v19 + 1.0f;
		}
	}
	else // high ping 100+
	{
		v20 = tick_count - (unlag_ticks + 1);
	}

	int v22 = v20 - 1;

	data_info v40{};
	v40.time = v19;
	if (v21 >= 0.0)
		v22 = v20;
	v40.tick = v22;
	if (v19 < (1.0 - 0.0099999998))
	{
		if (v19 <= 0.0099999998)
		{
			v40.time = 0.0;
		}
	}
	else
	{
		v40.tick = v22 + 1;
		v40.time = 0.0;
	}

	auto tick = v40.tick;
	auto v24 = (float)info.tick;
	auto v38 = v40;
	int new_tick = 0;
	data_info v39{};

	if (((v24 + info.time) * 0.015625) <= 0.0)
	{
		auto cmd_tick_count = tick_entity;
		v40.tick = cmd_tick_count;
		v40.time = 0.0;
		new_tick = v40.tick;
		tick = v38.tick;
		v39 = v40;
	}
	else
	{
		new_tick = info.tick;
		v39 = info;
	}

	int v32 = 0;
	float v30 = 0.0f;

	if (tick <= new_tick && (tick < new_tick || v39.time > v38.time))
	{
		auto v33 = new_tick - tick;
		v30 = v39.time - v38.time;
		if ((v39.time - v38.time) < 0.0)
			v30 = v30 + 1.0f;
		v32 = v33 - 1;
		if ((v39.time - v38.time) >= 0.0)
			v32 = v33;
		if (v30 >= (1.0 - 0.0099999998))
		{
			++v32;
		}
		if (v30 > 0.0099999998)
		{
			v30 = 0.0;
		}
	}
	else
	{
		v30 = v38.time - v39.time;
		int v31 = tick - new_tick;
		if ((v38.time - v39.time) < 0.0)
			v30 = v30 + 1.0f;
		v32 = v31 - 1;
		if ((v38.time - v39.time) >= 0.0)
			v32 = v31;
		if (v30 < (1.0 - 0.0099999998))
		{
			if (v30 > 0.0099999998)
			{
				v30 = 0.0;
			}
		}
		else
		{
			++v32;
			v30 = 0.0;
		}
	}

	auto delta_time = ((float)v32 + v30) * 0.015625f;
	return delta_time;
}
/*
void c_lagcompensation::run()
{
	auto controller = CCSPlayerController::Get();

	if (!controller)
		return;

	auto pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(controller->PawnHandle().index());
	if (!pawn )
		return;

	if (pawn->m_iHealth() <= 0)
		return;

	for (int i{}; i <= 64; i++)
	{
		auto entity = sdk::m_game->pGameEntitySystem->Get(i);
		if (!entity)
			continue;

		if (!entity->handle().valid())
			continue;

		SchemaClassInfoData_t* _class = nullptr;
		entity->dump_class_info(&_class);
		if (!_class)
			continue;

		const uint32_t hash = hash_32_fnv1a_const(_class->szName);

		if (hash != hash_32_fnv1a_const(xorstr_("CCSPlayerController")))
			continue;

		CCSPlayerController* p_controller = reinterpret_cast<CCSPlayerController*>(entity);
		if (!p_controller || p_controller == controller)
			continue;

		if (!p_controller->PawnHandle().valid())
			continue;

		auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle().index());
		if (!player)
			continue;

		if (player->m_iHealth() <= 0 || !player->GetActiveWeapon() || !player->enemy(pawn))
			continue;

		auto skeleton = player->m_pGameSceneNode()->m_pSkeleton();

		if (!skeleton)
			continue;

		skeleton->calc_world_space_bones(0, bone_flags::FLAG_ALL_BONE_FLAGS);

		if (m_animation.find(sdk::m_game->pGameEntitySystem->get_handle_entity(player)) != m_animation.end())
		{
			_log(LOG_INFO) << "inserting anim record";
			//for (auto it = m_animation[g_cs2->get_handle_entity(player)].m_lag_records_.rbegin(); it != m_animation[g_cs2->get_handle_entity(player)].m_lag_records_.rend();)
			//{
			//    if (!it->record_of_bounds())
			//    {
			//        it = decltype(it){m_animation[g_cs2->get_handle_entity(player)].m_lag_records_.erase(std::next(it).base())};
			//        continue;
			//    }

			//    it = next(it);
			//}

			if (m_animation[sdk::m_game->pGameEntitySystem->get_handle_entity(player)].m_lag_records_.size() > 32)
				m_animation[sdk::m_game->pGameEntitySystem->get_handle_entity(player)].m_lag_records_.pop_back();
		}

		auto& lag_ = m_animation[sdk::m_game->pGameEntitySystem->get_handle_entity(player)];

		auto& record = lag_.m_lag_records_.emplace_front();

		record.setup(player);

		lag_.latest_record = record;
		lag_.oldest_record = lag_.m_lag_records_.back();
	}
}

const c_lagrecord* c_lagcompensation::get_last_record() noexcept
{


	const c_lagrecord* last_record = nullptr;
	float last_record_time = std::numeric_limits<float>::max();

	for (auto& record : this->)
	{
		if (!record.m_valid || !record.is_valid())
			continue;

		if (record.m_sim_time < last_record_time)
		{
			last_record_time = record.m_sim_time;
			last_record = &record;
		}
	}

	if (!last_record)
		return nullptr;

	return last_record;
}

c_animation_table* c_lagcompensation::get_animation(CCSPlayerPawn* player_pawn)
{
	return nullptr;
}


void c_lagrecord::setup(CCSPlayerPawn* player_pawn)
{
	if (!player_pawn || !player_pawn->m_pGameSceneNode()->m_pSkeleton())
		return;

	m_simtime_ = player_pawn->m_flSimulationTime();
	origin_ = player_pawn->m_pGameSceneNode()->m_vecAbsOrigin();
	bone_count = player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().m_hModel()->hitboxes_num();

	if (bone_count > max_bones)
		bone_count = max_bones;

	memcpy(matrix_, player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().hitbox_data(), bone_count * sizeof(bone_data));
}

void c_lagrecord::apply(CCSPlayerPawn* player_pawn, bool extrapolation)
{
	if (!player_pawn || !player_pawn->m_pGameSceneNode()->m_pSkeleton() || !player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().hitbox_data())
		return;

	memcpy(backup_matrix_, player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().hitbox_data(), player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().m_hModel()->hitboxes_num() * sizeof(bone_data));

	memcpy(player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().hitbox_data(), matrix_, player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().m_hModel()->hitboxes_num() * sizeof(bone_data));

	//if (extrapolation)
	//{
	//    for (int i{}; i < 128; i++) {
	//        player_pawn->get_skeleton()->m_modelState().get_bone_data()[i].pos += ((0.017f * player_pawn->velocity()) * ((Interfaces::m_network_game_service->get_network_game_client()->get_net_channel2()->get_network_latency() * 64.f) + 0.5f));
	//    }
	//}
}

void c_lagrecord::restore(CCSPlayerPawn* player_pawn)
{
	memcpy(player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().hitbox_data(), backup_matrix_, player_pawn->m_pGameSceneNode()->m_pSkeleton()->GetModel().m_hModel()->hitboxes_num() * sizeof(bone_data));
}

bool c_lagrecord::record_of_bounds()
{

	const auto nci = sdk::m_engine->get_net_channel_info(0);
	if (!nci || !sdk::m_network->GetNetworkClient())
		return false;

	const float unlag = g::max_unlag;
	const auto latency = nci->latency(flow::FLOW_OUTGOING) + nci->latency(flow::FLOW_INCOMING);
	const float correct = std::clamp(latency + sdk::m_network->GetNetworkClient()->lerp(), 0.0f, g::max_unlag);
	auto max_delta = std::min(unlag - correct, 0.2f);
	const auto delta = max_delta - sdk::m_global_vars->m_curtime;

	return this->get_sim_time() > delta;
}
*/