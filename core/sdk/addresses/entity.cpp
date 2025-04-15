#include "entity.h"

#include "../sdk.h"
#include "..\..\utils\hook\hooks.h"
#include "..\..\cheats\skins\inventory.h"
#include "..\..\cheats\skins\inventory_manager.h"

#include "../interfaces/trace.h"

CCSPlayerController* CCSPlayerController::Get() {
	
	const int index = sdk::m_engine->get_local_player();
	return sdk::m_game->pGameEntitySystem->Get<CCSPlayerController>(index);

}

void CCSWeaponBase::AddStattrakEntity()
{
	auto mItem = &this->m_AttributeManager().m_Item();
	hooks::m_add_stattrack_entity(this, mItem);
}
void CCSWeaponBase::AddKeychainEntity()
{
	auto mItem = &this->m_AttributeManager().m_Item();
	hooks::m_add_keychain_entity(this, mItem);
}

void CCSWeaponBase::AddNametagEntity()
{
	auto mItem = &this->m_AttributeManager().m_Item();

	debug(hooks::m_add_name_tag_entity != nullptr);
	//return hooks::m_add_name_tag_entity(this, mItem);

}
CCSWeaponBase* CCSPlayerPawn::GetActiveWeapon( )
{
	if (!this)
		return nullptr;
	// get weapon services
	CCSPlayer_WeaponServices* weapon_services = this->m_pWeaponServices( );

	// sanity
	if ( weapon_services == nullptr )
		return nullptr;

	CCSWeaponBase* active_weapon = sdk::m_game->pGameEntitySystem->Get<CCSWeaponBase>(weapon_services->m_hActiveWeapon());
	if (!active_weapon)
		return nullptr;
	// return base weapon pointer from active weapon handle
	return active_weapon;
}

Vector_t CCSPlayerPawn::GetEyePosition() {
	Vector_t EyePosition;
	utils::CallVFunc<void, 169>(this, &EyePosition);
	return EyePosition;
}
bool CCSPlayerPawn::IsAlive( )
{
	if (!this)
		return false;

	return m_iHealth() > 0;//m_lifeState() == LIFE_ALIVE;
}
bool CCSPlayerPawn::enemy(CCSPlayerPawn* pOther)
{
	if (!pOther || !this)
		return false;

	if (this->get_entity_by_handle() == pOther->get_entity_by_handle())
		return false;

	if (g::deathmatch)
		return true;

	return this->m_iTeamNum() != pOther->m_iTeamNum();
}
CCSWeaponBaseVData* CCSWeaponBase::GetWeaponData()
{
	// return pointer to weapon data
	return *reinterpret_cast<CCSWeaponBaseVData**>((uintptr_t)this + hooks::m_get_weapon_data);
}
bool CBaseEntity::get_bounding_box(ImVec4& out, bool item) {

	if (!this )
		return false;

	CCollisionProperty* pCollision = this->m_pCollision();
	if (!pCollision) 
		return false;

	auto scene = this->m_pGameSceneNode();
	if (!scene)
		return false;

	Vector_t min{}, max{};

	CTransform nodeToWorldTransform = scene->m_nodeToWorld();
	const Matrix3x4_t matTransform = nodeToWorldTransform.quatOrientation.ToMatrix(nodeToWorldTransform.vecPosition);
	
	min = pCollision->m_vecMins();
	max = pCollision->m_vecMaxs();

	if (!item) {
		out.x = out.y = std::numeric_limits<float>::max();
		out.z = out.w = -std::numeric_limits<float>::max();
		for (int i = 0; i < 8; ++i) {
			const Vector_t vecPoint{ i & 1 ? max.x : min.x, i & 2 ? max.y : min.y, i & 4 ? max.z : min.z };
			ImVec2 vecScreen;
			if (!math::WorldToScreen(vecPoint.Transform(matTransform), vecScreen))
				return false;

			ImVec4 box;

			out.x = math::Min(out.x, vecScreen.x); 
			out.y = math::Min(out.y, vecScreen.y);
			out.z = math::Max(out.z, vecScreen.x); 
			out.w = math::Max(out.w, vecScreen.y);
		}

	}
	else {
		out.x = out.y = std::numeric_limits<float>::max();
		out.z = out.w = -std::numeric_limits<float>::max();
		for (int i = 0; i < 8; ++i) {
			const Vector_t vecPoint{ i & 1 ? max.x : min.x, i & 2 ? max.y : min.y, i & 4 ? max.z : min.z };
			ImVec2 vecScreen;
			if (!math::WorldToScreen(vecPoint.Transform(matTransform), vecScreen))
				return false;

			out.x = math::Min(out.x, vecScreen.x); out.y = math::Min(out.y, vecScreen.y); out.z = math::Max(out.z, vecScreen.x); out.w = math::Max(out.w, vecScreen.y);
		}

		if (item) {
			const float offset = 2.0f;  
			out.x -= offset;  // Left
			out.y -= offset;  // Top
			out.z += offset;  // Right
			out.w += offset;  // Bottom
		}

	}

	return true;
}


