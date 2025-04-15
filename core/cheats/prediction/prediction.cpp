#include "prediction.h"

#include "..\..\utils\hook\hooks.h"

#include "..\..\sdk\interfaces\network.h" 
#include "..\..\sdk\interfaces\prediction.h" 

Vector_t get_extrapolated_position(Vector_t pos, Vector_t vel) {
	float ticks = 1.f;
	const float intervalpertick = 1.f / 64.f;

	CNetworkGameClient* networkGameClient = sdk::m_network->GetNetworkClient();

	if (!networkGameClient)
		return pos;

	auto netChan = networkGameClient->get_net_channel2();

	if (!netChan)
		return pos;

	float magic = static_cast<float>((netChan->get_network_latency() * 64.f) / 1000.f);

	return (pos)+(vel * intervalpertick * static_cast<float>(ticks)) + (vel * magic);
}

void c_eng_pred::start(CCSGOInput* input, CUserCmd* user_cmd) {
	if (!sdk::m_game_rules || !sdk::m_engine || !sdk::m_global_vars || !g::m_local_player_pawn || !g::m_local_player_controller || !g::m_active_weapon)
		return;

	if (!user_cmd || !input)
		return;

	if (!g::m_local_player_pawn->IsAlive() || sdk::m_game_rules->m_bFreezePeriod()  )
		return;

	auto client_info = sdk::m_engine->get_networked_client_info();
	if (!client_info)
		return;

	auto networked_data = client_info->m_local_data;
	if (!networked_data)
		return;

	auto network_client = sdk::m_network->GetNetworkClient();
	if (!network_client)
		return;

	auto movement = g::m_local_player_pawn->m_pMovementServices();
	if (!movement)
		return;

	auto wep_services = g::m_local_player_pawn->m_pWeaponServices();
	if (!wep_services)
		return;

	auto message_tick = input->Message;

	movement_data = movement;

	int v16 = 0; int new_tick = 0; int tick_count = 0; float tick_base_time = 0.f;

	/* store all predicted info there */
	m_pred_data.m_tick_count = sdk::m_global_vars->m_tickcount;
	m_pred_data.m_current_time = sdk::m_global_vars->m_curtime;
	m_pred_data.m_current_time2 = sdk::m_global_vars->m_curtime2;
	m_pred_data.m_real_time = sdk::m_global_vars->m_realtime;
	m_pred_data.m_frame_time = sdk::m_global_vars->m_frame_time;
	m_pred_data.m_absolute_frame_time = sdk::m_global_vars->m_absolute_frame_time;
	m_pred_data.m_absolute_frame_start_time_std_dev = sdk::m_global_vars->m_absolute_frame_start_time_std_dev;

	/* player data */
	m_pred_data.m_view_angle = input->vecViewAngle;
	m_pred_data.m_flags = g::m_local_player_pawn->m_fFlags();
	m_pred_data.m_tick_base = g::m_local_player_controller->m_nTickBase();
	m_pred_data.m_origin = g::m_local_player_pawn->m_pGameSceneNode()->m_vecAbsOrigin();

	// calculate accuracy speed shit
	const float speed{ (g_prediction->get_local_data()->m_velocity.Length2D()) };
	const float max_weapon_speed{ g::m_local_player_pawn->m_bIsScoped() ? g::m_weapon_data->m_flMaxSpeed().flValue[1] : g::m_weapon_data->m_flMaxSpeed().flValue[0] };
	const float max_accurate_speed{ 0.34f * max_weapon_speed };

	/* predicted velocity data */
	m_pred_data.m_max_accurate_velocity = max_accurate_speed;
	m_pred_data.m_velocity = g::m_local_player_pawn->m_vecVelocity();

	/* weapon data */
	if (auto active_weapon = g::m_local_player_pawn->GetActiveWeapon(); active_weapon) {

		active_weapon->update_accuracy();
		m_pred_data.m_spread = active_weapon->get_spread();
		m_pred_data.m_inaccuracy = active_weapon->get_inaccuracy();
		m_pred_data.m_next_primary_tick = active_weapon->GetNextPrimaryAttackTick();
		m_pred_data.m_next_attack = wep_services->m_flNextAttack();
	}

	/* store game cnetworked Info*/
	m_pred_data.m_eye_pos = networked_data->m_eye_pos;
	m_pred_data.m_player_tick = input->Message ? input->Message->m_nPlayerTickCount : 0;
	m_pred_data.m_player_tick_fraction = input->Message ? input->Message->m_flPlayerTickFraction : 0.f;
	m_pred_data.m_render_tick = input->Message ? input->Message->m_nFrameTickCount : 0;
	m_pred_data.m_render_tick_fraction = input->Message ? input->Message->m_flFrameTickFraction : 0.f;

	/* store prediction state */
	m_pred_data.m_in_prediction = sdk::m_prediction->InPrediction;
	m_pred_data.m_in_first_prediction = sdk::m_prediction->bFirstPrediction;
	m_pred_data.m_cmd_has_been_predicted = user_cmd->bHasBeenPredicted;
	m_pred_data.m_should_predict = network_client->m_in_prediction();

	// incremeant fps on low frame rate
	if (m_pred_data.m_player_tick_fraction > (1.f - 0.0099999998f)) {
		m_pred_data.m_tick_base++;
	}

	// set prediction state
	user_cmd->bHasBeenPredicted = false;
	sdk::m_prediction->bFirstPrediction = false;
	sdk::m_prediction->InPrediction = true;
	network_client->m_set_prediction(true);

	// set pred data
	hooks::m_set_pred_dada(movement, user_cmd);
	g::m_local_player_controller->current_command() = user_cmd;

	// run prediction
	if (network_client->should_predict && network_client->m_delta_tick() > 0) {
		hooks::m_run_prediction(network_client, 0);
	}
	/* run physics think  */
	//g::m_local_player_controller->run_physics_think();

	m_initialized = true;

}
#include "..\movement\setup.h"
void c_eng_pred::end(CCSGOInput* input,CBaseUserCmdPB* base, CUserCmd* cmd) {
	if (sdk::m_game_rules ||  !sdk::m_engine->is_valid() || !g::m_local_player_controller || !g::m_local_player_pawn || !sdk::m_global_vars)
		return;

	if (!m_initialized || !movement_data || !g::m_local_player_pawn->IsAlive() || !cmd || !base || sdk::m_game_rules->m_bFreezePeriod() )
		return;

	auto network_client = sdk::m_network->GetNetworkClient();
	if (!network_client)
		return;
	
	// reset prediction command
	hooks::m_reset_pred_data(movement_data);
	g::m_local_player_controller->current_command() = nullptr;

	/* restore all predicted info there */
	sdk::m_global_vars->m_curtime = m_pred_data.m_current_time;
	sdk::m_global_vars->m_curtime2 = m_pred_data.m_current_time2;
	sdk::m_global_vars->m_frame_time = m_pred_data.m_frame_time;
	sdk::m_global_vars->m_tickcount = m_pred_data.m_tick_count;
	sdk::m_global_vars->m_absolute_frame_time = m_pred_data.m_absolute_frame_time;
	sdk::m_global_vars->m_absolute_frame_start_time_std_dev = m_pred_data.m_absolute_frame_start_time_std_dev;
	input->vecViewAngle = m_pred_data.m_view_angle;
	/* restore tickbase */
	m_pred_data.m_shoot_tick = m_pred_data.m_tick_base + 1;
	g::m_local_player_controller->set_player_tickbase(m_pred_data.m_shoot_tick);
	
	sdk::m_prediction->bFirstPrediction = m_pred_data.m_in_first_prediction;
	sdk::m_prediction->InPrediction = m_pred_data.m_in_prediction;
	cmd->bHasBeenPredicted = m_pred_data.m_cmd_has_been_predicted;
    network_client->m_set_prediction(m_pred_data.m_should_predict);
}