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


namespace autowall {
    class c_auto_wall {
    public:
        struct data_t {
            enum e_pos { e_local, e_target };
            data_t() = default;

            inline data_t(const Vector_t local_pos, const Vector_t target_pos,
                CBaseEntity* target,
                CBaseEntity* local,
                CCSPlayerPawn* localpawn, CCSPlayerPawn* targetpawn,
                CCSWeaponBaseVData* wpn_data, const bool fire_bullet) noexcept :
                m_pos{ local_pos, target_pos },
                m_target(target),
                m_local(local),
                m_local_pawn(localpawn),
                m_target_pawn(targetpawn),
                m_wpn_data(wpn_data),
                m_can_hit(fire_bullet){}


            std::array< Vector_t, static_cast<int>(sizeof(e_pos)) > m_pos{ };
            CBaseEntity* m_target{ }, * m_local{ };
            CCSPlayerPawn* m_local_pawn{};
            CCSPlayerPawn* m_target_pawn{};
            CCSWeaponBaseVData* m_wpn_data{ };

            bool m_can_hit{ };
            float m_dmg{ };
        } m_data;
        void pen(data_t& data, const Vector_t local_pos, const Vector_t target_pos, CBaseEntity* target, CBaseEntity* local, CCSPlayerPawn* localpawn, CCSPlayerPawn* targetpawn,
            CCSWeaponBaseVData* wpn_data, float& dmg, bool& valid);
        bool penetrate(data_t& data, const Vector_t local_pos, const Vector_t target_pos, CBaseEntity* target, CBaseEntity* local, CCSPlayerPawn* localpawn, CCSPlayerPawn* targetpawn,
            CCSWeaponBaseVData* wpn_data, float& dmg, bool& valid);
        void ScaleDamage(data_t& data, const int hitgroup, CCSPlayerPawn* entity);
        bool FireBullet(data_t& data, float& dmg, bool& valid);
        void ScaleDamage2(const int iHitGroup, CCSPlayerPawn* pCSPlayer, const float flWeaponArmorRatio, const float flWeaponHeadShotMultiplier, float* pflDamageToScale);
    public:
        [[nodiscard]] inline auto get_data(data_t& data) const noexcept { return data; }
    };

    extern std::unique_ptr<c_auto_wall> g_auto_wall;
}