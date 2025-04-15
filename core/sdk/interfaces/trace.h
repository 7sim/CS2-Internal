#pragma once
// used: color_t
#include "../datatypes/color.h"

// used: cbasehandle
#include "..\..\sdk\sdk.h"
#include "..\..\sdk\handle\handle.hpp"
#include "..\..\sdk\addresses\entity.h"
#define CLIP_TRACE_TO_PLAYERS "48 8B C4 55 56 48 8D A8 58 FF FF FF 48 81 EC 98 01 00 00 48"
#define TRACE_SHAPE "48 89 54 24 ? 48 89 4C 24 ? 55 53 56 57 41 55 41 56 48 8D AC 24"
#define COMBINE(x, y) x##y
#define COMBINE2(x, y) COMBINE(x, y)
#include <array>
#include <cstddef>

#define CLIENT_DLL "client.dll"
#define PAD_CLASS_DEBUG(sz) int COMBINE2(pad_, __COUNTER__)[sz];
#define CS2_PAD( number, size )                          \
private:                                                 \
    [[maybe_unused]] std::array< std::byte, size > m_unknown_##number{ }; \
public:
enum Contents_t {
	CONTENTS_EMPTY = 0,
	CONTENTS_SOLID = 0x1,
	CONTENTS_WINDOW = 0x2,
	CONTENTS_AUX = 0x4,
	CONTENTS_GRATE = 0x8,
	CONTENTS_SLIME = 0x10,
	CONTENTS_WATER = 0x20,
	CONTENTS_BLOCKLOS = 0x40,
	CONTENTS_OPAQUE = 0x80,
	CONTENTS_TESTFOGVOLUME = 0x100,
	CONTENTS_UNUSED = 0x200,
	CONTENTS_BLOCKLIGHT = 0x400,
	CONTENTS_TEAM1 = 0x800,
	CONTENTS_TEAM2 = 0x1000,
	CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
	CONTENTS_MOVEABLE = 0x4000,
	CONTENTS_AREAPORTAL = 0x8000,
	CONTENTS_PLAYERCLIP = 0x10000,
	CONTENTS_MONSTERCLIP = 0x20000,
	CONTENTS_CURRENT_0 = 0x40000,
	CONTENTS_CURRENT_90 = 0x80000,
	CONTENTS_CURRENT_180 = 0x100000,
	CONTENTS_CURRENT_270 = 0x200000,
	CONTENTS_CURRENT_UP = 0x400000,
	CONTENTS_CURRENT_DOWN = 0x800000,
	CONTENTS_ORIGIN = 0x1000000,
	CONTENTS_MONSTER = 0x2000000,
	CONTENTS_DEBRIS = 0x4000000,
	CONTENTS_DETAIL = 0x8000000,
	CONTENTS_TRANSLUCENT = 0x10000000,
	CONTENTS_LADDER = 0x20000000,
	CONTENTS_HITBOX = 0x40000000,
};

enum Masks_t {
	MASK_ALL = 0xFFFFFFFF,
	MASK_SOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_PLAYERSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_NPCSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_NPCFLUID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_WATER = CONTENTS_WATER | CONTENTS_MOVEABLE | CONTENTS_SLIME,
	MASK_OPAQUE = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE,
	MASK_OPAQUE_AND_NPCS = MASK_OPAQUE | CONTENTS_MONSTER,
	MASK_BLOCKLOS = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_BLOCKLOS,
	MASK_BLOCKLOS_AND_NPCS = MASK_BLOCKLOS | CONTENTS_MONSTER,
	MASK_VISIBLE = MASK_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE,
	MASK_VISIBLE_AND_NPCS = MASK_OPAQUE_AND_NPCS | CONTENTS_IGNORE_NODRAW_OPAQUE,
	MASK_SHOT = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE | CONTENTS_HITBOX,
	MASK_SHOT_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_DEBRIS,
	MASK_SHOT_HULL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE,
	MASK_SHOT_PORTAL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER,
	MASK_SOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE,
	MASK_PLAYERSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_PLAYERCLIP | CONTENTS_GRATE,
	MASK_NPCSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
	MASK_NPCWORLDSTATIC = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
	MASK_NPCWORLDSTATIC_FLUID = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP,
	MASK_SPLITAREPORTAL = CONTENTS_WATER | CONTENTS_SLIME,
	MASK_CURRENT = CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN,
	MASK_DEADSOLID = CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_GRATE,
};

