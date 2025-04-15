#pragma once

#include "..\..\sdk\interfaces\csgoinput.h"
#pragma once
#include "..\../sdk\datatypes/cusercmd/repfieldcontainer.h"
#include "..\../sdk\datatypes/cutlbuffer/utlbuffer.h"
#include "../../math/elements/vector.h"
#include "..\..\sdk\datatypes\cutlvectors\utlvector.h"
#include <string>
#include <Windows.h>
#include <stdint.h>
#pragma once

class CSubtick
{
public:
	float m_flWhen = 0.0f;
	bool m_bPressed = false;
	uint64_t m_uButton = 0;
	float m_flLeftMove = 0.0f;
	float m_flForwardMove = 0.0f;
};

namespace input_history
{
	inline int subtick_count;
	inline CCSGOInput* backup_input;
	inline CSubtick g_Subtick[12]{};
	void AddSubtick(uint64_t button, bool pressed, float when, float flmove, float lmove);
	void AddAction(uint64_t button, CUserCmd* cmd);
	void Store(CCSGOInput* input);
	void Restore(CCSGOInput* input);
	void Run(CCSGOInput* input, CUserCmd* cmd, CBaseUserCmdPB* base);
}