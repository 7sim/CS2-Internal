#pragma once
#include <cstdint>

#include "../../utils/utils.h"

#include "../schema/schema.h"
#include "../handle/handle.hpp"

#include "../../math/math.h"
#include "../../math/elements/vector.h"
#include "../../math/elements/qangle.h"

#include "../../math/elements/transform.h"
#include "../../math/elements/quaternion.h"


#include "../../sdk/datatypes/color.h"
#include "../../sdk/datatypes/cutlstring/utlstring.h"
#include "../../sdk/datatypes/cutlstring/utlstringtoken.h"

#include "../../sdk/datatypes/cutlvectors/utlvector.h"
#include "../../sdk/datatypes/cutlhandles/binding_handle.hpp"
#include "../interfaces/csgoinput.h"

#include "../interfaces/entitysystem.h"
#include "../interfaces/schemasystem.h"
#include "../interfaces/engineclient.h"
#include "..\..\sdk\datatypes\cutlstring\utlbufferstring.h"
#include "..\schema\schema.h"

class c_utl_binary_block
{
public:
	CUtlMemory<unsigned char> m_Memory;
	int m_nActualLength;
};
enum HitGroup_t : std::uint32_t {
	HITGROUP_INVALID = -1,
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD = 1,
	HITGROUP_CHEST = 2,
	HITGROUP_STOMACH = 3,
	HITGROUP_LEFTARM = 4,
	HITGROUP_RIGHTARM = 5,
	HITGROUP_LEFTLEG = 6,
	HITGROUP_RIGHTLEG = 7,
	HITGROUP_NECK = 8,
	HITGROUP_UNUSED = 9,
	HITGROUP_GEAR = 10,
	HITGROUP_SPECIAL = 11,
	HITGROUP_COUNT = 12,
};

enum HitBoxes : uint32_t {
	HEAD = 6,
	NECK = 5,
	CHEST = 4,
	RIGHT_CHEST = 8,
	LEFT_CHEST = 13,
	STOMACH = 3,
	PELVIS = 2,
	CENTER = 1,
	L_LEG = 23,
	L_FEET = 24,
	R_LEG = 26,
	R_FEET = 27
};
enum e_phitgroups : std::uint32_t
{
	hitgroup_generic,
	hitgroup_head,
	hitgroup_chest,
	hitgroup_stomach,
	hitgroup_leftarm,
	hitgroup_rightarm,
	hitgroup_leftleg,
	hitgroup_rightleg,
	hitgroup_neck,
	hitgroup_gear = 10U
};
enum EHitBoxes : std::int32_t
{
	HITBOX_INVALID = -1,
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_THORAX,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_MAX
};

enum e_hitboxes : std::uint32_t
{
	/* head */
	hitbox_head,
	hitbox_neck,
	/* body */
	hitbox_pelvis,
	hitbox_stomach,
	hitbox_chest,
	hitbox_lower_chest,
	hitbox_upper_chest,
	/* legs */
	hitbox_right_thigh,
	hitbox_left_thigh,
	hitbox_right_calf,
	hitbox_left_calf,
	hitbox_right_foot,
	hitbox_left_foot,
	/* arms */
	hitbox_right_hand,
	hitbox_left_hand,
	hitbox_right_upper_arm,
	hitbox_right_forearm,
	hitbox_left_upper_arm,
	hitbox_left_forearm,
	hitbox_max
};

class CFiringModeFloat
{
public:
	float flValue[2];
};


class CAnimDesc_Flag
{
public:
	bool m_bLooping; // 0x0	
	bool m_bAllZeros; // 0x1	
	bool m_bHidden; // 0x2	
	bool m_bDelta; // 0x3	
	bool m_bLegacyWorldspace; // 0x4	
	bool m_bModelDoc; // 0x5	
	bool m_bImplicitSeqIgnoreDelta; // 0x6	
	bool m_bAnimGraphAdditive; // 0x7	
};

class CAnimBoneDifference
{
public:
	c_buffer_string m_name; // 0x0	
	c_buffer_string m_parent; // 0x10	
	Vector_t m_posError; // 0x20	
	bool m_bHasRotation; // 0x2c	
	bool m_bHasMovement; // 0x2d	
};

class CAnimMorphDifference
{
public:
	c_buffer_string m_name; // 0x0	
};

class CAnimUserDifference
{
public:
	c_buffer_string m_name; // 0x0	
	int32_t m_nType; // 0x10	
};

class CAnimEncodeDifference
{
public:
	CUtlVector< CAnimBoneDifference > m_boneArray; // 0x0	
	CUtlVector< CAnimMorphDifference > m_morphArray; // 0x18	
	CUtlVector< CAnimUserDifference > m_userArray; // 0x30	
	CUtlVector< unsigned char > m_bHasRotationBitArray; // 0x48	
	CUtlVector< unsigned char > m_bHasMovementBitArray; // 0x60	
	CUtlVector< unsigned char > m_bHasMorphBitArray; // 0x78	
	CUtlVector< unsigned char > m_bHasUserBitArray; // 0x90	
};

class CAnimFrameBlockAnim
{
public:
	int32_t m_nStartFrame; // 0x0	
	int32_t m_nEndFrame; // 0x4	
	CUtlVector< int32_t > m_segmentIndexArray; // 0x8	
};

class CAnimEncodedFrames
{
public:
	c_buffer_string m_fileName; // 0x0	
	int32_t m_nFrames; // 0x10	
	int32_t m_nFramesPerBlock; // 0x14	
	CUtlVector< CAnimFrameBlockAnim > m_frameblockArray; // 0x18	
	// -> m_boneArray - 0x30
	// -> m_morphArray - 0x48
	// -> m_userArray - 0x60
	// -> m_bHasRotationBitArray - 0x78
	// -> m_bHasMovementBitArray - 0x90
	// -> m_bHasMorphBitArray - 0xa8
	// -> m_bHasUserBitArray - 0xc0
	CAnimEncodeDifference m_usageDifferences; // 0x30	
};

class CAnimEventDefinition
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x8]; // 0x0
public:
	int32_t m_nFrame; // 0x8	
	float m_flCycle; // 0xc	
	//KeyValues3 m_EventData; // 0x10	
	// MKV3TransferName "m_sOptions"
	c_buffer_string m_sLegacyOptions; // 0x20	
	//CGlobalSymbol m_sEventName; // 0x30	
};

class CAnimMovement
{
public:
	int32_t endframe; // 0x0	
	int32_t motionflags; // 0x4	
	float v0; // 0x8	
	float v1; // 0xc	
	float angle; // 0x10	
	Vector_t vector_; // 0x14	
	Vector_t position; // 0x20	
};

class CAnimLocalHierarchy
{
public:
	c_buffer_string m_sBone; // 0x0	
	c_buffer_string m_sNewParent; // 0x10	
	int32_t m_nStartFrame; // 0x20	
	int32_t m_nPeakFrame; // 0x24	
	int32_t m_nTailFrame; // 0x28	
	int32_t m_nEndFrame; // 0x2c	
};

class CAnimActivity
{
public:
	c_buffer_string m_name; // 0x0	
	int32_t m_nActivity; // 0x10	
	int32_t m_nFlags; // 0x14	
	int32_t m_nWeight; // 0x18	
};

class CAnimSequenceParams
{
public:
	float m_flFadeInTime; // 0x0	
	float m_flFadeOutTime; // 0x4	
};

class CAnimDesc
{
public:
	c_buffer_string m_name; // 0x0	
	// -> m_bLooping - 0x10
	// -> m_bAllZeros - 0x11
	// -> m_bHidden - 0x12
	// -> m_bDelta - 0x13
	// -> m_bLegacyWorldspace - 0x14
	// -> m_bModelDoc - 0x15
	// -> m_bImplicitSeqIgnoreDelta - 0x16
	// -> m_bAnimGraphAdditive - 0x17
	CAnimDesc_Flag m_flags; // 0x10	
	float fps; // 0x18	
private:
	[[maybe_unused]] uint8_t __pad001c[0x4]; // 0x1c
public:
	// MKV3TransferName "m_pData"
	// -> m_fileName - 0x20
	// -> m_nFrames - 0x30
	// -> m_nFramesPerBlock - 0x34
	// -> m_frameblockArray - 0x38
	// -> m_usageDifferences - 0x50
	CAnimEncodedFrames m_Data; // 0x20	
	CUtlVector< CAnimMovement > m_movementArray; // 0xf8	
	CUtlVector< CAnimEventDefinition > m_eventArray; // 0x110	
	CUtlVector< CAnimActivity > m_activityArray; // 0x128	
	CUtlVector< CAnimLocalHierarchy > m_hierarchyArray; // 0x140	
	float framestalltime; // 0x158	
	Vector_t m_vecRootMin; // 0x15c	
	Vector_t m_vecRootMax; // 0x168	
private:
	[[maybe_unused]] uint8_t __pad0174[0x4]; // 0x174
public:
	CUtlVector< Vector_t > m_vecBoneWorldMin; // 0x178	
	CUtlVector< Vector_t > m_vecBoneWorldMax; // 0x190	
	// -> m_flFadeInTime - 0x1a8
	// -> m_flFadeOutTime - 0x1ac
	CAnimSequenceParams m_sequenceParams; // 0x1a8	
};

class CAnimDecoder
{
public:
	c_buffer_string m_szName; // 0x0	
	int32_t m_nVersion; // 0x10	
	int32_t m_nType; // 0x14	
};

class CAnimFrameSegment
{
public:
	int32_t m_nUniqueFrameIndex; // 0x0	
	uint32_t m_nLocalElementMasks; // 0x4	
	int32_t m_nLocalChannel; // 0x8	
private:
	[[maybe_unused]] uint8_t __pad000c[0x4]; // 0xc
public:
	c_utl_binary_block m_container; // 0x10	
};

class CAnimData
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x10]; // 0x0
public:
	c_buffer_string m_name; // 0x10	
	CUtlVector< CAnimDesc > m_animArray; // 0x20	
	CUtlVector< CAnimDecoder > m_decoderArray; // 0x38	
	int32_t m_nMaxUniqueFrameIndex; // 0x50	
private:
	[[maybe_unused]] uint8_t __pad0054[0x4]; // 0x54
public:
	CUtlVector< CAnimFrameSegment > m_segmentArray; // 0x58	
};

struct ModelSkeletonData_t
{
public:
	CUtlVector< CUtlString > m_boneName; // 0x0	
	char pad[0x8];
	CUtlVector< int16_t > m_nParent; // 0x18	
	char pad2[0x8];
	CUtlVector< float > m_boneSphere; // 0x30	
	char pad3[0x8];
	CUtlVector< uint32_t > m_nFlag; // 0x48	
	char pad4[0x8];
	CUtlVector< Vector_t > m_bonePosParent; // 0x60	
	char pad5[0x8];
	CUtlVector< QAngle_t > m_boneRotParent; // 0x78	
	char pad6[0x8];
	CUtlVector< float > m_boneScaleParent; // 0x90	
	char pad7[0x8];
};

