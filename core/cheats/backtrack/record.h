#pragma once

#include <memory>
#include <array>

#include "..\..\utils\render\render.h"

#include "..\..\utils\utils.h"
#include "..\..\sdk\sdk.h"
#include "..\..\math\elements\qangle.h"
#include "..\..\math\elements\vector.h"
#include "..\..\math\elements\transform.h"
#include "..\..\sdk\handle\handle.hpp"

#include "..\..\utils\configs\config.h"
#include <deque>
#include <vector>
#include "..\..\sdk\interfaces\network.h"
#define max_bones 1024

namespace record
{
	struct c_tick
	{
	public:
		float m_tickcount;
		float m_tick_range;
	};

	struct c_lag_record
	{
	public:

		int m_flags;
		bool m_valid;
		float m_sim_time;
		Vector_t mins;
		Vector_t maxs;
		QAngle_t ang_eye;
		QAngle_t m_rot;
		QAngle_t m_abs_rot;
		Vector_t m_velocity;
		Vector_t m_abs_velocity;
		Vector_t m_abs_origin;
		Vector_t m_origin;
		Vector_t m_eye_pos;
		CCSPlayerPawn* m_player;


		std::array<bone_data, max_bones> m_data = {};
		float get_delta_time();

		struct backup_t {
			
			int m_flags;
			bool m_valid;
			float m_sim_time;
			Vector_t mins;
			Vector_t maxs;
			QAngle_t ang_eye;
			QAngle_t m_rot;
			QAngle_t m_abs_rot;
			Vector_t m_velocity;
			Vector_t m_abs_velocity;
			Vector_t m_abs_origin;
			Vector_t m_origin;
			Vector_t m_eye_pos;
			CCSPlayerPawn* m_player;
			std::array<bone_data, max_bones> m_data = {};
		};

		backup_t* backup;

	
		void validate() {
			this->m_valid = true;
		}
		void invalidate() {
			this->m_valid = false;
		}

		void init(CCSPlayerPawn* player);

		void setup(CCSPlayerPawn* player);

		void apply(CCSPlayerPawn* apply);

		bool is_valid();
	};

	struct record_list
	{
	public:
		c_tick* m_tick;
		c_lag_record* m_first_record;
		c_lag_record* m_last_record;
		float fract;
 	};

	class c_lag_entity {
	public:
		c_lag_entity(CBaseHandle handle) noexcept;

		void invalid() noexcept;
		void store(unsigned int index);
		const c_lag_record* get_last_record() noexcept;
		record::c_lag_record* get_last_sim_record() noexcept;
		const c_lag_record* get_first_record() noexcept;

		float get_distance(Vector_t p, Vector_t a, Vector_t b);

		float get_closest_distance(Vector_t start, Vector_t end, float last_valid_sim_time);
		
		record::record_list get_closest_record(Vector_t start, Vector_t end);

		record_list get_best_record(Vector_t start, Vector_t end);

		CBaseHandle handle;
		bool m_last_valid;
		CModel* model;
		uint32_t count;
		CCSPlayerPawn* m_player;
		std::vector<c_lag_record> m_records;
		CSkeletonInstance* skeleton;
		bool setup_bones(CCSPlayerPawn* player) {
	
			this->skeleton = player->m_pGameSceneNode()->m_pSkeleton();
			if (!this->skeleton) {
				invalid();
				return false;
			}

			auto& model_state = skeleton->GetModel();
			this->model = model_state.m_hModel();
			if (!this->model) {
				invalid();
				return false;
			}

			this->count = this->model->hitboxes_num();
			if (this->count == 0) {
				invalid();
				return false;
			}

			if (this->count > max_bones)
				this->count = max_bones;


			return true;
		}

		void validate() {
			this->m_last_valid = true;
		}

		void invalidate() {
			this->m_last_valid = false;
		}
		bool is_hitbox(int idx) {
			return (this->model->hitbox_flags(idx) & bone_flags::FLAG_HITBOX);
		}
		bool is_valid() {
			if (!this)
				return false;

			if (!this->m_last_valid || !this->m_player)
				return false;
			
			return this->m_player->IsAlive();
		}
	};

	class c_data 
	{
	public:
		void clear();
		void cache();
		void proccess(CUserCmd* cmd, CBaseUserCmdPB* base_cmd);
		void render_skeleton() noexcept;
		void pre(CCSInputMessage* input_message, CCSGOInputHistoryEntryPB* input_history, int tickcount, int fraction, bool aim, bool record);
		void post(CCSInputMessage* input_message, CCSGOInputHistoryEntryPB* input_history, int tickcount, int fraction, bool aim, bool record);
	};
	const auto data = std::make_unique<c_data>();
}
extern std::unordered_map<int, std::unique_ptr<record::c_lag_entity>> m_lag_records;

class c_bone
{
public:
	void copy();
	void set();
protected:
	bone_data matrix_[128];
};

class c_lagrecord
{
private:
	float m_simtime_;
public:
	bone_data matrix_[128];
private:
	bone_data backup_matrix_[128];
	int bone_count;
	Vector_t origin_;
public:
	void setup(CCSPlayerPawn* player_pawn);
	void apply(CCSPlayerPawn* player_pawn, bool extrapolation = false);
	void restore(CCSPlayerPawn* player_pawn);
	bool record_of_bounds();
public:
	float get_sim_time() { return m_simtime_; }
};

class c_animation_table
{
public:
	std::deque<c_lagrecord> m_lag_records_{};
	c_lagrecord latest_record{};
	c_lagrecord oldest_record{};
};

class c_lagcompensation
{
public:
	void run();
public:

	float get_delta_time(int tick_entity);
public:
	const c_lagrecord* get_last_record() noexcept;
	c_animation_table* get_animation(CCSPlayerPawn* player_pawn);
private:
	std::unordered_map<unsigned long, c_animation_table> m_animation{};
public:
	int level = 3;
};

inline auto g_lagcompensation = std::make_unique<c_lagcompensation>();