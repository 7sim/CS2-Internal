#include "hooks.h"

#include "../render/render.h"
#include "../render/ui/menu.h"

#include "../../sdk/cache/cache_entity.h"

#include "..\..\utils\render\d3d11\render_manager.h"

#include "..\..\security\invoke\spoof.hxx"
#include "..\..\sdk\interfaces\cvar.h"
#include "..\..\sdk\interfaces\game_event.h"
#include "..\..\sdk\interfaces\network.h"

#include "..\..\sdk\datatypes\cutlbuffer\utlbuffer.h"
#include "..\..\cheats\skins\skins.h"
#include "../../cheats/movement/setup.h"
#include "..\..\cheats\skins\inventory.h"
#include "..\..\cheats\visuals\proccess.h"
#include "..\..\cheats\ragebot\ragebot.h"
#include "..\..\cheats\backtrack\record.h"
#include "..\..\cheats\legitbot\legitbot.h"
#include "../../cheats/visuals/camera/camera.h" 
#include "../../cheats/visuals/world/world.h" 
#include "..\..\sdk\addresses\entity.h"
#include "../../cheats/visuals/grenades/grenade.h"
#include "../../cheats/prediction/prediction.h" 
#include "..\..\cheats\movement\setup.h"
#include "..\..\cheats\chams\chams.h"
#include "..\..\cheats\skins\inventory_manager.h"
#include "..\..\cheats\visuals\world\hitsound.h"
#include "..\..\cheats\visuals\world\effects.h"
#include "..\..\..\security\vac\anticheat.h"
#include "..\..\cheats\subtick\subtick.h"
#include "..\..\cheats\antiaim\antiaim.h"
#include "..\..\cheats\animations\animations.h"
#include "..\..\cheats\visuals\radar\radar.hpp"

// macro to release dxgi pointers
// @note: malloc may be better than method than nulling x

static  QAngle_t aim_point{};
static bool initialized = false;
static bool connected = false;
// macro to release dxgi pointers
// @note: malloc may be better than method than nulling x
#define RELEASE_DXGI( x )   \
if ( x != nullptr )         \
{                           \
    x->Release( );          \
    x = nullptr;            \
}                           \

// macro to hook (looks better lmao)
#define ATTACH_HOOK_IDX( vtable_ptr, index, target, basehookobj ) \
basehookobj.Create( utils::GetVFunc( vtable_ptr, index ), target ); \

#define ATTACH_HOOK_SIG( ptr, target, basehookobj ) \
basehookobj.Create( (void*)ptr, target ); \
// Declaration of m_decode_crc as void*
void* m_compute_size_cmd_crc_ret_address = nullptr;
#include "..\..\helpers\vpk\VTexParser.h"