bool CCSWeaponBase::can_primary_attack(const int nWeaponType, const float flServerTime)
{
	return (this->clip1() > 0) && (this->GetNextPrimaryAttackTick() <= flServerTime + 2);

}

bool CCSWeaponBase::can_seccondary_attack(const int nWeaponType, const float flServerTime)
{
	// check is weapon ready to attack
	if (this->GetNextSecondaryAttackTick() > TIME_TO_TICKS(flServerTime))
		return false;

	// we doesn't need additional checks for knives
	if (nWeaponType == WEAPONTYPE_KNIFE)
		return true;

	// check do weapon have ammo
	if (this->clip1() <= 0)
		return false;


	const auto nDefinitionIndex = this->m_AttributeManager().m_Item().m_iItemDefinitionIndex();

	// only revolver is allowed weapon for secondary attack
	if (nDefinitionIndex != WEAPON_R8_REVOLVER)
		return false;

	return true;
}
bool CCSPlayerPawn::can_shoot( int tick)
{
	if (!g::m_weapon_data || !sdk::m_global_vars)
		return false;

	if (const auto weapon_type{ g::m_weapon_data->m_WeaponType() }; weapon_type == WEAPONTYPE_KNIFE
		|| weapon_type == WEAPONTYPE_C4 || weapon_type == WEAPONTYPE_UNKNOWN
		|| weapon_type == WEAPONTYPE_GRENADE)
		return false;

	if (!g::m_active_weapon->clip1())
		return false;

	if (sdk::m_global_vars->m_curtime * 64 <= g::m_active_weapon->GetNextPrimaryAttackTick())
		return false;

	return true;
}


void CCSWeaponBase::UpdateModel(bool should)
{
	if (!this)
		return;

	auto composite_material = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(this) + 0x568);
	if (!composite_material) {
		_log(LOG_ERROR) << "[skin] couldn't get composite material for current weapon skin";
		return;
	}

	 hooks::m_upate_weapon_model(composite_material, should);
}
std::string CCSWeaponBase::GetWeaponName( )
{
	debug(hooks::m_get_weapon_name != nullptr);
	return hooks::m_get_weapon_name( this );
}

uint32_t CModel::hitboxes_num()
{
//	debug(hooks::m_bones_num != nullptr);
	return 27;//hooks::m_bones_num(this);
}

uint32_t CModel::hitbox_flags(uint32_t index)
{
	debug(hooks::m_bones_flags != nullptr);
	return hooks::m_bones_flags(this, index);
}

const char* CModel::hitbox_name(uint32_t index)
{
	debug(hooks::m_bones_name != nullptr);
	return hooks::m_bones_name(this, index);
}

uint32_t CModel::hitbox_parent(uint32_t index)
{
	debug(hooks::m_bones_parent != nullptr);
	return hooks::m_bones_parent(this, index);
}

void CCSWeaponBase::update_accuracy(void* a1) {
	return hooks::m_update_weapon_accuracy(this);
}

void CCSWeaponBase::UpdateSubClass()
{
	return hooks::m_update_weapon_sub_class(this);
}

void CGameSceneNode::SetMeshGroupMask(uint64_t meshGroupMask) {
	debug(hooks::m_set_mesh_group_mask != nullptr);

	return hooks::m_set_mesh_group_mask(this, meshGroupMask);
}
void C_BaseModelEntity::SetModel(const char* name)
{
	if (!this)
		return;

	if (hooks::m_set_wep_model == nullptr)
		return;

	return hooks::m_set_wep_model(this, name);
}

