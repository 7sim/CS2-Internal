#pragma once
#include "..\..\sdk\interfaces\csgoinput.h"
#include "..\..\sdk\addresses\entity.h"
#include "..\..\sdk\sdk.h"
class c_eng_pred {
	struct c_local_data {
		float m_real_time{}, m_absolute_frame_time{}, m_absolute_frame_start_time_std_dev{}, m_spread{}, m_inaccuracy{}, m_player_tick_fraction{}, m_render_tick_fraction{}, m_next_attack{}, m_max_accurate_velocity{};
		int m_current_time{}, m_flags{}, m_current_time2{}, m_tick_count{}, m_frame_time{}, m_tick_base{}, m_player_tick{}, m_render_tick{}, m_shoot_tick{}, m_next_primary_tick{};
		Vector_t m_velocity{}, m_eye_pos{}, m_view_angle{}, m_origin{};
		
		bool m_in_prediction{};
		bool m_in_first_prediction{};
		bool m_cmd_has_been_predicted{};
		bool m_should_predict{};

	} m_pred_data{};

	CCSPlayer_MovementServices* movement_data{};
	bool m_initialized = false;
public:
	void start(CCSGOInput* input, CUserCmd* cmd);
	void end(CCSGOInput* input, CBaseUserCmdPB* base, CUserCmd* cmd);

	c_local_data* get_local_data() {
		return &m_pred_data;
	}

	CCSPlayer_MovementServices* get_movement_data() {
		return movement_data;
	}
};

inline const auto g_prediction = std::make_unique<c_eng_pred>();