bool hooks::Init()
{
    
    bool success = true;

    // init minhook
    MH_Initialize();

    // hook pointers
    m_old_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(g::m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HkWndProc)));
    success &= (m_old_wndproc != nullptr);

    m_get_client_system = reinterpret_cast<decltype(m_get_client_system)>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ?? ?? ?? ?? 48 8B C8 E8 ?? ?? ?? ?? 8B D8 85 C0 74 33")), 0x1, 0x0));
    debug(m_get_client_system != nullptr);

    m_create_shared_object_subclass = reinterpret_cast<decltype(m_create_shared_object_subclass)>(utils::FindPattern("client.dll", xorstr_("48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85")));
    debug(m_create_shared_object_subclass != nullptr);

    m_set_dynamic_attribute_value_int = reinterpret_cast<decltype(m_set_dynamic_attribute_value_int)>(utils::FindPattern("client.dll", xorstr_("E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC 49 8B C0 48 8B CA 48 8B D0")));
    debug(m_set_dynamic_attribute_value_int != nullptr);

    m_run_physics_think = reinterpret_cast<decltype(m_run_physics_think)>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? 49 8B D6 48 8B CE E8 ? ? ? ? 48 8B 06")), 0x1, 0x0));
    debug(m_run_physics_think != nullptr);

    m_set_player_tickbase = reinterpret_cast<decltype(m_set_player_tickbase)>(utils::FindPattern("client.dll", xorstr_("39 91 28 06 00")));
    debug(m_set_player_tickbase != nullptr);

    m_get_vpk_buffer = reinterpret_cast<decltype(m_get_vpk_buffer)>(utils::FindPattern("client.dll", "40 55 57 41 56 48 83 EC 20 4C 8B F1"));
    debug(m_get_vpk_buffer != nullptr);

    m_get_lerp_tick = reinterpret_cast<decltype(m_get_lerp_tick)>(utils::FindPattern("client.dll", "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 81 EC 80 00 00 00 48 8B F1 0F"));
    debug(m_get_lerp_tick != nullptr);

    m_get_inventory_manager = reinterpret_cast<decltype(m_get_inventory_manager)>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ?? ?? ?? ?? 8B 93 D8 01 00 00")), 0x1, 0x0));
    debug(m_get_inventory_manager != nullptr);

    m_find_material_index = reinterpret_cast<decltype(m_find_material_index)>(utils::FindPattern("client.dll", xorstr_("48 89 5C 24 10 48 89 6C 24 18 56 57 41 57 48 83 EC 20 83 79 10 00 49 8B F1 41 8B E8 4C 8B FA 48 8B D9 74 72 44 8B 49 14 48 8B 39 41 FF C9 45 8B D9 45 23 D8 41")));
    debug(m_find_material_index != nullptr);

    m_set_mesh_group_mask = reinterpret_cast<decltype(m_set_mesh_group_mask)>(utils::GetAbsoluteAddress(utils::FindPattern(xorstr_("client.dll"), xorstr_("E8 ? ? ? ? 48 8B 5C 24 ? 4C 8B 7C 24 ? 4C 8B 74 24 ?")), 0x1, 0x0));
    debug(m_set_mesh_group_mask != nullptr);

    //CNetworkGameClient::ClientSidePredict
    m_run_prediction = reinterpret_cast<decltype(m_run_prediction)>(utils::FindPattern("engine2.dll", xorstr_("40 55 41 56 48 83 EC 28 80")));
    debug(m_run_prediction != nullptr);

    m_process_move = reinterpret_cast<decltype(m_process_move)>(utils::GetAbsoluteAddress(utils::FindPattern(xorstr_("client.dll"), xorstr_("E8 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 44 38 63")), 0x1, 0x0));
    debug(m_process_move != nullptr);

    m_set_pred_dada = reinterpret_cast<decltype(m_set_pred_dada)>(utils::FindPattern("client.dll", xorstr_("48 89 5C 24 08 57 48 83 EC 20 48 8B DA E8 ?? ?? ?? ?? 48 8B F8")));
    debug(m_set_pred_dada != nullptr);

    m_reset_pred_data = reinterpret_cast<decltype(m_reset_pred_data)>(utils::FindPattern("client.dll", xorstr_("48 83 EC 28 48 8B 41 30 48 89")));
    debug(m_reset_pred_data != nullptr);

    m_merge = reinterpret_cast<decltype(m_merge)>(utils::FindPattern("client.dll", xorstr_("48 89 5C 24 20 55 56 57 48 83 EC 30 49")));
    debug(m_merge != nullptr);

    m_parse = reinterpret_cast<decltype(m_parse)>(utils::FindPattern("client.dll", xorstr_("48 89 5C 24 10 48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 BF")));
    debug(m_parse != nullptr);

    m_serialize = reinterpret_cast<decltype(m_serialize)>(utils::FindPattern("client.dll", xorstr_("48 89 5C 24 18 55 56 57 48 81 EC 90")));
    debug(m_serialize != nullptr);

    m_update_cmd = reinterpret_cast<decltype(m_update_cmd)>(utils::FindPattern("client.dll", "48 89 5C 24 10 57 48 83 EC 20 48 8D 05 ?? ?? ?? ?? 48 8B D9 48 89 01 F6 41 08 01 74 0B 48 83 C1 08 E8 ?? ?? ?? ?? EB 08 48 8B 41 08 48 83 E0 FC 48 85 C0 75 4F"));
    debug(m_update_cmd != nullptr);

    m_get_bone_index = reinterpret_cast<decltype(m_get_bone_index)>(utils::FindPattern("client.dll", "E8 ? ? ? ? ? ? ? ? 33 DB 89 47 08"));
    debug(m_get_bone_index != nullptr);

    m_calc_space_world_bones = reinterpret_cast<decltype(m_calc_space_world_bones)>
        (utils::FindPattern("client.dll", "40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC D0"));
    debug(m_calc_space_world_bones != nullptr);

    m_locking_smoke_view = reinterpret_cast<decltype(m_locking_smoke_view)>
        (utils::FindPattern("client.dll", "48 83 EC 28 45 33 C0 E8"));
    debug(m_locking_smoke_view != nullptr);

    m_get_spread_weapon_calculation = reinterpret_cast<decltype(m_get_spread_weapon_calculation)>
        (utils::FindPattern("client.dll", "48 8B C4 48 89 58 08 48 89 68 18 48 89 70 20 57 41 54 41 55 41 56 41 57 48 81 EC E0"));
    debug(m_get_spread_weapon_calculation != nullptr);

    m_get_weapon_spread = reinterpret_cast<decltype(m_get_weapon_spread)>
        (utils::FindPattern("client.dll", "48 83 EC 38 48 63 91"));
    debug(m_get_weapon_spread != nullptr);
    
    m_get_innacuracy_weapon = reinterpret_cast<decltype(m_get_innacuracy_weapon)>
        (utils::FindPattern("client.dll", "48 89 5C 24 10 55 56 57 48 81 EC B0"));
    debug(m_get_innacuracy_weapon != nullptr);

    m_weapon_update_accuracy = reinterpret_cast<decltype(m_weapon_update_accuracy)>
        (utils::FindPattern("client.dll", "48 89 5C 24 18 57 48 83 EC 60 48 8B F9 E8"));
    debug(m_weapon_update_accuracy != nullptr);

    m_bones_num = reinterpret_cast<decltype(m_bones_num)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? 8B F8 89 44 24 ? EB ? 8B 7C 24")), 0x1, 0x5));
    debug(m_bones_num != nullptr);

    m_bones_flags = reinterpret_cast<decltype(m_bones_flags)>
        (utils::FindPattern("client.dll", "85 D2 78 16 3B 91"));
    debug(m_bones_flags != nullptr);


    m_bones_name = reinterpret_cast<decltype(m_bones_name)>
        (utils::FindPattern("client.dll", "85 D2 78 25 3B 91"));
    debug(m_bones_name != nullptr);

    m_bones_parent = reinterpret_cast<decltype(m_bones_parent)>
        (utils::FindPattern("client.dll", "85 D2 78 17 3B 91 78"));
    debug(m_bones_parent != nullptr);

    m_get_weapon_name = reinterpret_cast<decltype(m_get_weapon_name)>
        (utils::FindPattern("client.dll", "48 8B 81 ? ? ? ? 48 8B 88 ? ? ? ? 48 8D 05 ? ? ? ? 48 85 C9 48 0F 45 C1 C3 CC CC CC 44 0F B6 41 ?"));
    debug(m_get_weapon_name != nullptr);

    m_trace_init_info = reinterpret_cast<decltype(m_trace_init_info)>
        (utils::FindPattern("client.dll", "48 89 5C 24 08 57 48 83 EC 20 48 8B D9 33 FF 48 8B 0D"));
    debug(m_trace_init_info != nullptr);

    m_trace_init = reinterpret_cast<decltype(m_trace_init)>
        (utils::FindPattern("client.dll", "48 89 5C 24 08 57 48 83 EC 20 48 8B D9 33 FF 48 8B 0D"));
    debug(m_trace_init != nullptr);

    m_trace_filter_init = reinterpret_cast<decltype(m_trace_filter_init)>
        (utils::FindPattern("client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 0F B6 41 ? 33 F6"));
    debug(m_trace_filter_init != nullptr);

    m_clip_trace_to_players = reinterpret_cast<decltype(m_clip_trace_to_players)>
        (utils::FindPattern("client.dll", "48 8B C4 55 56 48 8D A8 48 FF FF FF 48 81 EC A8 01 00 00 48 89 58 08 4"));
    debug(m_clip_trace_to_players != nullptr);

    m_clip_trace_to_entity = reinterpret_cast<decltype(m_clip_trace_to_entity)>
        (utils::FindPattern("client.dll", "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC C0 00 00 00 48 8B 9C"));
    debug(m_clip_trace_to_entity != nullptr);


    m_trace_get_info = reinterpret_cast<decltype(m_trace_get_info)>
        (utils::FindPattern("client.dll", "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 48 8B E9 0F"));
    debug(m_trace_get_info != nullptr);

    m_trace_handle_bullet_penetration = reinterpret_cast<decltype(m_trace_handle_bullet_penetration)>
        (utils::FindPattern("client.dll", "48 8B C4 44 89 48 20 48 89 50 10 48"));
    debug(m_trace_handle_bullet_penetration != nullptr);

    m_trace_create = reinterpret_cast<decltype(m_trace_create)>
        (utils::FindPattern("client.dll", "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 40 F2"));
    debug(m_trace_create != nullptr);

    m_trace_shape = reinterpret_cast<decltype(m_trace_shape)>
        (utils::FindPattern("client.dll", "48 89 5C 24 20 48 89 4C 24 08 55 56 41"));
    debug(m_trace_shape != nullptr);

    m_create_sub_tick = reinterpret_cast<decltype(m_create_sub_tick)>
    (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ?? ?? ?? ?? 48 8B D0 8B 4A 10 F3 41 0F 10 46 F0")), 0x1, 0x0));
    debug(m_create_sub_tick != nullptr);
   
    // get button state 4C 63 C2 4B 8D 04  
    m_get_button_state = reinterpret_cast<decltype(m_get_button_state)>
        (utils::FindPattern("client.dll", ("4C 63 C2 4B 8D 04")));
    debug(m_get_button_state != nullptr);

    m_create_sub_tick_step_move = reinterpret_cast<decltype(m_create_sub_tick_step_move)>
    (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ?? ?? ?? ?? 48 8B D0 48 8D 4F 18 E8 ?? ?? ?? ?? 48 8B D0")), 0x1, 0x0));
    debug(m_create_sub_tick_step_move != nullptr);

    m_create_cmsg_qangle = reinterpret_cast<decltype(m_create_cmsg_qangle)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ? ? ? ? 49 89 46 40 49 8B 56 40")), 0x1, 0x0));
    debug(m_create_cmsg_qangle != nullptr);

    m_create_cmsg_vector = reinterpret_cast<decltype(m_create_cmsg_vector)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", "E8 ? ? ? ? 48 89 43 48 48 8B 4B 48"), 0x1, 0x0));
    debug(m_create_cmsg_vector != nullptr);
   
    m_create_ccsgointerpolation_info = reinterpret_cast<decltype(m_create_ccsgointerpolation_info)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ?? ?? ?? ?? 48 89 43 38 8B 45 EF")), 0x1, 0x0));
    debug(m_create_ccsgointerpolation_info != nullptr);

    m_create_ccsgointerpolation_info_cl = reinterpret_cast<decltype(m_create_ccsgointerpolation_info_cl)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ?? ?? ?? ?? 48 89 43 20 48 8B 43 20 F3 0F 10 45 F7")), 0x1, 0x0));
    debug(m_create_ccsgointerpolation_info_cl != nullptr);


    m_set_entity_origin = reinterpret_cast<decltype(m_set_entity_origin)>
        (utils::FindPattern("client.dll", ("48 89 5C 24 08 57 48 83 EC 40 48 8B 01 48 8B FA")));
    debug(m_set_entity_origin != nullptr);


    m_grenade_bomb_direction = reinterpret_cast<decltype(m_grenade_bomb_direction)>
        (utils::FindPattern("client.dll", ("48 83 EC 28 4C 8B C1 48 85 C9 0F")));
    debug(m_grenade_bomb_direction != nullptr);

    m_grenade_get_time = reinterpret_cast<decltype(m_grenade_get_time)>
        (utils::FindPattern("client.dll", ("48 89 5C 24 18 48 89 6C 24 20 57 48 83 EC 20 48 8B 3D")));
    debug(m_grenade_get_time != nullptr);

    m_grenade_valid_throw = reinterpret_cast<decltype(m_grenade_valid_throw)>
        (utils::FindPattern("client.dll", ("40 53 48 83 EC 30 48 8B 41 10 48 8B D9 0F 29 74 24 20 48 8D 4C 24 50")));
    debug(m_grenade_valid_throw != nullptr);

    m_input_hold_button = reinterpret_cast<decltype(m_input_hold_button)>
        (utils::FindPattern("client.dll", ("40 53 48 83 EC 20 48 8B 01 48 8B DA FF 90 ? ? ? ? ? ? ? ? 48 85 C0 74 0D")));
    debug(m_input_hold_button != nullptr);

    //
    m_grenade_direction = reinterpret_cast<decltype(m_grenade_direction)>
        (utils::FindPattern("client.dll", ("48 85 D2 0F 84 A1 04")));
    debug(m_grenade_direction != nullptr);

    m_init_grenade_trace = reinterpret_cast<decltype(m_init_grenade_trace)>
        (utils::FindPattern("client.dll", ("48 8B C4 48 89 50 10 55 41 56 48 8D 68 88")));
    debug(m_init_grenade_trace != nullptr);

    m_init_grenade_prediction = reinterpret_cast<decltype(m_init_grenade_prediction)>
        (utils::FindPattern("client.dll", ("48 8B C4 48 89 50 10 55 53 56 57 48")));
    debug(m_init_grenade_prediction != nullptr);

    m_upate_weapon_model = reinterpret_cast<decltype(m_upate_weapon_model)>
        (utils::FindPattern("client.dll", ("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 44 0F B6 F2 48 8B F1 E8")));
    debug(m_upate_weapon_model != nullptr);

    m_get_hash_function = reinterpret_cast<decltype(m_get_hash_function)>(utils::FindPattern("client.dll", "48 89 5C 24 08 44 0F"));
    debug(m_get_hash_function != nullptr);

    m_get_handle_entity = reinterpret_cast<decltype(m_get_handle_entity)>(utils::FindPattern("client.dll", "48 85 C9 74 32 48 8B 41 10 48 85 C0 74 29 44"));
    debug(m_get_handle_entity != nullptr);

    m_refresh_skin = reinterpret_cast<decltype(m_refresh_skin)>(utils::FindPattern("client.dll", "40 55 53 56 41 55 48 8D AC"));
    debug(m_refresh_skin != nullptr);
    

    m_set_view_model_origin = reinterpret_cast<decltype(m_set_view_model_origin)>
        (utils::FindPattern("client.dll", ("48 89 5C 24 08 57 48 83 EC 40 48 8B 01 48 8B FA 48")));
    debug(m_set_view_model_origin != nullptr);

    m_start_observer_glow = reinterpret_cast<decltype(m_start_observer_glow)>
        (utils::FindPattern("client.dll", ("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 3D")));
    debug(m_start_observer_glow != nullptr);
    // inherit from 48 89 5C 24 08 55 56 57 48 83 EC 20 48 8B 05
    m_get_base_entity = reinterpret_cast<decltype(m_get_base_entity)>
        (utils::FindPattern("client.dll", ("81 FA ? ? ? ? 77 ? 8B C2 C1 F8 ? 83 F8 ? 77 ? 48 98 48 8B 4C C1 ? 48 85 C9 74 ? 8B C2 25 ? ? ? ? 48 6B C0 ? 48 03 C8 74 ? 8B 41 ? 25 ? ? ? ? 3B C2 75 ? 48 8B 01")));
    debug(m_get_base_entity != nullptr);
    //smokedrawarray 48 89 54 24 10 55 41 55 48
    //radarspoted E8 ? ? ? ? 48 8B CE E8 ? ? ? ? 48 8B 9E ? ? ? ? 48 63 86 ? ? ? ? abs(1,0)

    m_add_stattrack_entity = reinterpret_cast<decltype(m_add_stattrack_entity)>(utils::FindPattern(CLIENT_DLL, xorstr_("48 89 5C 24 ? 48 89 7C 24 ? 55 41 54 41 55")));
    debug(m_add_stattrack_entity != nullptr);

    m_add_name_tag_entity = reinterpret_cast<decltype(m_add_name_tag_entity)>(utils::FindPattern(CLIENT_DLL, xorstr_("40 55 53 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA")));
    debug(m_add_name_tag_entity != nullptr);

    set_dynamic_attribute_value = reinterpret_cast<decltype(set_dynamic_attribute_value)>
        (utils::FindPattern("client.dll", ("48 89 6C 24 20 57 41 56 41 57 48 81 EC 90 00 00 00 48 8B FA C7 44 24 20 00 00 00 00 4D 8B F8 4C 8D 0D ? ? ? ? ? ? ? ? 48 8B E9 4C 8D 05 ? ? ? ? ? ? ? ? 33 D2 48 8B 4F 18 E8 ? ? ? ? ? ? ? ? 4C 8B F0 48 85 C0 0F 84 9A")));
    debug(set_dynamic_attribute_value != nullptr);
  

    m_init_attribute_string = reinterpret_cast<decltype(m_init_attribute_string)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? ? ? ? ? 48 8D 05 ? ? ? ? ? ? ? ? 48 89 7D 0F")), 0x1, 0x0));
    debug(m_init_attribute_string != nullptr);

    m_fill_attribute_string = reinterpret_cast<decltype(m_fill_attribute_string)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? 41 83 CF 08")), 0x1, 0x0));
    debug(m_fill_attribute_string != nullptr);


    m_find_base_type_so_cache = reinterpret_cast<decltype(m_find_base_type_so_cache)>
        (utils::FindPattern("client.dll", xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 44 0F B7 89")));
    debug(m_find_base_type_so_cache != nullptr);

    m_add_econ_item = reinterpret_cast<decltype(m_add_econ_item)>
        (utils::FindPattern("client.dll", xorstr_("44 88 4C 24 20 44 88 44 24 18 48")));
    debug(m_add_econ_item != nullptr);

    m_weapon_update_accuracy = reinterpret_cast<decltype(m_weapon_update_accuracy)>
        (utils::FindPattern("client.dll", xorstr_("48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 44 0F 29 44 24")));
    debug(m_weapon_update_accuracy != nullptr);

    m_create_base_type_cache = reinterpret_cast<decltype(m_create_base_type_cache)>
        (utils::FindPattern("client.dll", xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 44 0F B7 89")));
    debug(m_create_base_type_cache != nullptr);

    m_create_base_type_cache = reinterpret_cast<decltype(m_create_base_type_cache)>
        (utils::GetAbsoluteAddress(utils::FindPattern(xorstr_("client.dll"), xorstr_("E8 ? ? ? ? 33 C9 8B D1")), 1, 0));
    debug(m_create_base_type_cache != nullptr);
    

    m_regenerate_weapon_skin = reinterpret_cast<decltype(m_regenerate_weapon_skin)>
        (utils::FindPattern("client.dll", xorstr_("48 83 EC 38 E8 ?? ?? ?? ?? 48 85 C0 0F")));
    debug(m_regenerate_weapon_skin != nullptr);

    m_set_econ_item_attirubte_by_name = reinterpret_cast<decltype(m_set_econ_item_attirubte_by_name)>(
        utils::FindPattern("client.dll", xorstr_("40 53 48 83 EC 20 48 8B D9 48 81 C1 10 02 00 00 E8")));
    debug(m_set_econ_item_attirubte_by_name != nullptr);
    

    set_dynamic_attribute_value_string = reinterpret_cast<decltype(set_dynamic_attribute_value_string)>(
    utils::FindPattern("client.dll", xorstr_("48 89 7C 24 18 4C 89 64 24 20 55 41 56 41 57 48 8D 6C 24 B9 48 81 EC B0")));
    debug(set_dynamic_attribute_value_string != nullptr);

    get_attribute_definition_by_name = reinterpret_cast<decltype(get_attribute_definition_by_name)>
        (utils::FindPattern("client.dll", ("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 8B D9 E8")));
    debug(get_attribute_definition_by_name != nullptr);

    m_compute_size_cmd_crc_ret_address = reinterpret_cast<void*>(utils::FindPattern("client.dll", "48 8D 4C 24 ?? 4C 8B F8 8D 50 01"));


    m_get_weapon_data = *reinterpret_cast<int*>(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 8B 81 ? ? ? ? 85 D2 78 ? 48 83 FA ? 73 ? F3 0F 10 84 90 ? ? ? ? C3 F3 0F 10 80 ? ? ? ? C3 CC CC CC CC")) + 0x3);
    debug(m_get_weapon_data != 0);

    m_get_hitbox_set = reinterpret_cast<decltype(m_get_hitbox_set)>
        (utils::FindPattern("client.dll", ("48 89 ?? ?? ?? 48 89 ?? ?? ?? 57 48 ?? ?? ?? ?? ?? ?? 8B DA 48 8B F9 E8 ?? ?? ?? ?? 48 8B F0 48 85 C0 0F ?? ?? ?? ?? ?? 48 8D")));
    debug(m_get_hitbox_set != nullptr);
    
    m_get_hitbox_to_world_transform = reinterpret_cast<decltype(m_get_hitbox_to_world_transform)>
        (utils::FindPattern("client.dll", ("48 89 ?? ?? ?? 55 57 41 54 41 56 41 57 48 ?? ?? ?? 45")));
    debug(m_get_hitbox_to_world_transform != nullptr);


    //
    m_cache_particle_effect = reinterpret_cast<decltype(m_cache_particle_effect)>
        (utils::FindPattern("client.dll", ("4C 8B DC 53 48 83 EC 70 48 8B 84 24 A0 00 00 00")));
    debug(m_cache_particle_effect != nullptr);

    //
    m_create_particle_effect = reinterpret_cast<decltype(m_create_particle_effect)>
        (utils::FindPattern("client.dll", ("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 50 49 8B E9 41 8B F0")));
    debug(m_create_particle_effect != nullptr);

    m_unknown_particle_effect = reinterpret_cast<decltype(m_unknown_particle_effect)>
        (utils::FindPattern("client.dll", ("48 89 74 24 10 57 48 83 EC 30 4C 8B D9 49 8B F9 33 C9 41 8B F0 83 FA FF 0F")));
    debug(m_unknown_particle_effect != nullptr);

    m_get_aim_punch = reinterpret_cast<decltype(m_get_aim_punch)>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? 48 8D 4E ? 48 8D 54 24 ? E8 ? ? ? ? F2 0F 10 44 24")), 0x1, 0x0));
    debug(m_get_aim_punch != nullptr);

    m_interpolate_shoot_position = reinterpret_cast<decltype(m_interpolate_shoot_position)>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? 41 8B 86 ? ? ? ? C1 E8 ? A8 ? 0F 85")), 0x1, 0x0));
    debug(m_interpolate_shoot_position != nullptr);

    m_get_local_player = reinterpret_cast<decltype(m_get_local_player)>(utils::FindPattern("client.dll", xorstr_("48 83 EC 28 83 F9 FF 75 17 48 8B 0D ?? ?? ?? ?? 48 8D 54 24 30 48 8B 01 FF 90 ?? ?? ?? ?? 8B 08 48 63 C1 48 8D 0D ?? ?? ?? ?? 48 8B 0C")));
    debug(m_get_local_player != nullptr);

    m_update_weapon_sub_class = reinterpret_cast<decltype(m_update_weapon_sub_class)>(utils::FindPattern("client.dll", xorstr_("40 53 48 83 EC 30 48 8B 41 10 48 8B D9 8B 50 30")));
    debug(m_update_weapon_sub_class != nullptr);
   
    m_set_wep_model = reinterpret_cast<decltype(m_set_wep_model)>(utils::FindPattern("client.dll", xorstr_("40 53 48 83 EC 20 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24")));
    debug(m_set_wep_model != nullptr);

    m_equip_item_on_loadout_call = reinterpret_cast<decltype(m_equip_item_on_loadout_call)>(utils::FindPattern("client.dll", xorstr_("48 89 5C 24 08 4C 89 4C 24 20 89")));
    debug(m_equip_item_on_loadout_call != nullptr);
    
    m_add_keychain_entity = reinterpret_cast<decltype(m_add_keychain_entity)>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? 4C 8B B4 24 ? ? ? ? 48 8B BC 24 ? ? ? ? 48 8B 9C 24")), 0x1, 0x0));
    debug(m_add_keychain_entity != nullptr);

    // hook vtable functions 
    ATTACH_HOOK_IDX(sdk::m_swap_chain->m_swap_chain, 8, &hkPresent, m_present);
    ATTACH_HOOK_IDX(sdk::m_swap_chain->m_swap_chain, 13, &hkResizeBuffers, m_resize_buffers);

    //ATTACH_HOOK_IDX(sdk::m_game->pGameEntitySystem, 15, &hkAddCachedEntity, m_add_entity);
  //  ATTACH_HOOK_IDX(sdk::m_game->pGameEntitySystem, 16, &hkRemoveCachedEntity, m_remove_entity);

    /*ATTACH_HOOK_IDX(sdk::m_game->pGameEntitySystem, 14, &hkAddCachedEntity, m_add_entity);
     debug(m_add_entity.GetOriginal() != nullptr);

     ATTACH_HOOK_IDX(sdk::m_game->pGameEntitySystem, 15, &hkRemoveCachedEntity, m_remove_entity);
     debug(m_remove_entity.GetOriginal() != nullptr);
     */
    ATTACH_HOOK_IDX(sdk::m_input_system, 76, &hkRelativeMouse, m_relative_mouse);

    ATTACH_HOOK_IDX(sdk::m_csgo_input, 19, &hkMouseInputEnabled, m_mouse_input_enabled);
   // ATTACH_HOOK_IDX(sdk::m_csgo_input, 5, &hkCreateMove, m_create_move);
    //  ATTACH_HOOK_IDX(sdk::m_csgo_input, 7, &hkCamera, m_camera_input);

   // ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 53 48 83 EC 20 80 B9 ? ? ? ? ? 48 8B D9 75 78")), &hkMouseInputEnabled, m_mouse_input_enabled);
   // debug(m_mouse_input_enabled.GetOriginal() != nullptr);
  //  ATTACH_HOOK_IDX(sdk::m_input_system, 78, &hkRelativeMouse, m_relative_mouse);
  // 
    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 8B C1 4C 8D 15")), &hkSetMeshGroupCopy, m_set_mesh_group_copy);
    debug(m_set_mesh_group_copy.GetOriginal() != nullptr);


   ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("85 D2 0F 85 00")), &hkCamera, m_camera_input);
   debug(m_camera_input.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("animationsystem.dll"), xorstr_("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B 40 48")), &hkUpdateSubClass, m_update_sub_class);
    debug(m_update_sub_class.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40")), &hkCreateMove, m_create_move);
    debug(m_create_move.GetOriginal() != nullptr);
    
    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 ? 4C 89 4C 24 ? 89 54 24 ? 55")), &hkOnEquipItem, m_equip_item_on_loadout);
    debug(m_equip_item_on_loadout.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("4C 89 4C 24 20 55 53 57 41 56 48 8D")), &set_input_history, m_net_input_history);
    debug(m_net_input_history.GetOriginal() != nullptr);


   // ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("inputsystem.dll"), xorstr_("48 89 5C 24 18 57")), &hkRelativeMouse, m_relative_mouse);
   // debug(m_relative_mouse.GetOriginal() != nullptr);

   // ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("85 D2 0F 85 69")), &hkCreateMove, m_create_move);
   // debug(m_create_move.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("83 B9 ? ? ? ? ? 74 11")), &hkGrenadeArray, m_smoke_array);
    debug(m_smoke_array.GetOriginal() != nullptr);

  //  ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 8B C4 48 81 EC F8 00 00 00 48")), &hkRunCommand, m_run_command);
//    debug(m_run_command.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 ? 56 48 83 EC 30 8B 05 ? ? ? ?")), &hkFrameStageNotify, m_frame_stage_notify);
    debug(m_frame_stage_notify.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 18 56 48 83 EC 30 48 8B 0D")), &hkLevelInit, m_level_init);
    debug(m_level_init.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 74 24 10 57 48 83 EC 20 48 8B F9 E8 ?? ?? ?? ?? 48 8D 8F")), &hkLevelShutdown, m_level_shutdown);
    debug(m_level_shutdown.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 53 48 81 EC ? ? ? ? 49 8B C1")), &hkViewMatrix, m_view_matrix);
    debug(m_view_matrix.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern("client.dll", "48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 81 EC A0 00 00 00 4C"), &hkRenderViewSetup, m_render_view_setup); ;
    debug(m_render_view_setup.GetOriginal() != nullptr);

    //ATTACH_HOOK_SIG(utils::FindPattern("client.dll", "40 53 48 83 EC 20 48 8B 54"), &AggreegateSceneObject, m_aggregate_scene_object);
  //  debug(m_aggregate_scene_object.GetOriginal() != nullptr);
    // hook game functions from signatures
  //  ATTACH_HOOK_SIG(utils::FindPattern("client.dll", "48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 48 8D 6C"), &hkViewModel, m_view_model); ;
    ATTACH_HOOK_SIG(utils::FindPattern("client.dll", "40 53 48 83 EC 20 48 8B 54"), &hkGlowObject, m_glow_object);
    debug(m_glow_object.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern("client.dll", "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4"), &hkGetFov, m_get_fov); ;
    debug(m_get_fov.GetOriginal() != nullptr);

  
    //ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 20 55 41 56 41 57 48 81")), &hkCalculateViewBob, m_calc_view_bob);
   // debug(m_calc_view_bob.GetOriginal() != nullptr);

    //ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 56 48 83 EC 70 48")), &hkGrenadeTrace, m_grenade_trace);
   // debug(m_grenade_trace.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 08 48 89 74 24 18 57 48 83 EC 20 48 8B 0D")), &hkUnlockInventory, m_unlock_inventory);
    debug(m_unlock_inventory.GetOriginal() != nullptr);

    // ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 53 48 81 EC 80 00 00 00 48 8B D9 E8 ?? ?? ?? ?? 48 85")), &hkOverrideFOV, m_override_fov);
    // debug(m_override_fov.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 53 48 83 EC 20 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24")), &hkSetModel, m_set_model);
    debug(m_set_model.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 55 53 56 57 41 55 41 57 48 8D")), &hkGameEvent, m_game_event);
    debug(m_game_event.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("scenesystem.dll"), xorstr_("48 8B C4 48 89 50 10 53 41 55 41 56 48 81 EC ? ? ? ? 4D 63 F1")), &chams::hook, m_draw_object);
    debug(m_draw_object.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("scenesystem.dll"), xorstr_("48 89 54 24 ? 53 41 56 41 57")), &hkLightSceneObject, m_draw_light_modulation);
    debug(m_draw_light_modulation.GetOriginal() != nullptr);
    

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("scenesystem.dll"), xorstr_("48 89 5C 24 10 48 89 6C 24 18 56 57 41 54 41 56 41 57 48 83 EC 40")), &hkUpdateSceneObject, m_update_scene_object);
    debug(m_update_scene_object.GetOriginal() != nullptr);
    
    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("scenesystem.dll"), xorstr_("48 89 54 24 ? 55 57 41 55 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 49 63 F9")), &hkDrawSceneObject, m_draw_scene_object);
    debug(m_draw_scene_object.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8")), &hkOverrideView, m_override_view);
    debug(m_override_view.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("4C 8B DC 53 56 57 48 83 EC")), &hkScope, m_remove_scope);
    debug(m_remove_scope.GetOriginal() != nullptr);
   

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 41 8B F1")), &hkAntiTamperReport, m_anti_tamper_report);
    debug(m_anti_tamper_report.GetOriginal() != nullptr);

    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 89 64 24 20 55 41 56 41 57 48 8D 6C 24 B9 48 81 EC F0")), &hkAntiTamper, m_anti_tamper);
    debug(m_anti_tamper.GetOriginal() != nullptr);  
    
    ATTACH_HOOK_SIG(utils::FindPattern("client.dll", "85 D2 0F 84 ? ? ? ? 55 57 48 83 EC"), &hkEntityWantsInterpolation, m_interpolate_on_entity); ;
    debug(m_interpolate_on_entity.GetOriginal() != nullptr);


    /*

    // vac hooks
#ifndef _DEBUG
    auto netchannel = *reinterpret_cast<void**>(utils::ResolveRelativeAddress(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 8B 05 ? ? ? ? 48 8B 00 48 8B 0D ? ? ? ? FF 90 ? ? ? ? 0F B6 C0 85 C0 74 ? 48 8D 0D")), 0x3, 0x7));

    ATTACH_HOOK_IDX(netchannel, 48, &hooks::hkSendNetMessage, hooks::m_SendNetMessage);
    debug(hooks::m_SendNetMessage.GetOriginal() != nullptr);


    ATTACH_HOOK_SIG(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 53 57 41 56 48 83 EC ? 49 8B 00")), &hooks::hkCSVCMsg_UserMessage_Setup, hooks::m_CSVCMsg_UserMessage_Setup);
    debug(hooks::m_CSVCMsg_UserMessage_Setup.GetOriginal() != nullptr);
#endif*/

    // ATTACH_HOOK_SIG(utils::GetAbsoluteAddress(utils::FindPattern(xorstr_("client.dll"), xorstr_("E8 ?? ?? ?? ?? 48 8D 4C 24 ?? 4C 8B F8")), 1, 0), &hkProcessCRCcommands, m_crc_commands);
    // debug(m_crc_commands.GetOriginal() != nullptr);

     
   
    /* release DXGI hooks */
    {
        // create dxgi deviceB
        IDXGIDevice* dxgi_device = nullptr;

        sdk::m_device->QueryInterface(__uuidof(**(&dxgi_device)), IID_PPV_ARGS_Helper(&dxgi_device));
        success &= (dxgi_device != nullptr);

        // get dxgi adapter from dxgi device
        IDXGIAdapter* dxgi_adapter = nullptr;
        dxgi_device->GetAdapter(&dxgi_adapter);
        success &= (dxgi_adapter != nullptr);

        // get dxgi factory from dxgi adapter
        IDXGIFactory* dxgi_factory = nullptr;
        dxgi_adapter->GetParent(__uuidof(**(&dxgi_factory)), IID_PPV_ARGS_Helper(&dxgi_factory));
        success &= (dxgi_factory != nullptr);

        // hook create swap chain
        ATTACH_HOOK_IDX(dxgi_factory, 10, &hkCreateSwapChain, m_create_swap_chain);

        // free dxgi pointers
        dxgi_device->Release();
        dxgi_adapter->Release();
        dxgi_factory->Release();
    }

    // update CGameEvents 
    game_events->Initialize();
#ifdef _DEBUG
    // emulate vac hooks  (bypass vac anticheat) 
   // vac::data->emulate();
#endif

    initialized = true;

    // sometimes users might inject in game 
    if (sdk::m_cvar && sdk::m_engine->is_valid()) {
        _log(LOG_INFO) << "loaded cvars";
        g::m_yaw = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("m_yaw")))->value.fl;
        g::m_pitch = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("m_pitch")))->value.fl;
        g::max_unlag = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("sv_maxunlag")))->value.fl;
        g::sv_friction = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("sv_friction")))->value.fl;
        g::sensitivity = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("sensitivity")))->value.fl;
        g::deathmatch = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("mp_teammates_are_enemies")))->value.i1;
        g::sv_accelerate = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("sv_accelerate")))->value.fl;
        g::sv_airaccelerate = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("sv_airaccelerate")))->value.fl;
        g::sv_autobunnyhopping = sdk::m_cvar->find(hash_32_fnv1a_const(xorstr_("sv_autobunnyhopping")))->value.i1;

        /* for those who injected mid game */

        if (sdk::m_global_vars) {
            _log(LOG_WARNING) << "[hooks] on map load ( loading bsp texture on map: " << std::string(sdk::m_global_vars->m_current_map_name).c_str() << ")";
            g::update_radar = false;
            radar::instance->on_map_load();
        }
    }  

   // player_cache_system::g_entity_cache->startup();
