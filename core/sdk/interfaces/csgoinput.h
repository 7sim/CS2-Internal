#pragma once
#include "../datatypes/cusercmd/repfieldcontainer.h"
#include "../datatypes/cutlbuffer/utlbuffer.h"
#include "../../math/elements/vector.h"
#include "..\..\sdk\datatypes\cutlvectors\utlvector.h"
#include <string>
#include <Windows.h>
#include <stdint.h>
#pragma once

enum CachedBits : uint32_t
{
	BASE_BITS_MOVE_CRC = 0x1U, // used for both BaseCmdPB and MoveCRC
	BASE_BITS_BUTTONPB = 0x2U,
	BASE_BITS_VIEWANGLES = 0x4U,
	BASE_BITS_COMMAND_NUMBER = 0x8U,
	BASE_BITS_CLIENT_TICK = 0x10U,
	BASE_BITS_FORWARDMOVE = 0x20U,
	BASE_BITS_LEFTMOVE = 0x40U,
	BASE_BITS_UPMOVE = 0x80U,
	BASE_BITS_IMPULSE = 0x100U,
	BASE_BITS_WEAPON_SELECT = 0x200U,
	BASE_BITS_RANDOM_SEED = 0x400U,
	BASE_BITS_MOUSEDX = 0x800U,
	BASE_BITS_MOUSEDY = 0x1000U,
	BASE_BITS_CONSUMED_SERVER_ANGLE = 0x2000U,
	BASE_BITS_CMD_FLAGS = 0x4000U,
	BASE_BITS_ENTITY_HANDLE = 0x8000U
};

enum EButtonStatePBBits : uint32_t
{
	BUTTON_STATE_PB_BITS_BUTTONSTATE1 = 0x1U,
	BUTTON_STATE_PB_BITS_BUTTONSTATE2 = 0x2U,
	BUTTON_STATE_PB_BITS_BUTTONSTATE3 = 0x4U
};

// compiled protobuf messages and looked at what bits are used in them
enum ESubtickMoveStepBits : std::uint32_t
{
	MOVESTEP_BITS_BUTTON = 0x1U,
	MOVESTEP_BITS_PRESSED = 0x2U,
	MOVESTEP_BITS_WHEN = 0x4U,
	MOVESTEP_BITS_ANALOG_FORWARD_DELTA = 0x8U,
	MOVESTEP_BITS_ANALOG_LEFT_DELTA = 0x10U
};

enum EInputHistoryBits : std::uint32_t
{
	INPUT_HISTORY_BITS_VIEWANGLES = 0x1U,// seems fine
	INPUT_HISTORY_BITS_SHOOTPOSITION = 0x20U, // FIXED . ,
	INPUT_HISTORY_BITS_TARGETHEADPOSITIONCHECK = 0x4U, // dont care
	INPUT_HISTORY_BITS_TARGETABSPOSITIONCHECK = 0x8U,//dont care
	INPUT_HISTORY_BITS_TARGETANGCHECK = 0x10U, // dont care
	INPUT_HISTORY_BITS_CL_INTERP = 0x10U, // Fixed.
	INPUT_HISTORY_BITS_SV_INTERP0 = 0x4U, // Fixed.
	INPUT_HISTORY_BITS_SV_INTERP1 = 0x8U, // fixed.
	INPUT_HISTORY_BITS_PLAYER_INTERP = 0x10U, // fixed.
	INPUT_HISTORY_BITS_RENDERTICKCOUNT = 0x200U, // correct. 
	INPUT_HISTORY_BITS_RENDERTICKFRACTION = 0x400U, // dont care
	INPUT_HISTORY_BITS_PLAYERTICKCOUNT = 0x800U, // correct
	INPUT_HISTORY_BITS_PLAYERTICKFRACTION = 0x1000U, // dont care
	INPUT_HISTORY_BITS_FRAMENUMBER = 0x2000U, // dont care
	INPUT_HISTORY_BITS_TARGETENTINDEX = 0x4000U // correct.
};

enum ECSGOUserCmdBits : std::uint32_t
{

