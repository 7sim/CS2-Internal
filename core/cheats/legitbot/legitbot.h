#pragma once

#include <memory>
#include <array>

#include "..\..\utils\render\render.h"

#include "..\..\utils\utils.h"
#include "..\..\sdk\sdk.h"
#include "..\..\math\elements\qangle.h"
#include "..\..\math\elements\vector.h"
#include "..\..\math\elements\transform.h"

#include "..\..\utils\configs\config.h"
#include "..\..\sdk\interfaces\game_event.h"
#include "..\backtrack\record.h"
#define max_bones 1024
struct KeyBind_t;
enum e_legit_conditions : unsigned int
{
	cond_none = 0U,
	cond_smoke = 1 << 0,
	cond_flashed = 1 << 1
};
enum e_legit_hitboxes : unsigned int
{
	none = 0U,
	head = 1 << 0,
	chest = 1 << 1,
	stomach = 1 << 2,
	legs = 1 << 3,
	feet = 1 << 4

};
namespace legitbot
{
	struct anim_record_t {
		Vector_t m_origin{};
		Vector_t m_eye_pos{};

		CCSPlayerPawn* m_pawn{};
		CSkeletonInstance* m_skeleton = nullptr;
		bone_data m_bone_data[128];
		float m_simulation_time{};
		Vector_t m_vec_mins{};
		Vector_t m_vec_maxs{};
		Vector_t m_vec_velocity{};

		bool m_throwing{};

		// backup
		Vector_t m_eye_pos_backup{};
		bone_data m_bone_data_backup[128];


		anim_record_t() {}

		inline anim_record_t(CCSPlayerPawn* pawn) {
			this->store(pawn);
		}

		inline void store(CCSPlayerPawn* pawn) {
			if (!pawn)
				return;

			if (!pawn->IsAlive())
				return;

			auto scene_node = pawn->m_pGameSceneNode();
			if (!scene_node)
				return;

			auto skeleton_instance = pawn->m_pGameSceneNode()->m_pSkeleton();
			if (!skeleton_instance)
				return;

			auto collision = pawn->m_pCollision();
			if (!collision)
				return;

			// saving fps
			//static auto setup_bones = reinterpret_cast<void(__fastcall*)(void*, int)>(utils::FindPattern("client.dll", "40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC D0"));

			m_pawn = pawn;

			m_skeleton = skeleton_instance;
			m_origin = scene_node->m_vecAbsOrigin();
			m_eye_pos = pawn->GetEyePosition();
			m_skeleton->setup_bones(FLAG_ALL_BONE_FLAGS);

			std::memcpy(m_bone_data, skeleton_instance->GetModel().hitbox_data(), sizeof(bone_data) * skeleton_instance->GetModel().m_hModel()->hitboxes_num());

			m_simulation_time = pawn->m_flSimulationTime();
			m_vec_mins = collision->m_vecMins();
			m_vec_maxs = collision->m_vecMaxs();
			m_vec_velocity = pawn->m_vecVelocity();
		}

		inline void apply(CCSPlayerPawn* pawn) {
			if (!pawn)
				return;

			if (!pawn->IsAlive())
				return;

			auto scene_node = pawn->m_pGameSceneNode();
			if (!scene_node)
				return;

			auto skeleton_instance = scene_node->m_pSkeleton();
			if (!skeleton_instance)
				return;

			m_eye_pos_backup = pawn->GetEyePosition();

			std::memcpy(m_bone_data_backup, skeleton_instance->GetModel().hitbox_data(), sizeof(bone_data) * skeleton_instance->GetModel().m_hModel()->hitboxes_num());

			std::memcpy(skeleton_instance->GetModel().hitbox_data(), m_bone_data, sizeof(bone_data) * skeleton_instance->GetModel().m_hModel()->hitboxes_num());
		}

		inline void reset(CCSPlayerPawn* pawn) {
			if (!pawn)
				return;

			if (!pawn->IsAlive())
				return;

			auto scene_node = pawn->m_pGameSceneNode();
			if (!scene_node)
				return;

			auto skeleton_instance = scene_node->m_pSkeleton();
			if (!skeleton_instance)
				return;

			std::memcpy(skeleton_instance->GetModel().hitbox_data(), m_bone_data_backup, sizeof(bone_data) * skeleton_instance->GetModel().m_hModel()->hitboxes_num());
		}

		float calculate_delta_time(int newest_tick);

		bool is_valid();
	};


	class c_hitbox
	{
	public:

