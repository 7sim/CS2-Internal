#pragma once
#include "../../sdk/sdk.h"
#include "../../math/math.h"
#include "../../utils/utils.h"
#include "lib\basehookobj.h"
#include "..\..\sdk\datatypes\cutlbuffer\utlbuffer.h"
#include "../../math\elements\matrix.h"
#include "..\..\sdk\interfaces\trace.h"
#include "..\..\sdk\interfaces\particles.h"
#include "..\..\sdk\datatypes\cutlhandles\strong_handle.hpp"

#include "..\..\cheats\visuals\grenades\grenade.h"
#include "..\..\math\elements\vector.h"
class CRenderGameSystem;
class IViewRender;
class CCSGOInput;
class CViewSetup;
class CEconItem;
class c_grenade_trace;
class C_BaseCSGrenade;
class CGCClientSystem;
class CNetworkGameClient;
class CCSInventoryManager;
class IGameEvent;
class CUtlBuffer;
class trace_filter_t;
class trace_data_t;
class game_trace_t;

namespace hooks
{
	bool Init();

	HRESULT WINAPI hkPresent(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags);
	HRESULT __fastcall hkResizeBuffers(IDXGISwapChain* swap_chain, std::uint32_t count, std::uint32_t width, std::uint32_t height, DXGI_FORMAT new_format, std::uint32_t flags);
	HRESULT WINAPI hkCreateSwapChain(IDXGIFactory* factory, IUnknown* unknown_device, DXGI_SWAP_CHAIN_DESC* desc_swap_chain, IDXGISwapChain** swap_chain);
	long HkWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	bool __fastcall hkMouseInputEnabled(void* rcx);
	bool __fastcall hkUnlockInventory(void* rcx);
	float __fastcall hkOverrideFOV(CCSPlayer_CameraServices* a1);
	void __fastcall hkFrameStageNotify(void* a1, int stage);
	void __fastcall hkCreateMove(CCSGOInput* a1, unsigned int nSlot, char nUnk2);
	void __fastcall hkDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);
	void __fastcall hkMotionBlur(void* obj, unsigned int ev, Vector4D_t* value);
	void __fastcall hkCalculateViewBob(std::uintptr_t* a1, std::uintptr_t* a2, float* a3, float* a4);
	void __fastcall AggreegateSceneObject(std::uintptr_t* a1, std::uintptr_t* a2, std::uintptr_t* a3, int a4, std::uintptr_t* a5, std::uintptr_t* a6, std::uintptr_t* a7, std::uintptr_t* a8);
	ViewMatrix_t* __fastcall hkViewMatrix(CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels);
	void __fastcall hkAddCachedEntity(void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle);
	void __fastcall hkRemoveCachedEntity(void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle);
	bool __fastcall  hkOnEquipItem(void* rcx, int iTeam, int iSlot, uint64_t iItemID);
	void __fastcall hkSetModel(void* rcx, const char* model_name);
	void __fastcall hkGameEvent(void* rcx, IGameEvent* const ev);
	bool __fastcall hkPreCreateMove(CCSGOInput* a1, int nSlot, CUserCmd* cmd);
	inline WNDPROC m_old_wndproc = nullptr;
	void __fastcall hkLevelInit(void* client_mode, const char* new_map);
	void __fastcall hkLevelShutdown(std::uintptr_t* unk);
	void __fastcall hkGrenadeTrace(C_BaseCSGrenade* rcx);
	void __fastcall hkRenderViewSetup(IViewRender* rcx);
	void __fastcall hkViewModel(uintptr_t a1, uintptr_t a2, Vector_t view_model);
	float __fastcall hkGetFov(void* rcx);
	void __fastcall hkGlowObject(CGlowProperty* glow);
	void __fastcall hkRelativeMouse(void* a1, bool a);
	int __fastcall hkProcessCRCcommands(CBaseUserCmdPB* a1);
	void __fastcall hkRunCommand(CCSPlayer_MovementServices* a1, CUserCmd* cmd);
	void __fastcall hkDrawSceneObject(void* ptr, void* a2, CBaseSceneData* scene_data, int count, void* a5, void* a6, void* a7, void* a8);
	void* __fastcall hkUpdateSceneObject(C_AggregateSceneObject* object, void* unk);
	void* __fastcall hkLightSceneObject(void* ptr, C_SceneLightObject* object, void* unk);
	void __fastcall set_input_history(CCSInputMessage* input_message, CCSGOInputHistoryEntryPB* input_history, const bool verify, void* a4, void* a5, CBaseEntity* local_pawn);
	void __fastcall  hkCamera(CCSGOInput* pCSGOInput, int a2);
	void* __fastcall  hkGrenadeArray(C_SmokeGrenadeProjectile* smoke);
	void __fastcall hkOverrideView(void* pClientModeCSNormal, CViewSetup* pSetup);
	bool __fastcall hkCSVCMsg_UserMessage_Setup(void* CSVCMsg_UserMessage, int type, void* CUserMessage);
	void __fastcall hkSendNetMessage(void* netChannel, void* a2, void* protobufBinding, void* message, bool reliable) ;

	 void __fastcall hkScope(void* rcx, void* unk);	 
	 void* __fastcall hkUpdateSubClass(void* a1, void* a2, void* a3);
	 bool __fastcall hkAntiTamperReport(int data, void* dataLength, void* retAddress, int unknownVar);
	 bool __fastcall hkAntiTamper(int data, void* dataLength, void* retAddress, int unknownVar);

	 bool __fastcall hkEntityWantsInterpolation(CBaseEntity* ent, int unk);
	 
	 void __fastcall hkSetMeshGroupCopy(void* a1, void* a2, void* a3);

	inline CBaseHookObject<decltype(&hkSetMeshGroupCopy)> m_set_mesh_group_copy = {};

	 inline CBaseHookObject<decltype(&hkAntiTamper)> m_anti_tamper_report = {};
	inline CBaseHookObject<decltype(&hkAntiTamper)> m_anti_tamper = {};
	inline CBaseHookObject<decltype(&hkUpdateSubClass)> m_update_sub_class = {};
	inline CBaseHookObject<decltype(&hkEntityWantsInterpolation)> m_interpolate_on_entity = {};

	 inline CBaseHookObject<decltype(&hkCSVCMsg_UserMessage_Setup)> m_CSVCMsg_UserMessage_Setup = {};

	 inline CBaseHookObject<decltype(&hkSendNetMessage)> m_SendNetMessage = {};

	 inline CBaseHookObject<decltype(&hkScope)> m_remove_scope = {};

	inline CBaseHookObject<decltype(&hkGrenadeArray)> m_smoke_array = { };
	inline CBaseHookObject<decltype(&hkOverrideView)> m_override_view = {};

	inline CBaseHookObject<decltype(&set_input_history)> m_net_input_history = { };
	inline CBaseHookObject<decltype(&hkLightSceneObject)> m_draw_light_modulation = { };
	inline CBaseHookObject<decltype(&hkUpdateSceneObject)> m_update_scene_object = { };
	inline CBaseHookObject<decltype(&hkDrawSceneObject)> m_draw_scene_object = { };
	inline CBaseHookObject<decltype(&hkRunCommand)> m_run_command = { };

	inline CBaseHookObject<decltype(&hkProcessCRCcommands)> m_crc_commands = { };
	inline CBaseHookObject<decltype(&hkLevelInit)> m_level_init = { };
	inline CBaseHookObject<decltype(&hkLevelShutdown)> m_level_shutdown = { };
	inline CBaseHookObject<decltype(&hkGameEvent)> m_game_event = { };
	inline CBaseHookObject<decltype(&hkViewModel)> m_view_model = { };
	inline CBaseHookObject<decltype(&hkGetFov)> m_get_fov = { };
	inline CBaseHookObject<decltype(&AggreegateSceneObject)> m_aggregate_scene_object = { };
	inline CBaseHookObject<decltype(&hkGrenadeTrace)> m_grenade_trace = { };
	inline CBaseHookObject<decltype(&hkMotionBlur)> m_motion_blur = { };
	inline CBaseHookObject<decltype(&hkCalculateViewBob)> m_calc_view_bob = { };
	inline CBaseHookObject<decltype(&hkRenderViewSetup)> m_render_view_setup = { };
	inline CBaseHookObject<decltype(&hkGlowObject)> m_glow_object = { };
	inline CBaseHookObject<decltype(&hkAddCachedEntity)> m_add_entity = { };
	inline CBaseHookObject<decltype(&hkRemoveCachedEntity)> m_remove_entity = { };
	inline CBaseHookObject<decltype(&hkPresent)> m_present = { };
	inline CBaseHookObject<decltype(&hkViewMatrix)> m_view_matrix = {};
	inline CBaseHookObject<decltype(&hkResizeBuffers)> m_resize_buffers = { };
	inline CBaseHookObject<decltype(&hkCreateSwapChain)> m_create_swap_chain = { };
	inline CBaseHookObject<decltype(&hkMouseInputEnabled)> m_mouse_input_enabled = { };
	inline CBaseHookObject<decltype(&hkOverrideFOV)> m_override_fov = { };
	inline CBaseHookObject<decltype(&hkFrameStageNotify)> m_frame_stage_notify = { };
	inline CBaseHookObject<decltype(&hkCreateMove)> m_create_move = { };
	inline CBaseHookObject<decltype(&hkSetModel)> m_set_model = {};
	inline CBaseHookObject<decltype(&hkOnEquipItem)> m_equip_item_on_loadout = {};
	inline CBaseHookObject<decltype(&hkPreCreateMove)> m_pre_create_move = { };
	inline CBaseHookObject<decltype(&hkRelativeMouse)> m_relative_mouse = { };
	inline CBaseHookObject<decltype(&hkDrawObject)> m_draw_object = {};
	inline CBaseHookObject<decltype(&hkUnlockInventory)> m_unlock_inventory = {};
	inline CBaseHookObject<decltype(&hkCamera)> m_camera_input = { };
	inline unsigned int(__fastcall* m_get_handle_entity)(void*);
	inline void(__fastcall* m_decode_crc)(void*);
	inline void(__fastcall* m_interpolate_shoot_position)(void*, Vector_t*, int);
	inline void(__fastcall* m_update_weapon_sub_class)(void*);
	inline void(__fastcall* m_set_wep_model)(void*, const char*);
	inline void(__fastcall* m_refresh_skin)(void*, bool);
	inline void(__fastcall* m_regenerate_weapon_skin)();

	inline void(__fastcall* m_update_weapon_accuracy)(void*);
	inline bool(__fastcall* m_add_econ_item)(void*, void*, bool, bool, bool);
	inline CGCClientSharedObjectTypeCache* (__fastcall* m_create_base_type_cache)(void*, int);
	inline CGCClientSharedObjectCache* (__fastcall* m_find_base_type_so_cache)(void*, SOID_t, bool);

	inline bool(__fastcall* m_equip_item_on_loadout_call)(void*, int, int, uint64_t);
	inline bool (__fastcall* m_get_lerp_tick)(CGameSceneNode*, void*, void*, void*, int*);
	inline void(__fastcall* m_get_aim_punch)(void*, QAngle_t*, float, bool);
	inline void(__fastcall* m_init_grenade)(C_BaseCSGrenade*, CCSPlayerPawn*, Vector_t&, Vector_t&);
	inline void(__fastcall* m_set_mesh_group_mask)(void*, uint64_t);
	inline int(__fastcall* m_find_material_index)(void*, void*, int, bool);
	inline CEconItem* (*m_create_shared_object_subclass)();
	inline void* (__fastcall* m_set_dynamic_attribute_value_int)(void*, void*, void*);
	inline uint8_t* (__fastcall* m_get_vpk_buffer)(const char*, int*);
	inline void(__fastcall* m_run_prediction)(CNetworkGameClient*, int);
	inline void(__fastcall* m_process_move)(CCSPlayer_MovementServices*, CUserCmd*);
	inline void(__fastcall* m_set_pred_dada)(CCSPlayer_MovementServices*, CUserCmd*);
	inline bool(__fastcall* m_reset_pred_data)(void*);
	inline  void(__fastcall* m_parse)(uintptr_t*, CUtlBuffer, int );
	inline std::string* (__fastcall* m_merge)(void*, uintptr_t*, void*);
	inline  bool(__fastcall* m_serialize)(CBaseUserCmdPB*, CUtlBuffer, int);
	inline  bool(__fastcall* m_update_cmd)(CBaseUserCmdPB*);
	inline  int(__fastcall* m_crc_buffer_size)(void*);
	inline  CCSPlayerPawn*(__fastcall* m_get_local_player)(int);
	inline float(__fastcall* m_get_weapon_spread)(void*);
	inline bool(__fastcall* m_input_hold_button)(void*, int);
	inline c_grenade_trace* (__fastcall* m_init_grenade_trace)(int, C_BaseCSGrenade*);
	inline void(__fastcall* m_init_grenade_prediction)(c_grenade_trace*, Vector_t&, Vector_t&);
	inline void(__fastcall* m_grenade_bomb_direction)(void*);
	inline void(__fastcall* m_grenade_get_time)(float&, int);
	inline bool(__fastcall* m_grenade_valid_throw)(void*, float);
	inline void(__fastcall* m_grenade_direction)(C_BaseCSGrenade*, CBaseEntity*, Vector_t&, Vector_t&, bool);
	inline  int(__fastcall* m_get_bone_index)(void*, const char*);
	inline void(__fastcall* m_add_stattrack_entity)(void*, void*);
	inline void(__fastcall* m_add_keychain_entity)(void*, void*);
	inline void(__fastcall* m_add_name_tag_entity)(void*);
	inline void* (__fastcall* m_get_base_entity)(void*, int);
	inline CGCClientSystem* (*m_get_client_system)();
	inline CCSInventoryManager* (*m_get_inventory_manager)();
	inline float(__fastcall* m_get_innacuracy_weapon)(CBasePlayerWeapon*, void*, void*);
	inline void(__fastcall* m_get_spread_weapon_calculation)(void*, void*, int, unsigned int, float, float, float, float*, float*);
	inline void(__fastcall* m_weapon_update_accuracy)(void*);
	inline bool(__fastcall* m_locking_smoke_view)(Vector_t&, Vector_t&);
	inline void(__fastcall* m_calc_space_world_bones)(void*, uint32_t);
	inline void(__fastcall* m_upate_weapon_model)(void*, bool);
	inline void(__fastcall* m_set_econ_item_attirubte_by_name)(void*, const char*, float);

	
		inline void(__fastcall* m_run_physics_think)(CCSPlayerController*);
		inline void(__fastcall* m_set_player_tickbase)(CCSPlayerController*, int nTickBase);

	inline uint32_t(__fastcall* m_bones_num)(void*);
	inline int  m_get_weapon_data;
	inline uint32_t(__fastcall* m_bones_flags)(void*, uint32_t);
	inline const char* (__fastcall* m_bones_name)(void*, uint32_t);
	inline uint32_t(__fastcall* m_bones_parent)(void*, uint32_t);
	inline const char* (__fastcall* m_get_weapon_name)(CCSWeaponBase*);
	inline void(__fastcall* m_start_observer_glow)(CGlowProperty*, int, float);
	inline void(__fastcall* m_stop_observer_glow)(CGlowProperty*);
	inline uint64_t(__fastcall* m_get_hash_function)(const char*, int32_t);
	inline void(__fastcall* m_trace_init_info)(game_trace_t*);
	inline void(__fastcall* m_trace_init)(game_trace_t&);
	inline trace_filter_t* (__fastcall* m_trace_filter_init)(trace_filter_t&, void*, uint64_t, uint8_t, uint16_t);
	inline void(__fastcall* m_clip_trace_to_players)(Vector_t&, Vector_t&, trace_filter_t*, game_trace_t*, float, int, float);
	inline bool(__fastcall* m_clip_trace_to_entity)(i_trace*, ray_t*, const Vector_t&, const Vector_t&, CCSPlayerPawn*, trace_filter_t*, game_trace_t*);
	inline CHitBoxSets * (__fastcall* m_get_hitbox_set)(void*, int);
	inline int(__fastcall* m_get_hitbox_to_world_transform)(void*, CHitBoxSets*, CTransform*, int);
	inline void(__fastcall* m_trace_get_info)(trace_data_t*, game_trace_t*, float, void*);
	inline bool(__fastcall* m_trace_handle_bullet_penetration)(trace_data_t*, void*, UpdateValueT*, void*, void*, void*, void*, void*, bool);
	inline void(__fastcall* m_trace_create)(trace_data_t*, Vector_t, Vector_t, trace_filter_t, int);
	inline bool(__fastcall* m_trace_shape)(void*, ray_t&, Vector_t*, Vector_t*, trace_filter_t, game_trace_t&);
	inline CSubtickMoveStep* (__fastcall* m_create_sub_tick_step_move)(void*);
	inline CSubtickMoveStep* (__fastcall* m_create_sub_tick)(RepeatedPtrField_t<CSubtickMoveStep>&, CSubtickMoveStep*);
	inline CMsgQAngle* (__fastcall* m_create_cmsg_qangle)(DWORD64);
	inline CMsgVector* (__fastcall* m_create_cmsg_vector)(DWORD64);
    inline CCSGOInterpolationInfoPB_CL* (__fastcall* m_create_ccsgointerpolation_info_cl)(DWORD64);
	inline CCSGOInterpolationInfoPB* (__fastcall* m_create_ccsgointerpolation_info)(DWORD64);
	inline void(__thiscall* m_set_view_model_origin)(void*, Vector_t);
	inline void(__thiscall* m_set_entity_origin)(void*, Vector_t);
	inline void*(__fastcall* m_get_button_state)(void*, int);
	inline void* (__fastcall* m_init_attribute_string)(void*, uint64_t, bool);
	inline void* (__fastcall* m_fill_attribute_string)(void*, const char*);
	inline void(__fastcall* set_dynamic_attribute_value_string)(void*, void*, uint8_t*);
	inline void(__fastcall* set_dynamic_attribute_value)(C_EconItemView*, void*, void*);
	inline CEconItemAttributeDefinition* (__fastcall* get_attribute_definition_by_name)(const char**);
	inline int* (__fastcall* m_cache_particle_effect)(CGameParticleManager*, unsigned int*, const char*, int, __int64, __int64, __int64, int);
	inline bool(__fastcall* m_create_particle_effect)(CGameParticleManager*, unsigned int, int, void*, int);
	inline bool(__fastcall* m_unknown_particle_effect)(CGameParticleManager*, int, unsigned int, const CStrongHandle<CParticleSnapshot>*);

}
