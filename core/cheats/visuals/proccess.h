#pragma once

#include <memory>
#include <array>
#include <deque>

#include "..\..\utils\render\render.h"

#include "..\..\utils\utils.h"
#include "..\..\sdk\sdk.h"
#include "..\..\math\elements\qangle.h"
#include "..\..\math\elements\vector.h"
#include "..\..\math\elements\transform.h"
#include "..\..\utils\configs\config.h"
#include <imgui.h>
#include <shared_mutex>
enum object_type_t : int
{
	object_none = -1,
	object_player = 0,
	object_weapons = 1,
	object_molotovs = 2,
	object_planted_c4 = 3,

};
enum player_object_type_t : int
{
	player_none = -1,
	player_enemy = 0,
	player_teammate = 1,
};
enum projectile_object_type_t : int
{
	projectile_none = -1,
	projectile_flash = 0,
	projectile_grenade = 1,
	projectile_molotov = 2,
	projectile_smoke = 3,
};
struct cached_bone_data {
	Vector_t pos;
};
struct player_object_t
{
	player_object_t(CBaseEntity* a1, CCSPlayerPawn* plyr, CBaseHandle a2, ImVec4 a3, player_object_type_t a4, int a5, int a6, int a7,int a8, int a9,
		const char* a10, const char* a11, std::vector<cached_bone_data> a12, Vector_t a13, Vector_t a14, Vector_t a15, Vector_t a16, CTransform node_transf, int a17) :
		entity(a1), player(plyr), handle(a2), box(a3), type(a4) , ammo(a5), max_ammo(a6), health(a7), weapon_idx(a8), armour(a9), name(a10),
		weapon_name(a11), bone_cache(a12), eye_pos(a13), abs_origin(a14), mins(a15), maxs(a16), node(node_transf) , team_num(a17) { }

	CBaseEntity* entity;
	CCSPlayerPawn* player;
	CBaseHandle handle;
	ImVec4 box;
	player_object_type_t type;
	int ammo;
	int max_ammo;
	int health;
	int weapon_idx;
	int armour;
	const char* name;
	const char* weapon_name;
	std::vector<cached_bone_data> bone_cache;
	Vector_t eye_pos;
	Vector_t abs_origin;
	Vector_t mins;
	Vector_t maxs;
	CTransform node;
	int team_num;
};
struct object_t
{
	object_t(CBaseEntity* a1, CCSPlayerPawn* plyr, CBaseHandle a2, ImVec4 a3, object_type_t a4) :
		entity(a1), p_entity(plyr), handle(a2), box(a3), type(a4)  { }

	CBaseEntity* entity;
	CCSPlayerPawn* p_entity;
	CBaseHandle handle;
	ImVec4 box;
	object_type_t type;

};

struct c4_object_t
{
	c4_object_t(CBaseHandle a1, Vector_t a2) :
		handle(a1), abs_origin(a2) { }

	CBaseHandle handle;
	Vector_t abs_origin;
};
struct item_object_t
{
	item_object_t(CBaseEntity* a1, const char* a2,int a3, int a4, CBaseHandle a5, ImVec4 a6, int a7, CTransform a8, Vector_t a9, Vector_t a10 , int a11 ) :
		entity(a1), name(a2), ammo(a3), max_ammo(a4) , handle(a5), box(a6) , weapon_index(a7) , node(a8), mins(a9) , maxs(a10), state(a11) { }

	CBaseEntity* entity;
	const char* name;
	int ammo;
	int max_ammo;
	CBaseHandle handle;
	ImVec4 box;
	int weapon_index;
	CTransform node;
	Vector_t mins;
	Vector_t maxs;
	int state;
};

struct projectile_object_t
{
	projectile_object_t(CBaseEntity* a1, CBaseHandle a2, ImVec4 a3, projectile_object_type_t a4, C_BaseCSGrenade* a5, CBaseEntity* a6, Vector_t a7, bool a8, Vector_t a9, Vector_t a10, CTransform a11, bool a12) :
		entity(a1), handle(a2), box(a3), type(a4), grenade(a5), owner(a6) , position(a7), expired(a8) , mins(a9), maxs(a10) , node(a11), teammate(a12) { }

	CBaseEntity* entity;
	CBaseHandle handle;
	ImVec4 box;
	projectile_object_type_t type;
	C_BaseCSGrenade* grenade;
	CBaseEntity* owner;
	Vector_t position;
	bool expired;
	Vector_t mins;
	Vector_t maxs;
	CTransform node;
	bool teammate;
};
class c_cached_weapons {
public:
	bool is_invalid;
	CBaseHandle handle;
	Vector_t abs_origin;
	Vector_t mins;
	Vector_t maxs;
	std::string weapon_name;
	int item_index;
};

class c_cached_molotov {
public:
	CBaseHandle handle;
	int m_fireCount;
	bool m_bFireIsBurning[64];
	Vector_t m_firePositions[64];
	std::vector<Vector_t> points;

};

enum c_reset : int {
	players = 0,
	weapon = 1,
	molotov = 2
};