		float m_idx;
		float m_damage;

		c_hitbox()
		{
			clear();
		}

		c_hitbox(const float& m_idx, const float& damage)
		{
			this->m_idx = m_idx;
			this->m_damage = damage;
		}

		void clear()
		{
			m_idx = 0.f;
			m_damage = 0.f;
		}
	};

	class c_tick
	{
	public:

		int m_tickcount;
		float m_tickrange;

		c_tick()
		{
			clear();
		}

		c_tick(const int& tickcount, const float& tickrange)
		{
			this->m_tickcount = tickcount;
			this->m_tickrange = tickrange;
		}

		void clear()
		{
			m_tickcount = 0;
			m_tickrange = 0.f;
		}
	};

	struct hitbox_data_t
	{
		hitbox_data_t() = default;

		int m_num_hitbox;
		int m_num_bone;
		int m_num_hitgroup;
		CHitBox* m_hitbox_data;
		float m_radius;
		Matrix3x4_t m_matrix;
		Vector_t m_mins;
		Vector_t m_maxs;
		Vector_t m_transformed_mins;
		Vector_t m_transformed_maxs;
		bool m_invalid_data{ true };
		bone_data* m_bone_cache;
	};

	class c_multi_aim_point {
	public:
		Vector_t pos;
		bool center;

		c_multi_aim_point()
		{
			clear();
		}

		c_multi_aim_point(const Vector_t& pos, const bool& center) //-V818 //-V730
		{
			this->pos = pos;
			this->center = center;
		}

		void clear()
		{
			pos = Vector_t(0, 0, 0);
			center = false;
		}
	};
	class c_aim_point
	{
	public:
		int idx;
		float dmg;
		bool canHit;

		Vector_t pos;
		QAngle_t angle2d;
		QAngle_t angle;
		
		float dist;

		c_aim_point()
		{
			clear();
		}

		c_aim_point(const Vector_t& pos, const QAngle_t& pointangle, const QAngle_t& angle, const int& idx, const float& damage, const bool& canHit, const float& m_crosshair_dist) //-V818 //-V730
		{
			this->pos = pos;
			this->angle2d = pointangle;
			this->angle = angle;
			this->idx = idx;
			this->dmg = damage;
			this->canHit = canHit;
			this->dist = m_crosshair_dist;
		}

		void clear()
		{
			pos = Vector_t(0, 0, 0);
			angle2d = QAngle_t(0, 0, 0);
			angle = QAngle_t(0, 0, 0);
			dist = 0.f;
			idx = -1;
			dmg = 0.f;
			canHit = false;
		}
	};

	class c_aim_data
	{
	public:
		bool visible;
		c_tick m_tick;
		c_hitbox m_hitbox;
		c_aim_point m_point;
		std::vector<c_aim_point> render_points;

		legitbot::anim_record_t* record;
		c_aim_data()
		{
			clear();
		}

		void clear()
		{
			visible = false;
			m_tick.clear();
			m_point.clear();
			m_hitbox.clear();
			record = nullptr;

			if (!render_points.empty())
				render_points.clear();
		}
		bool valid()
		{
			return m_point.angle.IsValid();
		}

	};

	class c_best_target
	{
	public:
		c_aim_data m_data;
		CBaseEntity* m_entity;
		CCSPlayerPawn* m_player_pawn;
		legitbot::anim_record_t* m_lag_entity;
		float m_distance;

		c_best_target() { clear(); }

		c_best_target(const c_best_target& data) //-V688
		{
			this->m_data = data.m_data;
			this->m_player_pawn = data.m_player_pawn;
			this->m_entity = data.m_entity;
			this->m_distance = data.m_distance;
			this->m_lag_entity = data.m_lag_entity;
		}

		c_best_target& operator=(const c_best_target& data) //-V688
		{
			this->m_data = data.m_data;
			this->m_entity = data.m_entity;
			this->m_player_pawn = data.m_player_pawn;
			this->m_distance = data.m_distance;
			this->m_lag_entity = data.m_lag_entity;
			return *this;
		}

		c_best_target(CBaseEntity* entity, CCSPlayerPawn* player, legitbot::anim_record_t* lag_entity, const float& distance, const c_aim_data& data)
		{
			this->m_data = data;
			this->m_entity = entity;
			this->m_player_pawn = player;
			this->m_distance = distance;
			this->m_lag_entity = lag_entity;
		}

		bool can_hit() {
			return this->m_data.m_hitbox.m_damage > 0 && this->m_data.m_point.angle.valid();
		}