class CModelConfigElement
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x8]; // 0x0
public:
	CUtlString m_ElementName; // 0x8	
	CUtlVector< CModelConfigElement* > m_NestedElements; // 0x10	
};


class CModelConfig
{
public:
	CUtlString m_ConfigName; // 0x0	
	CUtlVector< CModelConfigElement* > m_Elements; // 0x8	
	bool m_bTopLevel; // 0x20	
};

class CModelConfigList
{
public:
	bool m_bHideMaterialGroupInTools; // 0x0	
	bool m_bHideRenderColorInTools; // 0x1	
private:
	[[maybe_unused]] uint8_t __pad0002[0x6]; // 0x2
public:
	CUtlVector< CModelConfig* > m_Configs; // 0x8	
};

enum class ModelBoneFlexComponent_t : uint32_t
{
	MODEL_BONE_FLEX_INVALID = 0xfffffff,
	MODEL_BONE_FLEX_TX = 0x0,
	MODEL_BONE_FLEX_TY = 0x1,
	MODEL_BONE_FLEX_TZ = 0x2,
};

struct ModelBoneFlexDriverControl_t
{
public:
	ModelBoneFlexComponent_t m_nBoneComponent; // 0x0	
	CUtlString m_flexController; // 0x8	
	uint32_t m_flexControllerToken; // 0x10	
	float m_flMin; // 0x14	
	float m_flMax; // 0x18	
};

struct ModelBoneFlexDriver_t
{
public:
	CUtlString m_boneName; // 0x0	
	uint32_t m_boneNameToken; // 0x8	
private:
	[[maybe_unused]] uint8_t __pad000c[0x4]; // 0xc
public:
	CUtlVector< ModelBoneFlexDriverControl_t > m_controls; // 0x10	
};

class i_material;

struct MaterialGroup_t
{
public:
	CUtlString m_name; // 0x0	
	CUtlVector< stronghandle< i_material > > m_materials; // 0x8	
};

struct PermModelDataAnimatedMaterialAttribute_t
{
public:
	CUtlString m_AttributeName; // 0x0	
	int32_t m_nNumChannels; // 0x8	
};

class CSequenceGroupData
{
public:
	char pad[0x138];
};

class CAnimBone
{
public:
	c_buffer_string m_name; // 0x0	
	int32_t m_parent; // 0x10	
	Vector_t m_pos; // 0x14	
	QAngle_t m_quat; // 0x20	
	float m_scale; // 0x30	
	QAngle_t m_qAlignment; // 0x34	
	int32_t m_flags; // 0x44	
};

// Registered binary: animationsystem.dll (project 'animationsystem')
// Alignment: 8
// Size: 0x18
// 
// MGetKV3ClassDefaults
class CAnimUser
{
public:
	c_buffer_string m_name; // 0x0	
	int32_t m_nType; // 0x10	
};


using ItemDefinitionIndex_t = std::uint16_t;
enum EItemDefinitionIndexes : ItemDefinitionIndex_t
{
	WEAPON_NONE,
	WEAPON_DESERT_EAGLE,
	WEAPON_DUAL_BERETTAS,
	WEAPON_FIVE_SEVEN,
	WEAPON_GLOCK_18,
	WEAPON_AK_47 = 7,
	WEAPON_AUG,
	WEAPON_AWP,
	WEAPON_FAMAS,
	WEAPON_G3SG1,
	WEAPON_GALIL_AR = 13,
	WEAPON_M249,
	WEAPON_M4A4 = 16,
	WEAPON_MAC_10,
	WEAPON_P90 = 19,
	WEAPON_REPULSOR_DEVICE,
	WEAPON_MP5_SD = 23,
	WEAPON_UMP_45,
	WEAPON_XM1014,
	WEAPON_PP_BIZON,
	WEAPON_MAG_7,
	WEAPON_NEGEV,
	WEAPON_SAWED_OFF,
	WEAPON_TEC_9,
	WEAPON_ZEUS_X27,
	WEAPON_P2000,
	WEAPON_MP7,
	WEAPON_MP9,
	WEAPON_NOVA,
	WEAPON_P250,
	WEAPON_RIOT_SHIELD,
	WEAPON_SCAR_20,
	WEAPON_SG_553,
	WEAPON_SSG_08,
	WEAPON_KNIFE0,
	WEAPON_KNIFE1,
	WEAPON_FLASHBANG,
	WEAPON_HIGH_EXPLOSIVE_GRENADE,
	WEAPON_SMOKE_GRENADE,
	WEAPON_MOLOTOV,
	WEAPON_DECOY_GRENADE,
	WEAPON_INCENDIARY_GRENADE,
	WEAPON_C4_EXPLOSIVE,
	WEAPON_KEVLAR_VEST,
	WEAPON_KEVLAR_and_HELMET,
	WEAPON_HEAVY_ASSAULT_SUIT,
	WEAPON_NO_LOCALIZED_NAME0 = 54,
	WEAPON_DEFUSE_KIT,
	WEAPON_RESCUE_KIT,
	WEAPON_MEDI_SHOT,
	WEAPON_MUSIC_KIT,
	WEAPON_KNIFE2,
	WEAPON_M4A1_S,
	WEAPON_USP_S,
	WEAPON_TRADE_UP_CONTRACT,
	WEAPON_CZ75_AUTO,
	WEAPON_R8_REVOLVER,
	WEAPON_TACTICAL_AWARENESS_GRENADE = 68,
	WEAPON_BARE_HANDS,
	WEAPON_BREACH_CHARGE,
	WEAPON_TABLET = 72,
	WEAPON_KNIFE3 = 74,
	WEAPON_AXE,
	WEAPON_HAMMER,
	WEAPON_WRENCH = 78,
	WEAPON_SPECTRAL_SHIV = 80,
	WEAPON_FIRE_BOMB,
	WEAPON_DIVERSION_DEVICE,
	WEAPON_FRAG_GRENADE,
	WEAPON_SNOWBALL,
	WEAPON_BUMP_MINE,
	WEAPON_BAYONET = 500,
	WEAPON_CLASSIC_KNIFE = 503,
	WEAPON_FLIP_KNIFE = 505,
	WEAPON_GUT_KNIFE,
	WEAPON_KARAMBIT,
	WEAPON_M9_BAYONET,
	WEAPON_HUNTSMAN_KNIFE,
	WEAPON_FALCHION_KNIFE = 512,
	WEAPON_BOWIE_KNIFE = 514,
	WEAPON_BUTTERFLY_KNIFE,
	WEAPON_SHADOW_DAGGERS,
	WEAPON_PARACORD_KNIFE,
	WEAPON_SURVIVAL_KNIFE,
	WEAPON_URSUS_KNIFE,
	WEAPON_NAVAJA_KNIFE,
	WEAPON_NOMAD_KNIFE,
	WEAPON_STILETTO_KNIFE,
	WEAPON_TALON_KNIFE,
	WEAPON_SKELETON_KNIFE = 525,
};

class CAnimDataChannelDesc
{
public:
	c_buffer_string m_szChannelClass; // 0x0	
	c_buffer_string m_szVariableName; // 0x10	
	int32_t m_nFlags; // 0x20	
	int32_t m_nType; // 0x24	
	c_buffer_string m_szGrouping; // 0x28	ru-chat
	c_buffer_string m_szDescription; // 0x38	
	CUtlVector< c_buffer_string > m_szElementNameArray; // 0x48	
	CUtlVector< int32_t > m_nElementIndexArray; // 0x60	
	CUtlVector< uint32_t > m_nElementMaskArray; // 0x78	
};

class CAnimKeyData
{
public:
	c_buffer_string m_name; // 0x0	
	CUtlVector< CAnimBone > m_boneArray; // 0x10	
	CUtlVector< CAnimUser > m_userArray; // 0x28	
	CUtlVector< c_buffer_string > m_morphArray; // 0x40	
	int32_t m_nChannelElements; // 0x58	
private:
	[[maybe_unused]] uint8_t __pad005c[0x4]; // 0x5c
public:
	CUtlVector< CAnimDataChannelDesc > m_dataChannelArray; // 0x60	
};

class CAnimationGroup
{
private:
	[[maybe_unused]] uint8_t __pad0000[0x10]; // 0x0
public:
	uint32_t m_nFlags; // 0x10	
private:
	[[maybe_unused]] uint8_t __pad0014[0x4]; // 0x14
public:
	c_buffer_string m_name; // 0x18	
private:
	[[maybe_unused]] uint8_t __pad0028[0x38]; // 0x28
public:
	// MKV3TransferName "m_localHAnimArray"
	CUtlVector< stronghandle< CAnimData > > m_localHAnimArray_Handle; // 0x60	
	// MKV3TransferName "m_includedGroupArray"
	CUtlVector< stronghandle< CAnimationGroup > > m_includedGroupArray_Handle; // 0x78	
	// MKV3TransferName "m_directHSeqGroup"
	CUtlVector<stronghandle< CSequenceGroupData >> m_directHSeqGroup_Handle; // 0x90	
	// -> m_name - 0x98
	// -> m_boneArray - 0xa8
	// -> m_userArray - 0xc0
	// -> m_morphArray - 0xd8
	// -> m_nChannelElements - 0xf0
	// -> m_dataChannelArray - 0xf8
	CAnimKeyData m_decodeKey; // 0x98	
	CUtlVector< c_buffer_string > m_szScripts; // 0x110	
};

class CAnimationGroupResource
{
public:
	char pad[0x10];
	CAnimationGroup* animation_group;
	const char* path_mdl;
};

using GameTime_t = std::int32_t;
using GameTick_t = std::int32_t;

class CCSPlayerPawn;
class CCSPlayer_CameraServices
{
public:
    schema( int, m_iFOV, "CCSPlayerBase_CameraServices->m_iFOV" );
    schema( int, m_iFOVStart, "CCSPlayerBase_CameraServices->m_iFOVStart" );
    schema( float, m_flFOVTime, "CCSPlayerBase_CameraServices->m_flFOVTime" );
    schema( float, m_flFOVRate, "CCSPlayerBase_CameraServices->m_flFOVRate" );
    schema( CBaseHandle, m_hZoomOwner, "CCSPlayerBase_CameraServices->m_hZoomOwner" );
    schema( float, m_flLastShotFOV, "CCSPlayerBase_CameraServices->m_flLastShotFOV" );
    schema( float, m_flDeathCameraTilt, "CCSPlayer_CameraServices->m_flDeathCameraTilt" );
	schema(C_NetworkUtlVectorBase<CBaseHandle>, m_PostProcessingVolumes, "CPlayer_CameraServices->m_PostProcessingVolumes");

};