	CSGOUSERCMD_BITS_BASECMD = 0x1U,
	CSGOUSERCMD_BITS_LEFTHAND = 0x2U,
	CSGOUSERCMD_BITS_PREDICTING_BODY_SHOT = 0x4U,
	CSGOUSERCMD_BITS_PREDICTING_HEAD_SHOT = 0x8U,
	CSGOUSERCMD_BITS_PREDICTING_KILL_RAGDOLLS = 0x10U,
	CSGOUSERCMD_BITS_ATTACK3START = 0x20U,
	CSGOUSERCMD_BITS_ATTACK1START = 0x40U,
	CSGOUSERCMD_BITS_ATTACK2START = 0x80U
};
enum EInButtonState : uint8_t
{
	IN_BUTTON_UP = 0,
	IN_BUTTON_DOWN,
	IN_BUTTON_DOWN_UP,
	IN_BUTTON_UP_DOWN,
	IN_BUTTON_UP_DOWN_UP,
	IN_BUTTON_DOWN_UP_DOWN,
	IN_BUTTON_DOWN_UP_DOWN_UP,
	IN_BUTTON_UP_DOWN_UP_DOWN,
	IN_BUTTON_STATE_COUNT
};

enum EButtonType : uint8_t
{
	HOLD,
	PRESSED,
	UNPRESSED
};

template <typename T>
struct RepeatedPtrField_t
{
	struct Rep_t
	{
		int nAllocatedSize;
		T* tElements[(std::numeric_limits<int>::max() - 2 * sizeof(int)) / sizeof(void*)];
	};

	void* pArena; // 0x18
	int nCurrentSize;
	int nTotalSize;
	Rep_t* pRep;
};


// @source: master/game/shared/in_buttons.h
enum ECommandButtons : uint64_t
{
	IN_ATTACK = 1 << 0,
	IN_JUMP = 1 << 1,
	IN_DUCK = 1 << 2,
	IN_FORWARD = 1 << 3,
	IN_BACK = 1 << 4,
	IN_USE = 1 << 5,
	IN_TURNLEFT = 1 << 7,
	IN_TURNRIGHT = 1 << 8,
	IN_MOVELEFT = 1 << 9,
	IN_MOVERIGHT = 1 << 10,
	IN_ATTACK2 = 1 << 11,
	IN_RELOAD = 1 << 13,
	IN_SPRINT = 1 << 16,
	IN_SHOWSCORES = 1ULL << 33,
	IN_SCOPE = 1ULL << 34,
	IN_LOOKATWEAPON = 1ULL << 35
};
class CBasePB
{
	pad(0x8); // 0x0 VTABLE
	std::uint32_t nHasBits; // 0x8
	std::uint64_t nCachedBits; // 0xC

	void bit(std::uint32_t nBits) {
		nCachedBits |= nBits;
	}
};

static_assert(sizeof(CBasePB) == 0x18);

class CMsgQAngle : public CBasePB
{
public:
	QAngle_t angValue; // 0x18
	pad(12);
};
static_assert(sizeof(CMsgQAngle) == 0x30);

class CMsgVector : public CBasePB
{
public:
	Vector4D_t vecValue; // 0x18
};
static_assert(sizeof(CMsgVector) == 0x28);

class CCSGOInterpolationInfoPB : public CBasePB
{
public:
	float flFraction; // 0x18
	int nSrcTick; // 0x1C
	int nDstTick; // 0x20
};
static_assert(sizeof(CCSGOInterpolationInfoPB) == 0x28);

class CCSGOInterpolationInfoPB_CL : public CBasePB
{
public:
	float flFraction; // 0x18
};
class CCSGOInputHistoryEntryPB : public CBasePB
{
public:
	CMsgQAngle* pViewAngles; // 0x18
	CCSGOInterpolationInfoPB_CL* cl_interp; // 0x20 CL interp at 0x20 in IDA
	CCSGOInterpolationInfoPB* sv_interp0; // 0x28 Interp here in ida
	CCSGOInterpolationInfoPB* sv_interp1; // 0x30 correct
	CCSGOInterpolationInfoPB* player_interp; // 0x38 correct.
	CMsgVector* pShootPosition; // 0x40 // correct
	CMsgVector* pTargetHeadPositionCheck; // 0x48 correct
	CMsgVector* pTargetAbsPositionCheck; // 0x50 correct
	CMsgQAngle* pTargetAngPositionCheck; // 0x58 correct
	int nRenderTickCount; // 0x60
	float flRenderTickFraction; // 0x64
	int nPlayerTickCount; // 0x68
	float flPlayerTickFraction; // 0x6C
	int nFrameNumber; // 0x70
	int nTargetEntIndex; // 0x74


public:
	CMsgQAngle* create_new_angle();
	CMsgVector* create_new_vector_angle();
	CCSGOInterpolationInfoPB* create_new_interp();
	CCSGOInterpolationInfoPB_CL* create_new_interp_cl();

