#pragma once
#include "../../sdk/sdk.h"

namespace misc
{
	//void BunnyHop( CUserCmd* cmd, CBaseUserCmd* base_cmd );
}
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

enum e_egebug_type : int {
	pred = 0,
	trace = 1
};
namespace misc
{
	class c_movement {
	public:
		struct {
			bool on_ground;
			bool should_check;
			bool subtick;
		} m_jump[150];

		bool is_subtick = false;
		bool should_fake = false;

		struct c_data {
			CCSPlayerPawn* local;
			QAngle_t m_real_angle;
			CUserCmd* m_cmd;
		}m_data;

		/* post prediction */
		void setup_post_prediction(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base);
	
		struct c_edgebug {
			int type;
			bool state;
			bool unduck;
			bool jumped;
			 bool processing;
			 int tick;
			int amount = 0;
			int pred_ticks = 0;
			int mouse_offset = 0;
			int pred_time_stamp = 0;

		} m_edgebug;

		struct c_longjump {
			bool state;
		} m_longjump;

		struct c_edgejump {
			bool state;
		} m_edgejump;

		struct c_ladderjump {
			bool state;
		} m_ladderjump;

		struct c_minijump {
			bool state;
		} m_minijump;
		int m_scrolls{ };
		bool m_switcher{ true };
		int m_max_scrolls{ };
		void ladder_jump(CUserCmd* cmd, CBaseUserCmdPB* base);
		void edge_jump(CUserCmd* cmd, CBaseUserCmdPB* base);
		void edge_bug(CUserCmd* cmd, CBaseUserCmdPB* base);
		void long_jump(CUserCmd* cmd, CBaseUserCmdPB* base);
		void mini_jump(CUserCmd* cmd, CBaseUserCmdPB* base);
		void fast_ladder(CUserCmd* cmd, CBaseUserCmdPB* base);
		void quick_stop(CBaseUserCmdPB* base_cmd, int type = 0) noexcept;
		void change_speed(CBaseUserCmdPB* base_cmd, const float max_speed);
		void slow_walk(CBaseUserCmdPB* base_cmd) noexcept;
		void get_mouse_speed_limit(float& x, float& y);
		void movement_fix(CBaseUserCmdPB* cmd, const Vector_t view_angle) noexcept;
		void apply_sub_tick(CUserCmd* user_cmd, CBaseUserCmdPB* base);
		void sub_tick_strafe(CUserCmd* user_cmd, CBaseUserCmdPB* base);

		/* pre prediction */
		void setup(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base);

		void strafe(CUserCmd* cmd, CBaseUserCmdPB* base);
		void bunny_hop(CUserCmd* cmd, CBaseUserCmdPB* base);
		void proccess_jump(CCSGOInput* a1, CBaseUserCmdPB* crc, CUserCmd* main);
		/* setup correction stuff */
		void proccess_data(CUserCmd* cmd, CBaseUserCmdPB* base);
		void validate_command(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry);
		void correct(CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry, const QAngle_t& angDesiredViewPoint);


	};
	const auto movement = std::make_unique<c_movement>();
}