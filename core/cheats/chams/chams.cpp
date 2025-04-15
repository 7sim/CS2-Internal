#include "chams.h"

// used: game's interfaces

#include "..\..\sdk\interfaces\materialsystem.h"
#include "..\..\sdk\interfaces\resourcesystem.h"
#include "..\..\sdk\datatypes\ckeyvalues\keyvalues.h"
#include "..\..\..\core\utils\hook\hooks.h"
#include "..\..\sdk\sdk.h"
#include "..\..\sdk\addresses\entity.h"
#include "../../utils/configs/config.h"
#include "..\..\sdk\datatypes\cutlhandles\strong_handle.hpp"
#include "..\..\math\elements\matrix.h"
#include "..\backtrack\record.h"
CStrongHandle<CMaterial2> chams::create(const char* name, const char szVmatBuffer[])
{
	CKeyValues3* pKeyValues3 = nullptr;

	pKeyValues3 = pKeyValues3->create_material_from_resource();

	pKeyValues3->LoadFromBuffer(szVmatBuffer);

	CStrongHandle<CMaterial2> pCustomMaterial = {};

	utils::create_material(nullptr, &pCustomMaterial, name, pKeyValues3, 0, 1);

	return pCustomMaterial;
}

struct resource_material_t
{
    CStrongHandle<CMaterial2> mat;
    CStrongHandle<CMaterial2> mat_invs;
};