	void set_view_angles(QAngle_t angle, bool force = false) {
		
		if (!pViewAngles && force)
			pViewAngles = create_new_angle();

		if (!pViewAngles)
			return;

		pViewAngles->angValue = angle;
		nCachedBits |= INPUT_HISTORY_BITS_VIEWANGLES;
		pViewAngles->nCachedBits |= 7;//???
	}

	void set_shoot_pos(Vector_t shoot_pos, bool force = false) {

		if (!pShootPosition && force)
			pShootPosition = create_new_vector_angle();

		if (!pShootPosition) 
			return;

		pShootPosition->vecValue = Vector4D_t(shoot_pos.x, shoot_pos.y,shoot_pos.z, 0.0f);
		nCachedBits |= INPUT_HISTORY_BITS_SHOOTPOSITION;

	}
	void set_target_ang_position(QAngle_t shoot_pos, bool force = false) {

		if (!pTargetAngPositionCheck && force)
			pTargetAngPositionCheck = create_new_angle();

		if (!pTargetAngPositionCheck)
			return;

		pTargetAngPositionCheck->angValue = shoot_pos;
		nCachedBits |= INPUT_HISTORY_BITS_TARGETANGCHECK;

	}
	void set_render_tick_count(int renderTickCount) {
		nRenderTickCount = renderTickCount;
		nCachedBits |= INPUT_HISTORY_BITS_RENDERTICKCOUNT;
	}

	void set_render_tick_fraction(float frac) {
		flRenderTickFraction = frac;
		nCachedBits |= INPUT_HISTORY_BITS_RENDERTICKFRACTION;
	}

	void set_player_tick_count(int playerTickCount) {
		nPlayerTickCount = playerTickCount;
		nCachedBits |= INPUT_HISTORY_BITS_PLAYERTICKCOUNT;
	}

	void set_player_tick_fraction(float frac) {
		flPlayerTickFraction = frac;
		nCachedBits |= INPUT_HISTORY_BITS_PLAYERTICKFRACTION;
	}

	void set_cl_interp(float frac, bool force = false) {
		if (!cl_interp && force)
			cl_interp = this->create_new_interp_cl();

		if (!cl_interp)
			return;

		cl_interp->flFraction = frac;
		nCachedBits |= INPUT_HISTORY_BITS_CL_INTERP;
	}

	void set_sv_interp(int srctick, int dsttick, float fract, bool dst_tick = false, bool force = false) {
		if (!sv_interp0 && force)
			sv_interp0 = this->create_new_interp();

		if (!sv_interp0)
			return;

		auto fraction_tick_bit = 2u;
		sv_interp0->bit(fraction_tick_bit);
		sv_interp0->flFraction = fract;

		auto src_tick_bit = fraction_tick_bit |= 4u;
		sv_interp0->bit(src_tick_bit);
		sv_interp0->nSrcTick = srctick;

		if (dst_tick) {
			auto dst_tick_bit = src_tick_bit | 1;
			sv_interp0->bit(dst_tick_bit);
			sv_interp0->nDstTick = dsttick + 2;
		}
	}

	void set_sv_interp1(int srctick, int dsttick, float fract, bool dst_tick = false, bool force = false) {
		if (!sv_interp1 && force)
			sv_interp1 = this->create_new_interp();

		if (!sv_interp1)
			return;

		auto fraction_tick_bit = 2u;
		sv_interp1->bit(fraction_tick_bit);
		sv_interp1->flFraction = fract;

		auto src_tick_bit = fraction_tick_bit |= 4u;
		sv_interp1->bit(src_tick_bit);
		sv_interp1->nSrcTick = srctick;

		if (dst_tick) {
			auto dst_tick_bit = src_tick_bit | 1;
			sv_interp1->bit(dst_tick_bit);
			sv_interp1->nDstTick = dsttick + 2;
		}
	}