		void clear()
		{
			m_data.clear();
			m_distance = 0.0f;
			m_entity = nullptr;
			m_player_pawn = nullptr;
			m_lag_entity = nullptr;
		}
	};

	class c_target
	{
	public:

		CBaseEntity* entity;
		CCSPlayerPawn* player;
		int handle;

		c_target()
		{
			player = nullptr;
			entity = nullptr;
			handle = 0;
		}

		c_target(CCSPlayerPawn* handle, CBaseEntity* entHandle, int hdl)
		{
			this->player = handle;
			this->entity = entHandle;
			this->handle = hdl;
		}

		record::record_list get_best_record(CUserCmd* cmd, CCSPlayerPawn* player, legitbot::anim_record_t* record);
		c_aim_data get_best_data(CUserCmd* cmd, CCSPlayerPawn* player, record::record_list record);
	};

	enum e_game_events : int {
		event_none = 0,
		round_start = 1,
		death = 2,
		kill = 3,
	};

	enum e_target_mode : int {
		players = 0,
		animals = 1,
		teammates = 2,
	};


	class c_events {
	public:
		void on_event(IGameEvent* ev, e_game_events type);
	};
	const auto events = std::make_unique<c_events>();


	class c_data {
	public:
		void store_records();

		void stop(CUserCmd* cmd);
		// select best target
		void search_best_target(QAngle_t view_angles);
		// loop & search for all targets 
		void search_targets(CUserCmd* cmd, e_target_mode mode);
		// self explainatory 
		void render_fov_circle();
		bool allowed();

		legitbot::anim_record_t* select_record(int handle);
		// start ragebot process all data find,scan,setup targets etc 
		bool trigger_bot(CUserCmd* cmd, CCSGOInput* input, CBaseUserCmdPB* base);

		void proccess(CUserCmd* cmd, CCSGOInput* input, CBaseUserCmdPB* base);
		// setup, reset all rage data
		void init();
		void add(Vector_t pos, bool center, std::vector<c_multi_aim_point>& point)
		{
			point.emplace_back(c_multi_aim_point(pos, center));
		}
		// peform smoothing 
		QAngle_t smooth(QAngle_t currentAngles, const QAngle_t& targetAngles);
		bool correct_punch(QAngle_t& adjusted);

		void get_best_data(CUserCmd* cmd, CCSPlayerPawn* player, legitbot::anim_record_t* records, c_aim_data& aim_data);
		bool hitchance(CBaseEntity* entity, QAngle_t vAimpoint);

		float get_damage(legitbot::anim_record_t* best_record, CCSPlayerPawn* target, CCSPlayerPawn* pLocal, Vector_t start, CCSWeaponBase* weapon, int id);
		float get_damage_crosshair(CBaseEntity* target, Vector_t start, Vector_t end);
		bool add_point(CBaseUserCmdPB* crc, CCSPlayerPawn* player, legitbot::anim_record_t* record, int index, std::vector<c_aim_point>& points);
	    hitbox_data_t get_hitbox_data(CCSPlayerPawn* player_pawn, const int hitbox_id);
		void renderize_fov();
		void menu_hitboxes();
		void render_multi_points();
		void menu_adaptive();
		// setup ragebot menu adaptive system
		struct c_adaptive {
			bool legit_enable;
			bool curve;
			float smooth_x;
			float smooth_y;
			float curve_x;
			float curve_y;
			float rcs_x;
			float rcs_y;
			bool rcs_control;
			bool smoothing;
			bool trigger_rcs;
			bool key_enabled;
			bool hitbox_head;
			bool hitbox_stomach;
			bool hitbox_legs;
			bool hitbox_feets;
			bool hitbox_chest;
			bool recoil_adjust;
			bool trigger_magnet;
			unsigned int hitboxes;
			unsigned int disable_conditions;
			int rcs_shots;
			int trigger_dmg;
			int trigger_delay;
			float trigger_hitchance;
			int trigger_shots;
			unsigned int trigger_hitbox;
			float accelaration;
			float fov;
			bool perfect_silent;
			bool trigger;
			bool trigger_on_key;
			int kill_delay;
			int head_scale;
			int body_scale;

		}adaptive;


		CUserCmd* m_cmd = nullptr;

		inline float crosshair_dist(const Vector_t& hitboxPos)
		{
			auto center =ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);
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

	};
	const auto data = std::make_unique<c_data>();
}
 extern KeyBind_t legit_key;