//    weapon_cache_system::g_weapons_cache->startup();

    // return status
    return success;
}

enum ModelType_t : int
{
    MODEL_SUN,
    MODEL_CLOUDS,
    MODEL_EFFECTS,
    MODEL_OTHER,
};

int GetModelType(const std::string_view& name)
{
    if (name.find(("sun")) != std::string::npos)
        return MODEL_SUN;

    if (name.find(("clouds")) != std::string::npos)
        return MODEL_CLOUDS;

    if (name.find(("effects")) != std::string::npos)
        return MODEL_EFFECTS;

    return MODEL_OTHER;
}

static Vector_t old_smoke_color{};
void * hooks::hkGrenadeArray(C_SmokeGrenadeProjectile* smoke) {
    static auto original = m_smoke_array.GetOriginal();

    if (!initialized)
        return original(smoke);


    if (g::m_unloading)
        return original(smoke);


    if (!sdk::m_engine->is_valid())
        return original(smoke);

    ImVec4 color = cfg_get(ColorPickerVar_t, g_cfg.smoke_color).colValue.GetVec4();
    Vector_t CUSTOM_SMOKE_CLR = Vector_t(color.x, color.y, color.z);
   
    old_smoke_color = smoke->m_vSmokeColor();

    if (cfg_get(bool, g_cfg.custom_smoke_color)) {
        smoke->m_vSmokeColor() = CUSTOM_SMOKE_CLR;
    }
    else 
        old_smoke_color = smoke->m_vSmokeColor();


    return original( smoke);
}
void  hooks::hkScope(void* rcx, void* unk) {
    static auto original = m_remove_scope.GetOriginal();
    if (!initialized)
        return original(rcx, unk);

    if (g::m_unloading)
        return original(rcx, unk);

    if (!sdk::m_engine->is_valid())
        return original(rcx, unk);

    // aka scope but wrong named
    if (cfg_get(bool, g_cfg.ladder_jump))
        return;
    

    original(rcx, unk);
}

