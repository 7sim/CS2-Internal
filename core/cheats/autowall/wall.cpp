#include "wall.h"
#include "..\..\utils\utils.h"
#include "..\backtrack\record.h"
#include "..\..\sdk\cache\cache_entity.h"
#include "..\..\sdk\interfaces\engineclient.h"
#include <iostream>
#include <memoryapi.h>
#include "../../sdk/interfaces/trace.h"
#include "..\..\helpers\log.h"
#include <mutex>
#include "..\..\sdk\interfaces\cvar.h"
#include <array>
static constexpr std::uint32_t PENMASK = 0x1C300Bu; // mask_shot_hull | contents_hitbox?

enum ETeamID : int
{
    TEAM_UNK,
    TEAM_SPECTATOR,
    TEAM_TT,
    TEAM_CT
};

namespace autowall {

    void autowall::c_auto_wall::pen(data_t& data, const Vector_t local_pos, const Vector_t target_pos, CBaseEntity* target, CBaseEntity* local, CCSPlayerPawn* localpawn, CCSPlayerPawn* targetpawn,
        CCSWeaponBaseVData* wpn_data, float& dmg, bool& valid) {
        data.m_local = local;
        data.m_target = target;
        data.m_local_pawn = localpawn;
        data.m_target_pawn = targetpawn;
        data.m_wpn_data = wpn_data;
        data.m_pos.at(autowall::c_auto_wall::data_t::e_pos::e_local) = local_pos;
        data.m_pos.at(autowall::c_auto_wall::data_t::e_pos::e_target) = target_pos;
        FireBullet(data, data.m_dmg, data.m_can_hit);

    }

    bool autowall::c_auto_wall::penetrate(data_t& data, const Vector_t local_pos, const Vector_t target_pos, CBaseEntity* target, CBaseEntity* local, CCSPlayerPawn* localpawn, CCSPlayerPawn* targetpawn,
        CCSWeaponBaseVData* wpn_data, float& dmg, bool& valid) {
        data.m_local = local;
        data.m_target = target;
        data.m_local_pawn = localpawn;
        data.m_target_pawn = targetpawn;
        data.m_wpn_data = wpn_data;
        data.m_pos.at(autowall::c_auto_wall::data_t::e_pos::e_local) = local_pos;
        data.m_pos.at(autowall::c_auto_wall::data_t::e_pos::e_target) = target_pos;
        return FireBullet(data, data.m_dmg, data.m_can_hit);

    }

