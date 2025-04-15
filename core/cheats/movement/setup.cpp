#include "setup.h"
#include "../../utils/configs/config.h"
#include "..\prediction\prediction.h"
#include "../../sdk/interfaces/trace.h"
#include "..\..\sdk\interfaces\cvar.h"
#include "..\subtick\subtick.h"
 int a = 0;

 __forceinline static float angle_normalize(float angle)
 {
	 angle = fmodf(angle, 360.0f);
	 if (angle > 180)
	 {
		 angle -= 360;
	 }
	 if (angle < -180)
	 {
		 angle += 360;
	 }
	 return angle;
 }


 float DeltaAngle(float first, float second)
 {
	 const float delta = first - second;
	 float res = std::isfinite(delta) ? std::remainder(delta, 6.283f) : 0.0f;

	 if (first > second)
	 {
		 if (res >= 3.1415f)
			 res -= 6.283f;
	 }
	 else
	 {
		 if (res <= -3.1415f)
			 res += 6.283f;
	 }

	 return res;
 };
 float CalcualteDelta(float m_speed)
 {
	 const static float maxSpeed = 300.f;
	 const static auto sv_airaccelerate = sdk::m_cvar->find(hash_32_fnv1a_const("sv_airaccelerate"))->value.fl;
	 const float term = 50.f / sv_airaccelerate / maxSpeed * 100.0f / m_speed;

	 if (term < 1.0f && term > -1.0f)
		 return std::acos(term);

	 return 0.0f;
 };
 float get_max_player_speed()
 {
	 if (!g::m_active_weapon)
		 return 260.f;

	 bool scoped = g::m_local_player_pawn->m_bIsScoped();

	 return scoped ? g::m_weapon_data->m_flMaxSpeed().flValue[1] : g::m_weapon_data->m_flMaxSpeed().flValue[0];
 }

 Vector_t PredictVelocity(const Vector_t& current_velocity, float friction, float frame_time) {
	 Vector_t new_velocity = current_velocity;

	 float speed = new_velocity.Length();
	 if (speed > 0.0f) {
		 float drop = speed * friction * frame_time;
		 new_velocity *= std::max(speed - drop, 0.0f) / speed;
	 }

	 float max_speed = get_max_player_speed();
	 if (new_velocity.Length() > max_speed) {
		 new_velocity.NormalizeInPlace();
		 new_velocity *= max_speed;
	 }

	 return new_velocity;
 }

 void reversed_sub_tick_strafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSPlayerPawn* pLocalPawn)
 {

	 if (!cfg_get(bool, g_cfg.auto_strafe))
		 return;

	 if (!pCmd || !pUserCmd)
		 return;

	 if (!pUserCmd->view_angles || !pUserCmd->pInButtonState)
		 return;

	 // Only strafe when in air
	 if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
		 return;

	 static uint64_t last_pressed = 0;
	 static uint64_t last_buttons = 0;

	 float frame_time{};

	 const auto current_buttons = pCmd->nButtons.nValue;
	 float yaw{};
	 if (pUserCmd->view_angles)
		 yaw = math::normalize_yaw(pUserCmd->view_angles->angValue.y)/*pUserCmd->pViewAngles->angValue.y)*/;

	 const auto check_button = [&](const uint64_t button)
	 {
		 if (current_buttons & button && (!(last_buttons & button) || button & IN_MOVELEFT && !(last_pressed & IN_MOVERIGHT) || button & IN_MOVERIGHT && !(last_pressed & IN_MOVELEFT) || button & IN_FORWARD && !(last_pressed & IN_BACK) || button & IN_BACK && !(last_pressed & IN_FORWARD)))
		 {
			 if (button & IN_MOVELEFT)
				 last_pressed &= ~IN_MOVERIGHT;
			 else if (button & IN_MOVERIGHT)
				 last_pressed &= ~IN_MOVELEFT;
			 else if (button & IN_FORWARD)
				 last_pressed &= ~IN_BACK;
			 else if (button & IN_BACK)
				 last_pressed &= ~IN_FORWARD;

			 last_pressed |= button;
		 }
		 else if (!(current_buttons & button))
			 last_pressed &= ~button;
	 };

	 check_button(IN_MOVELEFT);
	 check_button(IN_MOVERIGHT);
	 check_button(IN_FORWARD);
	 check_button(IN_BACK);

	 last_buttons = current_buttons;

	 Vector_t velocity = pLocalPawn->m_vecAbsVelocity();
	 Vector_t move{pLocalPawn->m_pMovementServices()->m_vecLastMovementImpulses() };
	 const float friction = pLocalPawn->m_pMovementServices()->m_flSurfaceFriction();

	 if (pUserCmd->subtickMovesField.pRep)
	 {
		 for (auto i = 0; i < pUserCmd->subtickMovesField.pRep->nAllocatedSize; i++)
		 {
			 frame_time = g::interval / static_cast<float>(i);
			 velocity = PredictVelocity(velocity, friction, frame_time);
		 }
	 }

	 auto offset = 0.f;
	 if (last_pressed & IN_MOVELEFT)
		 offset += 90.f;
	 if (last_pressed & IN_MOVERIGHT)
		 offset -= 90.f;
	 if (last_pressed & IN_FORWARD)
		 offset *= 0.5f;
	 else if (last_pressed & IN_BACK)
		 offset = -offset * 0.5f + 180.f;

	 yaw += offset;

	 auto velocity_angle = M_RAD2DEG(std::atan2f(velocity.y, velocity.x));
	 if (velocity_angle < 0.0f)
		 velocity_angle += 360.0f;

	 if (velocity_angle < 0.0f)
		 velocity_angle += 360.0f;

	 velocity_angle -= floorf(velocity_angle / 360.0f + 0.5f) * 360.0f;

	 const auto speed = velocity.Length2D();
	 auto ideal = 0.f;

	 if (speed > 0.f)
		 ideal = math::Clamp(M_RAD2DEG(std::atan2(15.f, speed)), 0.0f, 90.0f);

	 auto correct = (100.f - cfg_get(float, g_cfg.auto_strafe_smooth)) * 0.02f * (ideal + ideal);

	 pUserCmd->flForwardMove = 0.f;
	 pUserCmd->bit(BASE_BITS_FORWARDMOVE);
	 const auto velocity_delta = math::normalize_yaw(yaw - velocity_angle);

	 auto rotate_movement = [](CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, float target_yaw)
	 {
		 const float rot = M_DEG2RAD(pUserCmd->view_angles->angValue.y - target_yaw);

		 const float new_forward = std::cos(rot) * pUserCmd->flForwardMove - std::sin(rot) * pUserCmd->flSideMove;
		 const float new_side = std::sin(rot) * pUserCmd->flForwardMove + std::cos(rot) * pUserCmd->flSideMove;

		 pCmd->nButtons.nValue &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);
		 pCmd->nButtons.nValueChanged &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

		 pUserCmd->flForwardMove = math::Clamp(new_forward, -1.f, 1.f);
		 pUserCmd->bit(BASE_BITS_FORWARDMOVE);
		 pUserCmd->flSideMove = math::Clamp(new_side * -1.f, -1.f, 1.f);
		 pUserCmd->bit(BASE_BITS_LEFTMOVE);

		 if (pUserCmd->flForwardMove > 0.0f)
		 {
			 //	 sdk::m_csgo_input->add_button(IN_FORWARD);
			 pUserCmd->pInButtonState->nValue |= IN_FORWARD;
			 pUserCmd->pInButtonState->bit(IN_FORWARD);
		 }
		 else if (pUserCmd->flForwardMove < 0.0f)
		 {
			 // sdk::m_csgo_input->add_button(IN_BACK);
			 pUserCmd->pInButtonState->nValue |= IN_BACK;
			 pUserCmd->pInButtonState->bit(IN_BACK);
		 }
		 if (pUserCmd->flSideMove > 0.0f)
		 {
			 // sdk::m_csgo_input->add_button(IN_MOVELEFT);
			 pUserCmd->pInButtonState->nValue |= IN_MOVELEFT;
			 pUserCmd->pInButtonState->bit(IN_MOVELEFT);
		 }
		 else if (pUserCmd->flSideMove < 0.0f)
		 {
			 // sdk::m_csgo_input->add_button(IN_MOVERIGHT);
			 pUserCmd->pInButtonState->nValue |= IN_MOVERIGHT;
			 pUserCmd->pInButtonState->bit(IN_MOVERIGHT);
		 }
	 };

	 if (fabsf(velocity_delta) > 170.0f && speed > 80.0f || velocity_delta > correct && speed > 80.0f)
	 {
		 yaw = correct + velocity_angle;
		 pUserCmd->flSideMove = -1.0f;
		 pUserCmd->bit(BASE_BITS_LEFTMOVE);
		 rotate_movement(pCmd, pUserCmd, math::normalize_yaw(yaw));
		 return;
	 }
	 static bool m_switch_value;
	 m_switch_value = !m_switch_value;

	 if (-correct <= velocity_delta || speed <= 80.f)
	 {
		 if (m_switch_value)
		 {
			 yaw = yaw - ideal;
			 pUserCmd->flSideMove = -1.0f;
			 pUserCmd->bit(BASE_BITS_LEFTMOVE);
		 }
		 else
		 {
			 yaw = ideal + yaw;
			 pUserCmd->flSideMove = 1.0f;
			 pUserCmd->bit(BASE_BITS_LEFTMOVE);
		 }
	 }
	 else
	 {
		 yaw = velocity_angle - correct;
		 pUserCmd->flSideMove = 1.0f;
		 pUserCmd->bit(BASE_BITS_LEFTMOVE);
	 }

	 rotate_movement(pCmd, pUserCmd, math::normalize_yaw(yaw));
	 if (!cfg_get(bool, g_cfg.auto_strafe_wasd))
		 return;

	 if (!pUserCmd->subtickMovesField.pRep)
		 return;
	 for (auto i = 0; i < 12; i++)

	 {
		 CSubtickMoveStep* Subtick = pCmd->create_subtick();
		 if (Subtick) {
			 Subtick->flAnalogForwardDelta = pUserCmd->flForwardMove - move.x;
			 Subtick->bit(ESubtickMoveStepBits::MOVESTEP_BITS_ANALOG_FORWARD_DELTA);

			 Subtick->flAnalogLeftDelta = pUserCmd->flSideMove - move.y;
			 Subtick->bit(ESubtickMoveStepBits::MOVESTEP_BITS_ANALOG_LEFT_DELTA);

			 move.x += Subtick->flAnalogForwardDelta;
			 move.y += Subtick->flAnalogLeftDelta;

			 Subtick->nButton |= IN_SPRINT;
			 Subtick->bit(ESubtickMoveStepBits::MOVESTEP_BITS_BUTTON);

			 Subtick->flWhen = (1.f / 12.f) * (i);
			 Subtick->bit(ESubtickMoveStepBits::MOVESTEP_BITS_WHEN);

		 }
	 }
	 
 }

 void Strafer(CUserCmd* userCmd, CBaseUserCmdPB* baseUserCmd, Vector_t& oldAngle) {
	 if (!cfg_get(bool, g_cfg.auto_strafe))
		 return;

	 auto localPlayer = g::m_local_player_pawn;
	 if (!localPlayer)
		 return;


	 if (localPlayer->m_fFlags() & FL_ONGROUND)
		 return;

	 if (userCmd->nButtons.nValue & IN_SPRINT)
		 return;

	 const float speed = localPlayer->m_vecVelocity().Length();

	 const float deltaInAir = CalcualteDelta(speed);

	 if (deltaInAir == 0.0f)
		 return;
	 auto lastMoveInputsPtr = reinterpret_cast<uintptr_t*>(g::m_local_player_pawn->m_pMovementServices());
	 Vector_t last_move_inputs = *reinterpret_cast<Vector_t*>(*(lastMoveInputsPtr)+0x1B8);
	 float yaw = M_DEG2RAD(g::angle.y);
	 auto& vecVelocity = localPlayer->m_vecVelocity();
	 float velocityDirection = std::atan2(vecVelocity.y, vecVelocity.x) - yaw;
	 float bestAngleMove = std::atan2(baseUserCmd->flSideMove, baseUserCmd->flForwardMove);

	 float finalDelta = DeltaAngle(velocityDirection, bestAngleMove);
	 float finalMove = finalDelta < 0.0f ? velocityDirection + deltaInAir : velocityDirection - deltaInAir;

	 // Adjust the final movement based on last movement impulses
	 float adjustedForwardMove = (std::cos(finalMove) + last_move_inputs.x);
	 float adjustedSideMove = (std::sinf(finalMove) + last_move_inputs.y);

	 // Normalize the adjusted moves if necessary
	 float adjustedLength = std::sqrt(adjustedForwardMove * adjustedForwardMove + adjustedSideMove * adjustedSideMove);
	 if (adjustedLength > 2.0f) { // Assuming max speed is 2.0f
		 adjustedForwardMove = (adjustedForwardMove / adjustedLength) * 2.0f;
		 adjustedSideMove = (adjustedSideMove / adjustedLength) * 2.0f;
	 }

	 // Update base user command with adjusted moves
	 baseUserCmd->bit(CachedBits::BASE_BITS_FORWARDMOVE);
	 baseUserCmd->flForwardMove = adjustedForwardMove;
	 baseUserCmd->bit(CachedBits::BASE_BITS_LEFTMOVE);
	 baseUserCmd->flSideMove = adjustedSideMove;

	 // Add subticks
	 for (auto i = 0; i < 12; i++) {
		 if (const auto& subtick = baseUserCmd->create_new_subtick_step(); subtick)
		 {
			 subtick->bit(MOVESTEP_BITS_WHEN);
			 subtick->flWhen = ((1.f / 12.f) * (i));

			 subtick->bit(MOVESTEP_BITS_ANALOG_FORWARD_DELTA);
			 subtick->flAnalogForwardDelta = adjustedForwardMove;

			 subtick->bit(MOVESTEP_BITS_ANALOG_LEFT_DELTA);
			 subtick->flAnalogLeftDelta = adjustedSideMove;

			 input_history::AddSubtick(subtick->nButton, subtick->bPressed, subtick->flWhen, subtick->flAnalogForwardDelta, subtick->flAnalogLeftDelta);
		 }
	 }
 }

 bool strafeee(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base) {
	 static uint64_t last_pressed = 0;
	 static uint64_t last_buttons = 0;
	 static bool m_switch_value = false;
	 static float m_old_yaw;

	 if (!cfg_get(bool, g_cfg.auto_strafe))
		 return false;


	 if (!cmd || !base)
		 return false;

	 if (!base->view_angles || !base->pInButtonState)
		 return false;

	 // Only strafe when in air
	 if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
		 return false;



	 const auto current_buttons = cmd->nButtons.nValue;
	 auto yaw = angle_normalize(g::angle.y);

	 // Check button states (WASD movement keys)
	 const auto check_button = [&](const uint64_t button) {
		 if (current_buttons & button && (!(last_buttons & button) ||
			 (button & IN_MOVELEFT && !(last_pressed & IN_MOVERIGHT)) ||
			 (button & IN_MOVERIGHT && !(last_pressed & IN_MOVELEFT)) ||
			 (button & IN_FORWARD && !(last_pressed & IN_BACK)) ||
			 (button & IN_BACK && !(last_pressed & IN_FORWARD)))) {

			 if (button & IN_MOVELEFT) last_pressed &= ~IN_MOVERIGHT;
			 else if (button & IN_MOVERIGHT) last_pressed &= ~IN_MOVELEFT;
			 else if (button & IN_FORWARD) last_pressed &= ~IN_BACK;
			 else if (button & IN_BACK) last_pressed &= ~IN_FORWARD;

			 last_pressed |= button;
		 }
		 else if (!(current_buttons & button))
			 last_pressed &= ~button;
	 };

	 // Process WASD key inputs
	 check_button(IN_MOVELEFT);
	 check_button(IN_MOVERIGHT);
	 check_button(IN_FORWARD);
	 check_button(IN_BACK);

	 last_buttons = current_buttons;

	 // Get current player velocity
	 const auto velocity = g::m_local_player_pawn->m_vecAbsVelocity();

	 // No movement if player is on the ground
	 if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
		 return false;

	 // Calculate the movement offset based on pressed keys
	 float offset = 0.f;
	 if (last_pressed & IN_MOVELEFT) offset += 90.f;
	 if (last_pressed & IN_MOVERIGHT) offset -= 90.f;
	 if (last_pressed & IN_FORWARD) offset *= 0.5f;
	 else if (last_pressed & IN_BACK) offset = -offset * 0.5f + 180.f;

	 yaw += offset;

	 // Calculate the velocity direction
	 float velocity_angle = atan2f(velocity.y, velocity.x) * 57.295779513082320876798154f;
	 if (velocity_angle < 0.0f) {
		 velocity_angle += 360.0f;
	 }


	 // Normalize velocity angle between 0 and 360
	 velocity_angle -= floorf(velocity_angle / 360.0f + 0.5f) * 360.0f;

	 // Calculate speed and ideal correction
	 const auto speed = velocity.LengthSqr();
	 auto ideal = 0.f;

	 if (speed > 0.f) {
		 ideal = atan2(15.0f, speed) * 57.295779513082320876798154f;
		 ideal = std::clamp(ideal, 0.0f, 90.0f);
	 }

	 const auto correct = (100.f - cfg_get(float, g_cfg.auto_strafe_smooth)) * 0.02f * (ideal + ideal);

	 // Set movement defaults
	 base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
	 base->flForwardMove = 0.f;
	 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
	 base->flSideMove = 0.f;
	 const auto velocity_delta = angle_normalize(yaw - velocity_angle);

	 // Determine the yaw adjustment needed
	 auto delta = angle_normalize(yaw - m_old_yaw);
	 auto abs_delta = std::abs(delta);

	 // Update yaw based on velocity and movement conditions
	 m_old_yaw = yaw;
	 auto rotate_movement = [](CUserCmd* cmd, CBaseUserCmdPB* base, float target_yaw) {
		 auto test = g::angle.y;
		 const float rot = M_DEG2RAD(test - target_yaw);

		 const float new_forward = std::cos(rot) * base->flForwardMove - std::sin(rot) * base->flSideMove;
		 const float new_side = std::sin(rot) * base->flForwardMove + std::cos(rot) * base->flSideMove;

		 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		 base->pInButtonState->nValue &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

		 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE2);
		 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE2);
		 base->pInButtonState->nValueChanged &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

		 base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
		 base->flForwardMove = std::clamp(new_forward, -1.f, 1.f);
		 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
		 base->flSideMove = std::clamp(new_side * -1.f, -1.f, 1.f);

		 if (base->flForwardMove > 0.f) {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_FORWARD;
		 }
		 else if (base->flForwardMove < 0.f)
		 {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_BACK;
		 }

		 if (base->flSideMove > 0.f) {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_MOVELEFT;
		 }
		 else if (base->flSideMove < 0.f) {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_MOVERIGHT;
		 }
	 };

	 // Switch strafe direction based on conditions
	 m_switch_value = !m_switch_value;

	 if (fabsf(velocity_delta) > 170.f && speed > 80.f || velocity_delta > correct && speed > 80.f) {
		 yaw = correct + velocity_angle;
		 base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
		 base->flSideMove = -1.f;
		 rotate_movement(cmd, base, angle_normalize(yaw));
		 return false;
	 }

	 if (-correct <= velocity_delta || speed <= 80.f) {
		 if (m_switch_value) {
			 yaw = yaw - ideal;
			 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
			 base->flSideMove = -1.f;
		 }
		 else {
			 yaw = ideal + yaw;
			 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
			 base->flSideMove = 1.f;
		 }
	 }
	 else {
		 yaw = velocity_angle - correct;
		 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
		 base->flSideMove = 1.f;
	 }


	 rotate_movement(cmd, base, angle_normalize(yaw));

	 return true;

	 if (!base->subtickMovesField.pRep)
		 return false;

	 if (cfg_get(bool, g_cfg.auto_strafe_wasd))
		 return false;

	 _log(LOG_INFO) << " added";
	 for (auto i = 0; i < 12; i++)
	 {
		 CSubtickMoveStep* current_sub_tick = nullptr;

		 if (!current_sub_tick)
			 current_sub_tick = cmd->create_subtick();

		 if (!current_sub_tick)
			 continue;

		 current_sub_tick->bit(MOVESTEP_BITS_ANALOG_FORWARD_DELTA);
		 current_sub_tick->flAnalogForwardDelta = base->flForwardMove;
		 current_sub_tick->bit(MOVESTEP_BITS_ANALOG_LEFT_DELTA);
		 current_sub_tick->flAnalogLeftDelta = base->flSideMove;

		 current_sub_tick->bit(MOVESTEP_BITS_BUTTON);
		 current_sub_tick->nButton |= IN_SPRINT;

		 auto fl_when = (1.f / 12.f) * (i);
		 current_sub_tick->bit(MOVESTEP_BITS_WHEN);
		 current_sub_tick->flWhen = fl_when;

		 input_history::AddSubtick(IN_SPRINT, true, fl_when, base->flForwardMove, base->flSideMove);
		 input_history::Run(input, cmd, base);
	 }

 }


 void auto_strafe(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base) {
	 static uint64_t last_pressed = 0;
	 static uint64_t last_buttons = 0;
	 static bool m_switch_value = false;
	 static float m_old_yaw;

	 if (!cfg_get(bool, g_cfg.auto_strafe))
		 return;


	 if (!cmd || !base)
		 return;

	 if (!base->view_angles || !base->pInButtonState)
		 return;

	 // Only strafe when in air
	 if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
		 return;

	 const auto current_buttons = cmd->nButtons.nValue;
	 auto yaw = angle_normalize(g::angle.y);

	 // Check button states (WASD movement keys)
	 const auto check_button = [&](const uint64_t button) {
		 if (current_buttons & button && (!(last_buttons & button) ||
			 (button & IN_MOVELEFT && !(last_pressed & IN_MOVERIGHT)) ||
			 (button & IN_MOVERIGHT && !(last_pressed & IN_MOVELEFT)) ||
			 (button & IN_FORWARD && !(last_pressed & IN_BACK)) ||
			 (button & IN_BACK && !(last_pressed & IN_FORWARD)))) {

			 if (button & IN_MOVELEFT) last_pressed &= ~IN_MOVERIGHT;
			 else if (button & IN_MOVERIGHT) last_pressed &= ~IN_MOVELEFT;
			 else if (button & IN_FORWARD) last_pressed &= ~IN_BACK;
			 else if (button & IN_BACK) last_pressed &= ~IN_FORWARD;

			 last_pressed |= button;
		 }
		 else if (!(current_buttons & button))
			 last_pressed &= ~button;
	 };

	 // Process WASD key inputs
	 check_button(IN_MOVELEFT);
	 check_button(IN_MOVERIGHT);
	 check_button(IN_FORWARD);
	 check_button(IN_BACK);

	 last_buttons = current_buttons;

	 // Get current player velocity
	 const auto velocity = g::m_local_player_pawn->m_vecAbsVelocity();

	 // No movement if player is on the ground
	 if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
		 return;

	 // Calculate the movement offset based on pressed keys
	 float offset = 0.f;
	 if (last_pressed & IN_MOVELEFT) offset += 90.f;
	 if (last_pressed & IN_MOVERIGHT) offset -= 90.f;
	 if (last_pressed & IN_FORWARD) offset *= 0.5f;
	 else if (last_pressed & IN_BACK) offset = -offset * 0.5f + 180.f;

	 yaw += offset;

	 // Calculate the velocity direction
	 float velocity_angle = atan2f(velocity.y, velocity.x) * 57.295779513082320876798154f;
	 if (velocity_angle < 0.0f) {
		 velocity_angle += 360.0f;
	 }


	 // Normalize velocity angle between 0 and 360
	 velocity_angle -= floorf(velocity_angle / 360.0f + 0.5f) * 360.0f;

	 // Calculate speed and ideal correction
	 const auto speed = velocity.LengthSqr();
	 auto ideal = 0.f;

	 if (speed > 0.f) {
		 ideal = atan2(15.0f, speed) * 57.295779513082320876798154f;
		 ideal = std::clamp(ideal, 0.0f, 90.0f);
	 }

	 const auto correct = (100.f - cfg_get(float, g_cfg.auto_strafe_smooth)) * 0.02f * (ideal + ideal);

	 // Set movement defaults
	 base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
	 base->flForwardMove = 0.f;
	 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
	 base->flSideMove = 0.f;
	 const auto velocity_delta = angle_normalize(yaw - velocity_angle);

	 // Determine the yaw adjustment needed
	 auto delta = angle_normalize(yaw - m_old_yaw);
	 auto abs_delta = std::abs(delta);

	 // Update yaw based on velocity and movement conditions
	 m_old_yaw = yaw;
	 auto rotate_movement = [](CUserCmd* cmd, CBaseUserCmdPB* base, float target_yaw) {
		 auto test = g::angle.y;
		 const float rot = M_DEG2RAD(test - target_yaw);

		 const float new_forward = std::cos(rot) * base->flForwardMove - std::sin(rot) * base->flSideMove;
		 const float new_side = std::sin(rot) * base->flForwardMove + std::cos(rot) * base->flSideMove;

		 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		 base->pInButtonState->nValue &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

		 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE2);
		 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE2);
		 base->pInButtonState->nValueChanged &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

		 base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
		 base->flForwardMove = std::clamp(new_forward, -1.f, 1.f);
		 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
		 base->flSideMove = std::clamp(new_side * -1.f, -1.f, 1.f);

		 if (base->flForwardMove > 0.f) {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_FORWARD;
		 }
		 else if (base->flForwardMove < 0.f)
		 {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_BACK;
		 }

		 if (base->flSideMove > 0.f) {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_MOVELEFT;
		 }
		 else if (base->flSideMove < 0.f) {
			 base->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
			 base->pInButtonState->nValue |= IN_MOVERIGHT;
		 }
	 };

	 // Switch strafe direction based on conditions
	 m_switch_value = !m_switch_value;

	 if (fabsf(velocity_delta) > 170.f && speed > 80.f || velocity_delta > correct && speed > 80.f) {
		 yaw = correct + velocity_angle;
		 base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
		 base->flSideMove = -1.f;
		 rotate_movement(cmd, base, angle_normalize(yaw));
		 return;
	 }

	 if (-correct <= velocity_delta || speed <= 80.f) {
		 if (m_switch_value) {
			 yaw = yaw - ideal;
			 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
			 base->flSideMove = -1.f;
		 }
		 else {
			 yaw = ideal + yaw;
			 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
			 base->flSideMove = 1.f;
		 }
	 }
	 else {
		 yaw = velocity_angle - correct;
		 base->bit(CachedBits::BASE_BITS_LEFTMOVE);
		 base->flSideMove = 1.f;
	 }


	 rotate_movement(cmd, base, angle_normalize(yaw));

	 if (!base->subtickMovesField.pRep)
		 return;

	 if (cfg_get(bool, g_cfg.auto_strafe_wasd))
		 return;

	 _log(LOG_INFO) << " added";
	 for (auto i = 0; i < 12; i++)
	 {
		 CSubtickMoveStep* current_sub_tick = nullptr;
	
		 if (!current_sub_tick)
			 current_sub_tick = cmd->create_subtick();

		 if (!current_sub_tick)
			 continue;

		 current_sub_tick->bit(MOVESTEP_BITS_ANALOG_FORWARD_DELTA);
		 current_sub_tick->flAnalogForwardDelta = base->flForwardMove;
		 current_sub_tick->bit(MOVESTEP_BITS_ANALOG_LEFT_DELTA);
		 current_sub_tick->flAnalogLeftDelta = base->flSideMove;

		 current_sub_tick->bit(MOVESTEP_BITS_BUTTON);
		 current_sub_tick->nButton |= IN_SPRINT;

		 auto fl_when = (1.f / 12.f) * (i);
		 current_sub_tick->bit(MOVESTEP_BITS_WHEN);
		 current_sub_tick->flWhen = fl_when;

		 input_history::AddSubtick(IN_SPRINT, true, fl_when, base->flForwardMove, base->flSideMove);
		 input_history::Run(input, cmd, base);
	 }
	
 }



 /*
this is my autostrafer w subtick strafe for cs2 as you see it works perfectly fine but my problem is that the subtick only applys to yaw direction and not detect if i press for e.x D to put  "yaw" as if i got to D direction idk if its 90 or -90 but u got the ideia*/
/* run prediction movement */
 void misc::c_movement::setup_post_prediction(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* crc) {

	 if (!cmd || !crc || !input)
		 return;

	 auto local = g::m_local_player_pawn;
	 if (!local)
		 return;

	 if (!local->IsAlive() || !local->valid_move())
		 return;

	// Strafer(cmd, crc, g::angle);

 }
 Vector_t predict_velocity(Vector_t pos, Vector_t vel, float frame_time) {
	 float ticks = 1.f;
	 const float intervalpertick = 1.f / 64.f;

	 CNetworkGameClient* networkGameClient = sdk::m_network->GetNetworkClient();

	 if (!networkGameClient)
		 return pos;

	 auto netChan = networkGameClient->get_net_channel2();

	 if (!netChan)
		 return pos;

	 float magic = static_cast<float>((netChan->get_network_latency() * 64.f) / 1000.f);

	 return (vel * frame_time);
 }