CGCClientSharedObjectTypeCache* CGCClientSharedObjectCache::CreateBaseTypeCache(int nClassID)
{
	return hooks::m_create_base_type_cache(this, nClassID);
}
#include <sstream>
static CGCClientSharedObjectTypeCache* CreateBaseTypeCache(CCSPlayerInventory* pInventory) {

	CGCClientSystem* pGCClientSystem = hooks::m_get_client_system();
	if (!pGCClientSystem) return nullptr;

	CGCClient* pGCClient = pGCClientSystem->GetCGCClient();
	if (!pGCClient) return nullptr;

	CGCClientSharedObjectCache* pSOCache = pGCClient->FindSOCache(pInventory->GetOwner());
	if (!pSOCache) return nullptr;

	return pSOCache->CreateBaseTypeCache(k_EEconTypeItem);
}
CCSPlayerInventory* CCSPlayerInventory::GetInstance()
{

	CCSInventoryManager* pInventoryManager = CCSInventoryManager::GetInstance();
	if (!pInventoryManager)
		return nullptr;

	return pInventoryManager->GetLocalInventory();
}

void CEconItem::set_sticker(int index, int id)
{
	auto attribute_name = "sticker slot " + std::to_string(index) + " id";
	auto econ_item_attribute_definition = sdk::m_client->GetEconItemSystem()->GetEconItemSchema()->get_attribute_definition_by_name(attribute_name.c_str());

	if (!econ_item_attribute_definition)
		return;

	//hooks::set_dynamic_attribute_value(this, econ_item_attribute_definition, &id);
}
bool CCSPlayerInventory::AddEconItem(CEconItem* pItem)
{
	if (!pItem) return false;

	CGCClientSharedObjectTypeCache* pSOTypeCache = ::CreateBaseTypeCache(this);

	if (!pSOTypeCache || !pSOTypeCache->AddObject((CSharedObject*)pItem))
		return false;

	_log(LOG_INFO) << "[CEconItem] created object successfully.";

	return hooks::m_add_econ_item(this, pItem, true, true, true);
}
#include "..\..\cheats\skins\skins.h"
void CCSPlayerInventory::RemoveEconItem(CEconItem* pItem)
{    // Helper function to aid in removing items.
	if (!pItem) return;
	/*for (const auto& weapon_skin : m_weapon_skins) {
		if (weapon_skin.first == pItem->m_nDefIndex) {
			m_weapon_skins.erease(weapon_skin);
		}
	}*/
	CGCClientSharedObjectTypeCache* pSOTypeCache = ::CreateBaseTypeCache(this);
	if (!pSOTypeCache) return;

	const CUtlVector<CEconItem*>& pSharedObjects =
		pSOTypeCache->GetVecObjects<CEconItem*>();
	if (!pSharedObjects.Find(pItem)) return;

	pSOTypeCache->RemoveObject((CSharedObject*)pItem);

	SODestroyed(GetOwner(), (CSharedObject*)pItem, eSOCacheEvent_Incremental);

	pItem->Destruct();
}


std::pair<uint64_t, uint32_t> CCSPlayerInventory::GetHighestIDs()
{
	uint64_t max_item_id = 0;
	uint32_t max_inventory_id = 0;

	CGCClientSharedObjectTypeCache* so_type_cache = ::CreateBaseTypeCache(this);
	if (so_type_cache) {
		CUtlVector<CEconItem*>& items = so_type_cache->GetVecObjects<CEconItem*>();

		for (int i{}; i < items.Size(); i++)
		{
			auto it = items.Element(i);

			if (!it)
				continue;

			// Checks if item is default.
			if ((it->m_ulID & 0xF000000000000000) != 0) continue;

			max_item_id = std::max(max_item_id, it->m_ulID);
			max_inventory_id = std::max(max_inventory_id, it->m_unInventory);
		}
	}

	return std::make_pair(max_item_id, max_inventory_id);
}

C_EconItemView* CCSPlayerInventory::GetItemViewForItem(uint64_t itemID) {
	C_EconItemView* pEconItemView = nullptr;

	const CUtlVector<C_EconItemView*>& pItems = GetItemVector();
	for (int i{}; i < pItems.Count(); i++)
	{
		auto it = pItems.Element(i);
		if (it && it->m_iItemID() == itemID) {
			pEconItemView = it;
			break;
		}
	}

	return pEconItemView;
}