static resource_material_t resourceMaterials[e_chams::max_count];
bool chams::init()
{
	// check if we already initialized materials
	if (bInitialized)
		return bInitialized;

    // flat
    resourceMaterials[flat] = resource_material_t{
        .mat = create("materials/dev/flat.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
        {
            Shader = "csgo_unlitgeneric.vfx"
        
            F_IGNOREZ = 0
             F_DISABLE_Z_WRITE = 0
            F_DISABLE_Z_BUFFERING = 0
            F_BLEND_MODE = 1
            F_TRANSLUCENT = 1
            F_RENDER_BACKFACES = 0

            g_vColorTint = [1.000000, 1.000000, 1.000000, 1.000000]
            g_vGlossinessRange = [0.000000, 1.000000, 0.000000, 0.000000]
            g_vNormalTexCoordScale = [1.000000, 1.000000, 0.000000, 0.000000]
            g_vTexCoordOffset = [0.000000, 0.000000, 0.000000, 0.000000]
            g_vTexCoordScale = [1.000000, 1.000000, 0.000000, 0.000000]
            g_tColor = resource:"materials/dev/primary_white_color_tga_f7b257f6.vtex"
            g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
        })"),
        .mat_invs = create("materials/dev/flat_i.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
        {
            Shader = "csgo_unlitgeneric.vfx"
            F_IGNOREZ = 1
            F_DISABLE_Z_WRITE = 1
            F_DISABLE_Z_BUFFERING = 1
            F_BLEND_MODE = 1
            F_TRANSLUCENT = 1
            g_vColorTint = [1.000000, 1.000000, 1.000000, 0.000000]
            g_vGlossinessRange = [0.000000, 1.000000, 0.000000, 0.000000]
            g_vNormalTexCoordScale = [1.000000, 1.000000, 0.000000, 0.000000]
            g_vTexCoordOffset = [0.000000, 0.000000, 0.000000, 0.000000]
            g_vTexCoordScale = [1.000000, 1.000000, 0.000000, 0.000000]
            g_tColor = resource:"materials/dev/primary_white_color_tga_f7b257f6.vtex"
            g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
        })")
    };

    // default
    resourceMaterials[e_chams::regular] = resource_material_t{
        .mat = create("materials/dev/def.vmat",R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
        {
            Shader = "csgo_complex.vfx"
            F_IGNOREZ = 0
            F_DISABLE_Z_WRITE = 0
            F_DISABLE_Z_BUFFERING = 0
            F_RENDER_BACKFACES = 0
            F_TRANSLUCENT = 1
            F_PAINT_VERTEX_COLORS = 1
            g_vColorTint = [1.000000, 1.000000, 1.000000, 1.000000]
            TextureNormal = resource:"materials/default/default_normal.tga"
            g_tAmbientOcclusion = resource:"materials/default/default_ao_tga_559f1ac6.vtex"
            g_tColor = resource:"materials/default/default_color_tga_72dcfbfd.vtex"
            g_tNormal = resource:"materials/default/default_normal_tga_7be61377.vtex"
        })"),
        .mat_invs = create("materials/dev/dedi.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
        {
            Shader = "csgo_complex.vfx"
            F_IGNOREZ = 1
            F_DISABLE_Z_WRITE = 1
            F_DISABLE_Z_BUFFERING = 1
            F_RENDER_BACKFACES = 1
            F_TRANSLUCENT = 1
            F_PAINT_VERTEX_COLORS = 1
            g_vColorTint = [1.000000, 1.000000, 1.000000, 1.000000]
            TextureNormal = resource:"materials/default/default_normal.tga"
            g_tAmbientOcclusion = resource:"materials/default/default_ao_tga_559f1ac6.vtex"
            g_tColor = resource:"materials/default/default_color_tga_72dcfbfd.vtex"
            g_tNormal = resource:"materials/default/default_normal_tga_7be61377.vtex"
        })")
    };

    // illumin
    resourceMaterials[e_chams::illumin] = resource_material_t{
        .mat = create("materials/dev/il.vmat", R"#(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d}
        format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
        {
            shader = "solidcolor.vfx"
            g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
            g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
            g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
            g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
            g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
            F_IGNOREZ = 0
            F_DISABLE_Z_WRITE = 0
            F_DISABLE_Z_BUFFERING = 0
            F_RENDER_BACKFACES = 0
            g_vColorTint = [9.0, 9.0, 9.0, 9.0]
        } )#"),
        .mat_invs = create("materials/dev/ili.vmat", R"#(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d}
        format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
        {
            shader = "solidcolor.vfx"
            g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
            g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
            g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
            g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
            g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
            F_IGNOREZ = 1
            F_DISABLE_Z_WRITE = 1
            F_DISABLE_Z_BUFFERING = 1
            F_RENDER_BACKFACES = 0
            g_vColorTint = [9.0, 9.0, 9.0, 9.0]
        } )#")
    };

    // glow
    resourceMaterials[glow] = resource_material_t{
        .mat = create("materials/dev/glow.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
		     shader = "csgo_effects.vfx"
                g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
                g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_flOpacityScale = 0.70
                g_flFresnelExponent = 1
                g_flFresnelFalloff = 1
                g_flFresnelMax = 0.0
                g_flFresnelMin = 1.0
                F_ADDITIVE_BLEND = 1
                F_BLEND_MODE = 1
                F_TRANSLUCENT = 1
                F_IGNOREZ = 0
                F_DISABLE_Z_WRITE = 0
                F_DISABLE_Z_BUFFERING = 0
                F_RENDER_BACKFACES = 0
                g_vColorTint = [1.0, 1.0, 1.0, 0.0]
})"),
        .mat_invs = create("materials/dev/glowi.vmat",R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
				Shader = "csgo_effects.vfx"
                g_flFresnelExponent = 7.0
                g_flFresnelFalloff = 10.0
                g_flFresnelMax = 0.1
                g_flFresnelMin = 1.0
				g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tMask1 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tMask2 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tMask3 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                g_tSceneDepth = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_flToolsVisCubemapReflectionRoughness = 1.0
                g_flBeginMixingRoughness = 1.0
                g_vColorTint = [ 1.000000, 1.000000, 1.000000, 0 ]
                F_IGNOREZ = 1
                F_DISABLE_Z_WRITE = 1
                F_DISABLE_Z_BUFFERING = 1
                F_RENDER_BACKFACES = 0
})")
    };

    // metal
    resourceMaterials[latex] = resource_material_t{
        .mat = create("materials/dev/latex.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
                Shader = "solidcolor.vfx"
                F_BLEND_MODE = 1
                g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
                g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                F_IGNOREZ = 0
                F_DISABLE_Z_WRITE = 0
                F_DISABLE_Z_BUFFERING = 0
                F_RENDER_BACKFACES = 0
                g_vColorTint = [50.0, 50.0, 50.0, 50.0]
})"),
        .mat_invs = create("materials/dev/latex.vmat",R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
				Shader = "solidcolor.vfx"
				g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
				g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
				g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
				g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
				g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
				F_IGNOREZ = 1
				F_DISABLE_Z_WRITE = 1
				F_DISABLE_Z_BUFFERING = 1
				F_RENDER_BACKFACES = 0
                F_BLEND_MODE = 1
				g_vColorTint = [50.0, 50.0, 50.0, 50.0]
})")
    };

    // metal
    resourceMaterials[glass] = resource_material_t{
        .mat = create("materials/dev/latex.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
 {
                shader = "csgo_effects.vfx"
                g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
                g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_flOpacityScale = 0.45
                g_flFresnelExponent = 0.75
                g_flFresnelFalloff = 1
                g_flFresnelMax = 0.0
                g_flFresnelMin = 1
                F_ADDITIVE_BLEND = 1
                F_BLEND_MODE = 1
                F_TRANSLUCENT = 1
                F_IGNOREZ = 0
                F_DISABLE_Z_WRITE = 0
                F_DISABLE_Z_BUFFERING = 0
                F_RENDER_BACKFACES = 0
                g_vColorTint = [1.0, 1.0, 1.0, 0.0]
            })"),
        .mat_invs = create("materials/dev/latex.vmat",R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
 {
                shader = "csgo_effects.vfx"
                g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
                g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
                g_flOpacityScale = 0.45
                g_flFresnelExponent = 0.75
                g_flFresnelFalloff = 1
                g_flFresnelMax = 0.0
                g_flFresnelMin = 1
                F_ADDITIVE_BLEND = 1
                F_BLEND_MODE = 1
                F_TRANSLUCENT = 1
                F_IGNOREZ = 1
                F_DISABLE_Z_WRITE = 1
                F_DISABLE_Z_BUFFERING = 1
                F_RENDER_BACKFACES = 0
                g_vColorTint = [1.0, 1.0, 1.0, 0.0]
            })")
    };

    // golden glow
    resourceMaterials[golden] = resource_material_t{
        .mat = create("materials/dev/latex.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
 {
               Shader = "csgo_complex.vfx"
                F_IGNOREZ = 1
                F_DISABLE_Z_WRITE = 1
                F_DISABLE_Z_BUFFERING = 1
F_RENDER_BACKFACES = 0
	F_TRANSLUCENT = 1
    F_PAINT_VERTEX_COLORS = 1
    g_vColorTint = [1.000000, 1.000000, 1.000000, 1.000000]
    TextureNormal = resource:"materials/default/default_normal.tga"
    g_tAmbientOcclusion = resource:"materials/default/default_ao_tga_559f1ac6.vtex"
    g_tColor = resource:"materials/default/default_color_tga_72dcfbfd.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7be61377.vtex"
            })"),
        .mat_invs = create("materials/dev/latex.vmat",R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
 {
                 Shader = "solidcolor.vfx"
                F_BLEND_MODE = 0
                g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
                g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
                F_IGNOREZ = 1
                F_DISABLE_Z_WRITE = 1
                F_DISABLE_Z_BUFFERING = 1
                F_RENDER_BACKFACES = 1
            })")
    };

    // bubble
    resourceMaterials[bubble] = resource_material_t{
        .mat = create("materials/dev/latex.vmat", R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
 {
            		Shader = "csgo_effects.vfx"
                g_flFresnelExponent = 7.0
                g_flFresnelFalloff = 10.0
                g_flFresnelMax = 0.1
                g_flFresnelMin = 1.0
				g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tMask1 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tMask2 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tMask3 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tSceneDepth = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_flToolsVisCubemapReflectionRoughness = 1.0
                g_flBeginMixingRoughness = 1.0
                g_vColorTint = [ 1.000000, 1.000000, 1.000000, 0.5 ]
                F_IGNOREZ = 0
                F_DISABLE_Z_WRITE = 0
                F_DISABLE_Z_BUFFERING = 1
                F_RENDER_BACKFACES = 0
            })"),
        .mat_invs = create("materials/dev/latex.vmat",R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
 {
             		Shader = "csgo_effects.vfx"
                g_flFresnelExponent = 7.0
                g_flFresnelFalloff = 10.0
                g_flFresnelMax = 0.1
                g_flFresnelMin = 1.0
				g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
                g_tMask1 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tMask2 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tMask3 = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_tSceneDepth = resource:"materials/default/default_mask_tga_fde710a5.vtex"
                g_flToolsVisCubemapReflectionRoughness = 1.0
                g_flBeginMixingRoughness = 1.0
                g_vColorTint = [ 1.000000, 1.000000, 1.000000, 0 ]
                F_IGNOREZ = 1
                F_DISABLE_Z_WRITE = 1
                F_DISABLE_Z_BUFFERING = 1
                F_RENDER_BACKFACES = 0
            })")
    };
    bInitialized = true;

    // iterate.
    for (auto& [m_visible, m_invisible] : resourceMaterials) {
        if (m_visible == nullptr || m_invisible == nullptr)
            bInitialized = false;
    }

	return bInitialized;
}