enum LifeState_t : uint32_t
{
	LIFE_ALIVE = 0x0,
	LIFE_DYING = 0x1,
	LIFE_DEAD = 0x2,
	LIFE_RESPAWNABLE = 0x3,
	LIFE_RESPAWNING = 0x4,
};

enum MoveType_t : std::uint8_t
{
	MOVETYPE_NONE = 0,
	MOVETYPE_OBSOLETE,
	MOVETYPE_WALK,
	MOVETYPE_FLY,
	MOVETYPE_FLYGRAVITY,
	MOVETYPE_VPHYSICS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_OBSERVER,
	MOVETYPE_LADDER,
	MOVETYPE_CUSTOM,
	MOVETYPE_LAST,
	MOVETYPE_INVALID,
	MOVETYPE_MAX_BITS = 5
};
struct ClientClass_t
{
	char pad[ 0x8 ];
	const char* m_name;
};
class CViewSetup
{
public:
	pad(0x490);
	float flOrthoLeft; // 0x0494
	float flOrthoTop; // 0x0498
	float flOrthoRight; // 0x049C
	float flOrthoBottom; // 0x04A0
	pad(0x38);
	float flFov; // 0x04D8
	float flFovViewmodel; // 0x04DC
	Vector_t vecOrigin; // 0x04E0
	pad(0xC); // 0x04EC
	QAngle_t angView; // 0x04F8
	pad(0x14); // 0x0504
	float flAspectRatio; // 0x0518
	pad(0x71);
	BYTE nSomeFlags;
};
class IViewRender
{
public:
	char pad[0x10];
	CViewSetup view_setup;
};

class CEntityInstance;
class CEntityIdentity 
{
public:

	SCHEMA_ADD_OFFSET(std::uint32_t, index, 0x10);
	schema(std::uint32_t, flags, "CEntityIdentity->m_flags");
	schema(const char*, m_designerName, "CEntityIdentity->m_designerName");

	std::string GetSchemaName() {

		auto class_info = *(uintptr_t*)(std::uintptr_t(this) + 0x10);
		auto name = *(const char*)(std::uintptr_t(this) + 0x20);

		return std::to_string(name);  // Return the string constructed from the char pointer
	}

	[[nodiscard]] bool valid()
	{
		return index() != INVALID_EHANDLE_INDEX;
	}

	[[nodiscard]] int get_index()
	{
		if (!valid())
			return ENT_ENTRY_MASK;

		return index() & ENT_ENTRY_MASK;
	}

	[[nodiscard]] int get_serial_number()
	{
		return index() >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	CEntityInstance* pInstance;
};

class CEntityInstance
{
public:

	void dump_class_info(SchemaClassInfoData_t** pReturn)
	{
		return utils::CallVFunc<void, 38U>(this, pReturn);
	}
	

	[[nodiscard]] std::uint32_t get_entity_by_handle()
	{
		CEntityIdentity* identity = m_pEntityIdentity();
		if (identity == nullptr)
			return 0;


		return identity->get_index();
	}

	[[nodiscard]] CBaseHandle handle()
	{
		CEntityIdentity* identity = m_pEntityIdentity();
		if (identity == nullptr)
			return CBaseHandle();

		return CBaseHandle(identity->get_index(), identity->get_serial_number() - (identity->flags() & 1));
	}
	schema(bool, m_bVisibleinPVS, "CEntityInstance->m_bVisibleinPVS");

	schema( CEntityIdentity*, m_pEntityIdentity, "CEntityInstance->m_pEntity" );
};

class CGlowProperty
{
public:
	schema(bool, m_bGlowing, "CGlowProperty->m_bGlowing");
	schema(int, m_iGlowType, "CGlowProperty->m_iGlowType");
	schema(Color_t, m_glowColorOverride, "CGlowProperty->m_glowColorOverride");

	CEntityInstance* GetOwner()
	{
		return *reinterpret_cast<CEntityInstance**>((uintptr_t)this + 0x18);
	}

};

class CSkeletonInstance;
class CGameSceneNode 
{
public:
	schema( CEntityInstance*, m_pOwner, "CGameSceneNode->m_pOwner" );
	schema( CGameSceneNode*, m_pParent, "CGameSceneNode->m_pParent" );
	schema( CGameSceneNode*, m_pChild, "CGameSceneNode->m_pChild" );
	schema( CGameSceneNode*, m_pNextSibling, "CGameSceneNode->m_pNextSibling" );
	schema( Vector_t, m_vecOrigin, "CGameSceneNode->m_vecOrigin" );
	schema(QAngle_t, m_angRotation, "CGameSceneNode->m_angRotation" );
	schema(QAngle_t, m_angAbsRotation, "CGameSceneNode->m_angAbsRotation");


	schema( CTransform, m_nodeToWorld, "CGameSceneNode->m_nodeToWorld");
	schema( float, m_flScale, "CGameSceneNode->m_flScale" );
	schema( Vector_t, m_vecAbsOrigin, "CGameSceneNode->m_vecAbsOrigin" );
	schema( float, m_flAbsScale, "CGameSceneNode->m_flAbsScale" );
	schema( int16_t, m_nParentAttachmentOrBone, "CGameSceneNode->m_nParentAttachmentOrBone" );
	schema( bool, dormant, "CGameSceneNode->m_bDormant" );
	schema( Vector_t, m_vRenderOrigin, "CGameSceneNode->m_vRenderOrigin" );
	CSkeletonInstance* m_pSkeleton()
	{
		return utils::CallVFunc<CSkeletonInstance*, 8U>(this);
	}
	void SetMeshGroupMask(uint64_t meshGroupMask);
};

class VPhysicsCollisionAttribute_t {
public:
	schema(uint8_t, m_nCollisionFunctionMask, "VPhysicsCollisionAttribute_t->m_nCollisionFunctionMask");
};
class CCollisionProperty 
{
public:
	schema( Vector_t, m_vecMins, "CCollisionProperty->m_vecMins" );
	schema( Vector_t, m_vecMaxs, "CCollisionProperty->m_vecMaxs" );
	schema(uint8_t, m_usSolidFlags, "CCollisionProperty->m_usSolidFlags");
	schema(uint16_t, m_nSolidType, "CCollisionProperty->m_nSolidType");
	schema( uint8_t, m_CollisionGroup, "CCollisionProperty->m_CollisionGroup" );
	schema(VPhysicsCollisionAttribute_t, m_collisionAttribute, "CCollisionProperty->m_collisionAttribute");

};

enum EEconItemQuality {
	IQ_UNDEFINED = -1,
	IQ_NORMAL,
	IQ_GENUINE,
	IQ_VINTAGE,
	IQ_UNUSUAL,
	IQ_UNIQUE,
	IQ_COMMUNITY,
	IQ_DEVELOPER,
	IQ_SELFMADE,
	IQ_CUSTOMIZED,
	IQ_STRANGE,
	IQ_COMPLETED,
	IQ_HAUNTED,
	IQ_TOURNAMENT,
	IQ_FAVORED
};

enum EEconItemRarity {
	IR_DEFAULT,
	IR_COMMON,
	IR_UNCOMMON,
	IR_RARE,
	IR_MYTHICAL,
	IR_LEGENDARY,
	IR_ANCIENT,
	IR_IMMORTAL
};

// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/econ/econ_item_constants.h#L39
enum EEconTypeID {
	k_EEconTypeItem = 1,
	k_EEconTypePersonaDataPublic = 2,
	k_EEconTypeGameAccountClient = 7,
	k_EEconTypeGameAccount = 8,
	k_EEconTypeEquipInstance = 31,
	k_EEconTypeDefaultEquippedDefinitionInstance = 42,
	k_EEconTypeDefaultEquippedDefinitionInstanceClient = 43,
	k_EEconTypeCoupon = 45,
	k_EEconTypeQuest = 46,
};

// MGetKV3ClassDefaults
class SkeletonBoneBounds_t
{
public:
	Vector_t m_vecCenter; // 0x0	
	Vector_t m_vecSize; // 0xc	
};

// Registered binary: animationsystem.dll (project 'modellib')
// Alignment: 8
// Size: 0x60
// 
// MGetKV3ClassDefaults
class RenderSkeletonBone_t
{
public:

	SCHEMA_ADD_OFFSET(const char*, m_boneName, 0x0);
	SCHEMA_ADD_OFFSET(const char*, m_parentName, 0x8);
	SCHEMA_ADD_OFFSET(SkeletonBoneBounds_t, m_bbox, 0x40);
	SCHEMA_ADD_OFFSET(float, m_flSphereRadius, 0x58);

};

// Registered binary: animationsystem.dll (project 'modellib')
// Alignment: 8
// Size: 0x50
// 
// MGetKV3ClassDefaults
class CRenderSkeleton
{
public:
	SCHEMA_ADD_OFFSET(CUtlVector<RenderSkeletonBone_t>, m_bones, 0x0);
	SCHEMA_ADD_OFFSET(CUtlVector<int32_t>, m_boneParents, 0xB8);
	SCHEMA_ADD_OFFSET(int32_t, m_nBoneWeightCount, 0x48);

};


struct alignas(16) bone_data {
	Vector_t pos;
	float scale;
	Vector4D_t rot;
};

enum bone_flags : uint32_t {
	FLAG_NO_BONE_FLAGS = 0x0,
	FLAG_BONEFLEXDRIVER = 0x4,
	FLAG_CLOTH = 0x8,
	FLAG_PHYSICS = 0x10,
	FLAG_ATTACHMENT = 0x20,
	FLAG_ANIMATION = 0x40,
	FLAG_MESH = 0x80,
	FLAG_HITBOX = 0x100,
	FLAG_BONE_USED_BY_VERTEX_LOD0 = 0x400,
	FLAG_BONE_USED_BY_VERTEX_LOD1 = 0x800,
	FLAG_BONE_USED_BY_VERTEX_LOD2 = 0x1000,
	FLAG_BONE_USED_BY_VERTEX_LOD3 = 0x2000,
	FLAG_BONE_USED_BY_VERTEX_LOD4 = 0x4000,
	FLAG_BONE_USED_BY_VERTEX_LOD5 = 0x8000,
	FLAG_BONE_USED_BY_VERTEX_LOD6 = 0x10000,
	FLAG_BONE_USED_BY_VERTEX_LOD7 = 0x20000,
	FLAG_BONE_MERGE_READ = 0x40000,
	FLAG_BONE_MERGE_WRITE = 0x80000,
	FLAG_ALL_BONE_FLAGS = 0xfffff,
	BLEND_PREALIGNED = 0x100000,
	FLAG_RIGIDLENGTH = 0x200000,
	FLAG_PROCEDURAL = 0x400000,
};


class c_attachments
{
public:
	char		pad_0x0000[0x10];		  // 0x0000
	const char* m_key;					  // 0x0010
	char		pad_0x0018[0x8];		  // 0x0018
	const char* m_name;					  // 0x0020
	const char* m_influenceNames[3];	  // 0x0028
	char		pad_0x0030[0x10];		  // 0x0030
	Vector4D_t	m_vInfluenceRotations[3]; // 0x0040
	Vector_t		m_vInfluenceOffsets[3];	  // 0x0070
	float		m_influenceWeights[3];	  // 0x0094
	char		pad_0x00A0[0x10];		  // 0x00A0
};

class c_drawcalls
{
public:
	__int32 m_nPrimitiveType; // 0x0000
	__int32 m_nBaseVertex;	  // 0x0004
	__int32 m_nVertexCount;	  // 0x0008
	__int32 m_nStartIndex;	  // 0x000C
	__int32 m_nIndexCount;	  // 0x0010
	float	m_flUvDensity;	  // 0x0014
	float	m_vTintColor[3];  // 0x0018
	float	m_flAlpha;		  // 0x0024
	char	pad_0x0028[0xC0]; // 0x0028

}; // Size=0x00E8

class c_bones
{
public:
	const char* m_boneName;		   // 0x0000
	const char* m_parentName;	   // 0x0008
	float		m_invBindPose[12]; // 0x0010
	Vector_t		m_vecCenter;	   // 0x0040
	Vector_t		m_vecSize;		   // 0x004C
	float		m_flSphereradius;  // 0x0058
	char		pad_0x005C[0x4];   // 0x005C

}; // Size=0x0060


class CHitBox
{
public:


	const char* m_name() {
		return *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(this) + 0x0);
	}

	const char* m_sBoneName() {
		return *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(this) + 0x10);
	}

	Vector_t m_vMinBounds() {
		return *reinterpret_cast<Vector_t*>(reinterpret_cast<uintptr_t>(this) + 0x18);
	}

	Vector_t m_vMaxBounds() {
		return *reinterpret_cast<Vector_t*>(reinterpret_cast<uintptr_t>(this) + 0x24);
	}

	float m_flShapeRadius() {
		return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + 0x30);
	}

	int32_t m_hitbox_group_id() {
		return *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + 0x38);
	}
	uint16_t m_nHitBoxIndex() {
		return *reinterpret_cast<uint16_t*>(reinterpret_cast<uintptr_t>(this) + 0x48);
	}

private:
	// Size of 'CHitBox' class. Can be obtainted through the SchemaSystem.
	// Must have this here or we can't iterate the 'm_HitBoxes' vec3 that stores
	// CHitBox directly and not by a pointer.
	char pad[0x70];
};



class CHitBoxSets
{
public:

	CUtlVector<CHitBox>& m_HitBoxes() {
		return *reinterpret_cast<CUtlVector<CHitBox>*>(reinterpret_cast<uintptr_t>(this) + 0x10);
	}

};

class CRenderMesh
{
public:
	pad(0x28);                        // 0x0000
	Vector_t            m_vecMinBounds;        // 0x0028
	Vector_t            m_vecMaxBounds;        // 0x0034
	std::int32_t    m_nDrawCalls;        // 0x0040
	pad(0x4);                        // 0x0044
	void* m_pDrawCalls;        // 0x0048
	pad(0x68);                        // 0x0050
	std::int32_t    m_nSkeleton;        // 0x00B8
	pad(0x4);                        // 0x00BC
	void* m_pSkeleton;        // 0x00C0
	pad(0x8);                        // 0x00C8
	std::int32_t    m_nHashes;            // 0x00D0
	pad(0x4);                        // 0x00D4
	uint32_t* m_pHashes;            // 0x00D8
	pad(0x20);                        // 0x00E0
	std::int32_t    m_nBoneWeight;        // 0x0100
	pad(0xC);                        // 0x0104
	void* m_pAttachments;        // 0x0110
	std::int32_t    m_nAttachments;        // 0x0118
	pad(0x1C);                        // 0x011C
	CHitBoxSets* m_pHitboxSets;        // 0x0138
	std::int32_t    m_nHitboxSets;        // 0x0140

}; // Size=0x0200
class c_rendermeshes
{
public:
	CRenderMesh* m_meshes;
};

class CModelSkeleton
{
public:
	CModelSkeleton() = delete;
	CModelSkeleton(CModelSkeleton&&) = delete;
	CModelSkeleton(const CModelSkeleton&) = delete;

public:
	SCHEMA_ADD_OFFSET(CUtlVector<const char*>, vecBoneNames, 0x4);
	SCHEMA_ADD_OFFSET(CUtlVector<std::uint16_t>, vecBoneParent, 0x18);
	SCHEMA_ADD_OFFSET(CUtlVector<float>, m_boneSphere, 0x30);
	SCHEMA_ADD_OFFSET(CUtlVector<std::uint32_t>, m_nFlag, 0x48);
	SCHEMA_ADD_OFFSET(CUtlVector<Vector_t>, m_bonePosParent, 0x60);
	SCHEMA_ADD_OFFSET(CUtlVector<float>, m_boneScaleParent, 0x90);

};
class CModel;
class c_perm_model_ext_part
{
public:
	CTransform m_Transform; // 0x0	
	CUtlString m_Name; // 0x20	
	int32_t m_nParent; // 0x28	
private:
	[[maybe_unused]] uint8_t __pad002c[0x4]; // 0x2c
public:
	stronghandle< CModel > m_refModel; // 0x30	
};

class c_perm_model_info
{
public:
	uint32_t m_nFlags; // 0x0	
	Vector_t m_vHullMin; // 0x4	
	Vector_t m_vHullMax; // 0x10	
	Vector_t m_vViewMin; // 0x1c	
	Vector_t m_vViewMax; // 0x28	
	float m_flMass; // 0x34	
	Vector_t m_vEyePosition; // 0x38	
	float m_flMaxEyeDeflection; // 0x44	
	CUtlString m_sSurfaceProperty; // 0x48	
};

class CModel {
public:
	CModel() = delete;
	CModel(CModel&&) = delete;
	CModel(const CModel&) = delete;
public:
	SCHEMA_ADD_OFFSET(const char*, szName, 0x8);
	SCHEMA_ADD_OFFSET(CModelSkeleton, m_modelSkeleton, 0x188);


	


	uint32_t hitboxes_num();

	uint32_t hitbox_flags(uint32_t index);
	const char* hitbox_name(uint32_t index);
	uint32_t hitbox_parent(uint32_t index);


public:
	pad(0x70);
	std::int32_t m_nRendermeshCount; // 0x0070
	pad(0x4);
	c_rendermeshes* m_pRenderMeshes; // 0x0078
};
class CModelState
{
public:
	CModelState() = delete;
	CModelState(CModelState&&) = delete;
	CModelState(const CModelState&) = delete;

	schema(bool, m_bClientClothCreationSuppressed, "CModelState->m_bClientClothCreationSuppressed");
	schema(int8_t, m_nForceLOD, "CModelState->m_nForceLOD");
	schema(int8_t, m_nClothUpdateFlags, "CModelState->m_nClothUpdateFlags");

	schema(uint64_t, m_MeshGroupMask, "CModelState->m_MeshGroupMask");
	schema(stronghandle< CModel >, m_hModel, "CModelState->m_hModel");
	SCHEMA_ADD_OFFSET(CTransform*, BoneTransform, 0x80);

	bone_data* hitbox_data() noexcept {
		if (!this)
			return nullptr;

		bone_data* boneDataPtr = *reinterpret_cast<bone_data**>(this + 0x80);

		if (boneDataPtr == nullptr)
			boneDataPtr = *reinterpret_cast<bone_data**>(reinterpret_cast<uintptr_t>(this) + 0x80);

		if (!boneDataPtr)
			return nullptr;

		return boneDataPtr;
	}


	Vector_t hitbox_position(uint32_t index) {
		auto hitbox = this->hitbox_data();
		if (!hitbox)
			return nullptr;

		if (!(this->m_hModel()->hitbox_flags(index) & bone_flags::FLAG_HITBOX))
			return nullptr;

		auto parent_index = this->m_hModel()->hitbox_parent(index);
		if (parent_index == -1)
			return nullptr;

		return hitbox[index].pos;
	}


	const char* hitbox_name(uint32_t index) {
		auto hitbox = this->hitbox_data();
		if (!hitbox)
			return nullptr;

		if (!(this->m_hModel()->hitbox_flags(index) & bone_flags::FLAG_HITBOX))
			return nullptr;

		auto parent_index = this->m_hModel()->hitbox_parent(index);
		if (parent_index == -1)
			return nullptr;

		return this->m_hModel()->hitbox_name(index);
	}
};
class C_EconItemView;
class CSkeletonInstance
{

public:
	CSkeletonInstance() = delete;
	CSkeletonInstance(CSkeletonInstance&&) = delete;
	CSkeletonInstance(const CSkeletonInstance&) = delete;
	schema(CEntityInstance*, m_pSkelOwner, "CGameSceneNode->m_pOwner");
	schema(CModelState, GetModel, "CSkeletonInstance->m_modelState");
	schema(bool, m_bIsAnimationEnabled, "CSkeletonInstance->m_bIsAnimationEnabled");
	schema(bool, m_bUseParentRenderBounds, "CSkeletonInstance->m_bUseParentRenderBounds");
	schema(bool, m_bDisableSolidCollisionsForHierarchy, "CSkeletonInstance->m_bDisableSolidCollisionsForHierarchy");
	schema(bool, m_bDirtyMotionType, "CSkeletonInstance->m_bDirtyMotionType");
	schema(bool, m_bIsGeneratingLatchedParentSpaceState, "CSkeletonInstance->m_bIsGeneratingLatchedParentSpaceState");
	schema(uint8_t, m_nHitboxSet, "CSkeletonInstance->m_nHitboxSet");
	void get_bone_data(bone_data& data, int index);
	void setup_bones(int mask);
};

class CBaseAnimGraph {
public:
	schema(bool, m_sequence_finished, "CBaseAnimGraphController->m_bSequenceFinished");
	schema(float, m_sound_sync_time, "CBaseAnimGraphController->m_flSoundSyncTime");
	schema(std::uintptr_t, m_active_ik_chain_mask, "CBaseAnimGraphController->m_nActiveIKChainMask");
	schema(int, m_sequence, "CBaseAnimGraphController->m_hSequence");
	schema(int, m_seq_start_time, "CBaseAnimGraphController->m_flSeqStartTime");
	schema(int, m_seq_fixed_cycle, "CBaseAnimGraphController->m_flSeqFixedCycle");
	schema(int, m_anim_loop_mode, "CBaseAnimGraphController->m_nAnimLoopMode");
	schema(float, m_playback_rate, "CBaseAnimGraphController->m_flPlaybackRate");
	schema(bool, m_animation_inputs_changed, "CBaseAnimGraphController->m_bNetworkedAnimationInputsChanged");
	schema(bool, m_sequence_changed, "CBaseAnimGraphController->m_bNetworkedSequenceChanged");
	schema(bool, m_last_update_skipped, "CBaseAnimGraphController->m_bLastUpdateSkipped");
	SCHEMA_ADD_OFFSET(bool, m_cached_sequence_cycle_rate, 0x1EB);