void misc::c_movement::setup(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base)
{
	if (!cmd || !base)
		return;

	auto local = g::m_local_player_pawn;
	if (!local)
		return;

	if (!local->IsAlive() || !local->valid_move())
		return;

	if (cfg_get(bool, g_cfg.hand_switch)) {

		auto wep = local->GetActiveWeapon();
		if (wep) {
			auto data = wep->GetWeaponData();
			if (data) {
				bool knife = data->m_WeaponType() == WEAPONTYPE_KNIFE;
				 
					cmd->bit(CSGOUSERCMD_BITS_LEFTHAND);
					cmd->m_bLeftHandDesired = knife;
				
				
			}
		}
	}
//	apply_sub_tick(cmd, base);


	reversed_sub_tick_strafe(cmd, base, g::m_local_player_pawn);

	proccess_jump(input, base, cmd);

}
void misc::c_movement::proccess_jump(CCSGOInput* a1, CBaseUserCmdPB* crc, CUserCmd* cmd)
{
	if (!cfg_get(bool, g_cfg.bunny_hop))
		return;

	if (!sdk::m_engine->connected() || !sdk::m_engine->in_game())
		return;

	if (!crc || !cmd || g::sv_autobunnyhopping)
		return;

	if (!crc->pInButtonState)
		return;
	/*
	if ((crc->pInButtonState->nValue & IN_JUMP) && g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
	{
		crc->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		crc->pInButtonState->nValueChanged &= ~IN_JUMP;
	}
	*/
	auto player = g::m_local_player_pawn;
	if (!player)
		return;


	if (player->m_fFlags() & FL_ONGROUND)
	{
		crc->pInButtonState->bit(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		crc->pInButtonState->nValue &= ~IN_JUMP;
	}
	/*
	if (m_scrolls && m_scrolls < m_max_scrolls) {

#ifdef _DEBUG
		//	_log(LOG_INFO) << "[move] not reached max_scrolls | applyed nValueScroll: " << crc->pInButtonState->nValueScroll;
#endif
		m_scrolls++;
	}
	*/

}

void rotate_penis(CUserCmd* cmd, CBaseUserCmdPB* base, float yaw) {
	const float rotation_angle_rad = M_DEG2RAD(g::angle.y - yaw);
	const float forward_move = base->flForwardMove;
	const float side_move = base->flSideMove;

	float rotated_forward_move = forward_move * std::cos(rotation_angle_rad) - side_move * std::sin(rotation_angle_rad);
	float rotaded_side_move = forward_move * std::sin(rotation_angle_rad) + side_move * std::cos(rotation_angle_rad);

	base->flForwardMove=(rotated_forward_move);
	base->flSideMove =(rotaded_side_move);
}

void misc::c_movement::strafe(CUserCmd* cmd, CBaseUserCmdPB* base)
{
	if (!cfg_get(bool, g_cfg.auto_strafe))
		return;

	if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
		return;


	static float previous_yaw = 0.f;

	auto store_previous_yaw = [&]()
	{
		previous_yaw =g::angle.y;
	};


	auto local = g::m_local_player_pawn;

	bool on_ground = local->m_fFlags() & FL_ONGROUND;

	if (on_ground)
		return;

	static auto cl_sidespeed = 1.f;
	static auto cl_forwardspeed = 1.f;

	Vector_t viewangles, strafe, velocity = local->m_vecVelocity();

	velocity.z = 0.f;

	base->bit(BASE_BITS_FORWARDMOVE);

	base->flForwardMove=(0.f);

	float fl_velocity = velocity.Length2D();
	float strafe_angles = fl_velocity == 0.f ? 90.f : std::min(90.f, M_RAD2DEG(std::asin(15.f / fl_velocity)));

	if (strafe_angles > 90.0f)
		strafe_angles = 90.0f;
	else if (strafe_angles < 0.0f)
		strafe_angles = 0.0f;

	static float flOldYaw = 0.f;

	viewangles = g::angle;

	const Vector_t viewangles_backup = viewangles;

	float delta_yaw = viewangles.y - flOldYaw;

	static bool direction = false;

	if (std::abs(delta_yaw) < strafe_angles)
	{
		strafe = viewangles;

		if (direction)
		{
			strafe.y += strafe_angles;
			base->bit(BASE_BITS_LEFTMOVE);
			base->flSideMove = (cl_sidespeed);
		}
		else
		{
			strafe.y -= strafe_angles;
			base->bit(BASE_BITS_LEFTMOVE);
			base->flSideMove = (-cl_sidespeed);
		}

		direction = !direction;
	}
	else
	{
		if (delta_yaw < 0.0f) {
			base->bit(BASE_BITS_LEFTMOVE);
			base->flSideMove = (cl_sidespeed);
		}
		else if (delta_yaw > 0.0f) {
			base->bit(BASE_BITS_LEFTMOVE);
			base->flSideMove = (-cl_sidespeed);
		}
	}

	rotate_penis(cmd,base,strafe.y);

	flOldYaw = viewangles.y;


	float forward_move = base->flForwardMove;
	float side_move = base->flSideMove;

	bool dir = !direction;
	
	
	for (auto i = 0; i <12; i++) {

		strafe = viewangles;

		float sidemove = 0.f;

		velocity += velocity * 0.015625f;

		float fl_velocity = velocity.Length2D();
		float strafe_angle = fl_velocity == 0.f ? 90.f : std::min(90.f, M_RAD2DEG(std::asin(15.f / fl_velocity)));

		if (strafe_angle > 90.0f)
			strafe_angle = 90.0f;
		else if (strafe_angle < 0.0f)
			strafe_angle = 0.0f;

		if (dir)
		{
			strafe.y += strafe_angle;
			sidemove = cl_sidespeed;
		}
		else
		{
			strafe.y -= strafe_angle;
			sidemove = -cl_sidespeed;
		}

		dir = !dir;

		const float rotation_angle_rad = M_DEG2RAD(g::angle.y - strafe.y);

		float rotated_forward_move = 0.f * std::cos(rotation_angle_rad) - sidemove * std::sin(rotation_angle_rad);
		float rotaded_side_move = 0.f * std::sin(rotation_angle_rad) + sidemove * std::cos(rotation_angle_rad);

		for (auto i = 0; i < 12; i++) {
			input_history::AddSubtick(0, 0, (1.f / 12.f) * (i), (rotated_forward_move - forward_move), (rotaded_side_move - side_move));
		}

		forward_move = rotated_forward_move;
		side_move = rotaded_side_move;
	}
	
}
void misc::c_movement::apply_sub_tick(CUserCmd* user_cmd, CBaseUserCmdPB* base) {
	if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
		return;

	if (!cfg_get(bool, g_cfg.auto_strafe))
		return;

	for (auto i = 0; i < 12; i++) {
		auto subtick = base->create_new_subtick_step();
	}
	_log(LOG_INFO) << "size: " << base->subtickMovesField.pRep->nAllocatedSize;
	for (auto i = 0; i < base->subtickMovesField.pRep->nAllocatedSize; i++) {

		auto subtick = base->subtickMovesField.pRep->tElements[i];
		if (!subtick)
			continue;

		if (i == 0) {
			subtick->bit(MOVESTEP_BITS_WHEN);
			subtick->flWhen = (FLT_TRUE_MIN);
			continue;
		}
		subtick->bit(MOVESTEP_BITS_BUTTON);
		subtick->nButton = (0);

		subtick->bit(MOVESTEP_BITS_WHEN);
		subtick->flWhen = ((1.f / 12.f) * (i));
	}
}


void misc::c_movement::fast_ladder(CUserCmd* cmd, CBaseUserCmdPB* base) {

	if (!cfg_get(bool, g_cfg.fast_ladder))
		return;

	auto local_player = m_data.local;
	if (!local_player)
		return;

	float up_down;

	if (local_player->m_MoveType() == MOVETYPE_LADDER) {

	}
}

void misc::c_movement::edge_jump(CUserCmd* cmd, CBaseUserCmdPB* base) {
	bool should = cfg_get(bool, g_cfg.edge_jump_on_key) ? Input::GetBindState(cfg_get(KeyBind_t, g_cfg.edge_jump_key)) : true;
	if (!cfg_get(bool, g_cfg.edge_jump) || !should)
		return;

	if (  !(g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)) {
	
	}

}

void misc::c_movement::mini_jump(CUserCmd* cmd, CBaseUserCmdPB* base) {
	bool should = cfg_get(bool, g_cfg.mini_jump_on_key) ? Input::GetBindState(cfg_get(KeyBind_t, g_cfg.mini_jump_key)) : true;
	if (!cfg_get(bool, g_cfg.mini_jump) || !should)
		return;

	if ( !(g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)) {


	}
	
}

void misc::c_movement::ladder_jump(CUserCmd* cmd, CBaseUserCmdPB* base) {
	bool should = cfg_get(bool, g_cfg.ladder_jump_on_key) ? Input::GetBindState(cfg_get(KeyBind_t, g_cfg.ladder_jump_key)) : true;
	if (!cfg_get(bool, g_cfg.ladder_jump) || !should)
		return;

	if (m_data.local->m_MoveType() == MOVETYPE_NOCLIP)
		return;
	
	if ((g::m_local_player_pawn->m_MoveType() != MOVETYPE_LADDER)) {
	
		m_ladderjump.state = true;
	}

}
void misc::c_movement::get_mouse_speed_limit(float& x, float& y)
{
	bool mouse_speed_limit = false;
	if (!mouse_speed_limit)
		return;
	float msl_strength_x = 0.f;
	float msl_strength_y = 0.f;
	x = std::clamp< float >(x, -msl_strength_x, msl_strength_x);
	y = std::clamp< float >(y, -msl_strength_y, msl_strength_y);
}
/*
void misc::c_movement::edge_bug(CUserCmd* cmd, CBaseUserCmdPB* base) {
	bool should = cfg_get(bool, g_cfg.edge_bug_on_key) ? Input::GetBindState(cfg_get(KeyBind_t, g_cfg.edge_bug_key)) : true;
	if (!cfg_get(bool, g_cfg.edge_bug) || !should)
		return;

	m_edgebug.type = cfg_get(bool, g_cfg.edge_bug_type);

	if (!m_edgebug.type) {

		if ( m_data.local->m_fFlags() & FL_ONGROUND)
		{

			base->pInButtonState->add(IN_DUCK);
			base->pInButtonState->add(IN_DUCK, true);

		}
	}
	else if (m_edgebug.type) {

		float max_radius = math::_PI * 2;

		m_edgebug.jumped = false;

		if (!m_edgebug.processing) {

			int flags = m_data.local->m_fFlags();
			float z_velocity = floor(m_data.local->m_vecVelocity().z);

			for (int i = 0; i < 64; i++) {

				if (z_velocity < -7 && floor(m_data.local->m_vecVelocity().z) == -7 && !(flags & FL_ONGROUND) && m_data.local->m_MoveType() != MOVETYPE_NOCLIP) {
					m_edgebug.tick = base->tick_count + i;
					m_edgebug.processing = true;
					break;
				}
				else {
					z_velocity = floor(m_data.local->m_vecVelocity().z);
					flags = m_data.local->m_fFlags();
				}
			}
		}
		else {

			base->bit(CachedBits::BASE_BITS_LEFTMOVE);
			base->flSideMove = 0.f;

			base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
			base->flForwardMove = 0.f;

			base->bit(CachedBits::BASE_BITS_UPMOVE);
			base->flUpMove = 0.f;

			base->bit(CachedBits::BASE_BITS_MOUSEDX);
			base->nMousedX = 0.f;

			base->pInButtonState->add(IN_DUCK);
			base->pInButtonState->add(IN_DUCK, true);

			if ((m_data.local->m_fFlags() & 0x1)) {

				base->pInButtonState->remove(IN_DUCK);

			}

			Vector_t pos = m_data.local->m_pGameSceneNode()->m_vecOrigin();

			for (float a = 0.f; a < max_radius; a += max_radius / 128) {
				Vector_t pt;
				pt.x = (23 * cos(a)) + pos.x;
				pt.y = (23 * sin(a)) + pos.y;
				pt.z = pos.z;

				Vector_t pt2 = pt;
				pt2.z -= 8192;
				trace_filter_t filter = {};
				sdk::m_trace->Init(filter, m_data.local, 0x1400B, 3, 7);

				game_trace_t trace = {};
				ray_t ray = {};

				sdk::m_trace->TraceShape(ray, &pt, &pt2, filter, trace);

				if (trace.Fraction != 1.0f && trace.Fraction != 0.0f) {
					m_edgebug.jumped = true;
					base->pInButtonState->add(IN_DUCK);

				}
			}


			if (base->tick_count > m_edgebug.tick) {
				m_edgebug.processing = false;
				m_edgebug.tick = 0;
			}

		}
		trace_filter_t filter = {};
		sdk::m_trace->Init(filter, m_data.local, 0x1400B, 3, 7);

		Vector_t pos = m_data.local->m_pGameSceneNode()->m_vecOrigin();
		if (m_data.local->m_bInLanding()) {

			for (float a = 0.f; a < max_radius; a += max_radius / 128) {
				Vector_t pt;
				pt.x = (23 * cos(a)) + pos.x;
				pt.y = (23 * sin(a)) + pos.y;
				pt.z = pos.z;

				Vector_t pt2 = pt;
				pt2.z -= 8192;

				game_trace_t trace = {};
				ray_t ray = {};

				sdk::m_trace->TraceShape(ray, &pt, &pt2, filter, trace);

				if (trace.Fraction != 1.0f && trace.Fraction != 0.0f) {
					m_edgebug.jumped = true;


					base->bit(CachedBits::BASE_BITS_LEFTMOVE);
					base->flSideMove = 0.f;

					base->bit(CachedBits::BASE_BITS_FORWARDMOVE);
					base->flForwardMove = 0.f;

					base->bit(CachedBits::BASE_BITS_UPMOVE);
					base->flUpMove = 0.f;

					base->bit(CachedBits::BASE_BITS_MOUSEDX);
					base->nMousedX = 0.f;

					auto scroll = [&]() {
						base->pInButtonState->bit(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE3);
						base->pInButtonState->nValueScroll &= ~IN_JUMP;

					};
					if (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)
					{
						scroll();
						cmd->add_step_sub_tick(true, IN_JUMP, 0.999f);
						cmd->add_step_sub_tick(false, IN_JUMP, 0.999f);
					}

				}
			}

		}
		if (base->tick_count > m_edgebug.tick) {
		

			m_edgebug.processing = false;
			m_edgebug.tick = 0;
		}
	}
}
void misc::c_movement::long_jump(CUserCmd* cmd, CBaseUserCmdPB* base) {

	static int ticks = 0;
	static bool duck = false;
	bool should = cfg_get(bool, g_cfg.long_jump_on_key) ? Input::GetBindState(cfg_get(KeyBind_t, g_cfg.long_jump_key))
		: true;
	if (!cfg_get(bool, g_cfg.long_jump) || !should)
		return;

	if (!(g::m_local_player_pawn->m_fFlags() & FL_ONGROUND)) {

		ticks = base->tick_count + 2;
		duck = true;

		base->pInButtonState->add(IN_DUCK);
		base->pInButtonState->add(IN_DUCK, true);


		m_longjump.state = true;
	}
	
	
	if (duck && (base->tick_count < ticks)) {

		base->pInButtonState->add(IN_DUCK);
		base->pInButtonState->add(IN_DUCK, true);
	}
	else 
		if (duck) {

			duck = false;
		}
	}
}*/

Vector_t predict_position(Vector_t pos, Vector_t vel) {
	float ticks = 3.f;
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

constexpr float XMConvertToRadians(float fDegrees) noexcept { return fDegrees * (3.141592654f / 180.0f); }
inline QAngle_t calculate_angle_degree(Vector_t viewPos, Vector_t aimPos)
{
	QAngle_t angle = { 0, 0, 0 };

	Vector_t delta = aimPos - viewPos;

	angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
	angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

	return angle;
}
void  misc::c_movement::quick_stop(CBaseUserCmdPB* base_cmd, int type) noexcept {
	const auto local{ g::m_local_player_pawn };
	if (!local || !g::m_weapon_data || !sdk::m_global_vars)
		return;

	const auto movement_service{ local->m_pMovementServices() };
	if (!movement_service)
		return;

	auto predicted_data = g_prediction->get_local_data();
	if (!predicted_data)
		return;

	const auto velocity{ predicted_data->m_velocity };
	const auto speed = velocity.Length2D();
	if (speed < 1.f) {
		return;
	}

	/*
	auto remove_button = [&](int button) {
		base_cmd->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		base_cmd->pInButtonState->nValue &= ~button;
		base_cmd->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE2);
		base_cmd->pInButtonState->nValueChanged &= ~button;
		base_cmd->pInButtonState->bit(BUTTON_STATE_PB_BITS_BUTTONSTATE3);
		base_cmd->pInButtonState->nValueScroll &= ~button;
	};*/

	//remove_button(IN_SPRINT);

	if (type == 0) {

		static const auto accelerate{ sdk::m_cvar->find(HASH("sv_accelerate")) };
		static const auto max_speed{ sdk::m_cvar->find(HASH("sv_maxspeed")) };

		const auto surface_friction{ movement_service->m_flSurfaceFriction() };
		const auto max_accelspeed{ accelerate->value.fl * max_speed->value.fl
								   * surface_friction * g::interval };

		auto get_maximum_accelerate_speed = [&] {
			const auto speed_ratio{ speed / (accelerate->value.fl * g::interval) };
			return speed - max_accelspeed <= -1.f ? max_accelspeed / speed_ratio : max_accelspeed;
		};

		Vector_t velocity_angle{ };
		math::vec_angles(velocity * -1.0f, &velocity_angle);
		velocity_angle.y = g::angle.y - velocity_angle.y;

		Vector_t forward{ };
		math::angle_vectors(velocity_angle, forward);

		const auto total_speed{ get_maximum_accelerate_speed() };
		const auto max_weapon_speed = local->m_bIsScoped() ? g::m_weapon_data->m_flMaxSpeed().flValue[1]
			: g::m_weapon_data->m_flMaxSpeed().flValue[0];

		base_cmd->bit(CachedBits::BASE_BITS_FORWARDMOVE);
		base_cmd->flForwardMove = (forward.x * total_speed) / max_weapon_speed;

		base_cmd->bit(CachedBits::BASE_BITS_LEFTMOVE);
		base_cmd->flSideMove = (forward.y * -total_speed) / max_weapon_speed;
	}
	else {	
		Vector_t* move = (Vector_t*)&base_cmd->flForwardMove;
		Vector_t move_backup = *move;
		const QAngle_t& current_angles = QAngle_t(g::angle.x, g::angle.y, g::angle.z);

		QAngle_t angle = calculate_angle_degree(g::m_local_player_pawn->m_pGameSceneNode()->m_vecAbsOrigin(), predict_position(g::m_local_player_pawn->m_pGameSceneNode()->m_vecAbsOrigin(), velocity * -4.f));

		const float delta = remainderf(angle.y - current_angles.y, 360.f);
		const float yaw = XMConvertToRadians(delta);

		move->x = move_backup.x * std::cos(yaw) - move_backup.y * std::sin(yaw);
		move->y = move_backup.x * std::sin(yaw) + move_backup.y * std::cos(yaw);

		float mul = 1.f;

		if (std::fabsf(move->x) > 1.f)
			mul = 1.f / std::fabsf(move->x);
		else if (std::fabsf(move->y) > 1.f)
			mul = 1.f / std::fabsf(move->y);

		move->x *= mul;
		move->y *= mul;
		move->z = 0.f;

		base_cmd->bit(BASE_BITS_FORWARDMOVE);
		base_cmd->flForwardMove = move->x * mul;
		base_cmd->bit(BASE_BITS_LEFTMOVE);
		base_cmd->flSideMove = move->y * mul;
	}

	using function_t = void(__fastcall*)(CPrediction*, int, int);
	static auto update = reinterpret_cast<void(__fastcall*)(void*, int, int)>(utils::FindPattern(CLIENT_DLL, "89 54 24 10 48 89 4C 24 08 55 53 41 55 41"));
	auto network_client = sdk::m_network->GetNetworkClient();

	if (network_client->m_get_compression() && network_client->m_delta_tick() > 0 && network_client->m_server_tick() > 0) 
		update(sdk::m_prediction, 0, network_client->m_delta_tick());
	

}

void  misc::c_movement::change_speed(CBaseUserCmdPB* base_cmd, const float max_speed) {
	const auto local{ g::m_local_player_pawn };
	if (!local)
		return;

	auto predicted_data = g_prediction->get_local_data();
	if (!predicted_data)
		return;
	// backup
	const auto sidemove{ base_cmd->flSideMove };
	const auto forwardmove{ base_cmd->flForwardMove };
	const auto move_speed{ static_cast<float>(
		std::sqrt(std::pow(sidemove, 2) + std::pow(forwardmove, 2))) };
	if (move_speed > max_speed) {


		const auto invalid_speed{ max_speed + 1.0f < (predicted_data->m_velocity.Length2D()) };

		base_cmd->bit(CachedBits::BASE_BITS_LEFTMOVE);
		base_cmd->flSideMove = invalid_speed ? 0.0f : (sidemove / move_speed) * max_speed;

		base_cmd->bit(CachedBits::BASE_BITS_FORWARDMOVE);
		base_cmd->flForwardMove = invalid_speed ? 0.0f : (forwardmove / move_speed) * max_speed;


		using function_t = void(__fastcall*)(CPrediction*, int, int);
		static auto update = reinterpret_cast<void(__fastcall*)(void*, int, int)>(utils::FindPattern(CLIENT_DLL, "89 54 24 10 48 89 4C 24 08 55 53 41 55 41"));
		auto network_client = sdk::m_network->GetNetworkClient();

		if (network_client->m_get_compression() && network_client->m_delta_tick() > 0 && network_client->m_server_tick() > 0)
			update(sdk::m_prediction, 0, network_client->m_delta_tick());
	}
}

void  misc::c_movement::slow_walk(CBaseUserCmdPB* base_cmd) noexcept {
	auto local = g::m_local_player_pawn;
	if (!local || !g::m_weapon_data)
		return;

	auto predicted_data = g_prediction->get_local_data();
	if (!predicted_data)
		return;

	const float speed{ (predicted_data->m_velocity.Length2D()) };
	const float max_weapon_speed{ local->m_bIsScoped() ? g::m_weapon_data->m_flMaxSpeed().flValue[1]
														: g::m_weapon_data->m_flMaxSpeed().flValue[0] };

	const float walk_speed{ 0.34f * max_weapon_speed };

	if (speed <= walk_speed) {
		change_speed(base_cmd, max_weapon_speed * 0.34f);
		return;
	}

	quick_stop(base_cmd);
}

QAngle_t m_correct_sub_tick_view = QAngle_t();

void misc::c_movement::proccess_data(CUserCmd* cmd, CBaseUserCmdPB* base) {
	if (!g::m_local_player_pawn || !cmd || !base)
		return;
	
	if (!g::m_local_player_pawn->IsAlive())
		return;

	validate_command(cmd, base, nullptr);

}
void misc::c_movement::validate_command(CUserCmd* m_cmd, CBaseUserCmdPB* pCmd, CCSGOInputHistoryEntryPB* m_sub_tick)
{
	if (!pCmd || !m_cmd)
		return;

	auto should_clamp = true;
	auto valve_dedicated_server = true;// sdk::m_game_rules->m_bIsValveDS();
	// not valve server & rage 
	//if (!sdk::m_game_rules->m_bIsValveDS() && cfg_get(bool, g_cfg.rage_enable))
	{

		//should_clamp = false;
	}

	if (pCmd->view_angles) {
		if (pCmd->view_angles->angValue.IsValid()) {
			pCmd->bit(BASE_BITS_VIEWANGLES);
			pCmd->view_angles->angValue.Clamp();
		}
	}

	if (!valve_dedicated_server) {
		auto base_cmd = pCmd;

		for (int nSubTick = 0; nSubTick < m_cmd->inputHistoryField.pRep->nAllocatedSize; nSubTick++)
		{
			CCSGOInputHistoryEntryPB* pInputEntry = m_cmd->get_input_history_tick(nSubTick);
			if (pInputEntry == nullptr)
				continue;
			if (!pInputEntry->pViewAngles)
				continue;

			m_correct_sub_tick_view = pInputEntry->pViewAngles->angValue;

			Vector_t vecForward = {}, vecRight = {}, vecUp = {};
			m_correct_sub_tick_view.ToDirections(&vecForward, &vecRight, &vecUp);

			vecForward.z = vecRight.z = vecUp.x = vecUp.y = 0.0f;

			vecForward.NormalizeInPlace();
			vecRight.NormalizeInPlace();
			vecUp.NormalizeInPlace();

			Vector_t vecOldForward = {}, vecOldRight = {}, vecOldUp = {};
			pInputEntry->pViewAngles->angValue.ToDirections(&vecOldForward, &vecOldRight, &vecOldUp);

			vecOldForward.z = vecOldRight.z = vecOldUp.x = vecOldUp.y = 0.0f;

			vecOldForward.NormalizeInPlace();
			vecOldRight.NormalizeInPlace();
			vecOldUp.NormalizeInPlace();

			const float flPitchForward = vecForward.x * base_cmd->flForwardMove;
			const float flYawForward = vecForward.y * base_cmd->flForwardMove;
			const float flPitchSide = vecRight.x * base_cmd->flSideMove;
			const float flYawSide = vecRight.y * base_cmd->flSideMove;
			const float flRollUp = vecUp.z * base_cmd->flUpMove;

			base_cmd->bit(BASE_BITS_FORWARDMOVE);
			base_cmd->flForwardMove = vecOldForward.x * flPitchSide + vecOldForward.y * flYawSide + vecOldForward.x * flPitchForward + vecOldForward.y * flYawForward + vecOldForward.z * flRollUp;

			base_cmd->bit(BASE_BITS_LEFTMOVE);
			base_cmd->flSideMove = vecOldRight.x * flPitchSide + vecOldRight.y * flYawSide + vecOldRight.x * flPitchForward + vecOldRight.y * flYawForward + vecOldRight.z * flRollUp;

			base_cmd->bit(BASE_BITS_UPMOVE);
			base_cmd->flUpMove = vecOldUp.x * flYawSide + vecOldUp.y * flPitchSide + vecOldUp.x * flYawForward + vecOldUp.y * flPitchForward + vecOldUp.z * flRollUp;

			if (base_cmd->pInButtonState) {
				base_cmd->pInButtonState->bit(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE1);
				base_cmd->pInButtonState->nValue &= (~IN_FORWARD | ~IN_BACK | ~IN_TURNLEFT | ~IN_TURNRIGHT);
			}

			if (base_cmd->flForwardMove > 0.0f)
				base_cmd->pInButtonState->nValue |= IN_FORWARD;
			else if (base_cmd->flForwardMove < 0.0f)
				base_cmd->pInButtonState->nValue |= IN_BACK;

			if (base_cmd->flSideMove > 0.0f)
				base_cmd->pInButtonState->nValue |= IN_TURNRIGHT;
			else if (base_cmd->flSideMove < 0.0f)
				base_cmd->pInButtonState->nValue |= IN_TURNLEFT;

			if (!pInputEntry->pViewAngles->angValue.IsZero())
			{
				const float flDeltaX = std::remainderf(pInputEntry->pViewAngles->angValue.x - m_correct_sub_tick_view.x, 360.f);
				const float flDeltaY = std::remainderf(pInputEntry->pViewAngles->angValue.y - m_correct_sub_tick_view.y, 360.f);

				float flPitch = sdk::m_cvar->find(hash_32_fnv1a_const("m_pitch"))->value.fl;
				float flYaw = sdk::m_cvar->find(hash_32_fnv1a_const("m_yaw"))->value.fl;

				float flSensitivity = sdk::m_cvar->find(hash_32_fnv1a_const("sensitivity"))->value.fl;
				if (flSensitivity == 0.0f)
					flSensitivity = 1.0f;

				base_cmd->bit(BASE_BITS_MOUSEDX);
				base_cmd->nMousedX = static_cast<short>(flDeltaX / (flSensitivity * flPitch));

				base_cmd->bit(BASE_BITS_MOUSEDY);
				base_cmd->nMousedY = static_cast<short>(-flDeltaY / (flSensitivity * flYaw));
			}
		}
	}
}