void chams::unload()
{
	for (auto& [m_visible, pInvisible] : resourceMaterials)
	{
		if (m_visible)
			sdk::m_resource_utils->DeleteResource(m_visible.pBinding);

		if (pInvisible)
			sdk::m_resource_utils->DeleteResource(pInvisible.pBinding);
	}
}
#include "..\backtrack\record.h"





Matrix3x4_t CTransform::ToMatrix() const {
    Matrix3x4_t matrix{};

    const auto& rot = quatOrientation;
    const auto& pos = vecPosition;

    matrix[0][0] = 1.0f - 2.0f * rot.y * rot.y - 2.0f * rot.z * rot.z;
    matrix[1][0] = 2.0f * rot.x * rot.y + 2.0f * rot.w * rot.z;
    matrix[2][0] = 2.0f * rot.x * rot.z - 2.0f * rot.w * rot.y;

    matrix[0][1] = 2.0f * rot.x * rot.y - 2.0f * rot.w * rot.z;
    matrix[1][1] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.z * rot.z;
    matrix[2][1] = 2.0f * rot.y * rot.z + 2.0f * rot.w * rot.x;

    matrix[0][2] = 2.0f * rot.x * rot.z + 2.0f * rot.w * rot.y;
    matrix[1][2] = 2.0f * rot.y * rot.z - 2.0f * rot.w * rot.x;
    matrix[2][2] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.y * rot.y;

    matrix[0][3] = pos.x;
    matrix[1][3] = pos.y;
    matrix[2][3] = pos.z;

    return matrix;
}