	void write_new_sequence(int& sequence) {
		sequence = m_sequence();
	}
};
class CBodyComponent : public CEntityInstance {
public:
	schema(CBaseAnimGraph*, Get, "CBodyComponentBaseAnimGraph->m_animationController");
};

class CBaseEntity : public CEntityInstance 
{
public:
	schema( int, m_nLastThinkTick, "C_BaseEntity->m_nLastThinkTick" );
	schema( CGameSceneNode*, m_pGameSceneNode, "C_BaseEntity->m_pGameSceneNode" );
	schema( CCollisionProperty*, m_pCollision, "C_BaseEntity->m_pCollision" );
	schema( int, m_iMaxHealth, "C_BaseEntity->m_iMaxHealth" );
	schema( int, m_iHealth, "C_BaseEntity->m_iHealth" );
	schema(CBodyComponent*, m_CBodyComponent, "C_BaseEntity->m_CBodyComponent");
	schema( uint8_t, m_lifeState, "C_BaseEntity->m_lifeState" );
	schema( bool, m_bTakesDamage, "C_BaseEntity->m_bTakesDamage" );
	schema( int, m_iEFlags, "C_BaseEntity->m_iEFlags" );
	schema( int, m_nSimulationTick, "C_BaseEntity->m_nSimulationTick" );
	schema( float, m_flSimulationTime, "C_BaseEntity->m_flSimulationTime" );
	schema( float, m_flCreateTime, "C_BaseEntity->m_flCreateTime" );
	schema( float, m_flSpeed, "C_BaseEntity->m_flSpeed" );
	schema( uint8_t, m_iTeamNum, "C_BaseEntity->m_iTeamNum" );
	schema( int, m_nNextThinkTick, "C_BaseEntity->m_nNextThinkTick" );
	schema( uint32_t, m_fFlags, "C_BaseEntity->m_fFlags" );
	schema( Vector_t, m_vecAbsVelocity, "C_BaseEntity->m_vecAbsVelocity" );
	schema( Vector_t, m_vecVelocity, "C_BaseEntity->m_vecVelocity" );
	schema( Vector_t, m_vecBaseVelocity, "C_BaseEntity->m_vecBaseVelocity" );
	schema( int, m_MoveType, "C_BaseEntity->m_MoveType" );
	schema( uint32_t, m_fEffects, "C_BaseEntity->m_fEffects" );
	schema( float, m_flGravityScale, "C_BaseEntity->m_flGravityScale" );
	schema(CBaseHandle, m_hGroundEntity, "C_BaseEntity->m_hGroundEntity");
	schema(std::uint8_t, GetMoveType, "C_BaseEntity->m_nActualMoveType"); // m_nActualMoveType returns CSGO style movetype, m_nMoveType returns bitwise shifted move type
	schema(float, GetWaterLevel, "C_BaseEntity->m_flWaterLevel");
	schema(CUtlStringToken, m_nSubclassID, "C_BaseEntity->m_nSubclassID");

	schema( CBaseHandle, m_hOldMoveParent, "C_BaseEntity->m_hOldMoveParent" );
	schema( int, m_nCreationTick, "C_BaseEntity->m_nCreationTick" );
	schema( CBaseHandle, m_hOwnerEntity, "C_BaseEntity->m_hOwnerEntity" );
	schema( CGlowProperty, m_Glow, "C_BaseModelEntity->m_Glow" );
	schema(Color_t, sky_m_vTintColor, "C_EnvSky->m_vTintColor");
	schema(Color_t, sky_m_vTintColorLightingOnly, "C_EnvSky->m_vTintColorLightingOnly");
	schema(float, sky_m_flBrightnessScale, "C_EnvSky->m_flBrightnessScale");
	schema(bool, sky_m_bEnabled, "C_EnvSky->m_bEnabled");
	schema(bool, sky_m_bStartDisabled, "C_EnvSky->m_bStartDisabled");

	bool get_bounding_box(ImVec4& out, bool scan);
	void set_origin(Vector_t origin);
	ClientClass_t* GetClientClass( ) 
	{
		return utils::CallVFunc<ClientClass_t * ( __fastcall* )( CBaseEntity* )>( this, 37 )( this );
	}

	C_EconItemView* GetEconItemView()
	{
		return  reinterpret_cast<C_EconItemView*>(std::uintptr_t(this) + 0x10E8);
	}