    bool autowall::c_auto_wall::FireBullet(data_t& data, float& dmg, bool& valid)
    {
        //   L_PRINT(LOG_INFO) << "0";
        debug(data.m_local != nullptr || data.m_target != nullptr || data.m_wpn_data != nullptr || data.m_local_pawn != nullptr || data.m_target_pawn != nullptr);
        if (!data.m_local || !data.m_target || !data.m_wpn_data)
            return false;
        //    L_PRINT(LOG_INFO) << "1";

        trace_data_t trace_data = { };
        trace_data.m_arr_pointer = &trace_data.m_arr;
        void* data_pointer = &trace_data;
        //L_PRINT(LOG_INFO) << "created trace_data | trace_data_pointer:" << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(data_pointer);
        //L_PRINT(LOG_INFO) << "created trace_data | pointer:" << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(trace_data.m_arr_pointer);

        const Vector_t direction =
            data.m_pos.at(data_t::e_pos::e_target) - data.m_pos.at(data_t::e_pos::e_local),
            end_pos = direction * data.m_wpn_data->range();

        trace_filter_t filter = {};
        sdk::m_trace->Init(filter, data.m_local_pawn, PENMASK, 3, 7);
        void* filter_pointer = &filter;
        /* L_PRINT(LOG_INFO) << "created filter_data | filter_pointer:" << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(filter_pointer);

         L_PRINT(LOG_INFO) << "creating trace | filter > layer:" << filter.layer << "mask:" << filter.trace_mask;
         L_PRINT(LOG_INFO) << "creating trace | endpos:" << end_pos;
         */
        sdk::m_trace->CreateTrace(&trace_data, data.m_pos.at(data_t::e_pos::e_local), end_pos, filter, 4);

        struct handle_bullet_data_t {
            handle_bullet_data_t(const float dmg_mod, const float pen, const float range_mod, const float range,
                const int pen_count, const bool failed) :
                m_dmg(dmg_mod),
                m_pen(pen),
                m_range_mod(range_mod),
                m_range(range),
                m_pen_count(pen_count),
                m_failed(failed) {}

            float m_dmg{ }, m_pen{ }, m_range_mod{ }, m_range{ };
            int m_pen_count{ };
            bool m_failed{ };
        }

        handle_bullet_data(static_cast<float>(data.m_wpn_data->m_nDamage()), data.m_wpn_data->m_flPenetration(), data.m_wpn_data->range(),
            data.m_wpn_data->m_flRangeModifier(), 4, false);

        //  L_PRINT(LOG_INFO) << "Initialized handlebulletpen data";

        float corrected_dmg = static_cast<float>(data.m_wpn_data->m_nDamage());
        float flTraceLength = 0.f;
        auto flMaxRange = data.m_wpn_data->range();
        if (trace_data.m_num_update > 0) {
            for (int i{ }; i < trace_data.m_num_update; i++) {
                auto* value = reinterpret_cast<UpdateValueT* const>(
                    reinterpret_cast<std::uintptr_t>(trace_data.m_pointer_update_value)
                    + i * sizeof(UpdateValueT));

                game_trace_t game_trace = { };
               sdk::m_trace->InitializeTraceInfo(&game_trace);
                sdk::m_trace->get_trace_info(
                    &trace_data, &game_trace, 0.0f,
                    reinterpret_cast<void*>(
                        reinterpret_cast<std::uintptr_t>(trace_data.m_arr.data())
                        + sizeof(trace_arr_element_t) * (value->handleIdx & 0x7fffu)));
                /*
                L_PRINT(LOG_INFO) << "game_trace: entryindex:" << game_trace.HitEntity->GetRefEHandle().GetEntryIndex();
                L_PRINT(LOG_INFO) << "game_trace: m_target_pawn entryindex:" << data.m_target_pawn->GetRefEHandle().GetEntryIndex();
                L_PRINT(LOG_INFO) << "game_trace: m_target entryindex:" << data.m_target->GetRefEHandle().GetEntryIndex();
                */

                flMaxRange -= flTraceLength;

                // we didn't hit anything, stop tracing shoot
                if (game_trace.Fraction == 1.0f)
                {
                    break;
                }

                // calculate the damage based on the distance the bullet traveled
                flTraceLength += game_trace.Fraction * flMaxRange;
                corrected_dmg *= std::powf(data.m_wpn_data->m_flRangeModifier(), flTraceLength / 500.f);

                // check is actually can shoot through
                if (flTraceLength > 3000.f)
                    break;

                if (game_trace.HitEntity && game_trace.HitEntity->handle().index() == data.m_target_pawn->handle().index()) {
                    ScaleDamage2(game_trace.HitboxData->m_hitgroup, data.m_target_pawn, data.m_wpn_data->m_flArmorRatio(), data.m_wpn_data->m_flHeadshotMultiplier(), &corrected_dmg);
                    dmg = corrected_dmg;//data.m_dmg > 0.f ? data.m_dmg : handle_bullet_data.m_dmg;
                    valid = true;
                    return true;
                }
                else
                    valid = false;


                if (sdk::m_trace->handle_bullet_penetration(&trace_data, &handle_bullet_data, value, false))
                    return false;

                corrected_dmg = handle_bullet_data.m_dmg;

            }
        }

        return false;
    }