class cachec_local {
public:
	int handle;
	int armour;
	int health;
	int flags;
	bool valid;
	bool alive;
	bool has_grenade;
	Vector_t abs_origin;
	Vector_t eye;
	Vector_t velocity;
	void reset() {
		eye = Vector_t();
		abs_origin = Vector_t();
		velocity = Vector_t();
		flags = 0;
		health = 0;
		armour = 0;
		handle = 0;
		valid = false;
		has_grenade = false;
	}
};
namespace weapon_cache_system {
	struct cached_entity {
		bool valid() const noexcept {
			return entity && state == 0;
		}

		bool update_entity(CBaseEntity* ent) noexcept {
			if (this->entity != ent)
				this->entity = ent;

			return this->entity != nullptr;
		}

		CBaseHandle handle;
		CBaseEntity* entity{ nullptr };
		CBaseEntity* owner{ nullptr };
		int state;
	};

	class weapon_cache final {
	public:
		weapon_cache(const weapon_cache&) = delete;
		weapon_cache& operator=(const weapon_cache&) = delete;

		weapon_cache(weapon_cache&&) noexcept = default;
		weapon_cache& operator=(weapon_cache&&) noexcept = default;

		weapon_cache() = default;
		~weapon_cache() = default;

		template<typename Func>
		void read_entity(Func&& f) const {
			std::shared_lock lock(weapons_mutex);
			f(weapons);
		}

		template<typename Func>
		void write_entity(Func&& f) {
			std::unique_lock lock(weapons_mutex);
			f(weapons);
		}

		[[nodiscard]] std::vector<cached_entity> get_weapons_snapshot() const {
			std::shared_lock lock(weapons_mutex);
			return weapons;
		}

		void clear_weapons() noexcept {
			std::unique_lock lock(weapons_mutex);
			weapons.clear();
		}

		void reserve_weapons(size_t size) {
			std::unique_lock lock(weapons_mutex);
			weapons.reserve(size);
		}
		void startup();
	private:
		mutable std::shared_mutex weapons_mutex;
		std::vector<cached_entity> weapons;
	};

	inline std::unique_ptr<weapon_cache> g_weapons_cache = std::make_unique<weapon_cache>();
}
extern std::vector<item_object_t> m_items_object;

namespace player_cache_system {

	struct cached_entity {
		bool valid() const noexcept {
			return controller && pawn;
		}

		bool update_pawn() noexcept {
			this->pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(this->controller->PawnHandle());
			return this->pawn != nullptr;
		}

		CBaseHandle handle;
		CCSPlayerController* controller{ nullptr };
		CCSPlayerPawn* pawn{ nullptr };
	};

	class entity_cache final {
	public:
		entity_cache(const entity_cache&) = delete;
		entity_cache& operator=(const entity_cache&) = delete;

		entity_cache(entity_cache&&) noexcept = default;
		entity_cache& operator=(entity_cache&&) noexcept = default;

		entity_cache() = default;
		~entity_cache() = default;

		template<typename Func>
		void read_players(Func&& f) const {
			std::shared_lock lock(players_mutex);
			f(players);
		}

		template<typename Func>
		void write_players(Func&& f) {
			std::unique_lock lock(players_mutex);
			f(players);
		}

		[[nodiscard]] std::vector<cached_entity> get_players_snapshot() const {
			std::shared_lock lock(players_mutex);
			return players;
		}

		void clear_players() noexcept {
			std::unique_lock lock(players_mutex);
			players.clear();
		}

		void reserve_players(size_t size) {
			std::unique_lock lock(players_mutex);
			players.reserve(size);
		}

		void startup();
	private:
		mutable std::shared_mutex players_mutex;
		std::vector<cached_entity> players;
	};

	inline std::unique_ptr<entity_cache> g_entity_cache = std::make_unique<entity_cache>();
}
extern std::vector<player_object_t> m_player_object;

namespace visuals
{
	class c_visuals {
	public:
		static inline std::mutex players_mutex;
		static inline std::mutex weapons_mutex;

		void update(int mode);
		void update_players();
		void cache_players();
		void cache_weapons();
		void cache();
		void proccess();
		void add_hitmarker(int dmg);
		void proccess_bbox();
		bool processing_directx() {
			if (!ImGui::GetBackgroundDrawList())
				return false;

			return true;
		}
		static std::vector<item_object_t> m_get_cached_items() {
			std::lock_guard<std::mutex> lock(weapons_mutex);
			return m_items_object;
		}
		static void update_items(std::vector<item_object_t>&& new_item) {
			std::lock_guard<std::mutex> lock(weapons_mutex);
			m_items_object = std::move(new_item);
		}

		static std::vector<player_object_t> m_get_cached_players() {
			std::lock_guard<std::mutex> lock(players_mutex);
			return m_player_object;
		}

		static void update_players(std::vector<player_object_t>&& new_players) {
			std::lock_guard<std::mutex> lock(players_mutex);
			m_player_object = std::move(new_players);
		}
	};
	const auto visuals = std::make_unique<c_visuals>();
}


extern std::vector<object_t> m_entity_object;
extern std::vector<projectile_object_t> m_projectile_object;
extern std::vector< c_cached_weapons > m_weapons;
extern std::vector< c_cached_molotov > m_molotovs;

extern std::vector<c4_object_t> m_c4_object;

extern cachec_local m_local_player_data;