	bool is_attachment(int player)
	{
		
		if (m_pGameSceneNode()) {
			if (m_pGameSceneNode()->m_pParent())
			{
				auto owner = m_pGameSceneNode()->m_pParent()->m_pOwner();
				if (m_pGameSceneNode()->m_pParent()->m_pOwner()) {
					if (m_pGameSceneNode()->m_pParent()->m_pOwner()->handle().valid()) {
						if (m_pGameSceneNode()->m_pParent()->m_pOwner()->handle().index() == player)
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	bool IsBasePlayer();
	bool IsBasePlayerController();
	bool IsWeapon();
	bool IsViewModel();
	bool IsInferno();
	bool IsPlantedC4();
	bool IsViewModelWeapon();

};

class c_effect_data // CEffectData in schema
{
public:
	void* vftable = { };                // 0x0
	Vector_t origin = { };                // 0x8
	Vector_t start = { };                 // 0x14
	Vector_t normal = { };                // 0x20
	QAngle_t angles = { };               // 0x2c
	uint32_t handle_entity = { };       // 0x38
	uint32_t handle_other_entity = { }; // 0x3c
	float scale = { };                  // 0x40
	float magnitude = { };              // 0x44
	float radius = { };                 // 0x48
	int surface_prop = { };             // 0x4c
	void* effect_index = { };           // 0x50
	uint32_t damage_type = { };         // 0x58
	uint8_t penetrate = { };            // 0x5c
	uint8_t __pad005d[0x1] = { };     // 0x5d
	uint16_t material = { };            // 0x5e
	uint16_t hitbox = { };              // 0x60
	uint8_t color = { };                // 0x62
	uint8_t flags = { };                // 0x63
	uint8_t attachment_index = { };     // 0x64
	uint8_t __pad0065[0x3] = { };     // 0x65
	int attachment_name = { };          // 0x68
	uint16_t effect_name = { };         // 0x6c
	uint8_t explosion_type = { };       // 0x6e
	uint8_t __pad006f[0x9] = { };
};
class C_BaseModelEntity : public CBaseEntity
{
public:

	schema(CCollisionProperty, GetCollisionInstance, "C_BaseModelEntity->m_Collision");
	schema(CGlowProperty, GetGlowProperty, "C_BaseModelEntity->m_Glow");
	schema(GameTime_t, GetCreationTime, "C_BaseModelEntity->m_flCreateTime");
	schema(GameTick_t, GetCreationTick, "C_BaseModelEntity->m_nCreationTick");
	schema(std::float_t, GetAnimTime, "C_BaseModelEntity->m_flAnimTime");
	schema(std::float_t, GetSimulationTime, "C_BaseModelEntity->m_flSimulationTime");
	void SetModel(const char* name);

};
class C_BaseViewModel : public C_BaseModelEntity {
public:
	schema(CBaseHandle, m_hWeapon, "C_BaseViewModel->m_hWeapon");

};
class CAnimGraphNetworkedVariables;

class CAnimationGraphInstance {
public:
	char pad_0x0000[0x2E0]; //0x0000
	CAnimGraphNetworkedVariables* pAnimGraphNetworkedVariables; //0x02E0 
};
class C_CSGOViewModel : public C_BaseViewModel {
public:
	char pad_0x0000[0xD00]; //0x0000
	CAnimationGraphInstance* pAnimationGraphInstance; //0x0D08 

public:
	schema(bool, m_bNeedToQueueHighResComposite, "C_CSGOViewModel->m_bNeedToQueueHighResComposite");

	void set_viewmodel_origin(Vector_t pos);
};
enum class PlayerConnectedState_t : uint32_t
{
	PlayerNeverConnected = 0xffffffffffffffff,
	PlayerConnected = 0x0,
	PlayerConnecting = 0x1,
	PlayerReconnecting = 0x2,
	PlayerDisconnecting = 0x3,
	PlayerDisconnected = 0x4,
	PlayerReserved = 0x5,
};

class C_BasePlayerPawn;

class CBasePlayerController : public C_BaseModelEntity {
public:
	schema(std::uint64_t, GetSteamId, ("CBasePlayerController->m_steamID"));
};


enum EFlags 
{
	FL_ONGROUND = (1 << 0),
};

class CCSWeaponBase;
class CCSPlayer_WeaponServices
{
public:
	schema( bool, m_bAllowSwitchToNoWeapon, "CPlayer_WeaponServices->m_bAllowSwitchToNoWeapon" );
	schema( CBaseHandle, m_hMyWeapons, "CPlayer_WeaponServices->m_hMyWeapons" );
	schema( CBaseHandle, m_hActiveWeapon, "CPlayer_WeaponServices->m_hActiveWeapon" );
	schema( CBaseHandle, m_hLastWeapon, "CPlayer_WeaponServices->m_hLastWeapon" );
	schema( int, m_iAmmo, "CPlayer_WeaponServices->m_iAmmo" );

	schema( float, m_flNextAttack, "CCSPlayer_WeaponServices->m_flNextAttack" );
	schema( bool, m_bIsLookingAtWeapon, "CCSPlayer_WeaponServices->m_bIsLookingAtWeapon" );
	schema( bool, m_bIsHoldingLookAtWeapon, "CCSPlayer_WeaponServices->m_bIsHoldingLookAtWeapon" );

	__forceinline Vector_t server_shoot_pos() {
		Vector_t temp = Vector_t(0, 0, 0);
		utils::CallVFunc<void, 23>(this, &temp);
		return temp;
	}
};

class CCSPlayer_MovementServices
{
public:
	schema( int, m_nImpulse, "CPlayer_MovementServices->m_nImpulse" );
	schema( uint32_t, m_nButtons, "CPlayer_MovementServices->m_nButtons" );
	schema(float, m_flSurfaceFriction, "CPlayer_MovementServices_Humanoid->m_flSurfaceFriction");
	schema(float, m_flMaxspeed, "CPlayer_MovementServices->m_flMaxspeed");
	schema(Vector_t, m_vecLastMovementImpulses, "CPlayer_MovementServices->m_vecLastMovementImpulses");

	
	CCSPlayerPawn* GetOwner( )
	{
		return *reinterpret_cast< CCSPlayerPawn** >( ( uintptr_t ) this + 0x30 );
	}

	void extrapolate_lefting_sig(__int64 unk_target, Vector_t position, float* unk)
	{
		using function_t = void(__fastcall*)(CCSPlayer_MovementServices*, __int64, Vector_t, float*);
		static auto fnc = reinterpret_cast<function_t>(utils::FindPattern("client.dll", xorstr_("")));
	}

	void process_movement(CUserCmd* cmd);
	//48 83 EC 28 48 8B 41 30 48 89
	void set_prediction_command(CUserCmd* cmd) {
		utils::CallVFunc<void, 34>( this, cmd);
	}

	void reset_prediction_command() {
		utils::CallVFunc<void, 35>( this);
	}
};

class CCSPlayer_ViewModelServices {
public:

	schema(CBaseHandle, m_hViewModel, "CCSPlayer_ViewModelServices->m_hViewModel");

};
class CPlayer_ItemServices {
public:
	schema(bool, m_bHasDefuser, ("CCSPlayer_ItemServices->m_bHasDefuser"));
	schema(bool, m_bHasHelmet, ("CCSPlayer_ItemServices->m_bHasHelmet"));
	schema(bool, m_bHasHeavyArmor, "CCSPlayer_ItemServices->m_bHasHeavyArmor");
};
// NetworkVarNames: m_hObserverTarget (CHandle<CBaseEntity>)
class CPlayer_ObserverServices {
public:
	schema(CBaseHandle, m_hObserverTarget, "CPlayer_ObserverServices->m_hObserverTarget");
	schema(uint8_t, m_iObserverMode, "CPlayer_ObserverServices->m_iObserverMode");
};
// identifies how submerged in water a player is
enum : int
{
	WL_NOTINWATER = 0,
	WL_FEET,
	WL_WAIST,
	WL_EYES
};

struct c_hitbox_data_t {
	Vector_t min;
	Vector_t max;
	float rad;
};
struct bullet_info_t
{

	bullet_info_t()
	{
		position = Vector_t(0, 0, 0);
		time_stamp = 0;
		expire_time = -1;
	}

	bullet_info_t(Vector_t position, float time_stamp, float expire_time)
	{
		position = position;
		time_stamp = time_stamp;
		expire_time = expire_time;
	}

	Vector_t	position{};
	float	time_stamp{};
	float	expire_time{};
};

class CCSPlayer_BulletServices
{
public:
	//SCHEMA(CCSPlayer_BulletServices, m_totalHitsOnServer, m_total_hits_on_server, int);
public:
	int m_bullet_count()
	{
		return *reinterpret_cast<int*>(std::uintptr_t(this) + 0x48);
	}
	CUtlVector<bullet_info_t>& m_bullet_data()
	{
		return *reinterpret_cast<CUtlVector<bullet_info_t>*>(std::uintptr_t(this) + 0x48);
	}
};
class CCSPlayerPawn : public CBaseEntity 
{
public:
	schema(CPlayer_ItemServices*, GetItemServices, ("C_BasePlayerPawn->m_pItemServices"));

	schema( CCSPlayer_WeaponServices*, m_pWeaponServices, "C_BasePlayerPawn->m_pWeaponServices" );
	schema( void*, m_pItemServices, "C_BasePlayerPawn->m_pItemServices" );
	schema( void*, m_pAutoaimServices, "C_BasePlayerPawn->m_pAutoaimServices" );
	schema(CPlayer_ObserverServices*, m_pObserverServices, "C_BasePlayerPawn->m_pObserverServices" );
	schema( float, m_flFlashDuration, "C_CSPlayerPawnBase->m_flFlashDuration" );
	schema(c_handle, m_hOriginalController, "C_CSPlayerPawnBase->m_hOriginalController");
	schema(QAngle_t, m_angEyeAngles, "C_CSPlayerPawnBase->m_angEyeAngles");

	schema( void*, m_pWaterServices, "C_BasePlayerPawn->m_pWaterServices" );
	schema( void*, m_pUseServices, "C_BasePlayerPawn->m_pUseServices" );
	schema( void*, m_pFlashlightServices, "C_BasePlayerPawn->m_pFlashlightServices" );
	schema( CCSPlayer_CameraServices*, m_pCameraServices, "C_BasePlayerPawn->m_pCameraServices" );
	schema( CCSPlayer_MovementServices*, m_pMovementServices, "C_BasePlayerPawn->m_pMovementServices" );
	schema( Vector_t, v_angle, "C_BasePlayerPawn->v_angle" );
	schema( float, m_flTimeLastHurt, "C_BasePlayerPawn->m_flTimeLastHurt" );
	schema( float, m_flDeathTime, "C_BasePlayerPawn->m_flDeathTime" );
	schema( CBaseHandle, m_hController, "C_BasePlayerPawn->m_hController" );
	schema( bool, m_bWaitForNoAttack, "C_CSPlayerPawn->m_bWaitForNoAttack" );
	schema( bool, m_bIsScoped, "C_CSPlayerPawn->m_bIsScoped" );
	schema(bool, m_bInLanding, "C_CSPlayerPawn->m_bInLanding");

	schema(bool, immune, "C_CSPlayerPawnBase->m_bGunGameImmunity");
	schema(CCSPlayer_ViewModelServices*, m_pViewModelServices, "C_CSPlayerPawnBase->m_pViewModelServices");
	schema(std::int32_t, GetArmorValue, "C_CSPlayerPawn->m_ArmorValue");
	schema(bool, m_bNeedToReApplyGloves, "C_CSPlayerPawn->m_bNeedToReApplyGloves");
	schema(bool, IsWaitForNoAttack, "C_CSPlayerPawn->m_bWaitForNoAttack");
	schema(int, m_shots, "C_CSPlayerPawn->m_iShotsFired");
	schema(C_EconItemView, m_EconGloves, "C_CSPlayerPawn->m_EconGloves");
	schema(CUtlVectorCS2< QAngle_t >, m_aimPunchCache, "C_CSPlayerPawn->m_aimPunchCache");
	schema(QAngle_t, m_aimPunchAngle, "C_CSPlayerPawn->m_aimPunchAngle");
	schema(int, m_aimPunchTickBase, "C_CSPlayerPawn->m_aimPunchTickBase");
	schema(CCSPlayer_BulletServices*, m_pBulletServices, "C_CSPlayerPawn->m_pBulletServices ");

	
public:
	c_hitbox_data_t get_hitbox(int i);
	void create_shocked_effect(const char* effect_name);
	int get_lerp_tick(int player_tick_count);
	bool match_interpolation();
	__forceinline float get_max_speed() {
		if (!this)
			return 0.f;

		auto movement_system = this->m_pMovementServices();
		if (!movement_system)
			return 0.f;

		auto max_speed = movement_system->m_flMaxspeed();
		return max_speed;
	}

	inline bool can_attack(const float flServerTime)
	{
		if (CCSPlayer_WeaponServices* pWeaponServices = this->m_pWeaponServices(); pWeaponServices != nullptr)
			if (this->IsWaitForNoAttack() || pWeaponServices->m_flNextAttack() > flServerTime)
				return false;

		return true;
	}

	bool valid_move() {
		if (!this)
			return false;

		const int32_t nMoveType = this->GetMoveType(); 

		return nMoveType != MOVETYPE_NOCLIP && nMoveType != MOVETYPE_LADDER && this->GetWaterLevel() < WL_WAIST;
	}
	
	inline bool valid(CCSPlayerPawn* local) {
		if (!this || !local)
			return false;

		if (!this->IsAlive() || !local->enemy(this) || this->m_pGameSceneNode()->dormant() || this->immune())
			return false;

		return true;
	};

	inline bool hasArmour(const int hitgroup) {
		if (!this->GetItemServices())
			return false;

		switch (hitgroup) {
		case HITGROUP_HEAD:
			return this->GetItemServices()->m_bHasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	}

	bool IsAlive();
	bool can_shoot(int tick);
	Vector_t GetEyePosition();
	bool visible(CCSPlayerPawn* local, QAngle_t ang);
	CCSWeaponBase* GetActiveWeapon();
	bool on_crosshair(QAngle_t ang, float range,int* handle = 0,  CCSPlayerPawn* pawn = nullptr, CBaseEntity* entity = nullptr);
	int get_index_by_hitbox(int name);
	bool enemy(CCSPlayerPawn* pOther);
	void get_hitbox_data(uint32_t idx, Vector_t& pos, Vector4D_t& rot, float& scale, bool predict);
	bool locking_smoke();
	int get_bone_idx(const char* name);
};


class C_CSObserverPawn : public CCSPlayerPawn
{
public:
	CBaseHandle m_hDetectParentChange; // 0x1460	
};



class C_CSGameRules {

public:
	schema(bool, m_bFreezePeriod, "C_CSGameRules->m_bFreezePeriod");
	schema(bool, m_bWarmupPeriod, "C_CSGameRules->m_bWarmupPeriod");
	schema(bool, m_bIsValveDS, "C_CSGameRules->m_bIsValveDS");
	schema(bool, m_bBombPlanted, "C_CSGameRules->m_bBombPlanted");

};


class CCSPlayerController : public CBasePlayerController {

public:
	schema(CBaseHandle, m_hObserverPawn, "CCSPlayerController->m_hObserverPawn");
	schema(int, m_nFinalPredictedTick, "CBasePlayerController->m_nFinalPredictedTick");
	schema(uint32_t, m_nTickBase, "CBasePlayerController->m_nTickBase");
	schema(CBaseHandle, m_hPawn, "CBasePlayerController->m_hPawn");
	schema(CBaseHandle, PawnHandle, "CBasePlayerController->m_hPawn");
	schema(std::int32_t, GetPawnHealth, "CCSPlayerController->m_iPawnHealth");
	schema(PlayerConnectedState_t, m_iConnected, "CBasePlayerController->m_iConnected");
	schema(char*, m_iszPlayerName, "CBasePlayerCOntroller->m_iszPlayerName");
	schema(uint64_t, m_steamID, "CBasePlayerController->m_steamID");
	schema(bool, m_bIsLocalPlayerController, "CBasePlayerController->m_bIsLocalPlayerController");
	schema(int, m_iDesiredFOV, "CBasePlayerController->m_iDesiredFOV");
	schema(bool, m_bPawnIsAlive, "CCSPlayerController->m_bPawnIsAlive");
	schema(const char*, m_sSanitizedPlayerName, "CCSPlayerController->m_sSanitizedPlayerName");
	schema(const char*, GetPlayerName, ("CCSPlayerController->m_sSanitizedPlayerName"));
	SCHEMA_ADD_FIELD_OFFSET(CUserCmd*, current_command, "CBasePlayerController->m_steamID", -0x8);

	void set_player_tickbase(int nTickBase);
	void run_physics_think();

	__forceinline CUserCmd*& cmd() {
		return *(CUserCmd**)(std::uintptr_t(this) + 0x6B8);
	}

	static CCSPlayerController* Get();

};


enum EWeaponType : int
{
	WEAPONTYPE_KNIFE,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_TASER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_EQUIPMENT,
	WEAPONTYPE_STACKABLEITEM,
	WEAPONTYPE_FISTS,
	WEAPONTYPE_BREACHCHARGE,
	WEAPONTYPE_BUMPMINE,
	WEAPONTYPE_TABLET,
	WEAPONTYPE_MELEE,
	WEAPONTYPE_SHIELD,
	WEAPONTYPE_ZONE_REPULSOR,
	WEAPONTYPE_UNKNOWN
};

enum EWeaponCategory : int 
{
	WEAPONCATEGORY_OTHER,
	WEAPONCATEGORY_SECONDARY,
	WEAPONCATEGORY_SMG,
	WEAPONCATEGORY_RIFLE,
	WEAPONCATEGORY_HEAVY,
	WEAPONCATEGORY_COUNT
};

class CCSWeaponBaseVData
{
public:
	schema( EWeaponType, m_WeaponType, "CCSWeaponBaseVData->m_WeaponType" );
	schema( EWeaponCategory, m_WeaponCategory, "CCSWeaponBaseVData->m_WeaponCategory" );
	schema( int, m_nZoomLevels, "CCSWeaponBaseVData->m_nZoomLevels" );
	schema(int, m_nNumBullets, "CCSWeaponBaseVData->m_nNumBullets");

	schema(const char*, m_szName, "CCSWeaponBaseVData->m_szName");
	schema(CFiringModeFloat, m_flSpread, "CCSWeaponBaseVData->m_flSpread");
	schema(float, m_flHeadshotMultiplier, "CCSWeaponBaseVData->m_flHeadshotMultiplier");
	schema(float, m_flArmorRatio, "CCSWeaponBaseVData->m_flArmorRatio");
	schema(int, m_nDamage, "CCSWeaponBaseVData->m_nDamage");
	schema(float, m_flRange, "CCSWeaponBaseVData->m_flRange");
	schema(float, m_flRangeModifier, "CCSWeaponBaseVData->m_flRangeModifier");
	schema(float, m_flPenetration, "CCSWeaponBaseVData->m_flPenetration");
	schema(float, m_flThrowVelocity, "CCSWeaponBaseVData->m_flThrowVelocity");
	schema(CFiringModeFloat, m_flInaccuracyCrouch, "CCSWeaponBaseVData->m_flInaccuracyCrouch");
	schema(CFiringModeFloat, m_flInaccuracyStand, "CCSWeaponBaseVData->m_flInaccuracyStand");
	schema(CFiringModeFloat, m_flInaccuracyJump, "CCSWeaponBaseVData->m_flInaccuracyJump");
	schema(CFiringModeFloat, m_flInaccuracyMove, "CCSWeaponBaseVData->m_flInaccuracyMove");
	schema(CFiringModeFloat, m_flCycleTime, "CCSWeaponBaseVData->m_flCycleTime");
	schema(CFiringModeFloat, m_flMaxSpeed, "CCSWeaponBaseVData->m_flMaxSpeed");
	schema(float, range, "CCSWeaponBaseVData->m_flRange");
	schema(int, m_iMaxClip1, "CBasePlayerWeaponVData->m_iMaxClip1");

}; 
class C_PlantedC4 {
public:
	schema(bool, m_bCannotBeDefused, "C_PlantedC4->m_bCannotBeDefused");
	schema(bool, m_bBeingDefused, "C_PlantedC4->m_bBeingDefused");
	schema(int32_t, m_nBombSite, "C_PlantedC4->m_nBombSite");
	schema(float, m_flTimerLength, "C_PlantedC4->m_flTimerLength");
	schema(float, m_flDefuseLength, "C_PlantedC4->m_flDefuseLength");
	schema(GameTime_t, m_flDefuseCountDown, "C_PlantedC4->m_flDefuseCountDown");
	schema(GameTime_t, m_fLastDefuseTime, "C_PlantedC4->m_fLastDefuseTime");
	schema(GameTime_t, m_flC4Blow, "C_PlantedC4->m_flC4Blow");
	schema(bool, m_bHasExploded, "C_PlantedC4->m_bHasExploded");
	schema(CBaseHandle, m_hBombDefuser, "C_PlantedC4->m_hBombDefuser");

	

};

class CSharedObject;

enum ESOCacheEvent {
	/// Dummy sentinel value
	eSOCacheEvent_None = 0,

	/// We received a our first update from the GC and are subscribed
	eSOCacheEvent_Subscribed = 1,

	/// We lost connection to GC or GC notified us that we are no longer
	/// subscribed. Objects stay in the cache, but we no longer receive updates
	eSOCacheEvent_Unsubscribed = 2,

	/// We received a full update from the GC on a cache for which we were
	/// already subscribed. This can happen if connectivity is lost, and then
	/// restored before we realized it was lost.
	eSOCacheEvent_Resubscribed = 3,

	/// We received an incremental update from the GC about specific object(s)
	/// being added, updated, or removed from the cache
	eSOCacheEvent_Incremental = 4,

	/// A lister was added to the cache
	/// @see CGCClientSharedObjectCache::AddListener
	eSOCacheEvent_ListenerAdded = 5,

	/// A lister was removed from the cache
	/// @see CGCClientSharedObjectCache::RemoveListener
	eSOCacheEvent_ListenerRemoved = 6,
};

class CEconItem {
public:
	void set_sticker(int index, int id);
	void SetDynamicAttributeValue(int index, void* value);
	void SetDynamicAttributeValueString(int index, const char* value);

public:
	static CEconItem* CreateInstance();

	void Destruct() {
		return utils::CallVFunc<void, 1U>(this);
	}

	void SetPaintKit(float kit) { SetDynamicAttributeValue(6, &kit); }
	void SetPaintSeed(float seed) { SetDynamicAttributeValue(7, &seed); }
	void SetPaintWear(float wear) { SetDynamicAttributeValue(8, &wear); }
	void SetStatTrak(int count) { SetDynamicAttributeValue(80, &count); }
	void SetStatTrakType(int type) { SetDynamicAttributeValue(81, &type); }
	void SetCustomName(const char* pName) {
		SetDynamicAttributeValueString(111, pName);
	}

	char pad0[0x10];  // 2 vtables
	uint64_t m_ulID;
	uint64_t m_ulOriginalID;
	void* m_pCustomDataOptimizedObject;
	uint32_t m_unAccountID;
	uint32_t m_unInventory;
	uint16_t m_unDefIndex;
	uint16_t m_unOrigin : 5;
	uint16_t m_nQuality : 4;
	uint16_t m_unLevel : 2;
	uint16_t m_nRarity : 4;
	uint16_t m_dirtybitInUse : 1;
	int16_t m_iItemSet;
	int m_bSOUpdateFrame;
	uint8_t m_unFlags;
};


class CEconItemDefinition {
public:
	bool IsWeapon();
	bool IsCustomPlayer();
	bool IsAgent(bool bExcludeDefault);
	bool IsKnife(bool excludeDefault, const char* name);
	bool IsGlove(bool excludeDefault, const char* name);

	auto GetModelName() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0xD8);
	}

	auto GetStickersSupportedCount() {
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x100); // 0x118
	}

	auto GetSimpleWeaponName() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x1F0);
	}

	auto GetLoadoutSlot() {
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x2E8);
	}