void __fastcall hooks::hkDrawSceneObject(void* ptr, void* a2, CBaseSceneData* scene_data, int count, void* a5, void* a6, void* a7, void* a8) {

 
    const auto oDrawAggregateSceneObject = m_draw_scene_object.GetOriginal();
    if (!initialized || !g::chams_ready)
        return oDrawAggregateSceneObject(ptr, a2, scene_data, count, a5, a6, a7, a8);

    if (g::m_unloading)
        return oDrawAggregateSceneObject(ptr, a2, scene_data, count, a5, a6, a7, a8);

    if (!sdk::m_engine->is_valid())
        return oDrawAggregateSceneObject(ptr, a2, scene_data, count, a5, a6, a7, a8);

    if (!scene_data)
        return oDrawAggregateSceneObject(ptr, a2, scene_data, count, a5, a6, a7, a8);

    if (!scene_data->material || count <= 1)
        return oDrawAggregateSceneObject(ptr, a2, scene_data, count, a5, a6, a7, a8);


    auto apply = false;
    auto type = GetModelType(scene_data->material->GetName());
    Color_t color{ 255, 255, 255, 255};

    switch (type)
    {
    case MODEL_SUN:
        apply = cfg_get(bool, g_cfg.sun);
        color = !apply ? Color_t{ 255, 255, 255, 255 } : cfg_get(ColorPickerVar_t, g_cfg.sun_col).colValue;
        break;
    case MODEL_CLOUDS:
        apply = cfg_get(bool, g_cfg.clouds);
        color = !apply ? Color_t{ 255, 255, 255, 255 } : cfg_get(ColorPickerVar_t, g_cfg.cloud_clr).colValue;
        break;
    case MODEL_EFFECTS:
    case MODEL_OTHER:
        apply = cfg_get(bool, g_cfg.props);
        color = !apply ? Color_t{ 255, 255, 255, 255 } : cfg_get(ColorPickerVar_t, g_cfg.props_col).colValue;
        break;
    }

    for (int i = 0; i < count; i++)
    {
        CBaseSceneData* scene = &scene_data[i];
        if (!scene)
            continue;
        
        scene->r = color.r;
        scene->g = color.g;
        scene->b = color.b;
        scene->a = color.a;
    }

    oDrawAggregateSceneObject(ptr, a2, scene_data, count, a5, a6, a7, a8);
}

static bool set_night = false;
Color_t backup_light_color = Color_t(255, 255, 255, 255); // Store the original light color

void* __fastcall hooks::hkUpdateSceneObject(C_AggregateSceneObject* object, void* unk) {
    static auto original = m_update_scene_object.GetOriginal();
    auto result = original(object, unk);

    if (!initialized)
        return result;

    if (g::m_unloading)
        return result;

    if (!sdk::m_engine->is_valid())
        return result;

    if (object) {
        if (cfg_get(bool, g_cfg.night_mode)) {
            Color_t colors = cfg_get(ColorPickerVar_t, g_cfg.night_mode_col).colValue;

            for (int i = 0; i < object->m_nCount; i++) {
                object->m_pData[i].r = colors.r;
                object->m_pData[i].g = colors.g;
                object->m_pData[i].b = colors.b;
            }
        }
        else {
            // If night mode is disabled, apply the original colors
            for (int i = 0; i < object->m_nCount; i++) {
                object->m_pData[i].r = backup_light_color.r;
                object->m_pData[i].g = backup_light_color.g;
                object->m_pData[i].b = backup_light_color.b;
            }
        }
    }

    return result;
}
Color_t backup_light_color2; // Store the original light color
static bool light = false;
void* __fastcall hooks::hkLightSceneObject(void* ptr, C_SceneLightObject* object, void* unk) {
    static auto original = m_draw_light_modulation.GetOriginal();
    auto result = original(ptr, object, unk);

    if (!initialized)
        return result;

    if (g::m_unloading)
        return result;

    if (!sdk::m_engine->is_valid())
        return result;

    if (object) {
        if (!light) {
            backup_light_color2.r = object->r;
            backup_light_color2.g = object->g;
            backup_light_color2.b = object->b;
        }
        if (cfg_get(bool, g_cfg.lighting)) {
            Color_t colors = cfg_get(ColorPickerVar_t, g_cfg.lighting_col).colValue;
            object->r = colors.r / 255.f;
            object->g = colors.g / 255.f;
            object->b = colors.b / 255.f;
        }
        else {
            // Apply the original backup light color when lighting is disabled
            object->r = backup_light_color2.r;
            object->g = backup_light_color2.g;
            object->b = backup_light_color2.b;
            light = true;
        }
    }

    return result;
}

void hooks::hkRelativeMouse(void* input_system, bool enabled)
{
   const auto og = hooks::m_relative_mouse.GetOriginal();
  
    menu::m_wind_active = enabled;

    if (menu::m_opened)
        return og(input_system, false);

    return og(input_system, enabled);
}
void __fastcall hooks::hkSetModel(void* rcx, const char* model) {
    const auto oSetModel = m_set_model.GetOriginal();
    if (!initialized)
        return oSetModel(rcx, model);

    if (g::m_unloading)
        return oSetModel(rcx, model);

    if (!sdk::m_engine->is_valid())
        return oSetModel(rcx, model);


    Items::skins->set_model((C_BaseModelEntity*)rcx, model);


    return oSetModel(rcx, model);
}
bool __fastcall hooks::hkOnEquipItem(void* rcx, int iTeam, int iSlot, uint64_t iItemID) {
    const auto oEquipItemInLoadout = m_equip_item_on_loadout.GetOriginal();
    if (!initialized)
        return oEquipItemInLoadout(rcx, iTeam, iSlot, iItemID);

    if (g::m_unloading)
        return oEquipItemInLoadout(rcx, iTeam, iSlot, iItemID);


    Items::skins->equip_item(iTeam, iSlot, iItemID);
    return oEquipItemInLoadout(rcx, iTeam, iSlot, iItemID);
}

bool __fastcall hooks::hkAntiTamperReport(int data, void* dataLength, void* retAddress, int unknownVar)
{
    auto og = hooks::m_anti_tamper_report.GetOriginal();
    if (g::m_unloading)
        return og(data, dataLength, retAddress, unknownVar);

    return true;
}
bool __fastcall hooks::hkAntiTamper(int data, void* dataLength, void* retAddress, int unknownVar)
{
    auto og = hooks::m_anti_tamper.GetOriginal();
    if (g::m_unloading)
        return og(data, dataLength, retAddress, unknownVar);

    return true;
}
void hooks::hkLevelInit(void* client_mode, const char* new_map)
{
    auto og = hooks::m_level_init.GetOriginal();
    if (!initialized)
        return og(client_mode, new_map);

    if (g::m_unloading)
        return og(client_mode, new_map);

    _log(LOG_INFO) << "[level_init] new game detected resetting data...";

    //if (!sdk::m_global_vars)
    sdk::m_global_vars = *reinterpret_cast<CGlobalVarsBase**>(utils::ResolveRelativeAddress(utils::FindPattern("client.dll", xorstr_("48 89 15 ?? ?? ?? ?? 48 89 42")), 0x3, 0x7));
    sdk::m_particle_game = particles->GetGameParticleManager();
    sdk::m_game_rules = *reinterpret_cast<C_CSGameRules**>(utils::ResolveRelativeAddress(utils::FindPattern(xorstr_("client.dll"), xorstr_("48 8B 0D ? ? ? ? 48 85 C9 74 ? 48 8B 01 48 8D 54 24 ? 45 33 C0")), 0x3, 0x7));

    return og(client_mode, new_map);

}
void hooks::hkLevelShutdown(std::uintptr_t* unk)
{
    auto og = hooks::m_level_shutdown.GetOriginal();
    if (!initialized)
        return og(unk);

    if (g::m_unloading)
        return og(unk);
    _log(LOG_INFO) << "[level_shutdown] disconnecting from scene, resetting data...";
    
   g::chams_ready = false;


   sdk::m_global_vars = nullptr;
   sdk::m_particle_game = nullptr;
   sdk::m_game_rules = nullptr;
   g::update_radar = false;

   connected = false;
   return og(unk);
}

bool __fastcall hooks::hkUnlockInventory(void* rcx) {
    auto og =  hooks::m_unlock_inventory.GetOriginal();
    if (!initialized)
        return og( rcx);

    if (g::m_unloading)
        return og(rcx);

    if (cfg_get(bool, g_cfg.unlock_inventory)) 
        return true;

    return og(rcx);
}

