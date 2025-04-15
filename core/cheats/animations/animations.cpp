#include "animations.h"

#include "..\..\utils\utils.h"
#include "..\..\sdk\cache\cache_entity.h"
#include "..\..\sdk\interfaces\engineclient.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include <vector>
#include "..\..\utils\hook\hooks.h"

void animations::c_animations::set_playback_rate(CBaseAnimGraph* base_anim_graph) {

}

/*
void  animations::c_animations::on_frame_stage() {

	if (!sdk::m_engine->is_valid())
		return;

	if (!g::m_local_player_controller || !g::m_local_player_pawn)
		return;

	if (!g::m_local_player_pawn->IsAlive() || !g::m_active_weapon || !g::chams_ready)
		return;

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

			auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle());
			if (!player)
				continue;

			auto player_scene_node = player->m_pGameSceneNode();
			if (!player_scene_node)
				continue;

			if (!player->IsAlive() || !g::m_local_player_pawn->enemy(player))
				continue;

			auto body_component = entity->m_CBodyComponent();
			if (!body_component)
				continue;

			CBaseAnimGraph* base_anim_graph = body_component->Get();
			if (!base_anim_graph)
				continue;

			if (!(player->m_fFlags() & FL_ONGROUND))
				this->on_sequence_update(base_anim_graph);

		}
	}
}
*/
void  animations::c_animations::on_sequence_update(CBaseAnimGraph* base_anim_graph) {
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

float  animations::c_animations::get_finished_cycle_rate(CBaseAnimGraph* a1) {
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