CEconItem* CCSPlayerInventory::GetSOCDataForItem(uint64_t itemID) {
	CEconItem* pSOCData = nullptr;

	CGCClientSharedObjectTypeCache* pSOTypeCache = ::CreateBaseTypeCache(this);
	if (pSOTypeCache) {

		const CUtlVector<CEconItem*>& vecItems = pSOTypeCache->GetVecObjects<CEconItem*>();
		for (int i{}; i < vecItems.Count(); i++)
		{
			auto it = vecItems.Element(i);
			if (it && it->m_ulID == itemID) {
				pSOCData = it;
				break;
			}
		}
	}

	return pSOCData;
}


void CCSPlayerPawn::get_hitbox_data(uint32_t idx, Vector_t& pos, Vector4D_t& rot, float& scale, bool predict) {
	return;
}

CGCClientSharedObjectCache* CGCClient::FindSOCache(SOID_t ID, bool bCreateIfMissing)
{
	return hooks::m_find_base_type_so_cache(this, ID, bCreateIfMissing);
}

void CEconItem::SetDynamicAttributeValue(int index, void* value)
{
	CEconItemSchema* pItemSchema =
		sdk::m_client->GetEconItemSystem()->GetEconItemSchema();
	if (!pItemSchema) return;

	void* pAttributeDefinitionInterface =
		pItemSchema->GetAttributeDefinitionInterface(index);
	if (!pAttributeDefinitionInterface) return;


	hooks::m_set_dynamic_attribute_value_int(this, pAttributeDefinitionInterface,
		value);
}
void CSkeletonInstance::setup_bones( int mask) {
	hooks::m_calc_space_world_bones(this, mask);
}

void CCSWeaponBase::update_accuracy() {

	debug(hooks::m_weapon_update_accuracy != nullptr);
	return hooks::m_weapon_update_accuracy(this);
}

float CCSWeaponBase::get_inaccuracy()
{
	debug(hooks::m_get_innacuracy_weapon != nullptr);


	float x, y;
	return hooks::m_get_innacuracy_weapon(this, &x, &y);
	//return CALL_VIRTUAL(float, 359, this);;
}

void CCSWeaponBase::get_calculated_spread(CCSWeaponBaseVData* data, int mode, unsigned int seed, float innacuracy, float spread, float recoil_index, float* spread_x, float* spread_y)
{
	debug(hooks::m_get_spread_weapon_calculation != nullptr);
	return hooks::m_get_spread_weapon_calculation(this, data, mode, seed, innacuracy, spread, recoil_index, spread_x, spread_y);
}
void CBaseEntity::set_origin(Vector_t pos) {
	if (!this)
		return;

	hooks::m_set_entity_origin(this, pos);
}
void C_CSGOViewModel::set_viewmodel_origin(Vector_t pos) {
	debug(hooks::m_set_view_model_origin != nullptr);
	hooks::m_set_view_model_origin(this, pos);
}


bool CCSPlayerPawn::locking_smoke()
{
	if (!m_pCollision())
		return false;

	auto obb_min = m_pCollision()->m_CollisionGroup();
	auto obb_max = m_pCollision()->m_vecMaxs();
	auto obb_center = (obb_max + obb_min) / 2;
	auto obb_position = obb_center + m_pGameSceneNode()->m_vecAbsOrigin();

	auto eye_position = GetEyePosition();

	debug(hooks::m_locking_smoke_view != nullptr);
	return hooks::m_locking_smoke_view(eye_position, obb_position);
}

void CEconItem::SetDynamicAttributeValueString(int index, const char* value)
{
}