float __fastcall hooks::hkGetFov(void* rcx)
{
    const auto oSetViewModelFOV = hooks::m_get_fov.GetOriginal();
    if (!initialized)
        return oSetViewModelFOV(rcx);

    if (g::m_unloading)
        return oSetViewModelFOV(rcx);


    if (!sdk::m_engine->is_valid())
        return oSetViewModelFOV(rcx);

    auto local_controller = CCSPlayerController::Get();
    if (!local_controller)
        return oSetViewModelFOV(rcx);

    auto local = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(local_controller->PawnHandle());
    if (!local)
        return oSetViewModelFOV(rcx);

    if (!cfg_get(bool, g_cfg.camera_fov) || cfg_get(float, g_cfg.camera_fov_value) == 0.f)
        return oSetViewModelFOV(rcx);

    if (!local->IsAlive() || local->m_bIsScoped()) // Checking if your spectating and alive
        return oSetViewModelFOV(rcx);

    return  cfg_get(float, g_cfg.camera_fov_value);
    
}
void __fastcall hooks::hkGlowObject(CGlowProperty* rcx)
{
    auto og = m_glow_object.GetOriginal();

    if (!initialized)
    return og(rcx);

    if (g::m_unloading)
        return og(rcx);


    if (!cfg_get(bool, g_cfg.bGlow))
        return og(rcx);

    if (!sdk::m_engine->is_valid())
        return og(rcx);

    auto local = hooks::m_get_local_player(0);
    if (!local)
        return og(rcx);

    if (!local->IsAlive())
        return og(rcx);

    auto entity = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(rcx->GetOwner()->handle());
    if (entity) {
        if (entity->IsBasePlayer() && entity->IsAlive()) {
            if (entity != local && entity->enemy(local)) {
                rcx->m_glowColorOverride() = cfg_get(ColorPickerVar_t, g_cfg.bGlowClr).colValue;
                rcx->m_bGlowing() = true;
            }
        }
       /* else if (entity->IsWeapon()) {
            auto wep = (CCSWeaponBase*)entity;
            if (!wep->m_hOwnerEntity().valid()) {
                rcx->m_glowColorOverride() = cfg_get(ColorPickerVar_t, g_cfg.bGlowClr).colValue;
                rcx->m_bGlowing() = true;
            }
        }
        else {
            SchemaClassInfoData_t* class_info = nullptr;
            entity->dump_class_info(&class_info);

            if (class_info) {

                if (c_run_time::StringCompare(class_info->szName, xorstr_("C_SmokeGrenadeProjectile")) == 0) {
                    rcx->m_glowColorOverride() = cfg_get(ColorPickerVar_t, g_cfg.bGlowClr).colValue;
                    rcx->m_bGlowing() = true;
                }
                else  if (c_run_time::StringCompare(class_info->szName, xorstr_("C_HEGrenadeProjectile")) == 0) {
                    rcx->m_glowColorOverride() = cfg_get(ColorPickerVar_t, g_cfg.bGlowClr).colValue;
                    rcx->m_bGlowing() = true;
                }
                else  if (c_run_time::StringCompare(class_info->szName, xorstr_("C_MolotovProjectile")) == 0) {
                    rcx->m_glowColorOverride() = cfg_get(ColorPickerVar_t, g_cfg.bGlowClr).colValue;
                    rcx->m_bGlowing() = true;
                }
                else  if (c_run_time::StringCompare(class_info->szName, xorstr_("C_FlashBangProjectile")) == 0) {
                    rcx->m_glowColorOverride() = cfg_get(ColorPickerVar_t, g_cfg.bGlowClr).colValue;
                    rcx->m_bGlowing() = true;
                }
            }
        }*/
    }
    else
        return og(rcx);

  




}
void __fastcall hooks::hkMotionBlur(void* a1, unsigned int ev, Vector4D_t* value) {
    auto og = hooks::m_motion_blur.GetOriginal();
    if (!initialized)
        return og(a1, ev, value);

    if (g::m_unloading)
        return og(a1, ev, value);

    if (!sdk::m_engine->is_valid() )
        return og(a1, ev, value);

    auto controller = CCSPlayerController::Get();
    if (!controller)
        return og(a1, ev, value);

    auto local = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(controller->PawnHandle().index());
    if (!local)
        return og(a1, ev, value);


    int hash = hooks::m_get_hash_function(xorstr_("DofRanges"), 0x3141592F);
    unsigned int hash2 = (0x5BD1E995 * (hash ^ 0x73)) ^ ((0x5BD1E995 * (hash ^ 0x73u)) >> 13);
    unsigned int hash3 = (1540483477 * hash2) ^ ((1540483477 * hash2) >> 15);

    if (cfg_get(bool, g_cfg.motion_blur))
    {
        Vector4D_t new_value;
        new_value.y = 0.f;
        new_value.x = -100.f;
        new_value.w = 2000.f;
        new_value.z = 180.f;

        og(a1, hash3, &new_value);
    }

    return og(a1, ev, value);

}

static Vector_t backup_viewmodel = Vector_t();
static Vector_t backup_viewmodel_origin = Vector_t();
static float backup_viewmodel_fov;

void __fastcall hooks::hkRenderViewSetup(IViewRender* pViewRender) {
    auto og = hooks::m_render_view_setup.GetOriginal();
    if (!initialized)
        return og(pViewRender);

    if (g::m_unloading)
        return og(pViewRender);

    if (!sdk::m_engine->is_valid())
        return og(pViewRender);


    auto controller = CCSPlayerController::Get();
    if (!controller)
        return og(pViewRender);

    auto local = hooks::m_get_local_player(0);
    if (!local)
        return og(pViewRender);

    if (!local->IsAlive())
        return og(pViewRender);

    og(pViewRender);

    auto scoped = local->m_bIsScoped();

    if (cfg_get(bool, g_cfg.view_model) && pViewRender) {
        auto viewmodel = local->m_pViewModelServices();
        if (viewmodel) {
            if (!backup_viewmodel_fov)
                backup_viewmodel_fov = pViewRender->view_setup.flFovViewmodel;

            pViewRender->view_setup.flFovViewmodel = scoped ? backup_viewmodel_fov : cfg_get(float, g_cfg.view_model_fov);

            auto handle = viewmodel->m_hViewModel();

            if (handle.valid()) {

                C_CSGOViewModel* vi = (sdk::m_game->pGameEntitySystem->Get<C_CSGOViewModel>(handle.index()));

                if (vi) {
                    if (!backup_viewmodel.IsValid())
                        backup_viewmodel = Vector_t(pViewRender->view_setup.angView.x, pViewRender->view_setup.angView.y, pViewRender->view_setup.angView.z);

                    if (!backup_viewmodel_origin.IsValid())
                        backup_viewmodel_origin = pViewRender->view_setup.vecOrigin;

                    Vector_t vmorigin = scoped ? backup_viewmodel_origin : pViewRender->view_setup.vecOrigin;
                    Vector_t vmangle = scoped ? backup_viewmodel : Vector_t(pViewRender->view_setup.angView.x, pViewRender->view_setup.angView.y, pViewRender->view_setup.angView.z);

                    Vector_t forward{}, right{}, up{};
                    math::angle_vectors(vmangle, &forward, &right, &up);

                    float z_ = cfg_get(float, g_cfg.view_model_z);
                    float y_ = cfg_get(float, g_cfg.view_model_y);
                    float x_ = cfg_get(float, g_cfg.view_model_x);

                    vmorigin += (up * z_) + (forward * y_) + (right * -x_);
                    vi->set_viewmodel_origin(vmorigin);
                }
            }
        }
    }

    if (pViewRender) {

        if (cfg_get(bool, g_cfg.aspect_ratio))
        {
            pViewRender->view_setup.flAspectRatio = cfg_get(float, g_cfg.aspect_ratio_value);
            pViewRender->view_setup.nSomeFlags |= 2;
        }
        else {
            pViewRender->view_setup.nSomeFlags &= ~2;
        }
    }

}