enum {
	SURF_LIGHT = 0x0001,
	SURF_SKY2D = 0x0002,
	SURF_SKY = 0x0004,
	SURF_WARP = 0x0008,
	SURF_TRANS = 0x0010,
	SURF_NOPORTAL = 0x0020,
	SURF_TRIGGER = 0x0040,
	SURF_NODRAW = 0x0080,
	SURF_HINT = 0x0100,
	SURF_SKIP = 0x0200,
	SURF_NOLIGHT = 0x0400,
	SURF_BUMPLIGHT = 0x0800,
	SURF_NOSHADOWS = 0x1000,
	SURF_NODECALS = 0x2000,
	SURF_NOPAINT = SURF_NODECALS,
	SURF_NOCHOP = 0x4000,
	SURF_HITBOX = 0x8000
};
struct ray_t
{
	Vector_t Start = Vector_t(0, 0, 0);
	Vector_t End = Vector_t(0, 0, 0);
	Vector_t Mins = Vector_t(0, 0, 0);
	Vector_t Maxs = Vector_t(0, 0, 0);
	char __pad0000[0x4];
	std::uint8_t UnkownType = 0x0;
};
static_assert(sizeof(ray_t) == 0x38);

class trace_filter_t
{
public:
	char __pad0000[0x8];
	std::uint64_t trace_mask;
	std::uint64_t null_it[2];
	std::uint32_t SkipHandles[4];
	std::uint16_t Collisions[2];
	std::uint16_t N0000011C;
	std::uint8_t layer;
	std::uint8_t N00000104;
	std::uint8_t null_it3;
};

struct C_TraceHitboxData {
	CS2_PAD(0, 0x38)
		int m_hitgroup{ };
};

class game_trace_t {
public:
	void* Surface;
	CBaseEntity* HitEntity;
	C_TraceHitboxData* HitboxData;
	CS2_PAD(0, 0x38)
		std::uint32_t Contents;
	CS2_PAD(1, 0x24)
		Vector_t m_start_pos, m_end_pos, m_normal, m_pos;
	CS2_PAD(2, 0x4);
	float Fraction;
	CS2_PAD(3,0x6);
	bool m_all_solid;
	CS2_PAD(4, 0x4D)
};


struct UpdateValueT {
	float previousLenght{ };
	float currentLenght{ };
	CS2_PAD(0, 0x8)
		std::int16_t handleIdx{ };
	CS2_PAD(1, 0x6)
};
struct trace_arr_element_t {
	CS2_PAD(0, 0x30)
};

struct trace_data_t {
	std::int32_t m_uk1{ };
	float m_uk2{ 52.0f };
	void* m_arr_pointer{ };
	std::int32_t m_uk3{ 128 };
	std::int32_t m_uk4{ static_cast<std::int32_t>(0x80000000) };
	std::array< trace_arr_element_t, 0x80 > m_arr = { };
	CS2_PAD(0, 0x8)
		std::int64_t m_num_update{ };
	void* m_pointer_update_value{ };
	CS2_PAD(1, 0xC8)
		Vector_t m_start { }, m_end{ };
	CS2_PAD(2, 0x50)
};
class i_trace
{
public:	// cHoca
	void InitializeTraceInfo(game_trace_t* const hit);
	void InitializeTrace(game_trace_t& trace)
		;
	void Init(trace_filter_t& filter, CCSPlayerPawn* skip, uint64_t mask, uint8_t layer, uint16_t idk)
		;
	void ClipTraceToPlayers(Vector_t& start, Vector_t& end, trace_filter_t* filter, game_trace_t* trace, float min, int length, float max)
		;
	
	bool ClipTraceToEntity(ray_t* ray, const Vector_t& start, const Vector_t& end, CCSPlayerPawn* player, trace_filter_t* filter, game_trace_t* trace);

	static void get_trace_info(trace_data_t* trace, game_trace_t* hit,
		const float unknown_float, void* unknown);
	
	static bool handle_bullet_penetration(trace_data_t* const trace, void* stats,
		UpdateValueT* const mod_value,
		const bool draw_showimpacts = false);

	static void CreateTrace(trace_data_t* const trace, const Vector_t start,
		const Vector_t end, const trace_filter_t& filler,
		const int penetration_count);

	// #STR: "Physics/TraceShape (Client)" then xref
	void TraceShape(ray_t& ray, Vector_t* start, Vector_t* end, trace_filter_t filter, game_trace_t& trace);
};
