#include "trace.h"
#include "..\..\utils\hook\hooks.h"

void i_trace::InitializeTraceInfo(game_trace_t* const hit)
{
	debug(hooks::m_trace_init_info != nullptr);
	hooks::m_trace_init_info(hit);

}
void i_trace::InitializeTrace(game_trace_t& trace)
{
	debug(hooks::m_trace_init != nullptr);
	hooks::m_trace_init(trace);
}
void i_trace::Init(trace_filter_t& filter, CCSPlayerPawn* skip, uint64_t mask, uint8_t layer, uint16_t idk)
{
	auto fn = hooks::m_trace_filter_init;
	debug(fn != nullptr);

	fn(filter, skip, mask, layer, idk);
}
void i_trace::ClipTraceToPlayers(Vector_t& start, Vector_t& end, trace_filter_t* filter, game_trace_t* trace, float min, int length, float max)
{	// cHoca

	auto fn = hooks::m_clip_trace_to_players;

	debug(fn != nullptr);

	fn(start, end, filter, trace, min, max, length);
}

bool i_trace::ClipTraceToEntity(ray_t* ray, const Vector_t& start, const Vector_t& end, CCSPlayerPawn* player, trace_filter_t* filter, game_trace_t* trace)
{
	using function_t = bool(__fastcall*)(i_trace*, ray_t*, const Vector_t&, const Vector_t&, CCSPlayerPawn*, trace_filter_t*, game_trace_t*);
	return hooks::m_clip_trace_to_entity(this, ray, start, end, player, filter, trace);
}
 void i_trace::get_trace_info(trace_data_t* trace, game_trace_t* hit,
	const float unknown_float, void* unknown) {

	auto fn = hooks::m_trace_get_info;
	return fn(trace, hit, unknown_float, unknown);
}

 bool i_trace::handle_bullet_penetration(trace_data_t* const trace, void* stats,
	UpdateValueT* const mod_value,
	const bool draw_showimpacts) {

	auto fn = hooks::m_trace_handle_bullet_penetration;
	return fn(trace, stats, mod_value, nullptr, nullptr, nullptr, nullptr, nullptr, draw_showimpacts);
}

 void i_trace::CreateTrace(trace_data_t* const trace, const Vector_t start,
	const Vector_t end, const trace_filter_t& filler,
	const int penetration_count) {


	auto fn = hooks::m_trace_create;
	debug(fn != nullptr);
	return fn(trace, start, end, filler, penetration_count);
}

// #STR: "Physics/TraceShape (Client)" then xref
void i_trace::TraceShape(ray_t& ray, Vector_t* start, Vector_t* end, trace_filter_t filter, game_trace_t& trace)
{
	auto fn = hooks::m_trace_shape;
	debug(fn != nullptr);
	fn(this, ray, start, end, filter, trace);

}