void __fastcall hooks::hkViewModel(std::uintptr_t a1, std::uintptr_t a2, Vector_t view_model) {
    auto og = hooks::m_view_model.GetOriginal();
    if (!initialized)
        return og(a1, a2, view_model);

    if (g::m_unloading)
        return  og(a1, a2, view_model);

    if (!sdk::m_engine->is_valid())
        return  og(a1, a2, view_model);

    auto local_controller = CCSPlayerController::Get();
    if (!local_controller)
        return  og(a1, a2, view_model);

    auto local = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(local_controller->PawnHandle());
    if (!local)
        return  og(a1, a2, view_model);
    if (view_model.IsValid()) {

        // remove recoil visualisation
        if (cfg_get(unsigned int, g_cfg.game_removals) & Removals_t::visual_recoil) {
            view_model.x = g::angle.x;
            view_model.y = g::angle.y;
        }
    }

    return og(a1, a2, view_model);
}
void __fastcall hooks::hkCalculateViewBob(std::uintptr_t* a1, std::uintptr_t* a2, float* a3, float* a4) {
    auto og = hooks::m_calc_view_bob.GetOriginal();
    if (!initialized)
        return og(a1, a2, a3, a4);

    if (g::m_unloading)
        return og(a1, a2, a3, a4);

    if (!sdk::m_engine->is_valid())
        return og(a1, a2, a3, a4);

    auto local_controller = CCSPlayerController::Get();
    if (!local_controller)
        return og(a1, a2, a3, a4);

    auto local = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(local_controller->PawnHandle());
    if (!local)
        return og(a1, a2, a3, a4);

    if (cfg_get(unsigned int, g_cfg.game_removals) & Removals_t::view_bob)
        return;

    return og(a1, a2, a3, a4);

}
ViewMatrix_t* __fastcall hooks::hkViewMatrix(CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels)
{    // Call the original function
    const auto original = m_view_matrix.GetOriginal();
    if (!initialized)
        return  original(pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return  original(pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels);

    if (!sdk::m_engine->is_valid())
        return  original(pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels);

    ViewMatrix_t* view_mat = original(pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels);
    g::m_view_matrix = *pOutWorldToProjection;
    g::camera_position = pViewRender->view_setup.vecOrigin;

    
    return view_mat;
}
// Alignment: 4
          // Members count: 13
enum class ObjectTypeFlags_t : uint32_t {
    OBJECT_TYPE_NONE = 0x0,
    OBJECT_TYPE_MODEL = 0x8,
    OBJECT_TYPE_BLOCK_LIGHT = 0x10,
    OBJECT_TYPE_NO_SHADOWS = 0x20,
    OBJECT_TYPE_WORLDSPACE_TEXURE_BLEND = 0x40,
    OBJECT_TYPE_DISABLED_IN_LOW_QUALITY = 0x80,
    OBJECT_TYPE_NO_SUN_SHADOWS = 0x100,
    OBJECT_TYPE_RENDER_WITH_DYNAMIC = 0x200,
    OBJECT_TYPE_RENDER_TO_CUBEMAPS = 0x400,
    OBJECT_TYPE_MODEL_HAS_LODS = 0x800,
    OBJECT_TYPE_OVERLAY = 0x2000,
    OBJECT_TYPE_PRECOMPUTED_VISMEMBERS = 0x4000,
    OBJECT_TYPE_STATIC_CUBE_MAP = 0x8000
};

namespace AggregateSceneObject_t {
    constexpr std::ptrdiff_t m_allFlags = 0x0; // ObjectTypeFlags_t
    constexpr std::ptrdiff_t m_anyFlags = 0x4; // ObjectTypeFlags_t
    constexpr std::ptrdiff_t m_nLayer = 0x8; // int16
    constexpr std::ptrdiff_t m_aggregateMeshes = 0x10; // CUtlVector<AggregateMeshInfo_t>
    constexpr std::ptrdiff_t m_lodSetups = 0x28; // CUtlVector<AggregateLODSetup_t>
    constexpr std::ptrdiff_t m_visClusterMembership = 0x40; // CUtlVector<uint16>
    constexpr std::ptrdiff_t m_fragmentTransforms = 0x58; // CUtlVector<matrix3x4_t>
    constexpr std::ptrdiff_t m_renderableModel = 0x70; // CStrongHandle<InfoForResourceTypeCModel>
}
void __fastcall hooks::AggreegateSceneObject(std::uintptr_t* a1, std::uintptr_t* a2, std::uintptr_t* a3, int a4, std::uintptr_t* a5, std::uintptr_t* a6, std::uintptr_t* a7, std::uintptr_t* a8)
{
    auto og = hooks::m_aggregate_scene_object.GetOriginal();

    L_PRINT(LOG_INFO) << "a1: " << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(a1);
    L_PRINT(LOG_INFO) << "a2: " << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(a2);
    L_PRINT(LOG_INFO) << "a3: " << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(a3);

    og(a1, a2, a3, a4, a5, a6, a7, a8);
    
    Color_t color = cfg_get(ColorPickerVar_t, g_cfg.sky_col).colValue;
    sdk::m_material->SetColor(a3, color);

    return og(a1, a2, a3, a4, a5, a6, a7, a8);
}
 bool just_died = false;
#include <d3d11.h>
#include <wrl/client.h>

 void set_affinity()
 {
     HWND gameWindow = FindWindowA("Valve001", NULL);
     if (!gameWindow)
         return;
     

    SetWindowDisplayAffinity(gameWindow, WDA_EXCLUDEFROMCAPTURE);
   
    
 }
 void reset_affinity()
 {
     HWND gameWindow = FindWindowA("Valve001", NULL);
     if (!gameWindow)
         return;


     SetWindowDisplayAffinity(gameWindow, WDA_NONE);


 }
#include <D3DX11tex.h>
#include <d3d11.h>
#pragma comment(lib, "D3DX11.lib")
#include "..\..\utils\render\ui\cope_bg.h"

 ID3D11ShaderResourceView* cope_bg_image = nullptr;
 D3DX11_IMAGE_LOAD_INFO info2; ID3DX11ThreadPump* pump2{ nullptr };

 HRESULT __stdcall hooks::hkPresent(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags)
 {
     // get original function
     const auto og = m_present.GetOriginal();

     if (!sdk::m_swap_chain->m_swap_chain)
         return og(swap_chain, sync_interval, flags);

     // check if our swap chain and game swap chain diffear and log it
     if (swap_chain != sdk::m_swap_chain->m_swap_chain)
         CS2_LOG(xorstr_("swap chain pointers arent the same. they should be the same...\n"));

     // unload may fuck us up, dont run the hook
     if (g::m_unloading){
      //   pump2->Release();
    //     cope_bg_image = nullptr;
         return  og(swap_chain, sync_interval, flags);
    }

     // check if the caller is OBS or related services
     if (cfg_get(bool, g_cfg.obs_proof))
         set_affinity();
     else
         reset_affinity();


     // recreate it if it's not valid
     if (sdk::m_render_target_view == nullptr)
         render::CreateRenderTarget();


     if (sdk::m_render_target_view)
         sdk::m_device_context->OMSetRenderTargets(1, &sdk::m_render_target_view, nullptr);

     // init render
     if (!render::m_initialized) {
         //blur::InitResources(sdk::m_device);
         if (!render::Init(g::m_hwnd, sdk::m_device, sdk::m_device_context))
         {
             // tell the user and the developer we failed.
             CS2_LOG(xorstr_("Failed to initialize render.\n"));
             return  og(swap_chain, sync_interval, flags);
         }
     }

     // check if we are initialized
     if (!render::m_initialized)
         return og(swap_chain, sync_interval, flags);


     // create a new frame
     ImGui_ImplDX11_NewFrame();
     ImGui_ImplWin32_NewFrame();

     // create the fram
     ImGui::NewFrame();


         visuals::visuals->proccess();

         // render data
         menu::Render();
         // set our render target
      //   blur::initialize(ImGui::GetBackgroundDrawList(), sdk::m_device_context);



    // render the queued frame
    ImGui::Render();


    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    // let the game render now
    return og(swap_chain, sync_interval, flags);
}

HRESULT __fastcall hooks::hkResizeBuffers( IDXGISwapChain* swap_chain, std::uint32_t count, std::uint32_t width, std::uint32_t height, DXGI_FORMAT new_format, std::uint32_t flags )
{
    // get original function
    const auto og = m_resize_buffers.GetOriginal( );

    // unload may fuck us up, dont run the hook
    if ( g::m_unloading )
        return og( swap_chain, count, width, height, new_format, flags );

    L_PRINT(LOG_WARNING) << "[DirectX11 CRenderSystem] resized buffers, destroying render target";

    render::DestroyRenderTarget();

    // return og value
    return og(swap_chain, count, width, height, new_format, flags);
}

HRESULT __stdcall hooks::hkCreateSwapChain( IDXGIFactory* factory, IUnknown* unknown_device, DXGI_SWAP_CHAIN_DESC* desc_swap_chain, IDXGISwapChain** swap_chain )
{
    // get original function
    const auto og = m_create_swap_chain.GetOriginal( );


    // unload may fuck us up, dont run the hook
    if ( g::m_unloading )
        return  og(factory, unknown_device, desc_swap_chain, swap_chain);

    // destroy render target
    render::DestroyRenderTarget( );

    // let the game create its new swap chain
    return og(factory, unknown_device, desc_swap_chain, swap_chain);

}


void __fastcall hooks::hkGameEvent(void* rcx, IGameEvent* const ev)
{
    const auto original = m_game_event.GetOriginal();
    if (!initialized)
        return original(rcx, ev);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return original(rcx, ev);

    if (!sdk::m_engine->is_valid())
        return 	original(rcx, ev);

    auto local_controller = CCSPlayerController::Get();
    if (!local_controller)
        return 	original(rcx, ev);

    auto local = hooks::m_get_local_player(0);
    if (!local)
        return 	original(rcx, ev);



    switch (const char* event_name{ ev->GetName() }; hash_32_fnv1a_const(event_name)) {
    case hash_32_fnv1a_const(("player_hurt")): {
        auto controller = CCSPlayerController::Get();
        if (!controller)
            break;

        const auto event_controller = ev->get_player_controller("userid");
        if (!event_controller)
            break;

        const auto event_attacker = ev->get_player_controller("attacker");
        if (!event_attacker)
            break;

        auto local_idx = controller->m_pEntityIdentity()->index();
        auto pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(event_controller->PawnHandle());
        if (!pawn)
            break;
        // we did damage to some entity
        if (event_attacker->handle().index() == controller->handle().index()) {

            if (cfg_get(bool, g_cfg.hit_sound))
                hitsound::sound->run(); 

            hitsound::hitmarker->on_hit(Vector_t(0, 0, 0), 0);
        }
    } break;
    case hash_32_fnv1a_const(("player_death")): {
        auto controller = CCSPlayerController::Get();
        if (!controller)
            break;

        const auto event_controller = ev->get_player_controller("userid");
        if (!event_controller)
            break;

        const auto event_attacker = ev->get_player_controller("attacker");
        if (!event_attacker)
            break;

        // skinchanger ghetto fixes
        auto local_idx = controller->m_pEntityIdentity()->index();

        if (event_controller->m_pEntityIdentity()->index() == local_idx) {
            just_died = true;
            legitbot::events->on_event(ev, legitbot::e_game_events::death);
        }
        else  if (event_attacker->m_pEntityIdentity()->index() == local_idx) {
            legitbot::events->on_event(ev, legitbot::e_game_events::kill);
        }

    } break;
    case hash_32_fnv1a_const(("round_start")): {
        

        auto controller = CCSPlayerController::Get();
        if (!controller || !controller->m_bPawnIsAlive())
            break;

        world::c4->clear();
        // fix skins not showing up on round start event
        update = true;
        // fix gloves after round
        update_gloves = true;
        // fix knifes after round
        update_knife = true;

        hitsound::hitmarker->reset();

        // reset some shit related to legitbot
        legitbot::events->on_event(ev, legitbot::e_game_events::round_start);
    } break;
    case hash_32_fnv1a_const(("round_end")): {

        auto controller = CCSPlayerController::Get();
        if (!controller || !controller->m_bPawnIsAlive())
            break;

        world::c4->clear();

    } break;
    default:
        break;
    }

    original(rcx, ev);
}

struct grenade_positions
{
    int size;
    char pad[0x4];
    Vector_t* data;
    std::uint64_t unk;
};

class c_grenade_trace
{
public:
    char pad[0xD60];
    std::uint64_t m_size;
    grenade_positions* m_data; // 0xD60
public:
    float duration() {
        return *(float*)(std::uintptr_t(this) + 0xD40);
    }
};
float last_update_time = 0.0f;
const float kExpirationTime = 3.0f; // 3 seconds expiration time
//40 57 48 83 EC 40 48 8B F9 BA
Vector_t direction_grenade[2];
void __fastcall hooks::hkGrenadeTrace(C_BaseCSGrenade* rcx)
{
    const auto og = hooks::m_grenade_trace.GetOriginal();

    if (!initialized || g::m_unloading || !cfg_get(bool, g_cfg.grenade_prediction))
        return og(rcx);

    if (!sdk::m_engine->is_valid() || !rcx)
        return og(rcx);

    auto base_entity = (CBaseEntity*)rcx;
    if (!base_entity)
        return og(rcx);

    if (!base_entity->handle().valid() || !base_entity->m_hOwnerEntity().valid())
        return og(rcx);

    auto owner = sdk::m_game->pGameEntitySystem->Get(base_entity->m_hOwnerEntity());
    if (!owner)
        return og(rcx);

    // store last lines to create ge trace 
    if (rcx->m_bRedraw())
    {
        if (!g::lines.empty())
            g::lines.clear();

        if (!g::lines_3d.empty())
            g::lines_3d.clear();

        return og(rcx);
    }
  
    int type = rcx->get_type();

    c_grenade_trace* grenade_trace = hooks::m_init_grenade_trace(type, rcx);
    if (!grenade_trace)
        return og(rcx);

    if (!g::lines.empty())
        g::lines.clear();

    if (!g::lines_3d.empty())
        g::lines_3d.clear();

    hooks::m_grenade_direction(rcx, owner, direction_grenade[0], direction_grenade[1], false);

    hooks::m_init_grenade_prediction(grenade_trace, direction_grenade[0], direction_grenade[1]);



    for (int i = 0; i < grenade_trace->m_size; i++) {
        auto data = grenade_trace->m_data[i];
        if (!data.size)
            continue;

        if (!data.data)
            continue;

        for (int j{}; j < data.size; j++)
        {
            ImVec2 screen{};
            
            if (!math::w2s(data.data[j], screen))
                continue;

            g::lines_3d.push_back(data.data[j]);
            g::lines.push_back(screen);
        }
    }

    // utl_remove_entity
    hooks::m_grenade_bomb_direction(grenade_trace);

    return og(rcx);
}


long hooks::HkWndProc( HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam )
{
    // unload may fuck us up, dont run the hook
    if ( g::m_unloading )
       return ::CallWindowProcW( m_old_wndproc, hwnd, umsg, wparam, lparam );

    // handle render
    if ( render::OnWndProc( hwnd, umsg, wparam, lparam ) )
        return 1L;

    // run game's code
    return ::CallWindowProcW( m_old_wndproc, hwnd, umsg, wparam, lparam );
}

Vector_t CalculateCameraPosition(Vector_t anchorPos, float distance, QAngle_t viewAngles)
{
    float yaw = viewAngles.y * (math::_PI / 180.0f);
    float pitch = viewAngles.x * (math::_PI / 180.0f);

    float x = anchorPos.x + distance * cosf(yaw) * cosf(pitch);
    float y = anchorPos.y + distance * sinf(yaw) * cosf(pitch);
    float z = anchorPos.z + distance * sinf(pitch);

    return Vector_t{ x, y, z };
}
QAngle_t CalcAngle(Vector_t viewPos, Vector_t aimPos)
{
    QAngle_t angle = { 0, 0, 0 };

    Vector_t delta = aimPos - viewPos;

    angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
    angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

    return angle;
}
Vector_t NormalizeAngles(QAngle_t angles)
{
    while (angles.x > 89.0f)
        angles.x -= 180.0f;
    while (angles.x < -89.0f)
        angles.x += 180.0f;
    while (angles.y > 180.0f)
        angles.y -= 360.0f;
    while (angles.y < -180.0f)
        angles.y += 360.0f;
    angles.z = 0.0f;
    return Vector_t(angles.x, angles.y, angles.z);
}

void __fastcall hooks::hkOverrideView(void* pClientModeCSNormal, CViewSetup* pSetup)
{
    const auto oOverrideView = m_override_view.GetOriginal();

    if (!sdk::m_engine->is_valid())
        return oOverrideView(pClientModeCSNormal, pSetup);

    if (!initialized)
        return oOverrideView(pClientModeCSNormal, pSetup);
    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return oOverrideView(pClientModeCSNormal, pSetup);

    if (!sdk::m_csgo_input)
        return oOverrideView(pClientModeCSNormal, pSetup);

    auto pawn = hooks::m_get_local_player(0);
    if (!pawn)
        return oOverrideView(pClientModeCSNormal, pSetup);

    if (!pawn->IsAlive() || !cfg_get(bool, g_cfg.thirdperson))
        return oOverrideView(pClientModeCSNormal, pSetup);

    auto input = sdk::m_csgo_input;

    if (Input::GetBindState(cfg_get(KeyBind_t, g_cfg.third_person_key)) && pawn && input) {
        QAngle_t adjusted_cam_view_angle = g::command__number_angle;//QAngle_t(input->vecViewAngle.x, input->vecViewAngle.y, input->vecViewAngle.z);
        adjusted_cam_view_angle.x = -adjusted_cam_view_angle.x;
        auto eyepos = pawn->GetEyePosition();

        pSetup->vecOrigin = CalculateCameraPosition(pawn->GetEyePosition(), -cfg_get(int, g_cfg.thirdperson_fov), adjusted_cam_view_angle);

        trace_filter_t filter = {};
        sdk::m_trace->Init(filter, pawn, 0x1C3003, 0, 7);
        ray_t ray{};
        game_trace_t trace{};

        Vector_t direction = (eyepos - pSetup->vecOrigin).normalized();

        sdk::m_trace->TraceShape(ray, &eyepos, &pSetup->vecOrigin, filter, trace);

        if (trace.HitEntity != nullptr) {
            pSetup->vecOrigin = trace.m_pos + (direction * 10.f);
        }

        Vector_t p = NormalizeAngles(CalcAngle(pSetup->vecOrigin, pawn->GetEyePosition()));
        pSetup->angView = QAngle_t{ p.x, p.y };
    }

    oOverrideView(pClientModeCSNormal, pSetup);

    g::view_origin = pSetup->vecOrigin;
}

void hooks::hkCamera(CCSGOInput* pCSGOInput, int a2)
{
    const auto Original = m_camera_input.GetOriginal();
    if (!initialized)
        return Original(pCSGOInput, a2);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return Original(pCSGOInput, a2);

    if (!sdk::m_engine->is_valid() || !pCSGOInput)
        return Original(pCSGOInput, a2);

    auto controller = CCSPlayerController::Get();
    if (!controller)
        return Original(pCSGOInput, a2);
    if (!controller->m_bPawnIsAlive())
        return Original(pCSGOInput, a2);

    CUserCmd* pCmd = CUserCmd::Get();
    if (!pCmd)
        return Original(pCSGOInput, a2);

    if (!pCmd->pBaseCmd->view_angles)
        return Original(pCSGOInput, a2);

    auto m_backup_input_angle = pCSGOInput->get_view_angles(); 
    auto m_backup_command_angle = pCmd->pBaseCmd->view_angles->angValue;
    g::m_original_camera_view_angles = m_backup_input_angle;

    if (command::m_legit_bot)
        return    Original(pCSGOInput, a2);

    Original(pCSGOInput, a2);

    pCSGOInput->set_input_angle(m_backup_input_angle);
}
bool __fastcall hooks::hkMouseInputEnabled( void* rcx )
{
    // get original function
    const auto og = m_mouse_input_enabled.GetOriginal( );

    // unload may fuck us up, dont run the hook
    if ( g::m_unloading )
        return og( rcx );
    
    return menu::m_opened ? false : og(rcx);
}

float __fastcall hooks::hkOverrideFOV( CCSPlayer_CameraServices* rcx )
{
    // get original function
    const auto og = m_override_fov.GetOriginal( );
    if (!initialized)
        return og(rcx);
    // unload may fuck us up, dont run the hook
    if ( g::m_unloading )
        return og( rcx );

    if (!sdk::m_engine->is_valid())
        return og(rcx);
    // if user wants to override game fov, replace this function with ours
    if ( cfg::visuals::override_fov )
        return camera::override_fov( og( rcx ) );

    // return original value
    return og( rcx );
}
enum c_stages : std::uint32_t
{
    frame_handler_start = 0,
    frame_handler_end = 1,
    frame_prediction = 3,
    frame_net_full_frame_update_on_remove = 2,
    frame_net_update_start = 4,
    frame_net_update_postdataupdate_start = 5,
    frame_net_update_postdataupdate_end = 6,
    frame_net_update_end = 7,
    frame_start = 8,
    frame_render = 9
};
void m_update_pvs() {
  
}

void __fastcall hooks::hkSetMeshGroupCopy(void* rcx, void* rdx, void* r8)
{
    const auto og = m_set_mesh_group_copy.GetOriginal();

    if (!initialized)
        return   og(rcx, rdx, r8);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return  og(rcx, rdx, r8);

    auto local_player = hooks::m_get_local_player(0);
    if (!local_player)
        return  og(rcx, rdx, r8);

    if (!local_player->IsAlive())
        return  og(rcx, rdx, r8);

    if (!local_player->GetActiveWeapon())
        return  og(rcx, rdx, r8);

    if (!local_player->GetActiveWeapon()->GetWeaponData())
        return  og(rcx, rdx, r8);

    if (local_player->GetActiveWeapon()->GetWeaponData()->m_WeaponType() == WEAPONTYPE_KNIFE)
        return  og(rcx, rdx, r8);

    auto view_model_services = local_player->m_pViewModelServices();
    if (!view_model_services)
        return  og(rcx, rdx, r8);

    if (!view_model_services->m_hViewModel().valid())
        return  og(rcx, rdx, r8);

    auto view_model = sdk::m_game->pGameEntitySystem->Get<C_CSGOViewModel>(view_model_services->m_hViewModel());
    if (!view_model)
        return  og(rcx, rdx, r8);

    auto scene_node = view_model->m_pGameSceneNode();
    if (!scene_node)
        return  og(rcx, rdx, r8);

    auto m_skeleton = scene_node->m_pSkeleton();
    if (!m_skeleton)
        return  og(rcx, rdx, r8);

    auto model = &m_skeleton->GetModel();
    if (!model)
        return  og(rcx, rdx, r8);

    void* offsetToMask = &model->m_MeshGroupMask();
    if (offsetToMask != rdx)
        return  og(rcx, rdx, r8);
    
    //  Items::skins->on_weapon(a2);
    Items::skins->on_weapon(rdx);

    return og(rcx, rdx, r8);
}
void __fastcall hooks::hkFrameStageNotify(void* a1, int stage)
{
    // get original function
    const auto og = m_frame_stage_notify.GetOriginal();

    if (!initialized)
        return  m_frame_stage_notify.GetOriginal()(a1, stage);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return  m_frame_stage_notify.GetOriginal()(a1, stage);

    //sdk::m_pvs->set(false);
    // call og 
    m_frame_stage_notify.GetOriginal()(a1, stage);

    //sdk::m_pvs->set(false);


    g::interval = 0.015625f;

    Items::skins->on_knife(stage);

    switch (stage) {

    case frame_net_update_postdataupdate_end:   
        // gloves
        Items::skins->fsn(stage);
        break;
    case frame_net_update_end:
#ifdef _DEBUG

            ragebot::data->store_records();
#endif
        legitbot::data->store_records();
        //animations::data->on_frame_stage();
        break;
    case frame_render:
        world::esp->run();
        world::c4->cache();
        visuals::visuals->cache();
       // visuals::visuals->cache_weapons();
       // visuals::visuals->cache_players();
        break;
    default:
        break;
    }


}
/*
void __fastcall input_history(CNetInputMessage* input_message, CCSGOInputHistory* history, bool has_attack, std::uint64_t a4, std::uint64_t a5, std::uint64_t a6)
{
    if (!g::m_local_player_pawn || !g::m_local_player_pawn->IsAlive())
        return  original_input_history(input_message, history, has_attack, a4, a5, a6);


    if (has_attack)
        original_input_history(input_message, history, has_attack, a4, a5, a6);

    return original_irapidfirenput_history(input_message, history, true, a4, a5, a6);
}*/
int cmd_random_seed = 0;

void* __fastcall hooks::hkUpdateSubClass(void* a1, void* a2, void* a3) {
    const auto original = m_update_sub_class.GetOriginal();
    if (!initialized)
        return original(a1, a2, a3);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return original(a1, a2, a3);

    if (sdk::m_engine->is_valid()) {
        const char* modelName = *reinterpret_cast<const char**>((uintptr_t)a1 + 0x8);
        if (modelName == nullptr)
            return original(a1, a2, a3);

        if (std::strstr(modelName, xorstr_("weapons/models/knife/")) != nullptr) {
            *reinterpret_cast<void**>((uintptr_t)a3 + 0x30) = nullptr; // networked vars sets to nullptr, this is the knife fix
        }
    }

    return original(a1, a2, a3);
}


void __fastcall hooks::hkRunCommand(CCSPlayer_MovementServices* a1, CUserCmd* cmd) {
    const auto og = m_run_command.GetOriginal();
    if (!initialized)
        return og(a1, cmd);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return og(a1, cmd);


    if (!sdk::m_engine->is_valid())
        return og(a1, cmd);

    if (!cmd || !a1 || !sdk::m_global_vars)
        return og(a1, cmd);

    auto base = cmd->pBaseCmd;
    if (!base)
        return og(a1, cmd);

    auto controller = CCSPlayerController::Get();
    if (!controller)
        return og(a1, cmd);

    auto pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(controller->PawnHandle());
    if (pawn)
        return og(a1, cmd);

    if (pawn->m_iHealth() <= 0)
        return og(a1, cmd);

    int v16 = 0;
    int new_tick = 0;
    int ntickcount = 0;
    auto tick_base = controller->m_nTickBase();

    //SetPredictionCommand 
    a1->set_prediction_command(cmd);

    if (!sdk::m_global_vars->m_frame_time != 0.f) {
        new_tick = controller->m_nTickBase();
        controller->set_player_tickbase(new_tick + 1);
    }
    
    if (controller) {
        v16 = controller->m_nTickBase();
        ntickcount = v16;
    }

    v16 = ntickcount * g::interval;

    const auto v23 = sdk::m_global_vars->m_tickcount;
    const auto m_player_time = sdk::m_global_vars->m_curtime2;
    const auto  m_frame_time = sdk::m_global_vars->m_frame_time;
   // const auto m_tick_fraction = sdk::m_global_vars->m_tick_fraction;

    sdk::m_global_vars->m_curtime2 = v16;
    sdk::m_global_vars->m_tickcount = ntickcount;
    sdk::m_global_vars->m_curtime = v16;
    sdk::m_global_vars->m_frame_time = 0.f;
   // sdk::m_global_vars->m_tick_fraction = 0.f;

    // run PhysicsThink on entity
    controller->run_physics_think();

    // proccess current command number 
    a1->process_movement(cmd);

    //ResetPredictionCommand
    a1->reset_prediction_command();

    sdk::m_global_vars->m_curtime2 = m_player_time;
    sdk::m_global_vars->m_frame_time = m_frame_time;
//    sdk::m_global_vars->m_tick_fraction = m_tick_fraction;
    sdk::m_global_vars->m_tickcount = v23;

    return og(a1, cmd);

}
int random_seed = 0;

void __fastcall hooks::set_input_history(CCSInputMessage* input_message, CCSGOInputHistoryEntryPB* input_history, const bool verify, void* a4, void* a5, CBaseEntity* local_pawn) {
    const auto original = m_net_input_history.GetOriginal();

    if (!initialized)
        return original(input_message, input_history, verify, a4, a5, local_pawn);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return original(input_message, input_history, verify, a4, a5, local_pawn);

    auto local_player_pawn = hooks::m_get_local_player(0);

    if (!sdk::m_engine->is_valid() || !local_player_pawn)
        return original(input_message, input_history, verify, a4, a5, local_pawn);

    bool has_been_called = command::m_rage_aim_target || command::m_aim_target;
    bool invalid = !has_been_called || !input_message || !verify ;
    if (invalid) {
        original(input_message, input_history, verify, a4, a5, local_pawn);
        return original(input_message, input_history, verify, a4, a5, local_pawn);
    }

    // run pre backtrack and set inputMessage view angles if should 
    record::data->pre(input_message, input_history, command::m_tick_count, 0.f, false, command::m_best_point.IsValid());

    original(input_message, input_history, verify, a4, a5, local_pawn);

    // run post backtrack and set inputHistory view angles if should 
    record::data->post(input_message, input_history, command::m_tick_count, 0.f, false, command::m_best_point.IsValid());

    return original(input_message, input_history, verify, a4, a5, local_pawn);
}

bool __fastcall hooks::hkPreCreateMove(CCSGOInput* a1, int slot, CUserCmd* a4)
{
    return true;
}
int prev_attack = 0;
static int last_trigger_shot_time = 0;
static int shot_time = 0;

// runcommand
int __fastcall hooks::hkProcessCRCcommands(CBaseUserCmdPB* base_cmd) {
    static auto og = m_crc_commands.GetOriginal();
    if (!og)
        og = m_crc_commands.GetOriginal();

    if (!initialized)
        return og(base_cmd);
    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return og(base_cmd);

    if (!sdk::m_engine->is_valid())
        return og(base_cmd);

    if (_ReturnAddress() != m_compute_size_cmd_crc_ret_address)
        return og(base_cmd);



    return og(base_cmd);
}

bool rank = false;
enum MedalRank_t : uint32_t
{
    MEDAL_RANK_NONE = 0,
    MEDAL_RANK_BRONZE = 1,
    MEDAL_RANK_SILVER = 2,
    MEDAL_RANK_GOLD = 3,
    MEDAL_RANK_COUNT = 4,
};
class CCSPlayerController_InventoryServices {
public:
    pad(0x5C);
    int32_t m_nPersonaDataPublicLevel = 0x5C; // int32
    int32_t m_nPersonaDataPublicCommendsLeader = 0x60; // int32
    int32_t m_nPersonaDataPublicCommendsTeacher = 0x64; // int32
    int32_t m_nPersonaDataPublicCommendsFriendly = 0x68; // int32
    int32_t m_nPersonaDataXpTrailLevel = 0x6C; // int32
    int32_t m_vecServerAuthoritativeWeaponSlots = 0x70; // C_UtlVectorEmbeddedNetworkVar<ServerAuthoritativeWeaponSlot_t>
};
void fix_command(CUserCmd* user_cmd,CBaseUserCmdPB* base, Vector_t angle) {


    if (!base)
        return;

    if (!base->view_angles)
        return;

    Vector_t wish_angle{base->view_angles->angValue.x,base->view_angles->angValue.y, base->view_angles->angValue.z };
    int sign = wish_angle.x > 89.f ? -1.f : 1.f;
    wish_angle.clamp();

    Vector_t forward, right, up, old_forward, old_right, old_up;
    Vector_t view_angles = angle;

    math::angle_vectors(wish_angle, forward, right, up);

    forward.z = right.z = up.x = up.y = 0.f;

    forward.NormalizeInPlace();
    right.NormalizeInPlace();
    up.NormalizeInPlace();

    math::angle_vectors(view_angles, old_forward, old_right, old_up);

    old_forward.z = old_right.z = old_up.x = old_up.y = 0.f;

    old_forward.NormalizeInPlace();
    old_right.NormalizeInPlace();
    old_up.NormalizeInPlace();

    forward *= base->flForwardMove;
    right *= base->flSideMove;
    up *= base->flUpMove;

    float fixed_forward_move = old_forward.Dot(right) + old_forward.Dot(forward) + old_forward.Dot(up, true);

    float fixed_side_move = old_right.Dot(right) + old_right.Dot(forward) + old_right.Dot(up, true);

    float fixed_up_move = old_up.Dot(right, true) + old_up.Dot(forward, true) + old_up.Dot(up);
  
    base->bit(BASE_BITS_FORWARDMOVE);
    base->flForwardMove=(fixed_forward_move);
    base->bit(BASE_BITS_LEFTMOVE);
    base->flSideMove=(fixed_side_move);
    base->bit(BASE_BITS_UPMOVE);
    base->flUpMove=(fixed_up_move);

    fixed_forward_move = sign * (old_forward.Dot(right) + old_forward.Dot(forward));
    fixed_side_move = old_right.Dot(right) + old_right.Dot(forward);

    base->bit(BASE_BITS_FORWARDMOVE);
    base->flForwardMove = (std::clamp(fixed_forward_move, -1.f, 1.f));
    base->bit(BASE_BITS_LEFTMOVE);
    base->flSideMove = (std::clamp(fixed_side_move, -1.f, 1.f));
}

bool __fastcall hooks::hkEntityWantsInterpolation(CBaseEntity* ent, int unk) {

    static auto original = m_interpolate_on_entity.GetOriginal();
    if (!initialized)
        return original(ent, unk);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading || !sdk::m_game_rules || !sdk::m_engine || !sdk::m_global_vars)
        return original(ent, unk);


    auto player = hooks::m_get_local_player(0);

    bool invlaid = !player;

    // only force interp on enemy
    if (ent && !invlaid) {

        if (ent->IsBasePlayer() && ent->m_iHealth() > 0 && ent->get_entity_by_handle() != player->get_entity_by_handle())
        {
            return true;
        }
        
    }

    return original(ent, unk);

}