CEconItem* CEconItem::CreateInstance()
{
	return hooks::m_create_shared_object_subclass();
}
int CCSPlayerPawn::get_bone_idx(const char* name) {
	return hooks::m_get_bone_index(this, name);
}
 int CCSPlayerPawn::get_index_by_hitbox(int idx) {
	
	 switch (idx) {
	 case HitBoxes::HEAD:
	 case HitBoxes::NECK:
		 return e_hitboxes::hitbox_head;
		 break;
	 case HitBoxes::LEFT_CHEST:
	 case HitBoxes::RIGHT_CHEST:
	 case HitBoxes::CHEST:
		 return e_hitboxes::hitbox_chest;
		 break;
	 case HitBoxes::PELVIS:
	 case HitBoxes::STOMACH:
		 return e_hitboxes::hitbox_stomach;
		 break;
	 case HitBoxes::L_FEET:
		 return e_hitboxes::hitbox_left_foot;
	 case HitBoxes::R_FEET:
		 return e_hitboxes::hitbox_right_foot;
		 break;
	 case HitBoxes::L_LEG:
		 return e_hitboxes::hitbox_left_calf;
	 case HitBoxes::R_LEG:
		 return e_hitboxes::hitbox_right_calf;
		 break;

	 }

	 return -999;
}
 float CCSWeaponBase::get_spread() {
	 if (!this)
		 return 0.f;
	 // 364, 363, 
	 return hooks::m_get_weapon_spread(this);
 }
bool CEconItemDefinition::IsWeapon()
{
	return GetStickersSupportedCount() >= 4;
}