e_chams_targets chams::get_rendering_target(CBaseEntity* render_ent) noexcept {
    auto local = hooks::m_get_local_player(0);
    if (!local)
        return e_invalid;

    if (render_ent->IsViewModel())
        return e_view_model;

    if (render_ent->IsViewModelWeapon())
    {
        //auto pOwnerEntity = sdk::m_game->pGameEntitySystem->Get<CBaseEntity>(render_ent->m_hOwnerEntity());
       // if (pOwnerEntity && pOwnerEntity == g::m_local_player_pawn)
            return e_weapon;

    }

    if (!render_ent->IsBasePlayer() && !render_ent->IsBasePlayerController())  
        return e_invalid;

    auto player = (CCSPlayerPawn*)render_ent;
    if (!player)
        return e_invalid;

    auto ragdoll = cfg_get(bool, g_cfg.bVisualChamsRagDoll) ? true : player->IsAlive();
    if (!ragdoll)
        return e_invalid;

    if (player->enemy(local))
        return e_enemies;

    if (local->IsAlive() && sdk::m_game->pGameEntitySystem->get_handle_entity(player) == sdk::m_game->pGameEntitySystem->get_handle_entity(local))
        return e_local;

    return e_team;
}

CMaterial2* get_material(int type, bool invisible) { return invisible ? resourceMaterials[type].mat_invs : resourceMaterials[type].mat; }
/*bool chams::hook(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{

    if (!bInitialized)
        return false;

    const auto oDrawObject = hooks::m_draw_object.GetOriginal();

    if (!g::m_local_player_pawn)
        return false;

    if (arrMeshDraw == nullptr)
        return false;

    if (arrMeshDraw->pSceneAnimatableObject == nullptr)
        return false;

    auto h_owner = arrMeshDraw->pSceneAnimatableObject->GetOwner();
    if (!h_owner.valid())
        return false;

    auto entity = sdk::m_game->pGameEntitySystem->Get<CBaseEntity>(h_owner);
    if (!entity)
        return false;

    SchemaClassInfoData_t* class_type;
    entity->dump_class_info(&class_type);

    if (!class_type)
        return false;

    if (c_run_time::StringCompare(arrMeshDraw->pMaterial->GetName(), xorstr_("materials/dev/glowproperty.vmat")) == 0)
        return false;

    if (entity->IsBasePlayerController() || entity->IsBasePlayer()) {
        auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(h_owner);


        if (player->enemy(g::m_local_player_pawn)) {
     

            bool valid = cfg_get(bool, g_cfg.bVisualChamsRagDoll) ? true : player->IsAlive();
        
            
            if (valid) {
                bool og = !cfg_get(bool, g_cfg.bVisualChams) && !cfg_get(bool, g_cfg.bVisualChamsIgnoreZ);

                if (og)
                    return false;
       

                //xqz
                if (cfg_get(bool, g_cfg.bVisualChamsIgnoreZ) && player->IsAlive()) {
                    int type = cfg_get(int, g_cfg.nVisualChamMaterial);

                    arrMeshDraw->pMaterial = get_material(type, true);
                    arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsIgnoreZ);
                    oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
                }
                int type2 = cfg_get(int, g_cfg.nVisualChamMaterial2);

                arrMeshDraw->pMaterial = get_material(type2, false);
                arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChams);
                oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
            
            }
        }
        else if (player != g::m_local_player_pawn && !player->enemy(g::m_local_player_pawn)) {
            if (player->IsAlive()) {
                bool og = !cfg_get(bool, g_cfg.bVisualChamsT) && !cfg_get(bool, g_cfg.bVisualChamsIgnoreZT);

                if (og)
                    return false;

                if (cfg_get(bool, g_cfg.bVisualChamsT)) {


                    //xqz
                    if (cfg_get(bool, g_cfg.bVisualChamsIgnoreZT)) {
                        int type = cfg_get(int, g_cfg.nVisualChamMaterialT);

                        arrMeshDraw->pMaterial = get_material(type, true);
                        arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsIgnoreZT);
                        oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
                    }
                    int type2 = cfg_get(int, g_cfg.nVisualChamMaterialT2);

                    arrMeshDraw->pMaterial = get_material(type2, false);
                    arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsT);
                    oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
                }
            }
        }
        else if (player->m_pEntityIdentity()->index() == g::m_local_player_pawn->m_pEntityIdentity()->index()) {
            if (player->IsAlive()) {
                bool og = !cfg_get(bool, g_cfg.bVisualChamsL) && !cfg_get(bool, g_cfg.bVisualChamsIgnoreZL);

                if (og)
                    return false;

                if (cfg_get(bool, g_cfg.bVisualChamsL)) {

                    int type = cfg_get(int, g_cfg.nVisualChamMaterialL);

                    arrMeshDraw->pMaterial = get_material(type, false);
                    arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsL);
                   
                    oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
                }
            }
        }
    }
    else if (entity->IsViewModel()) {
        auto viewmodel = sdk::m_game->pGameEntitySystem->Get<C_CSGOViewModel>(h_owner);
        int type = cfg_get(int, g_cfg.nVisualChamMaterialV);

        if (!cfg_get(bool, g_cfg.bVisualChamsV))
            return false;

        arrMeshDraw->pMaterial = get_material(type, false);
        arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsV);

        oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
    }

    oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

   return true;
}
*/