	char pad0[0x8];  // vtable
	void* m_pKVItem;
	uint16_t m_nDefIndex;
	CUtlVectorCS2<uint16_t> m_nAssociatedItemsDefIndexes;
	bool m_bEnabled;
	const char* m_szPrefab;
	uint8_t m_unMinItemLevel;
	uint8_t m_unMaxItemLevel;
	uint8_t m_nItemRarity;
	uint8_t m_nItemQuality;
	uint8_t m_nForcedItemQuality;
	uint8_t m_nDefaultDropItemQuality;
	uint8_t m_nDefaultDropQuantity;
	CUtlVectorCS2<void*> m_vecStaticAttributes;
	uint8_t m_nPopularitySeed;
	void* m_pPortraitsKV;
	const char* m_pszItemBaseName;
	bool m_bProperName;
	const char* m_pszItemTypeName;
	uint32_t m_unItemTypeID;
	const char* m_pszItemDesc;
	char pad3[0x10];
	const char* path_image;
	char pad5[0x8];
	const char* path_mdl2;
	char pad6[0x18];
	const char* path_vft;
};

struct SOID_t {
	uint64_t m_id;
	uint32_t m_type;
	uint32_t m_padding;
};
class C_EconItemView;

class C_Inferno {
public:
	schema(bool[64], m_bFireIsBurning, "C_Inferno->m_bFireIsBurning");
	schema(int, m_fireCount, "C_Inferno->m_fireCount");
	schema(Vector_t[64], m_firePositions, "C_Inferno->m_firePositions");
	schema(Vector_t[64], m_fireParentPositions, "C_Inferno->m_fireParentPositions");
};
class CGCClientSharedObjectTypeCache {
public:
	auto AddObject(CSharedObject* pObject) {
		return utils::CallVFunc<bool, 1u>(this, pObject);
	}

	auto RemoveObject(CSharedObject* soIndex) {
		return utils::CallVFunc<CSharedObject*, 3u>(this, soIndex);

	}

	template <typename T>
	auto& GetVecObjects() {
		return *reinterpret_cast<CUtlVector<T>*>((uintptr_t)(this) + 0x8);
	}
};

class CGCClientSharedObjectCache {
public:
	CGCClientSharedObjectTypeCache* CreateBaseTypeCache(int nClassID);
};
class CGCClient {
public:
	CGCClientSharedObjectCache* FindSOCache(SOID_t ID,
		bool bCreateIfMissing = true);
};

class CGCClientSystem {
public:
	CGCClient* GetCGCClient() {
		return reinterpret_cast<CGCClient*>((uintptr_t)(this) + 0xB8);
	}
}; 
class CCSPlayerInventory;

class C_EconItemView {
public:
	CEconItem* GetSOCData(CCSPlayerInventory* inventory);
	auto GetCustomPaintKitIndex() { return utils::CallVFunc<int, 2u>(this); }
	auto GetStaticData() { return utils::CallVFunc<CEconItemDefinition*, 13u>(this); }
	schema(uint32_t, m_iAccountID, "C_EconItemView->m_iAccountID");
	schema(uint32_t, m_iItemIDHigh, "C_EconItemView->m_iItemIDHigh");
	schema(bool, m_bDisallowSOC, "C_EconItemView->m_bDisallowSOC");
	schema(uint32_t, m_iItemIDLow, "C_EconItemView->m_iItemIDLow");
	schema(bool, m_bInitialized, "C_EconItemView->m_bInitialized");
	SCHEMA_ADD_OFFSET(bool, m_bIsStoreItem, 0x1EA);
	SCHEMA_ADD_OFFSET(bool, m_bIsTradeItem, 0x1EB);