bool CCSPlayerPawn::visible(CCSPlayerPawn* local_player, QAngle_t ang)
{
	auto player = this;
	if (!this || this->m_iHealth() <= 0)
		return false;

	if (!local_player)
		return false;

	trace_filter_t filter = {}; sdk::m_trace->Init(filter, local_player, 0x1C300Bu, 3, 7);

	game_trace_t trace = {}; ray_t ray = {};
	Vector_t src = local_player->GetEyePosition();
	Vector_t dst = this->GetEyePosition();
	sdk::m_trace->TraceShape(ray, &src, &dst, filter, trace);

	if (trace.HitEntity) {
		if (trace.HitEntity->handle().valid()) {
			if (trace.HitEntity->handle().index() == player->handle().index() || trace.Fraction > 0.97f)
				return true;
		}
	}


	return false;
}
const char* CMaterial2::GetName() {

	return utils::CallVFunc<const char*, 1>(this);
}  

 bool CCSPlayerPawn::on_crosshair(QAngle_t ang, float range, int* handle, CCSPlayerPawn* pawn, CBaseEntity* ent)
{
	if (!this || this->m_iHealth() <= 0)
		return false;
	
	trace_filter_t filter = {};
	sdk::m_trace->Init(filter, this, 0x1C300Bu, 3, 7);

	game_trace_t trace = {};
	ray_t ray = {};
	Vector_t vecForward = { };
	ang.ToDirections(&vecForward);
	vecForward *= range;
	Vector_t src = this->GetEyePosition();
	Vector_t dst = src + vecForward;
	sdk::m_trace->TraceShape(ray, &src, &dst, filter, trace);

	if (trace.HitEntity && trace.HitEntity->handle().index() != 0) {
		auto entity = sdk::m_game->pGameEntitySystem->Get<CBaseEntity>(trace.HitEntity->handle());

		if (entity && (entity->IsBasePlayer() || entity->IsBasePlayerController())) {
			auto controller = sdk::m_game->pGameEntitySystem->Get<CCSPlayerController>(entity->handle());

			ent = entity;
			if (controller) {
				auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(controller->PawnHandle());

				if (player)
				{
					pawn = player;
					*handle = controller->PawnHandle().index();

					return true;
				}
				return false;
			}
			return false;
		}
		return false;
	}
	return false;
	
}

 bool CCSPlayerPawn::match_interpolation()
 {
	 if (!this)
		 return false;

	 if (!sdk::m_csgo_input)
		 return false;
	 /*
	 if (sdk::m_csgo_input->m_InputHistory.Size() <= 0)
		 return false;

	 _log(LOG_INFO) << "tick size: " << sdk::m_csgo_input->m_InputHistory.Size();

	 for (int i{}; i < sdk::m_csgo_input->m_InputHistory.Size(); i++)
	 {
		 auto history = sdk::m_csgo_input->m_InputHistory.Element(i);

		 int player_tick_count = history.m_nPlayerTickCount;
		 float player_fraction = history.m_flPlayerTickFraction;
	

		 // Adjust tick count based on fraction
		 if (player_fraction >= (1.0f - 0.0099999998f))
			 player_tick_count += 1;

		 // Log the adjusted player_tick_count
		 float time = static_cast<float>(player_tick_count) + player_fraction;

		 CCSGOInterpolationInfoPB tick;
		 CCSGOInterpolationInfoPB tick1;
		 CCSGOInterpolationInfoPB tick2;

		 // Get the lerp tick and validate
		 hooks::m_get_lerp_tick(m_pGameSceneNode(), &tick, &tick1, &tick2, &player_tick_count);

		 // Log the tick value from get_lerp_tick
		 _log(LOG_INFO) << "fraction: " << tick.flFraction << "nSrcTick: " << tick.nSrcTick << "| nDstTick" << tick.nDstTick << " | player_tick_count:" << player_tick_count;

		 // Validate tick comparison with player_tick_count
		 if (tick.nSrcTick >= player_tick_count) {
			 _log(LOG_ERROR) << "tick is greater than or equal to player_tick_count.";
			 return false;
		 }
	 }*/

	 return true;
 }
 int CCSPlayerPawn::get_lerp_tick(int player_tick_count)
 {
	 if (!this)
		 return 0;
	 return 0;
 }

 c_hitbox_data_t CCSPlayerPawn::get_hitbox(int i)
 {
	 c_hitbox_data_t return_data{};
	 constexpr int MAX_HITBOXES = 64;

	 CHitBoxSets* m_hitbox_set = hooks::m_get_hitbox_set(this, 0);
	 if (!m_hitbox_set)
		 return return_data;

	 CHitBox* hitbox = &m_hitbox_set->m_HitBoxes()[i];
	 if (!hitbox)
		 return return_data;

	 CTransform m_hitbox_transform[MAX_HITBOXES];
	 int m_count = hooks::m_get_hitbox_to_world_transform(this, m_hitbox_set, m_hitbox_transform, 1024);
	 if (!m_count)
		 return return_data;

	 const Matrix3x4_t hitBoxMatrix = m_hitbox_transform[i].ToMatrix();
	 Vector_t w_mins, w_maxs;

	 math::TransformAABB(hitBoxMatrix, hitbox->m_vMinBounds(), hitbox->m_vMaxBounds(), w_mins, w_maxs);

	 return_data.rad = hitbox->m_flShapeRadius();
	 return_data.min = w_mins;
	 return_data.max = w_maxs;

	 return return_data;
 }

 void CCSPlayerPawn::create_shocked_effect(const char* effect_name)
 {
//	 		 (utils::FindPattern("client.dll", "48 8D 05 ? ? ? ? 4C 89 B4 24 ? ? ? ? 48 8D 4D ? 48 89 45"));
	 return;
	 static auto EffectDataVftable = utils::FindPattern(xorstr_("client.dll"), xorstr_("48 8D 05 ? ? ? ? 4C 89 B4 24 ? ? ? ? 48 8D 4D ? 48 89 45"));
	 static auto DispatchEffect = reinterpret_cast<void(__fastcall*)(const char* name, c_effect_data * data)>(utils::FindPattern("client.dll", "48 89 5C 24 08 57 48 81 EC 50 01 ?? ?? 48 8B F9"));
	 //40 55 56 48 83 EC 28 65
	 c_effect_data effect = { };
	 effect.vftable = EffectDataVftable;
	 effect.handle_entity = this->get_entity_by_handle();
	 effect.handle_other_entity = static_cast<uint32_t>(-1);

	 DispatchEffect(effect_name, &effect);
	
 }
 bool CBaseEntity::IsBasePlayerController()
 {
	 SchemaClassInfoData_t* _class = nullptr;
	 dump_class_info(&_class);
	 if (!_class)
		 return false;

	 const uint32_t hash = hash_32_fnv1a_const(_class->szName);
	
	 return (hash == hash_32_fnv1a_const(xorstr_("CCSPlayerController")));
 }
 bool CBaseEntity::IsBasePlayer()
 {
	 SchemaClassInfoData_t* pClassInfo;
	 dump_class_info(&pClassInfo);
	 if (pClassInfo == nullptr)
		 return false;

	 return hash_32_fnv1a_const(pClassInfo->szName) == hash_32_fnv1a_const("C_CSPlayerPawn");
 }

 bool CBaseEntity::IsViewModel()
 {
	 SchemaClassInfoData_t* pClassInfo;
	 dump_class_info(&pClassInfo);
	 if (pClassInfo == nullptr)
		 return false;

	 return hash_32_fnv1a_const(pClassInfo->szName) == hash_32_fnv1a_const("C_ViewmodelAttachmentModel");
 }
  

  bool CBaseEntity::IsInferno()
  {
	  SchemaClassInfoData_t* pClassInfo;
	  dump_class_info(&pClassInfo);
	  if (pClassInfo == nullptr)
		  return false;

	  return hash_32_fnv1a_const(pClassInfo->szName) == hash_32_fnv1a_const("C_Inferno");
  }
  bool CBaseEntity::IsPlantedC4()
  {
	  SchemaClassInfoData_t* pClassInfo;
	  dump_class_info(&pClassInfo);
	  if (pClassInfo == nullptr)
		  return false;

	  return hash_32_fnv1a_const(pClassInfo->szName) == hash_32_fnv1a_const("C_PlantedC4");
  }


  bool CBaseEntity::IsViewModelWeapon()
  {
	  SchemaClassInfoData_t* pClassInfo;
	  dump_class_info(&pClassInfo);
	  if (pClassInfo == nullptr)
		  return false;

	  return hash_32_fnv1a_const(pClassInfo->szName) == hash_32_fnv1a_const("C_CSGOViewModel");
  }
 bool CBaseEntity::IsWeapon()
 {
	 static SchemaClassInfoData_t* pWeaponBaseClass = nullptr;
	 if (pWeaponBaseClass == nullptr)
		 sdk::m_schema_system->FindTypeScopeForModule(xorstr_("client.dll"))->FindDeclaredClass(&pWeaponBaseClass, xorstr_("C_CSWeaponBase"));


	 SchemaClassInfoData_t* pClassInfo;
	 dump_class_info(&pClassInfo);
	 if (pClassInfo == nullptr)
		 return false;

	 return (pClassInfo->InheritsFrom(pWeaponBaseClass));
 }