	void set_player_interp(int srctick, int dsttick, float fract, bool dst_tick = false, bool force = false) {
		if (!player_interp && force)
			player_interp = this->create_new_interp();

		if (!player_interp)
			return;

		auto fraction_tick_bit = 2u;
		player_interp->bit(fraction_tick_bit);
		player_interp->flFraction = fract;

		auto src_tick_bit = fraction_tick_bit |= 4u;
		player_interp->bit(src_tick_bit);
		player_interp->nSrcTick = srctick;

		if (player_interp) {
			auto dst_tick_bit = src_tick_bit | 1;
			player_interp->bit(dst_tick_bit);
			player_interp->nDstTick = dsttick + 2;
		}
	}
};
static_assert(sizeof(CCSGOInputHistoryEntryPB) == 0x78);

struct CInButtonStatePB : CBasePB
{
	std::uint64_t nValue;
	std::uint64_t nValueChanged;
	std::uint64_t nValueScroll;
};

static_assert(sizeof(CInButtonStatePB) == 0x30);

struct CSubtickMoveStep : CBasePB
{
public:
	std::uint64_t nButton;
	bool bPressed;
	float flWhen;
	float flAnalogForwardDelta;
	float flAnalogLeftDelta;

};
static_assert(sizeof(CSubtickMoveStep) == 0x30);

class CCSGOInput;

class CBaseUserCmdPB : public CBasePB
{
public:
	RepeatedPtrField_t<CSubtickMoveStep> subtickMovesField; // 0x18
	std::string* strMoveCrc;
	CInButtonStatePB* pInButtonState;
	CMsgQAngle* view_angles;
	std::int32_t nLegacyCommandNumber;
	std::int32_t nClientTick;
	float flForwardMove;
	float flSideMove;
	float flUpMove;
	std::int32_t nImpulse;
	std::int32_t nWeaponSelect;
	std::int32_t nRandomSeed;
	std::int32_t nMousedX;
	std::int32_t nMousedY;
	std::uint32_t nConsumedServerAngleChanges;
	std::int32_t nCmdFlags;
	std::uint32_t nPawnEntityHandle;
public:

	std::uint8_t get_crc_size() {
		return utils::CallVFunc<std::uint8_t, 7u>(this);
	}
	CSubtickMoveStep* create_new_subtick_step();
	void set_view_angles(QAngle_t angle) {
		if (!this)
			return;

		if (!view_angles)
			return;

		bit(BASE_BITS_VIEWANGLES);
		view_angles->bit(BASE_BITS_VIEWANGLES);
		view_angles->angValue = angle;
	}
	void set_tick_count(int tick) {
		if (!this)
			return;

		/* set current client tick */
		bit(BASE_BITS_CLIENT_TICK);
		nClientTick = tick;
	}
	void attack() {
		if (!this)
			return;

		// set the IN_ATTACK flag to simulate the shot
		this->bit(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		this->pInButtonState->nValue |= IN_ATTACK;
		this->bit(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE2);
		this->pInButtonState->nValueChanged |= IN_ATTACK;
	}

	void update_button() {
		if (!this)
			return;

		this->bit(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		this->pInButtonState->nValue |= IN_ATTACK;
	}

	void invalidate_pitch() {
		if (!this || !this->view_angles)
			return;

		this->bit(CachedBits::BASE_BITS_VIEWANGLES);
		this->view_angles->angValue.x = 179.f;
	}
};
static_assert(sizeof(CBaseUserCmdPB) == 0x80);


struct CInButtonState
{
public:
	pad(0x8); // 0x0 VTABLE
	std::uint64_t nValue; // 0x8
	std::uint64_t nValueChanged; // 0x10
	std::uint64_t nValueScroll; // 0x18
};
static_assert(sizeof(CInButtonState) == 0x20);



class CUserCmd
{
public:
	pad(0x8); // 0x0 VTABLE
	pad(0x10); // TODO: find out what this is, added 14.08.2024
	std::uint32_t nHasBits;
	std::uint64_t nCachedSize;
	RepeatedPtrField_t<CCSGOInputHistoryEntryPB> inputHistoryField;
	CBaseUserCmdPB* pBaseCmd;
	bool m_bLeftHandDesired;
	bool m_bIsPredictingBodyShotFX;
	bool m_bIsPredictingHeadShotFX;
	bool m_bIsPredictingKillRagdolls;
	std::int32_t nAttack3StartHistoryIndex;
	std::int32_t nAttack1StartHistoryIndex; //"0x50" for ida
	std::int32_t nAttack2StartHistoryIndex;
	CInButtonState nButtons; // 0x58
	pad(0x10);
	bool bHasBeenPredicted;
	pad(0xF);
public:
	bool CRC(CBaseUserCmdPB* m_base);
	void bit(std::uint32_t nBits)
	{
		if (!(nHasBits & nBits))
			nHasBits |= nBits;
	}
	CSubtickMoveStep* create_subtick_step(void* arena);
	CSubtickMoveStep* create_subtick();

	CSubtickMoveStep* get_move_step(int nIndex)
	{
		if (nIndex >= pBaseCmd->subtickMovesField.pRep->nAllocatedSize || nIndex >= pBaseCmd->subtickMovesField.nCurrentSize)
			return nullptr;

		return pBaseCmd->subtickMovesField.pRep->tElements[nIndex];
	}

	CCSGOInputHistoryEntryPB* get_input_history_tick(int nIndex)
	{
		if (nIndex >= inputHistoryField.pRep->nAllocatedSize || nIndex >= inputHistoryField.nCurrentSize)
			return nullptr;

		return inputHistoryField.pRep->tElements[nIndex];
	}

	void set_sub_tick_angle(const QAngle_t& angView)
	{
		for (int i = 0; i < this->inputHistoryField.pRep->nAllocatedSize; i++) {
			auto sub_tick = this->get_input_history_tick(i);
			if (!sub_tick)
				continue;

			// viewangles can be null rarely
	//		if (!sub_tick->pViewAngles)
	//			sub_tick->pViewAngles = sub_tick->create_new_angle();

			// something went creating MsgAngle on this tick
			if (!sub_tick->pViewAngles)
				continue;

			sub_tick->bit(EInputHistoryBits::INPUT_HISTORY_BITS_VIEWANGLES);
			sub_tick->pViewAngles->angValue = angView;
			sub_tick->pViewAngles->angValue.Clamp();
		}
	}

	QAngle_t get_view_angle() {
	
		if (!this)
			return QAngle_t(0, 0, 0);

		if (!this->pBaseCmd)
			return QAngle_t(0, 0, 0);

		if (!this->pBaseCmd->view_angles)
			return QAngle_t(0, 0, 0);

		return this->pBaseCmd->view_angles->angValue;
	}
	

	void add_step_sub_tick(bool pressed, int button, float fraction = 1.f) {
		if (!this)
			return;

		auto correct_subtick_move_step = this->create_subtick();
		if (!correct_subtick_move_step)
			return;

		auto base_cmd = this->pBaseCmd;
		if (!base_cmd)
			return;

		auto cur_sub_tick = base_cmd->subtickMovesField.pRep;
		if (!cur_sub_tick)
			return;

		auto v46 = correct_subtick_move_step->nCachedBits | 4;
		correct_subtick_move_step->nCachedBits = v46;

		if (pressed)
			correct_subtick_move_step->flWhen = fraction;

		auto v48 = v46 | 1;
		correct_subtick_move_step->nCachedBits = v48;
		correct_subtick_move_step->nButton = button;

		correct_subtick_move_step->nCachedBits = v48 | 2;
		correct_subtick_move_step->bPressed = pressed;

	}

	void filter_sub_tick(int button) {
		if (!this)
			return;

		auto base_cmd = this->pBaseCmd;
		if (!base_cmd)
			return;

		auto cur_sub_tick = base_cmd->subtickMovesField.pRep;
		if (!cur_sub_tick)
			return;

		for (int i{ }; i < base_cmd->subtickMovesField.nCurrentSize; i++) {
			auto step_move = base_cmd->subtickMovesField.pRep->tElements[i];
			
			if (!step_move || step_move->nButton != button)
				continue;

			auto v46 = step_move->nCachedBits | 4;
			auto v48 = v46 | 1;

			step_move->nCachedBits = v48;
			step_move->nButton = 0;

		}
	}

	static CBaseUserCmdPB* GetBase();
	static CUserCmd* Get();


};


#define MULTIPLAYER_BACKUP 150

class CTinyMoveStepData
{
public:
	float flWhen; //0x0000
	pad(0x4); //0x0004
	std::uint64_t nButton; //0x0008
	bool bPressed; //0x0010
	pad(0x7); //0x0011
}; //Size: 0x0018

class CMoveStepButtons
{
public:
	std::uint64_t nKeyboardPressed; //0x0000
	std::uint64_t nMouseWheelheelPressed; //0x0008
	std::uint64_t nUnPressed; //0x0010
	std::uint64_t nKeyboardCopy; //0x0018
}; //Size: 0x0020

// @credits: www.unknowncheats.me/forum/members/2943409.html
class CExtendedMoveData : public CMoveStepButtons
{
public:
	float flForwardMove; //0x0020
	float flSideMove; //0x0024
	float flUpMove; //0x0028
	std::int32_t nMouseDeltaX; //0x002C
	std::int32_t nMouseDeltaY; //0x0030
	std::int32_t nAdditionalStepMovesCount; //0x0034
	CTinyMoveStepData tinyMoveStepData[12]; //0x0038
	Vector_t vecViewAngle; //0x0158
	std::int32_t nTargetHandle; //0x0164
}; //Size:0x0168
class CCSInputMessage
{
public:
	std::int32_t m_nFrameTickCount; //0x0000
	float m_flFrameTickFraction; //0x0004
	std::int32_t m_nPlayerTickCount; //0x0008
	float m_flPlayerTickFraction; //0x000C
	QAngle_t m_angViewAngles; //0x0010
	Vector_t m_vecShootPosition; //0x001C 
	std::int32_t m_nTargetIndex; //0x0028
	Vector_t m_vecTargetHeadPosition; //0x002C What's this???
	Vector_t m_vecTargetAbsOrigin; //0x0038
	Vector_t m_vecTargetAngle; //0x0044
	std::int32_t m_nsvShowHitRegistration; //0x0050
	std::int32_t m_nEntryIndexMax; //0x0054
	std::int32_t m_nIndexUnk; //0x0058
};
static_assert(sizeof(CCSInputMessage) == 0x5C);

class CTinyStepButtonPB
{
public:
	char pad[0x158];
	Vector_t angle;
};

class CTinyStepButton
{
public:
	int64_t size;
	CTinyStepButtonPB* pb;
};
#define recast_at_pointer( type, base, offset ) *reinterpret_cast< type* >( ( reinterpret_cast< std::uintptr_t >( base ) + offset ) )

class CCSGOInput
{

public:
	char pad_0000[592]; //0x0000
	bool bBlockShot; //0x0250
	bool bInThirdPerson; //0x0251
	char pad_0252[6]; //0x0252
	QAngle_t angThirdPersonAngles; //0x0258
	char pad_0264[20]; //0x0264
	uint64_t nKeyboardPressed; //0x0278
	uint64_t nMouseWheelheelPressed; //0x0280
	uint64_t nUnPressed; //0x0288
	uint64_t nKeyboardCopy; //0x0290
	float flForwardMove; //0x0298
	float flSideMove; //0x029C
	float flUpMove; //0x02A0
	Vector2D_t nMousePos; //0x02A4
	int32_t SubticksCount; //0x02AC
	CTinyMoveStepData Subticks[12]; //0x02B0
	Vector_t vecViewAngle; //0x03D0
	int32_t nTargetHandle; //0x03DC
	char pad_03E0[560]; //0x03E0
	int32_t pad1; //0x0610
	int32_t pad12; //0x0614
	int32_t nAttackStartHistoryIndex1; //0x0618
	int32_t nAttackStartHistoryIndex2; //0x061c
	int32_t nAttackStartHistoryIndex3; //0x0620
	char pad_061C[4]; //0x061C
	int32_t MessageSize; //0x0620
	char pad_0624[4]; //0x0624
	CCSInputMessage* Message; //0x0628
public:
	bool push(uint64_t button, bool held, float when = 0.90f) {
		if (SubticksCount >= 12)
			return false;

		Subticks[SubticksCount++].flWhen = when;
		Subticks[SubticksCount++].nButton = button;
		Subticks[SubticksCount++].bPressed = held;

		return true;
	}

	uint8_t release(uint64_t button) {
		uint8_t c = 0;
		for (int i = 0; i < SubticksCount; i++) {
			if (Subticks[i].nButton == button) {
				memmove(Subticks + i, Subticks + i + 1, (SubticksCount - i - 1) * sizeof(CTinyMoveStepData));
				SubticksCount--;
				i--;
				c++;
			}
		}
		return c;
	}

	void add_button(ECommandButtons button);
	/*
	void remove_button(ECommandButtons button) {
		const auto button_state{ hooks::m_get_button_state(this, 0) };

		auto& buttonCount{ recast_at_pointer(int, button_state, 0x34) };

		auto v6{ recast_at_pointer(int, this, 0x54F8) };

		while ((int)v6 < buttonCount) {
			auto currentButton{ (BYTE*)(reinterpret_cast<std::uintptr_t>(button_state) + 24 * (3 + v6)) };

			auto& v15{ recast_at_pointer(__int64, currentButton, -8) };
			if (v15 & button)
				v15 = 0;

			++v6;
		}
	}
	*/
	QAngle_t get_view_angles()
	{
		using fnGetViewAngles = std::int64_t(CS_FASTCALL*)(CCSGOInput*, std::int32_t);
		static auto oGetViewAngles = reinterpret_cast<fnGetViewAngles>(utils::FindPattern(xorstr_("client.dll"), xorstr_("4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3")));

		return *reinterpret_cast<QAngle_t*>(oGetViewAngles(this, 0));
	}
	void set_input_angle(QAngle_t& angView, bool client_side = false)
	{
		if (client_side)
			angView.z = 0.f;
		// @ida: this got called before GetMatricesForView
		using fnSetViewAngle = std::int64_t(__fastcall*)(void*, std::int32_t, QAngle_t&);
		static auto oSetViewAngle = reinterpret_cast<fnSetViewAngle>(utils::FindPattern("client.dll", ("85 D2 75 3F 48")));


		oSetViewAngle(this, 0, std::ref(angView));
	}
};

namespace crt_hash {
	struct c_in_button_vtable
	{
	public:
		std::uint64_t nValue;
		std::uint64_t nValueChanged;
		std::uint64_t nValueScroll;
	};

	struct stored_data
	{
		c_in_button_vtable cmdButtons;
		c_in_button_vtable nButtons;
		QAngle_t angView;
		CMsgQAngle* view_nagles;
		float flForwardMove;
		float flSideMove;
		float flUpMove;
		float nMousedX;
		float nMousedY;
		int nClientTick;
		RepeatedPtrField_t<CSubtickMoveStep> subtickMovesField; // 0x18
		std::string* strMoveCrc;
		std::int32_t nLegacyCommandNumber;
		std::int32_t nImpulse;
		std::int32_t nWeaponSelect;
		std::int32_t nRandomSeed;
		std::uint32_t nConsumedServerAngleChanges;
		std::int32_t nCmdFlags;
		std::uint32_t nPawnEntityHandle;
	};

	inline stored_data m_data;

	inline void store(CUserCmd* cmd, CBaseUserCmdPB* pBaseCmd)
	{
		if (!cmd)
			return;

		if (!pBaseCmd)
			return;

		//m_data.cmdButtons.nValue = cmd->nButtons.nValue;
		//m_data.cmdButtons.nValueChanged = cmd->nButtons.nValueChanged;
		//m_data.cmdButtons.nValueScroll = cmd->nButtons.nValueScroll;

		if (pBaseCmd->view_angles != nullptr)
			m_data.angView = pBaseCmd->view_angles->angValue;

		m_data.nButtons.nValue = pBaseCmd->pInButtonState->nValue;
		m_data.nButtons.nValueChanged = pBaseCmd->pInButtonState->nValueChanged;
		m_data.nButtons.nValueScroll = pBaseCmd->pInButtonState->nValueScroll;

		m_data.flForwardMove = pBaseCmd->flForwardMove;
		m_data.flSideMove = pBaseCmd->flSideMove;
		m_data.flUpMove = pBaseCmd->flUpMove;

		m_data.nMousedX = pBaseCmd->nMousedX;
		m_data.nMousedY = pBaseCmd->nMousedY;

		m_data.nClientTick = pBaseCmd->nClientTick;
		/*
		m_data.subtickMovesField = pBaseCmd->subtickMovesField;
		m_data.nImpulse = pBaseCmd->nImpulse;
		m_data.nCmdFlags = pBaseCmd->nCmdFlags;
		m_data.nWeaponSelect = pBaseCmd->nWeaponSelect;
		m_data.nRandomSeed = pBaseCmd->nRandomSeed;
		m_data.nPawnEntityHandle = pBaseCmd->nPawnEntityHandle;
		m_data.strMoveCrc = pBaseCmd->strMoveCrc;
		m_data.nLegacyCommandNumber = pBaseCmd->nLegacyCommandNumber;*/
	}

	inline void update(CUserCmd* pCmd)
	{
		if (!pCmd)
			return;

		CBaseUserCmdPB* pBaseCmd = pCmd->pBaseCmd;
		if (!pBaseCmd)
			return;

		pCmd->nButtons.nValue = m_data.nButtons.nValue;
		pCmd->nButtons.nValueChanged = m_data.nButtons.nValueChanged;
		pCmd->nButtons.nValueScroll = m_data.nButtons.nValueScroll;

	//	pBaseCmd->pInButtonState->nValue = m_data.nButtons.nValue;
	//	pBaseCmd->pInButtonState->nValueChanged = m_data.nButtons.nValueChanged;
	//	pBaseCmd->pInButtonState->nValueScroll = m_data.nButtons.nValueScroll;

		pBaseCmd->flForwardMove = m_data.flForwardMove;
		pBaseCmd->flSideMove = m_data.flSideMove;
		pBaseCmd->flUpMove = m_data.flUpMove;

		pBaseCmd->nMousedX = m_data.nMousedX;
		pBaseCmd->nMousedY = m_data.nMousedY;
		pBaseCmd->nClientTick = m_data.nClientTick;

		if (pBaseCmd->view_angles != nullptr)
			pBaseCmd->view_angles->angValue = m_data.angView;

	//		pBaseCmd->view_angles = m_data.view_nagles;
	/*	pBaseCmd->nImpulse = m_data.nImpulse;
		pBaseCmd->nCmdFlags = m_data.nCmdFlags;
		pBaseCmd->nWeaponSelect = m_data.nWeaponSelect;
		pBaseCmd->nRandomSeed = m_data.nRandomSeed;
		pBaseCmd->nPawnEntityHandle = m_data.nPawnEntityHandle;
		pBaseCmd->strMoveCrc = m_data.strMoveCrc;
		pBaseCmd->nLegacyCommandNumber = m_data.nLegacyCommandNumber;*/
	}


}


// helper class to handle cs2 input system (and subticks)

class c_input_handler {
public:
	static c_input_handler& Get() {
		static c_input_handler inst;
		return inst;
	}

	void Poll(CCSGOInput* input);
	void Press(uint64_t button, float when = 0.5f);
	bool Release(uint64_t button);
	void Dump();

	bool IsHeld(uint64_t button) const { return buttonsHeld & button; }
	bool IsPressed(uint64_t button) const { return IsScrolled(button) || (IsHeld(button) && !(prevButtonsHeld & button)); }
	bool IsReleased(uint64_t button) const { return IsScrolled(button) || (!IsHeld(button) && (prevButtonsHeld & button)); }
	bool IsScrolled(uint64_t button) const { return buttonsScroll & button; }

private:
	CExtendedMoveData* move; // not needed but convenient

	uint64_t prevButtonsHeld;
	uint64_t buttonsHeld;
	uint64_t buttonsScroll;
};