	schema(uint64_t, m_iItemID, "C_EconItemView->m_iItemID");
	schema(uint64_t, m_iItemDefinitionIndex, "C_EconItemView->m_iItemDefinitionIndex");
	schema(const char*, m_szCustomName, "C_EconItemView->m_szCustomName");

};

class CAttributeManager {
public:


	SCHEMA_ADD_OFFSET(C_EconItemView, m_Item, 0x50);
};

class C_EconEntity : public C_BaseModelEntity {
public:
	/* offset */
	schema(CAttributeManager, m_AttributeManager, "C_EconEntity->m_AttributeManager");
	schema(uint32_t, m_OriginalOwnerXuidLow, "C_EconEntity->m_OriginalOwnerXuidLow");
	schema(uint32_t, m_OriginalOwnerXuidHigh, "C_EconEntity->m_OriginalOwnerXuidHigh");
	schema(std::int32_t, m_nFallbackPaintKit, "C_EconEntity->m_nFallbackPaintKit");
	schema(std::int32_t, m_nFallbackStatTrak, "C_EconEntity->m_nFallbackStatTrak");
	schema(std::int32_t, m_nFallbackSeed, "C_EconEntity->m_nFallbackSeed");
	schema(std::int32_t, m_flFallbackWear, "C_EconEntity->m_flFallbackWear");
	
	SCHEMA_ADD_OFFSET(bool, m_bShouldGlow, 0x15B8);

	uint64_t GetOriginalOwnerXuid() {
		return ((uint64_t)(m_OriginalOwnerXuidHigh()) << 32) |
			m_OriginalOwnerXuidLow();
	}

};
enum class weapon_state : uint32_t {
	WEAPON_NOT_CARRIED = 0x0,
	WEAPON_IS_CARRIED_BY_PLAYER = 0x1,
	WEAPON_IS_ACTIVE = 0x2,
};
enum CSWeaponMode : int /* it works atleast */
{
	Primary_Mode = 0,
	Secondary_Mode = 1,
	WeaponMode_MAX = 2,
};
class CBasePlayerWeapon : public C_EconEntity {
public:
	schema(GameTick_t, GetNextPrimaryAttackTick, "C_BasePlayerWeapon->m_nNextPrimaryAttackTick");
	schema(float, GetNextPrimaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextPrimaryAttackTickRatio");
	schema(GameTick_t, GetNextSecondaryAttackTick, "C_BasePlayerWeapon->m_nNextSecondaryAttackTick");
	schema(float, GetNextSecondaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextSecondaryAttackTickRatio");

	schema(CSWeaponMode, m_weaponMode, xorstr_("C_CSWeaponBase->m_weaponMode"));
	schema(float, m_flRecoilIndex, xorstr_("C_CSWeaponBase->m_flRecoilIndex"));

	schema(int32_t, clip1, "C_BasePlayerWeapon->m_iClip1");

}; 

// modified impl to fit entity list
template<typename T>
class CHHandle {
	int handleValue;

public:

	int Get() {
		return this->handleValue;
	}

	void Set(int newValue) {
		this->handleValue = newValue;
	}

	bool IsInvalid() {
		return this->handleValue == -1;
	}
};
class CCSWeaponBase : public CBasePlayerWeapon
{
public:
	schema(int, m_zoomLevel, "C_CSWeaponBaseGun->m_zoomLevel");
	schema(int32_t, clip1, "C_BasePlayerWeapon->m_iClip1");
	schema(int32_t, clip2, "C_BasePlayerWeapon->m_iMaxClip1");

	schema(int, m_iOriginalTeamNumber, xorstr_("C_CSWeaponBase->m_iOriginalTeamNumber"));
	schema(weapon_state, m_iState, "C_CSWeaponBase->m_iState");
	schema(std::int32_t, GetBurstShotsRemaining, "C_CSWeaponBaseGun->m_iBurstShotsRemaining");
	schema(bool, IsBurstMode, "C_CSWeaponBase->m_bBurstMode");
	schema(float, GetPostponeFireReadyFrac, "C_CSWeaponBase->m_flPostponeFireReadyFrac");
	schema(bool, m_bUIWeapon, "C_CSWeaponBase->m_bUIWeapon");
	schema_pfield(CBaseHandle, m_hStattrakAttachment, "C_CSWeaponBase->m_iNumEmptyAttacks", 0x4);
	schema_pfield(CBaseHandle, m_hNametagAttachment, "C_CSWeaponBase->m_iNumEmptyAttacks", 20);
	SCHEMA_ADD_OFFSET(bool, someint, 0xA08);
	SCHEMA_ADD_OFFSET(bool, someint2, 0xA20);


	void AddStattrakEntity();
	void AddNametagEntity();
	void AddKeychainEntity();



	inline void * UpdateComposite(bool should ) { return utils::CallVFunc<void*, 7u>(this, should); }
	inline void* UpdateWeaponData() { return utils::CallVFunc<void*, 189u>(this); }

	inline void *UpdateCompositeSec(bool should) { return utils::CallVFunc<void*, 100u>(this, should); }
	CCSWeaponBaseVData* GetWeaponData();
	std::string GetWeaponName();
	void UpdateModel(bool should);
	void update_accuracy(void* a1);
	void UpdateSubClass();

	bool can_primary_attack(const int nWeaponType, const float flServerTime);

	bool can_seccondary_attack(const int nWeaponType, const float flServerTime);
	inline bool valid() {
		if (!this)
			return false;

		auto data = this->GetWeaponData();
		if (!data)
			return false;

		// enough to check we have a weapon that can fire
		if (this->clip1() <= 0)
			return false;

		return true;
	}

	bool can_fire(int tick);


	void update_accuracy();
	float get_inaccuracy();

	//364
	float get_spread();

	_forceinline float get_max_speed() {
		using original_fn = float(__fastcall*)(void*);
		return (*(original_fn**)this)[348](this); // 352
	}

	 void get_calculated_spread(CCSWeaponBaseVData* data, int mode, unsigned int seed, float innacuracy, float spread, float recoil_index, float* spread_x, float* spread_y);
};



class C_SmokeGrenadeProjectile {
public:
	schema(Vector_t, m_vSmokeColor, "C_SmokeGrenadeProjectile->m_vSmokeColor");
	schema(Vector_t, m_vSmokeDetonationPos, "C_SmokeGrenadeProjectile->m_vSmokeDetonationPos ");
	schema(int, m_nSmokeEffectTickBegin, "C_SmokeGrenadeProjectile->m_nSmokeEffectTickBegin");



};

class C_BaseCSGrenade : public CCSWeaponBase {
public:
	schema(bool, m_bClientPredictDelete, "C_BaseCSGrenade->m_bClientPredictDelete");
	schema(bool, m_bRedraw, "C_BaseCSGrenade->m_bRedraw");
	schema(bool, m_bIsHeldByPlayer, "C_BaseCSGrenade->m_bIsHeldByPlayer");
	schema(bool, m_bPinPulled, "C_BaseCSGrenade->m_bPinPulled");
	schema(bool, m_bJumpThrow, "C_BaseCSGrenade->m_bJumpThrow");
	schema(GameTime_t, m_flDetonateTime, "C_BaseGrenade->m_flDetonateTime");
	schema(bool, m_bThrowAnimating, "C_BaseCSGrenade->m_bThrowAnimating");
	schema(GameTime_t, m_fThrowTime, "C_BaseCSGrenade->m_fThrowTime");
	schema(float, m_flThrowStrength, "C_BaseCSGrenade->m_flThrowStrength");
	schema(float, m_flThrowStrengthApproach, "C_BaseCSGrenade->m_flThrowStrengthApproach");
	schema(GameTime_t, m_fDropTime, "C_BaseCSGrenade->m_fDropTime");
	schema(GameTime_t, m_fPinPullTime, "C_BaseCSGrenade->m_fPinPullTime");
	schema(bool, m_bJustPulledPin, "C_BaseCSGrenade->m_bJustPulledPin");
	schema(GameTick_t, m_nNextHoldTick, "C_BaseCSGrenade->m_nNextHoldTick");
	schema(float, m_flNextHoldFrac, "C_BaseCSGrenade->m_flNextHoldFrac");
	schema(CBaseHandle, m_hSwitchToWeaponAfterThrow, "C_BaseCSGrenade->m_hSwitchToWeaponAfterThrow");
	schema(Vector_t, grenade_lines, "C_BaseCSGrenadeProjectile->vecLastTrailLinePos ");
	schema(bool, m_bIsLive, "C_BaseGrenade->m_bIsLive");
	schema(int, m_nExplodeEffectTickBegin, "C_BaseCSGrenadeProjectile->m_nExplodeEffectTickBegin");



public:
	int get_type()
	{
		return *(int*)(std::uintptr_t(this) + 0xD20);
	}

};

class C_PostProcessingVolume {
public:

	schema(float, m_flFadeDuration, "C_PostProcessingVolume->m_flFadeDuration");
	schema(float, m_flMinLogExposure, "C_PostProcessingVolume->m_flMinLogExposure");
	schema(float, m_flMaxLogExposure, "C_PostProcessingVolume->m_flMaxLogExposure");
	schema(float, m_flMinExposure, "C_PostProcessingVolume->m_flMinExposure");
	schema(float, m_flMaxExposure, "C_PostProcessingVolume->m_flMaxExposure");
	schema(float, m_flExposureCompensation, "C_PostProcessingVolume->m_flExposureCompensation");
	schema(float, m_flExposureFadeSpeedUp, "C_PostProcessingVolume->m_flExposureFadeSpeedUp");
	schema(float, m_flExposureFadeSpeedDown, "C_PostProcessingVolume->m_flExposureFadeSpeedDown");
	schema(float, m_flTonemapEVSmoothingRange, "C_PostProcessingVolume->m_flTonemapEVSmoothingRange");
	schema(bool, m_bMaster, "C_PostProcessingVolume->m_bMaster");
	schema(bool, m_bExposureControl, "C_PostProcessingVolume->m_bExposureControl");
	schema(float, m_flRate, "C_PostProcessingVolume->m_flRate");
	schema(float, m_flTonemapPercentTarget, "C_PostProcessingVolume->m_flTonemapPercentTarget");
	schema(float, m_flTonemapPercentBrightPixels, "C_PostProcessingVolume->m_flTonemapPercentBrightPixels");
	schema(float, m_flTonemapMinAvgLum, "C_PostProcessingVolume->m_flTonemapMinAvgLum");
};