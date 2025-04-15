#include "game_event.h"
#include "..\..\cheats\visuals\world\effects.h"
#include "..\..\cheats\visuals\world\hitsound.h"
#include "..\..\sdk\interfaces\trace.h"
uint32_t MurmurHash22(const void* key, int len, uint32_t seed)
{
	/* 'm' and 'r' are mixing constants generated offline.
	   They're not really 'magic', they just happen to work well.  */

	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	/* Initialize the hash to a 'random' value */

	uint32_t h = seed ^ len;

	/* Mix 4 bytes at a time into the hash */

	const unsigned char* data = (const unsigned char*)key;

	while (len >= 4)
	{
		uint32_t k = *(uint32_t*)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	/* Handle the last few bytes of the input array  */

	switch (len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	/* Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.  */

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}
EventHash_t::EventHash_t(const char* szString)
{
	m_szString = szString;
	m_uHash = MurmurHash22(szString, static_cast<int>(c_run_time::StringLength(szString)), STRINGTOKEN_MURMURHASH_SEED);
}
void CEvent::Initialize() {
	sdk::m_game_event->add(this, xorstr_("bullet_impact"), false);
}
void CEvent::Destroy()
{
    sdk::m_game_event->remove(this);
}
void CEvent::bullet_impact(IGameEvent* ev)
{
}

#include "..\..\utils\hook\hooks.h"
bool is_trace_visible(CCSPlayerPawn* player) {
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
void CEvent::fire_game_event(IGameEvent* ev) {

	if (!sdk::m_engine->is_valid())
		return;

	auto local_player = hooks::m_get_local_player(0);
	if (!local_player)
		return;

    const char* event_name = ev->GetName();
	auto controller = CCSPlayerController::Get();
	if (!controller)
		return;

	const auto event_controller = ev->get_player_controller("userid");
	if (!event_controller)
		return;

	auto local_idx = controller->m_pEntityIdentity()->index();

	if (event_controller->m_pEntityIdentity()->index() == local_idx) {

		auto local_pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(event_controller->PawnHandle());

		// get bullet x,y,z pos
		float x = ev->GetFloat("x");
		float y = ev->GetFloat("y");
		float z = ev->GetFloat("z");
		auto position = Vector_t(x, y, z);

		// local beam
		if (cfg_get(bool, g_cfg.bullet_beam_local)) {
			
			auto color = cfg_get(ColorPickerVar_t, g_cfg.bullet_beam_local_clr).colValue;
			auto width = cfg_get(float, g_cfg.bullet_beam_width);
			// parse into beam
			effects::beam->add(g::eye_pos, position, color, width);

		}

		// local impact
		if (cfg_get(bool, g_cfg.impacts)) {
			auto overlay = sdk::m_client->GetDebugOverlay();
			if (overlay) {
				overlay->add_box(
					Vector_t(x + 2, y + 1, z),
					Vector_t(-2, -2, -2),
					Vector_t(2, 2, 2),
					Vector_t(),
					Color_t(255, 0, 0, 127)
				);
			}
			if (overlay) {		
				overlay->add_box(
					position,
					Vector_t(-2, -2, -2),
					Vector_t(2, 2, 2),
					Vector_t(),
					Color_t(0, 0, 255, 127)
				);
			}
		}

		if (local_pawn) {
			// start position of trace is where we took the shot.
			auto start = g::eye_pos;

			// the impact pos contains the spread from the server
			// which is generated with the server seed, so this is where the bullet
			// actually went, compute the direction of this from where the shot landed
			// and from where we actually took the shot.
			auto dir = (position - start).normalized();

			// get end pos by extending direction forward.
			// todo; to do this properly should save the weapon range at the moment of the shot, cba..
			auto end = start + (dir * 8192.f);

			// intersect our historical matrix with the path the shot took.
			trace_filter_t filter = {};
			sdk::m_trace->Init(filter, local_pawn, MASK_SHOT, 3, 7);
			game_trace_t trace = {};
			ray_t ray = {};

			sdk::m_trace->TraceShape(ray, &start, &end, filter, trace);
			sdk::m_trace->ClipTraceToPlayers(start, end, &filter, &trace, 0.F, 60.F, (1.F / (start - end).Length()) * (trace.m_end_pos - start).Length());

			if (trace.HitEntity) {
				if (trace.HitEntity->handle().valid()) {

					if (cfg_get(bool, g_cfg.hitmarker3d) && trace.HitEntity->IsBasePlayer() ) {		
						hitsound::hitmarker->on_hit(position, 1);				
					}
				}
			}
		}
	}
	else if (cfg_get(bool, g_cfg.bullet_beam_enemy) && event_controller->m_pEntityIdentity()->index() != local_idx) {
		auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(event_controller->PawnHandle());
		if (!player  )
			return;

		if (player->m_iTeamNum() == local_player->m_iTeamNum())
			return;

		// Get bullet x, y, z position
		float x = ev->GetFloat("x");
		float y = ev->GetFloat("y");
		float z = ev->GetFloat("z");

		auto bullet_position = Vector_t(x, y, z);
		ImVec2 bullet_pos;
		if (!math::WorldToScreen(bullet_position, bullet_pos))
			return;


		auto color2 = cfg_get(ColorPickerVar_t, g_cfg.bullet_beam_enemy_clr).colValue;
		auto width = cfg_get(float, g_cfg.bullet_beam_width);
		// Parse into beam
		effects::beam->add(player->GetEyePosition(), bullet_position, color2, width);
		
	}
	

}