bool CEconItemDefinition::IsKnife(bool excludeDefault, const char* name)
{
	if (strcmp(m_pszItemTypeName, "#CSGO_Type_Knife"))
		return false;


	return excludeDefault ? m_nDefIndex >= 500 : true;

}
bool CEconItemDefinition::IsCustomPlayer()
{
	if (strcmp(m_pszItemTypeName, "#Type_CustomPlayer"))
		return false;

	if (m_nDefIndex >= 5036 && m_nDefIndex < 5106)
		return false;

	if (m_nDefIndex >= 5200 && m_nDefIndex <= 5205)
		return false;

	if (m_nDefIndex >= 5300 && m_nDefIndex <= 5305)
		return false;

	if (m_nDefIndex == 5600)
		return false;

	return true;
}
bool CEconItemDefinition::IsGlove(bool excludeDefault, const char* name)
{
	static constexpr auto Type_Hands = hash_32_fnv1a_const("#Type_Hands");

	if (hash_32_fnv1a_const(m_pszItemTypeName) != Type_Hands) 
		return false;

	const bool defaultGlove = m_nDefIndex == 5028 || m_nDefIndex == 5029;

	return excludeDefault ? !defaultGlove : true;
}

bool CEconItemDefinition::IsAgent(bool bExcludeDefault)
{
	return false;
}

CEconItem* C_EconItemView::GetSOCData(CCSPlayerInventory* inventory)
{
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory) return nullptr;
	return pInventory->GetSOCDataForItem(m_iItemID());
}

CCSInventoryManager* CCSInventoryManager::GetInstance()
{
	if (!hooks::m_get_inventory_manager) {
		CS2_LOG("\ncouldnt get InventoryMngr");
		return nullptr;
	}
	debug(hooks::m_get_inventory_manager != nullptr);
	return hooks::m_get_inventory_manager();
}

void CCSPlayer_MovementServices::process_movement(CUserCmd* cmd)
{
	if (!this)
		return;

	return hooks::m_process_move(this, cmd);
}

void CCSPlayerController::set_player_tickbase(int nTickBase)
{
	if (!this)
		return;

	return hooks::m_set_player_tickbase(this, nTickBase);
}

void CCSPlayerController::run_physics_think()
{
	if (!this)
		return;

	return hooks::m_run_physics_think(this);
}
