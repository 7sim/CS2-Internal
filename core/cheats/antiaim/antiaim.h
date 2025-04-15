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

class c_antiaim
{
	enum e_manual
	{
		none,
		left,
		backward,
		right
	};
public:
	void run(CUserCmd* cmd, CBaseUserCmdPB* base);
private:
	void pitch(CUserCmd* cmd, CBaseUserCmdPB* base);
	void yaw(CUserCmd* cmd, CBaseUserCmdPB* base);
public:
	float spinner_amount = 0.f;
	int manual = 0;
	Vector_t antiaim_angle{};
	int jitter_side = -1;
};

inline auto g_antiaim = std::make_unique<c_antiaim>();