    void autowall::c_auto_wall::ScaleDamage2(const int iHitGroup, CCSPlayerPawn* pCSPlayer, const float flWeaponArmorRatio, const float flWeaponHeadShotMultiplier, float* pflDamageToScale)
    {
        if (!pCSPlayer)
            return;

        // @ida CCSPlayer::TraceAttack(): server.dll -> "55 8B EC 83 E4 F8 81 EC ? ? ? ? 56 8B 75 08 57 8B F9 C6"
        auto WeaponServices = pCSPlayer->GetItemServices();
        if (!WeaponServices)
            return;

        auto bHeavyArmor = WeaponServices->m_bHasHeavyArmor();
        // ida: server.dll; 80 78 42 00 74 08 F3 0F 59 35 ?? ?? ?? ?? 80 BE 04 0D 00 00 00
        static CConVar* mp_damage_scale_ct_head = sdk::m_cvar->find(
            hash_32_fnv1a_const("mp_damage_scale_ct_head")),
            * mp_damage_scale_t_head = sdk::m_cvar->find(
                hash_32_fnv1a_const("mp_damage_scale_t_head")),
            * mp_damage_scale_ct_body = sdk::m_cvar->find(
                hash_32_fnv1a_const("mp_damage_scale_ct_body")),
            * mp_damage_scale_t_body = sdk::m_cvar->find(
                hash_32_fnv1a_const("mp_damage_scale_t_body"));


        const auto damage_scale_ct_head = mp_damage_scale_ct_head->value.fl,
            damage_scale_t_head = mp_damage_scale_t_head->value.fl,
            damage_scale_ct_body = mp_damage_scale_ct_body->value.fl,
            damage_scale_t_body = mp_damage_scale_t_body->value.fl;

        float flHeadDamageScale = (pCSPlayer->m_iTeamNum() == TEAM_CT ? damage_scale_ct_head : damage_scale_t_head);
        const float flBodyDamageScale = (pCSPlayer->m_iTeamNum() == TEAM_CT ? damage_scale_ct_body : damage_scale_t_body);

        if (bHeavyArmor)            flHeadDamageScale *= 0.5f;

        switch (iHitGroup)
        {
        case HITGROUP_HEAD:
            *pflDamageToScale *= flWeaponHeadShotMultiplier * flHeadDamageScale;
            break;
        case HITGROUP_CHEST:
        case HITGROUP_LEFTARM:

        case HITGROUP_RIGHTARM:
        case HITGROUP_NECK:
            *pflDamageToScale *= flBodyDamageScale;
            break;
        case HITGROUP_STOMACH:
            *pflDamageToScale *= 1.25f * flBodyDamageScale;
            break;
        case HITGROUP_LEFTLEG:
        case HITGROUP_RIGHTLEG:
            *pflDamageToScale *= 0.75f * flBodyDamageScale;
            break;
        default:
            break;
        }

        if (pCSPlayer->hasArmour(iHitGroup))
        {
            // @ida CCSPlayer::OnTakeDamage(): server.dll -> "80 BF ? ? ? ? ? F3 0F 10 5C 24 ? F3 0F 10 35"

            const int iArmor = pCSPlayer->GetArmorValue();
            float flHeavyArmorBonus = 1.0f, flArmorBonus = 0.5f, flArmorRatio = flWeaponArmorRatio * 0.5f;

            if (bHeavyArmor)
            {
                flHeavyArmorBonus = 0.25f;
                flArmorBonus = 0.33f;
                flArmorRatio *= 0.20f;
            }

            float flDamageToHealth = *pflDamageToScale * flArmorRatio;
            if (const float flDamageToArmor = (*pflDamageToScale - flDamageToHealth) * (flHeavyArmorBonus * flArmorBonus); flDamageToArmor > static_cast<float>(iArmor))
                flDamageToHealth = *pflDamageToScale - static_cast<float>(iArmor) / flArmorBonus;

            *pflDamageToScale = flDamageToHealth;
        }
    }
    
    std::unique_ptr<c_auto_wall> g_auto_wall{};

}