void __fastcall hooks::hkAddCachedEntity(void* game_entity_interface, CEntityInstance* p_instance, CBaseHandle handle) {
    const auto original = hooks::m_add_entity.GetOriginal();
    if (!initialized)
        return original(game_entity_interface, p_instance, handle);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return original(game_entity_interface, p_instance, handle);

    if (!sdk::m_engine->is_valid())
        return original(game_entity_interface, p_instance, handle);

    const int entry_index = handle.index();
    if (entry_index <= 1) {
        return original(game_entity_interface, p_instance, handle);
    }

    // Validate entity
    auto* entity = static_cast<CBaseEntity*>(p_instance);
    if (!entity || entry_index > ENT_MAX_NETWORKED_ENTRY || entity->handle().index() != handle.index()) {
        return original(game_entity_interface, p_instance, handle);
    }

    // Get and validate class info
    SchemaClassInfoData_t* class_info = nullptr;
    entity->dump_class_info(&class_info);
    if (!class_info) {
        return original(game_entity_interface, p_instance, handle);
    }

    const uint32_t hashed_name = hash_32_fnv1a_const(class_info->szName);
   /* if (entity->IsWeapon())
    {
        // Call original before modifying our data to maintain proper game state
        original(game_entity_interface, p_instance, handle);

 
        weapon_cache_system::g_weapons_cache->write_entity([&](auto& items) {
            items.emplace_back(weapon_cache_system::cached_entity{ handle, entity, nullptr, 0 });
            });

        _log(LOG_INFO) << "[cache] added Item entity:" << entry_index;
   
        return;
    }
    else */{

        /* store molotovs */
        if (hashed_name == hash_32_fnv1a_const("CCSPlayerController")) {
            // Call original before modifying our data to maintain proper game state
            original(game_entity_interface, p_instance, handle);

            auto* controller = static_cast<CCSPlayerController*>(entity);

            auto* pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(controller->PawnHandle());

            // Use write_players to safely modify the cache
            player_cache_system::g_entity_cache->write_players([&](auto& players) {
                players.emplace_back(player_cache_system::cached_entity{ entity->handle(), controller, pawn });
                });

            _log(LOG_INFO) << "[cache] added entity:" << entity->handle().index();
            return;
        }
    }

    original(game_entity_interface, p_instance, handle);
}

