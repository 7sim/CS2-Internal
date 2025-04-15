#pragma once
#pragma once
// used: stronghandle
#include "../../sdk/datatypes/cutlhandles/strong_handle.hpp"
#include "..\backtrack\record.h"

class CMeshData;
class CMaterial2;
enum e_chams
{
    flat,
    regular,
    illumin,
    glow,
	latex,
	glass,
	golden,
	bubble,
    max_count
};
enum e_chams_targets : std::int32_t {
    e_invalid = 0,
    e_enemies,
    e_weapon,
    e_team,
    e_local_weapon,
    e_view_model,
    e_local
};
enum e_materials { e_visible, e_invisible, e_max_material };

namespace chams
{
  
	static e_chams_targets get_rendering_target(CBaseEntity* render_ent) noexcept;

	bool init();
	void unload();

	record::c_lag_entity* get_lag_record(CBaseHandle handle);

	void CS_FASTCALL hook(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);

	// @note: bDisableZBuffering == true to create invisible material
	CStrongHandle<CMaterial2> create(const char* name, const char szVmatBuffer[]);

	bool set_material(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);

	inline bool bInitialized = false;
}