void CS_FASTCALL chams::hook(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{


    static auto original = hooks::m_draw_object.GetOriginal();

    if (!bInitialized)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    if (g::m_unloading)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    if (!sdk::m_engine->is_valid())
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    auto local_player = hooks::m_get_local_player(0);
    if (!local_player)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    if (!arrMeshDraw)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    if (!arrMeshDraw->pSceneAnimatableObject)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    if (!g::chams_ready)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    if (nDataCount < 1)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    CMeshData* render_data = arrMeshDraw;

    if (!render_data)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

 

    if (!render_data->pSceneAnimatableObject)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    auto render_ent = render_data->pSceneAnimatableObject->GetOwner();
    if (!render_ent.valid())
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    auto entity = sdk::m_game->pGameEntitySystem->Get(render_ent);
    if (!entity)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    // fix glow 
    if (c_run_time::StringCompare(arrMeshDraw->pMaterial->GetName(), xorstr_("materials/dev/glowproperty.vmat")) == 0)
        return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    const auto target = get_rendering_target(entity);

   

    if (target == e_enemies && cfg_get(bool, g_cfg.bVisualChams)) {

        bool og = !cfg_get(bool, g_cfg.bVisualChams) && !cfg_get(bool, g_cfg.bVisualChamsIgnoreZ);

        if (og) {
            return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
        }


        //xqz
        if (cfg_get(bool, g_cfg.bVisualChamsIgnoreZ)) {
            int type = cfg_get(int, g_cfg.nVisualChamMaterial);
            arrMeshDraw->pMaterial = get_material(type, true);
            arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsIgnoreZ);
            original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
        }

        int type2 = cfg_get(int, g_cfg.nVisualChamMaterial2);
        arrMeshDraw->pMaterial = get_material(type2, false);
        arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChams);
        original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
    }
    else if (target == e_team && cfg_get(bool, g_cfg.bVisualChamsT)) {

        bool og = !cfg_get(bool, g_cfg.bVisualChamsT) && !cfg_get(bool, g_cfg.bVisualChamsIgnoreZT);

        if (og) {
            return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

        }

        if (cfg_get(bool, g_cfg.bVisualChamsT)) {
            //xqz
            if (cfg_get(bool, g_cfg.bVisualChamsIgnoreZT)) {
                int type = cfg_get(int, g_cfg.nVisualChamMaterialT);

                arrMeshDraw->pMaterial = get_material(type, true);
                arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsIgnoreZT);
                original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
            }
            int type2 = cfg_get(int, g_cfg.nVisualChamMaterialT2);

            arrMeshDraw->pMaterial = get_material(type2, false);
            arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsT);
            original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
        }
    }
    else if (target == e_local && cfg_get(bool, g_cfg.bVisualChamsL)) {
        bool og = !cfg_get(bool, g_cfg.bVisualChamsL);

        if (og) {
            return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

        }

        if (cfg_get(bool, g_cfg.bVisualChamsL)) {

            int type = cfg_get(int, g_cfg.nVisualChamMaterialL);

            arrMeshDraw->pMaterial = get_material(type, false);
            arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsL);

            original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
        }

    }
    else if (target == e_view_model && cfg_get(bool, g_cfg.bVisualChamsV)) {
        bool og = !cfg_get(bool, g_cfg.bVisualChamsV);

        if (og)
        {
            return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

        }


        int type = cfg_get(int, g_cfg.nVisualChamMaterialV);

        arrMeshDraw->pMaterial = get_material(type, false);
        arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsV);

        original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

    }
    else if (target == e_weapon && cfg_get(bool, g_cfg.bVisualChamsW)) {
        bool og = !cfg_get(bool, g_cfg.bVisualChamsW);

        if (og)
        {
            return original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

        }


        int type = cfg_get(int, g_cfg.nVisualChamMaterialW);

        arrMeshDraw->pMaterial = get_material(type, false);
        arrMeshDraw->colValue = cfg_get(Color_t, g_cfg.colVisualChamsW);

        original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
    }
    


   original(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

}