void __fastcall hooks::hkRemoveCachedEntity(void* game_entity_interface, CEntityInstance* p_instance, CBaseHandle handle) {
    const auto original = hooks::m_add_entity.GetOriginal();
    if (!initialized)
        return original(game_entity_interface, p_instance, handle);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return original(game_entity_interface, p_instance, handle);

    if (!sdk::m_engine->is_valid())
        return original(game_entity_interface, p_instance, handle);

    const int entry_index = handle.index();
    if (entry_index <= 1) {
        return original(game_entity_interface, p_instance, handle);
    }

    // Validate entity
    auto* entity = static_cast<CBaseEntity*>(p_instance);
    if (!entity || entry_index > ENT_MAX_NETWORKED_ENTRY || entity->handle().index() != handle.index()) {
        return original(game_entity_interface, p_instance, handle);
    }

    // Get and validate class info
    SchemaClassInfoData_t* class_info = nullptr;
    entity->dump_class_info(&class_info);
    if (!class_info) {
        return original(game_entity_interface, p_instance, handle);
    }


   const uint32_t hashed_name = hash_32_fnv1a_const(class_info->szName);
   /* if (entity->IsWeapon())
    {
        // Call original before modifying our data to maintain proper game state
        original(game_entity_interface, p_instance, handle);

        auto item = (CCSWeaponBase*)entity;
        auto owner = sdk::m_game->pGameEntitySystem->Get(item->m_hOwnerEntity());

        if (item) {
            if (item->m_iState() != weapon_state::WEAPON_NOT_CARRIED) {

                // Use write_players to safely modify the cache
                weapon_cache_system::g_weapons_cache->write_entity([&](auto& items) {
                    auto it = std::ranges::find_if(items, [&](const weapon_cache_system::cached_entity& e) {
                        return e.handle == handle;
                        });

                    if (it != items.end()) {
                        _log(LOG_INFO) << ("removed Item from cache. Index: {}") << entry_index;
                        items.erase(it);
                    }
                });
            }
        }
    }
    else */{

        if (hashed_name == hash_32_fnv1a_const("CCSPlayerController")) {

            // Use write_players to safely modify the cache
            player_cache_system::g_entity_cache->write_players([&](auto& players) {
                auto it = std::ranges::find_if(players, [&](const player_cache_system::cached_entity& e) {
                    return e.handle.index() == entity->handle().index();
                    });

                if (it != players.end()) {
                    _log(LOG_INFO) << ("Removed entity from cache. Index: ") << entity->handle().index();
                    players.erase(it);
                }
                });
        }
    }

    original(game_entity_interface, p_instance, handle);
}
enum server_type_t : int {
    normal = 0,
    valve_dedicated_server = 1
};
int last_count_bullet = 0;
int server_type = 0;
void __fastcall hooks::hkCreateMove(CCSGOInput* a1, unsigned int slot, char a3)
{

    const auto og = m_create_move.GetOriginal();
    if (!initialized)
        return og(a1, slot, a3);

    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return og(a1, slot, a3);

    static auto original = m_create_move.GetOriginal();
    original(a1, slot, a3);

    g::m_local_player_controller = CCSPlayerController::Get();
    if (!g::m_local_player_controller)
        return;

    g::m_local_player_pawn = hooks::m_get_local_player(0);
    if (!g::m_local_player_pawn)
        return;

    // get user cmd
    auto cmd = CUserCmd::Get();
    if (!cmd)
        return;

    // get base cmd from user cmd
    auto base_cmd = cmd->pBaseCmd;
    if (!base_cmd)
        return;

    if (sdk::m_game_rules) 
        server_type = sdk::m_game_rules->m_bIsValveDS() ? server_type_t::valve_dedicated_server : server_type_t::normal;
    
    g::chams_ready = true;
    g::m_active_weapon = g::m_local_player_pawn && g::m_local_player_pawn->IsAlive() && g::m_local_player_pawn->GetActiveWeapon() != nullptr ? g::m_local_player_pawn->GetActiveWeapon() : nullptr;
    g::m_weapon_data = g::m_active_weapon ? g::m_active_weapon->GetWeaponData() : nullptr;

    g::attack = g::m_local_player_pawn && g::m_local_player_pawn->IsAlive() ? cmd->nButtons.nValue & IN_ATTACK : false;
    g::attack2 = g::m_local_player_pawn && g::m_local_player_pawn->IsAlive() ? cmd->nButtons.nValue & IN_ATTACK2 : false;
    g::eye_pos = g::m_local_player_pawn && g::m_local_player_pawn->IsAlive() ? g::m_local_player_pawn->GetEyePosition() : Vector_t();
    g::on_ground = g::m_local_player_pawn && g::m_local_player_pawn->IsAlive() ? (g::m_local_player_pawn->m_fFlags() & FL_ONGROUND) : false;
    g::tickbase = g::m_local_player_pawn && g::m_local_player_pawn->IsAlive() ? g::m_local_player_controller->m_nTickBase() : 0;
    if (sdk::m_cvar) {
        g::deathmatch = sdk::m_cvar->find(hash_32_fnv1a_const("mp_teammates_are_enemies"))->value.i1;
        g::sv_jump_impulse = sdk::m_cvar->find(hash_32_fnv1a_const("sv_jump_impulse"))->value.fl;

        
        //   g::max_unlag = sdk::m_cvar ? sdk::m_cvar->find(hash_32_fnv1a_const("sv_maxunlag"))->value.fl : 0.2f;
    }
 
    if (g::m_local_player_pawn->IsAlive() && g::m_active_weapon) {
        /* store usefull vars */
        if (!connected)  {
            g::update_radar = false;
            radar::instance->on_map_load(); 
        }

        connected = true;

        /* force full update */
        if (full_update && sdk::m_network) {
        
        }

#ifdef _DEBUG
            // filter subticks 
            if (base_cmd->subtickMovesField.pRep) {
                if (base_cmd->subtickMovesField.nCurrentSize != 0)
                    base_cmd->subtickMovesField.nCurrentSize = 0;
            }     
#else
        if (server_type == normal ) {
            // filter subticks 
            if (base_cmd->subtickMovesField.pRep) {
                if (base_cmd->subtickMovesField.nCurrentSize != 0)
                    base_cmd->subtickMovesField.nCurrentSize = 0;
            }
        }
#endif


        g::ammo = g::m_local_player_pawn->IsAlive() ? g::m_active_weapon->clip1() : 0;
        g::scoped = g::m_local_player_pawn->IsAlive() ? g::m_local_player_pawn->m_bIsScoped() : false;

        // no flash
        if (cfg_get(unsigned int, g_cfg.game_removals) & Removals_t::flash)
            g::m_local_player_pawn->m_flFlashDuration() = 0;

        // update skins
        if (just_died) {
            weapons_update_stlye = 1;
            update_stlye = 1;
        
            update = update_gloves = update_knife = true; just_died = false;
        }
      
          /* save current original view angles */
        if (base_cmd->view_angles) {
            g::m_backup_angle = base_cmd->view_angles->angValue;
            g::command__number_angle = base_cmd->view_angles->angValue;
            g::angle = Vector_t(base_cmd->view_angles->angValue.x, base_cmd->view_angles->angValue.y, base_cmd->view_angles->angValue.z);
        }

        // store last attack tick
        static float m_attack_tick = 0.f;
#ifdef _DEBUG
        /* Initialize sub tick loop */
         if (sdk::m_game_rules) {       
         //    input_history::Store(a1);
       //      input_history::Run(a1, cmd, base_cmd);      
         }
#endif 
        {       

            // run strafe outside of prediction before anti aim 

            if (g::history_on_tick)
                g::history_on_tick = false;

    


            // run engine prediction
            g_prediction->start(a1, cmd);
            {         
#ifdef _DEBUG
                misc::movement->setup_post_prediction(a1, cmd, base_cmd);
                g::shoot_pos = g::m_local_player_pawn->GetEyePosition();
#endif
                misc::movement->setup(a1, cmd, base_cmd);

                legitbot::data->menu_adaptive();
                legitbot::data->proccess(cmd, a1, base_cmd);
           
                /* run legit bot trigger bot here */
                if (legitbot::data->trigger_bot(cmd, a1, base_cmd)) {
                    g::client_trigger_attack = true;
                    if (g::client_trigger_attack && sdk::m_global_vars->m_tickcount - m_attack_tick >= g::trigger_delay && cmd->nAttack1StartHistoryIndex == -1) {
                        /* run lag comp */
                        auto message = a1->Message;
                        if (message) {
                            message->m_nFrameTickCount = command::m_tick_count;
                            message->m_nTargetIndex = command::m_target_ent_index;
                            message->m_nPlayerTickCount = g_prediction->get_local_data()->m_shoot_tick;
                            message->m_vecShootPosition = g_prediction->get_local_data()->m_eye_pos;
                        }

                        _import(mouse_event).safe()(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                        _import(mouse_event).safe()(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    } g::client_trigger_attack = false;
                }
                else { if (!g::trigger_enable) m_attack_tick = 0.f; }

                if (!m_attack_tick)
                    m_attack_tick = sdk::m_global_vars->m_tickcount;

                /* run ragebot */
#ifdef _DEBUG
                ragebot::data->menu_adaptive();
                ragebot::data->proccess(a1, cmd, base_cmd);
#endif           
            }
            g_prediction->end(a1, base_cmd, cmd);


            if (sdk::m_game_rules) {
                // run antiaim after move & pred stuff 
                g_antiaim->run(cmd, base_cmd);
            }
            /* clamp invalid angles & fix movement stuff */
            misc::movement->proccess_data(cmd, base_cmd);

            /* fix current command */
#ifdef _DEBUG
            fix_command(cmd, base_cmd, g::angle);

#else
            if (server_type == normal)
                fix_command(cmd, base_cmd, g::angle);
#endif
         
         }
    }
    else {
        g::ammo = 0;
        g::scoped =  false;

        just_died = true;
    }

        
    // update crc before sending to server our current base command
    crt_hash::store(cmd, base_cmd);
    if (cmd->CRC(base_cmd))
        crt_hash::update(cmd);
 
}

void CS_FASTCALL hooks::hkDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
    const auto dobj = m_draw_object.GetOriginal();

    if (!initialized)
        return dobj(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
    // unload may fuck us up, dont run the hook
    if (g::m_unloading)
        return dobj(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

   return dobj(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}
