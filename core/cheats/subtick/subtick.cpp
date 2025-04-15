#include "subtick.h"
#include "..\..\sdk\sdk.h"
void input_history::Store(CCSGOInput* input) {
	backup_input = input;

	subtick_count = input->SubticksCount;
	for (auto i = 0; i < subtick_count; i++) {
		auto subtick_moves = &input->Subticks[i];
		if (!subtick_moves)
			continue;

		auto subticks = &g_Subtick[i];
		subticks->m_bPressed = subtick_moves->bPressed;
		subticks->m_flWhen = subtick_moves->flWhen;
		subticks->m_uButton = subtick_moves->nButton;

	}
}

void input_history::Restore(CCSGOInput* input) {
	input->SubticksCount = subtick_count;

	for (auto i = 0; i < subtick_count; i++) {
		auto subtick_moves = &input->Subticks[i];
		if (!subtick_moves)
			continue;

		auto subticks = &g_Subtick[i];
		if (!subticks)
			continue;

		subtick_moves->bPressed = subticks->m_bPressed;
		subtick_moves->flWhen = subticks->m_flWhen;
		subtick_moves->nButton = subticks->m_uButton;
	}
}

void input_history::Run(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base) {

	if (command::m_legit_bot || !sdk::m_game_rules)
		return;

	if (sdk::m_game_rules->m_bIsValveDS())
	{
		return;
	}



		for (auto i = 0; i < subtick_count; i++)
		{
			auto subtick = &g_Subtick[i];
			if (!subtick)
				continue;

			auto subtickMoves = base->create_new_subtick_step();
			if (!subtickMoves)
				continue;

			subtickMoves->bit(MOVESTEP_BITS_WHEN);
			subtickMoves->flWhen = (subtick->m_flWhen);

			if (subtick->m_bPressed)
			{
				subtickMoves->bit(MOVESTEP_BITS_BUTTON);
				subtickMoves->nButton = (subtick->m_uButton);
				subtickMoves->bit(MOVESTEP_BITS_PRESSED);
				subtickMoves->bPressed = (subtick->m_bPressed);
				subtickMoves->bit(MOVESTEP_BITS_WHEN);
				subtickMoves->flWhen = (subtick->m_flWhen);
			}
			else
			{

				if (subtick->m_flForwardMove != 0.0) {
					subtickMoves->bit(MOVESTEP_BITS_ANALOG_FORWARD_DELTA);
					subtickMoves->flAnalogForwardDelta = (subtick->m_flForwardMove);
				}

				if (subtick->m_flLeftMove != 0.0)
				{
					subtickMoves->bit(MOVESTEP_BITS_ANALOG_LEFT_DELTA);
					subtickMoves->flAnalogLeftDelta = (subtick->m_flLeftMove);
				}
			}
		}
	
	Restore(input);
}

void input_history::AddSubtick(uint64_t button, bool pressed, float when, float flmove, float lmove) {
	for (auto i = 0; i < subtick_count; i++) {
		auto subtick = &g_Subtick[i];
		if (!subtick)
			continue;

		subtick->m_bPressed = pressed;
		subtick->m_flWhen = when;
		subtick->m_uButton |= button;
		subtick->m_flForwardMove = flmove;
		subtick->m_flLeftMove = lmove;
	}
}

void input_history::AddAction(uint64_t button, CUserCmd* cmd)
{
	/*
	if (button == IN_ATTACK)
	{
		if (backup_input->MessageSize > 1)
			backup_input->Message[backup_input->MessageSize - 1]->m_nPlayerTickCount = backup_input->Message[backup_input->MessageSize - 1]->m_nPlayerTickCount;

		if (backup_input->nAttackStartHistoryIndex1 != -1)
			 cmd->nAttack1StartHistoryIndex = (backup_input->nAttackStartHistoryIndex1);

		if (backup_input->nAttackStartHistoryIndex2 != -1)
			cmd->nAttack2StartHistoryIndex = (backup_input->nAttackStartHistoryIndex2);

		if (backup_input->nAttackStartHistoryIndex3 != -1)
			cmd->nAttack3StartHistoryIndex = (backup_input->nAttackStartHistoryIndex3);

		cmd->nAttack1StartHistoryIndex = (backup_input->Message[backup_input->nAttackStartHistoryIndex1]->m_nEntryIndexMax);
		cmd->nAttack2StartHistoryIndex = (backup_input->Message[backup_input->nAttackStartHistoryIndex2]->m_nEntryIndexMax);
		cmd->nAttack3StartHistoryIndex = (backup_input->Message[backup_input->nAttackStartHistoryIndex3]->m_nEntryIndexMax);
	}*/
}