#include <vector>
#include "skins.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include "..\..\sdk\addresses\entity.h"
#include "inventory.h"
#include "inventory_manager.h"
#include "..\..\helpers\vpk\VTexParser.h"
#include "..\..\helpers\tfm\tinyformat.h"
#include <unordered_set>
#include "..\..\sdk\interfaces\network.h"
#include "../../utils/hook/hooks.h"
#include "../../utils/hook/hooks.h"
static         std::vector<uint64_t> m_added_items;
std::unordered_set<int> uniqueTypes;
std::vector<gui_skins> item_default;
std::vector<gui_skins> items;
std::vector<gui_skins> item_knife_default;
std::vector<gui_skins> item_glove_default;
std::vector<gui_skins> item_weapon;
std::vector<gui_skins> item_agent;
std::vector<gui_skins> item_glove;
std::vector<gui_skins> skin_preview;
std::vector<gui_skins> items_list;
std::unordered_map<int, std::vector< c_paintKits>>  m_paint_kits;
std::vector<c_paintKits> added_skins;
std::vector<EconItem_t> added_item_list;
std::vector<gui_skins> item_stickers;
std::vector<gui_stickers> item_medals;
std::vector<gui_skins> item_key_chains;

std::vector<gui_skins> item_stickers_default;

enum loadout_slot_t {
    LOADOUT_SLOT_C4 = 1,
    LOADOUT_SLOT_CLOTHING_APPEARANCE = 46,
    LOADOUT_SLOT_CLOTHING_CUSTOMPLAYER = 38,
    LOADOUT_SLOT_CLOTHING_EYEWEAR = 42,
    LOADOUT_SLOT_CLOTHING_FACEMASK = 40,
    LOADOUT_SLOT_CLOTHING_HANDS = 41,
    LOADOUT_SLOT_CLOTHING_HAT = 43,
    LOADOUT_SLOT_CLOTHING_LOWERBODY = 44,
    LOADOUT_SLOT_CLOTHING_TORSO = 45,
    LOADOUT_SLOT_COUNT = 57,
    LOADOUT_SLOT_EQUIPMENT0 = 32,
    LOADOUT_SLOT_EQUIPMENT1 = 33,
    LOADOUT_SLOT_EQUIPMENT2 = 34,
    LOADOUT_SLOT_EQUIPMENT3 = 35,
    LOADOUT_SLOT_EQUIPMENT4 = 36,
    LOADOUT_SLOT_EQUIPMENT5 = 37,
    LOADOUT_SLOT_FIRST_ALL_CHARACTER = 54,
    LOADOUT_SLOT_FIRST_AUTO_BUY_WEAPON = 0,
    LOADOUT_SLOT_FIRST_COSMETIC = 41,
    LOADOUT_SLOT_FIRST_PRIMARY_WEAPON = 8,
    LOADOUT_SLOT_FIRST_WHEEL_EQUIPMENT = 32,
    LOADOUT_SLOT_FIRST_WHEEL_GRENADE = 26,
    LOADOUT_SLOT_FIRST_WHEEL_WEAPON = 2,
    LOADOUT_SLOT_FLAIR0 = 55,
    LOADOUT_SLOT_GRENADE0 = 26,
    LOADOUT_SLOT_GRENADE1 = 27,
    LOADOUT_SLOT_GRENADE2 = 28,
    LOADOUT_SLOT_GRENADE3 = 29,
    LOADOUT_SLOT_GRENADE4 = 30,
    LOADOUT_SLOT_GRENADE5 = 31,
    LOADOUT_SLOT_HEAVY0 = 20,
    LOADOUT_SLOT_HEAVY1 = 21,
    LOADOUT_SLOT_HEAVY2 = 22,
    LOADOUT_SLOT_HEAVY3 = 23,
    LOADOUT_SLOT_HEAVY4 = 24,
    LOADOUT_SLOT_HEAVY5 = 25,
    LOADOUT_SLOT_INVALID = 4294967295,
    LOADOUT_SLOT_LAST_ALL_CHARACTER = 56,
    LOADOUT_SLOT_LAST_AUTO_BUY_WEAPON = 1,
    LOADOUT_SLOT_LAST_COSMETIC = 41,
    LOADOUT_SLOT_LAST_PRIMARY_WEAPON = 25,
    LOADOUT_SLOT_LAST_WHEEL_EQUIPMENT = 37,
    LOADOUT_SLOT_LAST_WHEEL_GRENADE = 31,
    LOADOUT_SLOT_LAST_WHEEL_WEAPON = 25,
    LOADOUT_SLOT_MELEE = 0,
    LOADOUT_SLOT_MISC0 = 47,
    LOADOUT_SLOT_MISC1 = 48,
    LOADOUT_SLOT_MISC2 = 49,
    LOADOUT_SLOT_MISC3 = 50,
    LOADOUT_SLOT_MISC4 = 51,
    LOADOUT_SLOT_MISC5 = 52,
    LOADOUT_SLOT_MISC6 = 53,
    LOADOUT_SLOT_MUSICKIT = 54,
    LOADOUT_SLOT_PET = 39,
    LOADOUT_SLOT_PROMOTED = 4294967295,
    LOADOUT_SLOT_RIFLE0 = 14,
    LOADOUT_SLOT_RIFLE1 = 15,
    LOADOUT_SLOT_RIFLE2 = 16,
    LOADOUT_SLOT_RIFLE3 = 17,
    LOADOUT_SLOT_RIFLE4 = 18,
    LOADOUT_SLOT_RIFLE5 = 19,
    LOADOUT_SLOT_SECONDARY0 = 2,
    LOADOUT_SLOT_SECONDARY1 = 3,
    LOADOUT_SLOT_SECONDARY2 = 4,
    LOADOUT_SLOT_SECONDARY3 = 5,
    LOADOUT_SLOT_SECONDARY4 = 6,
    LOADOUT_SLOT_SECONDARY5 = 7,
    LOADOUT_SLOT_SMG0 = 8,
    LOADOUT_SLOT_SMG1 = 9,
    LOADOUT_SLOT_SMG2 = 10,
    LOADOUT_SLOT_SMG3 = 11,
    LOADOUT_SLOT_SMG4 = 12,
    LOADOUT_SLOT_SMG5 = 13,
    LOADOUT_SLOT_SPRAY0 = 56
};




bool update;
bool update_knife;
bool update_gloves;
bool full_update; 
bool added_knife_to_inventory = false;
static bool recieved_prev_update = false;
static bool recieved_prev_update2 = false;

knife_information m_added_knife;

std::unordered_map<int, weapon_information>  m_weapon_skins;

void SetDynamicAttributeValueString(CEconItem* econ, const char* index, const char* value) {
    if (!econ)
        return;

    CEconItemSchema* pItemSchema = sdk::m_client->GetEconItemSystem()->GetEconItemSchema();
    if (!pItemSchema)
        return;

    void* pAttributeDefinitionInterface = pItemSchema->get_attribute_definition_by_name(index);
    if (!pAttributeDefinitionInterface)
        return;

    uint8_t attributestringdata[0x100] = {};

    hooks::m_init_attribute_string(attributestringdata, 0, false);

    *reinterpret_cast<uint32_t*>(attributestringdata + 0x10) = 1;

    hooks::m_fill_attribute_string(reinterpret_cast<void*>(attributestringdata + 0x20), value);
    *reinterpret_cast<void**>(attributestringdata + 0x18) = reinterpret_cast<void*>(attributestringdata + 0x20);

    hooks::set_dynamic_attribute_value_string(econ, pAttributeDefinitionInterface, attributestringdata);
}

// Sets the stattrak on our weapon depending whether it is enabled or not
void set_sttatrack(CCSWeaponBase* weapon) {

}


void set_name(CCSWeaponBase* weapon, C_EconItemView* item) {

    const char* name_tag = "";
    memcpy(&item->m_szCustomName(), name_tag, sizeof(name_tag));
 //   hooks::m_add_name_tag_entity(weapon, weapon->m_hNametagAttachment());
}
int update_stlye = 0;
int weapons_update_stlye = 0;
bool added_knife_skin = true;

void Items::c_skins::on_knife(int stage)
{
    if (stage != 6)
        return;

    if (!sdk::m_engine->is_valid())
        return;

    auto inventory = CCSPlayerInventory::GetInstance();
    if (!inventory)
        return;

    auto controller = CCSPlayerController::Get();
    if (!controller)
        return;

    auto local_player = hooks::m_get_local_player(0);
    if (!local_player)
        return;

    if (!local_player->IsAlive() || !local_player->GetActiveWeapon())
        return;

    if (!local_player->GetActiveWeapon()->GetWeaponData())
        return;

    auto view_model_services = local_player->m_pViewModelServices();
    if (!view_model_services)
        return;

    auto view_model = sdk::m_game->pGameEntitySystem->Get<C_CSGOViewModel>(view_model_services->m_hViewModel());
    if (!view_model)
        return;

    if (local_player->GetActiveWeapon()->GetWeaponData()->m_WeaponType() != WEAPONTYPE_KNIFE)
        return;

    const uint64_t steam_id = inventory->GetOwner().m_id;

    for (int i = 64 + 1; i <= sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); ++i) {
        auto entity = sdk::m_game->pGameEntitySystem->Get(i);
        if (!entity)
            continue;

        if (!entity->IsWeapon())
            continue;

        auto weapon = reinterpret_cast<CCSWeaponBase*>(entity);
        if (!weapon || !weapon->GetWeaponData())
            continue;

        // make sure weapon's valid
        if (weapon->GetOriginalOwnerXuid() != steam_id || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_GRENADE || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_C4 || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_EQUIPMENT
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_STACKABLEITEM || weapon->GetWeaponData()->m_WeaponType() != WEAPONTYPE_KNIFE
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_FISTS
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_BREACHCHARGE
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_BUMPMINE
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_TABLET
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SHIELD
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_ZONE_REPULSOR
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_TASER)
            continue;

        if (weapon->GetWeaponData()->m_WeaponType() != WEAPONTYPE_KNIFE)
            continue;

        auto attribute_container = &weapon->m_AttributeManager();
        if (!attribute_container) continue;

        auto item_view = &attribute_container->m_Item();
        if (!item_view) continue;

        auto item_definition = item_view->GetStaticData();
        if (!item_definition) continue;

        auto item_scene_node = weapon->m_pGameSceneNode();
        if (!item_scene_node) continue;


        C_EconItemView* item_in_loadout = nullptr;

        if (item_definition->IsWeapon()) 
            continue;
        
        item_in_loadout = inventory->GetItemInLoadout(weapon->m_iOriginalTeamNumber(), item_definition->GetLoadoutSlot());

        if (!item_in_loadout)
            continue;

        auto item_loadout_definition = item_in_loadout->GetStaticData();
        if (!item_loadout_definition) continue;


        auto h_weapon = weapon->handle();
        const bool is_knife = item_loadout_definition->IsKnife(false, item_loadout_definition->m_pszItemBaseName);

        if (!is_knife)
            continue;

        if (stage == 6 ) {
            item_view->m_bDisallowSOC() = false;

            //weapon->m_nFallbackPaintKit() = m_added_knife.skin_id;

            item_view->m_iItemID() = m_added_knife.item_id;
            item_view->m_iItemIDHigh() = m_added_knife.item_high_id;
            item_view->m_iItemIDLow() = m_added_knife.item_low_id;
            item_view->m_iAccountID() = uint32_t(inventory->GetOwner().m_id);
            item_view->m_iItemDefinitionIndex() = item_loadout_definition->m_nDefIndex;
        }

        const char* knife_model = item_loadout_definition->GetModelName();

        if (stage == 6 && weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_KNIFE) {
            {
                weapon->GetWeaponData()->m_szName() = item_loadout_definition->GetSimpleWeaponName();
                weapon->UpdateWeaponData();
            }

            {
                weapon->SetModel(knife_model);
                auto view_model_idx = view_model->m_hWeapon();
                if (view_model_idx.index() == h_weapon.index()) {
                    view_model->SetModel(knife_model);
                }
            }
        }

        if (full_update) {
            _log(LOG_WARNING) << "[fsn] knife full update composite";
            weapon->UpdateModel(true);
            weapon->UpdateComposite(true);
            weapon->UpdateCompositeSec(true);
            full_update = false;
        }

        if (update_knife && !local_player->immune() ) {
            _log(LOG_WARNING) << "[knife] update composite";
            weapon->UpdateModel(true);
            weapon->UpdateComposite(true);
            weapon->UpdateCompositeSec(true);

            update_knife = false;
        }

    }

 
}


void Items::c_skins::fsn(int stage)
{
    if (stage != 6)
        return;

    if (!sdk::m_engine->is_valid())
        return;

    auto inventory = CCSPlayerInventory::GetInstance();
    if (!inventory)
        return;

    auto controller = CCSPlayerController::Get();
    if (!controller)
        return;

    auto local_player = hooks::m_get_local_player(0);
    if (!local_player)
        return;

    if (!local_player->IsAlive() || !local_player->GetActiveWeapon())
        return;

    if (!local_player->GetActiveWeapon()->GetWeaponData())
        return;

    auto view_model_services = local_player->m_pViewModelServices();
    if (!view_model_services)
        return;

    auto view_model = sdk::m_game->pGameEntitySystem->Get<C_CSGOViewModel>(view_model_services->m_hViewModel());
    if (!view_model)
        return;

    const uint64_t steam_id = inventory->GetOwner().m_id;

    auto m_gloves = &local_player->m_EconGloves();
    if (m_gloves) {
        if (m_gloves->GetStaticData()) {
            auto m_gloves_data = m_gloves->GetStaticData();

            if (m_gloves_data)
                Items::gloves->glove(inventory, local_player, view_model, m_gloves, m_gloves_data);
        }
    }

}

void Items::c_skins::on_weapon(void* rdx)
{
    if (!sdk::m_engine->is_valid())
        return;

    auto inventory = CCSPlayerInventory::GetInstance();
    if (!inventory)
        return;

    auto controller = CCSPlayerController::Get();
    if (!controller)
        return;

    auto local_player = hooks::m_get_local_player(0);
    if (!local_player)
        return;

    if (!local_player->IsAlive())
        return;

    if (!local_player->GetActiveWeapon())
        return;

    if (!local_player->GetActiveWeapon()->GetWeaponData())
        return;

    auto view_model_services = local_player->m_pViewModelServices();
    if (!view_model_services)
        return;

    if (!view_model_services->m_hViewModel().valid())
        return;

    auto view_model = sdk::m_game->pGameEntitySystem->Get<C_CSGOViewModel>(view_model_services->m_hViewModel());
    if (!view_model)
        return;

    auto scene_node = view_model->m_pGameSceneNode();
    if (!scene_node)
        return;

    auto m_skeleton = scene_node->m_pSkeleton();
    if (!m_skeleton)
        return;

    auto model = &m_skeleton->GetModel();
    if (!model)
        return;

    void* offsetToMask = &model->m_MeshGroupMask();
    if (offsetToMask != rdx)
        return;

    const uint64_t steam_id = inventory->GetOwner().m_id;

    for (int i = 64 + 1; i <= sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); ++i) {
        auto entity = sdk::m_game->pGameEntitySystem->Get(i);
        if (!entity)
            continue;

        if (!entity->IsWeapon())
            continue;

        auto weapon = reinterpret_cast<CCSWeaponBase*>(entity);
        if (!weapon)
            continue;

        if (!weapon->GetWeaponData())
            continue;
        // make sure weapon's valid
        if (weapon->GetOriginalOwnerXuid() != steam_id || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_GRENADE || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_C4 || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_EQUIPMENT
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_STACKABLEITEM
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_FISTS
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_BREACHCHARGE
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_BUMPMINE
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_TABLET
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SHIELD || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_KNIFE
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_ZONE_REPULSOR
            || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_TASER)
            continue;

        auto attribute_container = &weapon->m_AttributeManager();
        if (!attribute_container) continue;

        auto item_view = &attribute_container->m_Item();
        if (!item_view) continue;

        auto item_definition = item_view->GetStaticData();
        if (!item_definition) continue;

        auto item_scene_node = weapon->m_pGameSceneNode();
        if (!item_scene_node) continue;

        auto data = weapon->GetWeaponData();
        if (!data)
            continue;


        if (!m_weapon_skins.empty()) {
            if (weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_KNIFE)
                continue;

            if (!weapon->m_bUIWeapon())
                weapon->AddKeychainEntity();

            bool found_paint_kit_for_current_weapon = false;
            weapon_information cur_skin;
            for (auto paint_kit : m_weapon_skins) {
                if (paint_kit.first == item_definition->m_nDefIndex) {
                    cur_skin = paint_kit.second;
                    found_paint_kit_for_current_weapon = true;
                    break;
                }
            }

            if (!found_paint_kit_for_current_weapon)
                continue;

            // set inventory data into current weapon
            int paint_kit_index = cur_skin.skin_id;
            item_view->m_bDisallowSOC() = false;
            item_view->m_iItemID() = cur_skin.item_id;
            item_view->m_iItemIDHigh() = cur_skin.item_high_id;
            item_view->m_iItemIDLow() = cur_skin.item_low_id;
            item_view->m_iAccountID() = uint32_t(inventory->GetOwner().m_id);
            item_view->m_iItemDefinitionIndex() = cur_skin.item_def_id;

            // save up current desired paint kit
            auto paint_kit = sdk::m_client->GetEconItemSystem()->GetEconItemSchema()->GetPaintKits().FindByKey(paint_kit_index);
            const bool oldModel = paint_kit.has_value() && paint_kit.value()->UsesOldModel();
            uint64_t mesh_mask = 1 + static_cast<uint64_t>(oldModel);

            // set econ item paint kit 
            hooks::m_set_econ_item_attirubte_by_name(item_view, const_cast<char*>("set item texture prefab"), static_cast<float>(paint_kit_index));

            // set econ item mesh group mask 
            item_scene_node->SetMeshGroupMask(mesh_mask);

            // set econ item view model mesh group mask 
            auto view_model_idx = view_model->m_hWeapon();
            if (view_model_idx.index() == weapon->handle().index()) {
                auto view_model_scene = view_model->m_pGameSceneNode();
                if (view_model_scene)
                    view_model_scene->SetMeshGroupMask(mesh_mask);
            }

            if (full_update) {
                _log(LOG_WARNING) << "[fsn] weapon full update composite";
                weapon->UpdateModel(true);
                weapon->UpdateComposite(true);
                weapon->UpdateCompositeSec(true);
                full_update = false;
            }

         
        }
    }

     if (update) {
         _log(LOG_WARNING) << "[weapon] update composite";

        hooks::m_regenerate_weapon_skin();

        update  = false;
     }

}
bool CCSInventoryManager::EquipItemInLoadout(int iTeam, int iSlot, uint64_t iItemID) {

    return hooks::m_equip_item_on_loadout_call(this , iTeam, iSlot, iItemID);
}
#include "..\..\utils\utils.h"

void Items::c_skins::remove()
{
    if (m_added_items.empty())
        return;

    CCSPlayerInventory* p_inventory = CCSPlayerInventory::GetInstance();
    if (!p_inventory) return;

    for (uint64_t id : m_added_items) {
        auto soc = p_inventory->GetSOCDataForItem(id);
        if (soc)
            p_inventory->RemoveEconItem(soc);
    }
}

void Items::c_skins::add(CEconItem* pItem)
{
    m_added_items.push_back(pItem->m_ulID);
}
static int old_knife_skin_id = 0;
static int old_knife_slot = 0;
static int old_knife_id = 0;
static int old_knife_team = 0;

void Items::c_skins::equip_item(int team, int slot, uint64_t itemID)
{

    auto it = std::find(m_added_items.begin(), m_added_items.end(), itemID);
    if (it == m_added_items.end()) {
        _log(LOG_ERROR) << "[equip] attempted to equip item that has not been added to list yet:" << itemID;
        return;
    }   
    
    CCSInventoryManager* p_inventory_manager = CCSInventoryManager::GetInstance();
    if (!p_inventory_manager) return;

    CCSPlayerInventory* p_inventory = CCSPlayerInventory::GetInstance();
    if (!p_inventory) return;

    C_EconItemView* pItemViewToEquip = p_inventory->GetItemViewForItem(*it);
    if (!pItemViewToEquip) return;

    C_EconItemView* pItemInLoadout = p_inventory->GetItemInLoadout(team, slot);
    if (!pItemInLoadout) return;


    CEconItemDefinition* toEquipData = pItemViewToEquip->GetStaticData();
    if (!toEquipData)
        return;
    CEconItemDefinition* pItemInLoadoutStaticData = pItemInLoadout->GetStaticData();
    if (!pItemInLoadoutStaticData)
        return;
    _log(LOG_INFO) << "[soc] called on item:" << *it << " original_id: " << itemID << " team:" << team << " slot:" << slot;

    for (auto& skinlist : added_item_list) {
        if (skinlist.ulID == itemID ) {
            skinlist.slot = slot;
            // if slot != 1 means it has been added on a prev team and adding again so ensure we notify cfg its index 4 ( both teams )
            if (skinlist.equiped) {
                skinlist.team = 4;
               
            }
            else
                skinlist.team = team;

            skinlist.equiped = true;
            _log(LOG_INFO) << "found valid item to set slot";
        }
    }

    if (toEquipData->IsWeapon() && !toEquipData->IsKnife(false, pItemInLoadoutStaticData->m_pszItemTypeName) && !toEquipData->IsGlove(false, pItemInLoadoutStaticData->m_pszItemTypeName)) {
        // this is just a ghetto way of duplicating SocData for unk items
        const uint64_t defaultItemID = (std::uint64_t(0xF) << 60) | pItemViewToEquip->m_iItemDefinitionIndex();
        p_inventory_manager->EquipItemInLoadout(team, slot, defaultItemID);
        CEconItem* pItemInLoadoutSOCData2 = nullptr; CEconItem* pItemInLoadoutSOCData = pItemInLoadout->GetSOCData(nullptr);
        if (!pItemInLoadoutSOCData) {
            p_inventory_manager->EquipItemInLoadout(team, slot, defaultItemID); pItemInLoadoutSOCData2 = pItemViewToEquip->GetSOCData(nullptr);
            if (pItemInLoadoutSOCData2) { pItemInLoadoutSOCData = pItemInLoadoutSOCData2; }
            else {  return; }
        }

        weapon_information weapon_info;
        weapon_info.item_id = pItemViewToEquip->m_iItemID();
        weapon_info.item_high_id = pItemViewToEquip->m_iItemIDHigh();
        weapon_info.item_low_id = pItemViewToEquip->m_iItemIDLow();
        weapon_info.item_def_id = pItemViewToEquip->GetStaticData()->m_nDefIndex;
        weapon_info.account_id = pItemViewToEquip->m_iAccountID();
        weapon_info.skin_id = pItemViewToEquip->GetCustomPaintKitIndex();
        

        m_weapon_skins.insert_or_assign(pItemViewToEquip->GetStaticData()->m_nDefIndex, weapon_info);
        _log(LOG_INFO) << "[m_weapon_skins] added idx: " << pItemViewToEquip->m_iItemDefinitionIndex();

        p_inventory->SOUpdated(p_inventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
        p_inventory->SOUpdateInventory();
        update = true;


    }
    else if (toEquipData->IsKnife(false, pItemInLoadoutStaticData->m_pszItemTypeName) && !toEquipData->IsGlove(false, pItemInLoadoutStaticData->m_pszItemTypeName)) {
        // this is just a ghetto way of duplicating SocData for unk items
         // this is just a ghetto way of duplicating SocData for unk items
        const uint64_t defaultItemID = (std::uint64_t(0xF) << 60) | pItemViewToEquip->m_iItemDefinitionIndex();
        p_inventory_manager->EquipItemInLoadout(team, slot, defaultItemID);
        CEconItem* pItemInLoadoutSOCData2 = nullptr; CEconItem* pItemInLoadoutSOCData = pItemInLoadout->GetSOCData(nullptr);
        if (!pItemInLoadoutSOCData) {
            p_inventory_manager->EquipItemInLoadout(team, slot, defaultItemID); pItemInLoadoutSOCData2 = pItemViewToEquip->GetSOCData(nullptr);
            if (pItemInLoadoutSOCData2) { pItemInLoadoutSOCData = pItemInLoadoutSOCData2; }
            else {  return; }
        }
        
        m_added_knife.item_id = pItemViewToEquip->m_iItemID();
        m_added_knife.item_high_id = pItemViewToEquip->m_iItemIDHigh();
        m_added_knife.item_low_id = pItemViewToEquip->m_iItemIDLow();
        m_added_knife.item_def_id = pItemViewToEquip->GetStaticData()->m_nDefIndex;
        m_added_knife.account_id = pItemViewToEquip->m_iAccountID();
        m_added_knife.skin_id = pItemViewToEquip->GetCustomPaintKitIndex();
        m_added_knife.model_name = pItemViewToEquip->GetStaticData()->GetModelName();
        m_added_knife.name = pItemViewToEquip->GetStaticData()->GetSimpleWeaponName();
        // update old skin id 
        {
            old_knife_skin_id = m_added_knife.skin_id;
            old_knife_id = pItemViewToEquip->m_iItemID();
            old_knife_slot = slot;
            old_knife_team = team;
        }

    

        _log(LOG_INFO) << "[soc] updated knife: " << m_added_knife.item_id << " highid: " << m_added_knife.item_high_id << " defid: " << m_added_knife.item_def_id << " skinid:" << m_added_knife.skin_id;

        added_knife_to_inventory = true;
        p_inventory->SOUpdated(p_inventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
        p_inventory->SOUpdateInventory();
        update_knife = true;
      
    }
    else  if (toEquipData->IsGlove(false, pItemInLoadoutStaticData->m_pszItemTypeName)) {
        // this is just a ghetto way of duplicating SocData for unk items
        const uint64_t defaultItemID = (std::uint64_t(0xF) << 60) | pItemViewToEquip->m_iItemDefinitionIndex();
        p_inventory_manager->EquipItemInLoadout(team, slot, defaultItemID);
        CEconItem* pItemInLoadoutSOCData2 = nullptr; CEconItem* pItemInLoadoutSOCData = pItemInLoadout->GetSOCData(nullptr);
        if (!pItemInLoadoutSOCData) {
            p_inventory_manager->EquipItemInLoadout(team, slot, defaultItemID); pItemInLoadoutSOCData2 = pItemViewToEquip->GetSOCData(nullptr);
            if (pItemInLoadoutSOCData2) { pItemInLoadoutSOCData = pItemInLoadoutSOCData2; }
            else { return; }
        }

        Items::gloves->m_added_gloves.item_id = pItemViewToEquip->m_iItemID();
        Items::gloves->m_added_gloves.item_high_id = pItemViewToEquip->m_iItemIDHigh();
        Items::gloves->m_added_gloves.item_low_id = pItemViewToEquip->m_iItemIDLow();
        Items::gloves->m_added_gloves.item_def_id = pItemViewToEquip->m_iItemDefinitionIndex();
        update_gloves = true;


    }
}

void Items::c_skins::set_model(C_BaseModelEntity* pEntity, const char*& model)
{

    if (!sdk::m_engine->is_valid())
        return;

    auto p_local_pawn = hooks::m_get_local_player(0);
    if (!p_local_pawn)
        return;

    if (!p_local_pawn->IsAlive() || !p_local_pawn->GetActiveWeapon())
        return;

    if (!pEntity || !pEntity->IsViewModelWeapon())
        return;

    C_BaseViewModel* p_view_model = (C_BaseViewModel*)pEntity;
    if (!p_view_model)
        return;

    CCSPlayerInventory* p_inventory = CCSPlayerInventory::GetInstance();
    if (!p_inventory) return;

    const uint64_t steam_id = p_inventory->GetOwner().m_id;

    auto p_weapon = sdk::m_game->pGameEntitySystem->Get<CCSWeaponBase>(p_view_model->m_hWeapon());
    if (!p_weapon) return;

    auto weapon = p_weapon;
    // make sure weapon's valid
    if (weapon->GetOriginalOwnerXuid() != steam_id || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_GRENADE || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_C4 || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_EQUIPMENT
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_STACKABLEITEM
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_FISTS
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_BREACHCHARGE
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_BUMPMINE
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_TABLET
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_SHIELD
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_ZONE_REPULSOR
        || weapon->GetWeaponData()->m_WeaponType() == WEAPONTYPE_TASER || weapon->GetWeaponData()->m_WeaponType() != WEAPONTYPE_KNIFE) {
        return;
    }

    CAttributeManager* p_attribute_container = &p_weapon->m_AttributeManager();
    if (!p_attribute_container) return;

    C_EconItemView* p_weapon_item_view = &p_attribute_container->m_Item();
    if (!p_weapon_item_view) return;

    CEconItemDefinition* p_weapon_definition = p_weapon_item_view->GetStaticData();
    if (!p_weapon_definition) return;

    C_EconItemView* p_weapon_in_loadout_item_view = p_inventory->GetItemInLoadout(
        p_weapon->m_iOriginalTeamNumber(), p_weapon_definition->GetLoadoutSlot());
    if (!p_weapon_in_loadout_item_view) return;

    auto it = std::find(m_added_items.cbegin(), m_added_items.cend(), p_weapon_in_loadout_item_view->m_iItemID());
    if (it == m_added_items.cend()) return;

    CEconItemDefinition* p_weapon_in_loadout_definition = p_weapon_in_loadout_item_view->GetStaticData();

    if (!p_weapon_in_loadout_definition || !p_weapon_in_loadout_definition->IsKnife(false, "")) return;

   
    //_log(LOG_INFO) << "[model] set model on item: " << p_weapon_in_loadout_definition->GetModelName();
    model = p_weapon_in_loadout_definition->GetModelName();


}

void invalidate_glove_material(C_BaseViewModel* viewmodel) {
    if (!viewmodel)
        return;

    Items::c_gloves::material* p_mat_info = reinterpret_cast<Items::c_gloves::material*>(reinterpret_cast<uint8_t*>(viewmodel) + 0x1030);

    for (uint32_t i = 0; i < p_mat_info->ui32_count; i++) {
        if (p_mat_info->p_mat_records[i].identifer == Items::c_gloves::material_identifier::material_magic_number__gloves) {
           p_mat_info->p_mat_records[i].ui32_type_index = 0xffffffff;
           break;
        }
    }
}
void Items::c_agent_model::set_agent(CCSPlayerInventory* pInventory, CCSPlayerController* pLocalPawn)
{
}

void Items::c_gloves::glove(CCSPlayerInventory* p_inventory, CCSPlayerPawn* p_local_pawn, C_BaseViewModel* p_view_model, C_EconItemView* gloves_item, CEconItemDefinition* gloves_definition)
{
    if (!p_local_pawn) return;

    if (!p_local_pawn->IsAlive())
        return;

    if (!p_view_model)
        return;

    if (!gloves_item)
        return;

    if (!gloves_definition) return;

    if (m_added_gloves.item_id == 0)
        return;

    auto scene_node = p_view_model->m_pGameSceneNode();
    if (!scene_node)
        return;

    if (gloves_item->m_iItemID() != m_added_gloves.item_id || update_gloves) {
        glove_frame = 10;
        gloves_item->m_bDisallowSOC() = false;

        gloves_item->m_iItemID() = m_added_gloves.item_id;
        gloves_item->m_iItemIDHigh() = m_added_gloves.item_high_id;
        gloves_item->m_iItemIDLow() = m_added_gloves.item_low_id;
        gloves_item->m_iAccountID() = uint32_t(p_inventory->GetOwner().m_id);
        gloves_item->m_iItemDefinitionIndex() = m_added_gloves.item_def_id;

        //scene_node->SetMeshGroupMask(1);
        if (update_gloves)
            update_gloves = false;
    }

    if (glove_frame > 0) {
        invalidate_glove_material(p_view_model);

        gloves_item->m_bInitialized() = true;
        p_local_pawn->m_bNeedToReApplyGloves() = true;

        glove_frame--;
    }
}
#include "..\..\utils\render\d3d11\render_manager.h"

std::vector<gui_skins> skins_preview;
void Items::c_skins::create_texture(int image_width, int image_height,
    unsigned char* image_data,
    ID3D11ShaderResourceView** out_srv) {
    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    sdk::m_device->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    sdk::m_device->CreateShaderResourceView(pTexture, &srvDesc,
        out_srv);
    pTexture->Release();
}
// Helper function to check if a glove type already exists in the vector
bool glove_type_exists(const std::vector<gui_skins>& vec, int item_def) {
    return std::any_of(vec.begin(), vec.end(), [item_def](const gui_skins& skin) {
        return skin.item_def == item_def;
        });
}
#include <fstream>
#include "..\..\utils\render\ui\menu.h"
void Items::c_skins::parse()
{
    CEconItemSchema* pItemSchema = sdk::m_client->GetEconItemSystem()->GetEconItemSchema();
    if (!pItemSchema)
        return;
    auto& items = pItemSchema->GetSortedItemDefinitionMap();
    auto& paint_kits = pItemSchema->GetPaintKits();
    auto& skin_image_path = pItemSchema->GetAlternateIconsMap();
    auto& key_chains = pItemSchema->GetKeychains();

    items_list.reserve(80000);
    item_weapon.reserve(80000);
    skins_preview.reserve(80000);
    item_glove.reserve(80000);
    item_knife_default.reserve(80000);
    item_stickers.reserve(7000);
    item_medals.reserve(7000);
    auto& sticker_kits = pItemSchema->GetStickers();
    // 49 8D AD (? ? ? ?) 32 DB 44 89 4C 24 ? 41 BE ? ? ? ? 41 3B C6 74 70

    // Iterate over each item in the sorted items map
    for (const auto& it : items) {
        CEconItemDefinition* pItem = it.m_value;
        if (!pItem) continue;


        // Some items don't have names.
        const char* itemBaseName = pItem->m_pszItemTypeName;
        if (!itemBaseName || itemBaseName[0] == '\0') continue;

        if (strstr(itemBaseName, xorstr_("#CSGO_CollectibleCoin")) != nullptr)
            continue;

        const char* name = pItem->m_pszItemBaseName;
        if (!name || name[0] == '\0') continue;

        std::string medal_name = sdk::m_localize->FindSafe(name);

        gui_stickers temp_medal{};
        temp_medal.item_def = pItem->m_nDefIndex;
        temp_medal.name = medal_name;
        temp_medal.path_image = "";
        temp_medal.rarity = pItem->m_nItemRarity;
        temp_medal.image = nullptr;

        item_medals.emplace_back(temp_medal);
    }
    
    /*
    for (auto keychains : key_chains)
    {
        int type_ = 0;

        std::string name = sdk::m_localize->FindSafe(keychains.m_value->m_strName);
        std::string path_large = tfm::format(xorstr_("panorama/images/%s_png.vtex_c"), keychains.m_value->m_strInventoryImage);

        _log(LOG_INFO) << "adding keychains as default : " << path_large.c_str() << " img:" << keychains.m_value->m_strInventoryImage;

        auto vtex = VTexParser::Load(path_large.c_str());

      

    
        gui_skins temp_keychain{};
        temp_keychain.name = name;
        temp_keychain.id = keychains.m_value->m_nID;
        temp_keychain.path_image = path_large.c_str();
        temp_keychain.rarity = 4;
        temp_keychain.type = 5; // type_sticker
        temp_keychain.is_initilization = true;

        
        if (  uniqueTypes.find(5) == uniqueTypes.end()) {
            
            _log(LOG_INFO) << "adding keychains as default : " << temp_keychain.path_image.c_str() << " size:" << vtex.data.size();
            items_list.emplace_back(temp_keychain);
            uniqueTypes.insert(5);
        }

        item_key_chains.emplace_back(temp_keychain);
    }

    for (auto sticker : sticker_kits)
    {
        int type_ = 0;

        if (!strcmp(sticker.m_value->item_name, xorstr_("#StickerKit_Default")))
            continue;

        if (item_stickers.size() > 300)
            break;
        std::string name = sdk::m_localize->FindSafe(sticker.m_value->item_name);

        std::string path_large = tfm::format(xorstr_("panorama/images/%s_png.vtex_c"), sticker.m_value->inventory_image);
        auto vtex = VTexParser::Load(path_large.c_str());

        gui_skins temp_sticker{};
        temp_sticker.item_def = sticker.m_value->id;

        temp_sticker.name = name;
        temp_sticker.path_image = path_large.c_str();
        temp_sticker.rarity = sticker.m_value->rarity;
        temp_sticker.type = 4; // type_sticker
        temp_sticker.is_initilization = true;


        if (!vtex.data.empty() && uniqueTypes.find(type_item::type_sticker) == uniqueTypes.end()) {

            create_texture(vtex.w, vtex.h, reinterpret_cast<unsigned char*>(vtex.data.data()), &temp_sticker.texture_id);
            _log(LOG_INFO) << "adding image : " << temp_sticker.path_image.c_str();



            _log(LOG_INFO) << "adding sticker as default : " << temp_sticker.path_image.c_str();
            items_list.emplace_back(temp_sticker);
            uniqueTypes.insert(type_item::type_sticker);
        }

        item_stickers.emplace_back(temp_sticker);
    }
   
    _log(LOG_INFO) << items_list.size();
    */


    for (auto item : items)
    {
        if (!item.m_value->IsGlove(false, ""))
            continue;

        gui_skins temp_item{};
        temp_item.item_def = item.m_value->m_nDefIndex;
        temp_item.paint_name = "";
        temp_item.name = item.m_value->m_pszItemBaseName;
        temp_item.skin_key = 0;
        temp_item.paint_id = -1;
        temp_item.rarity = item.m_value->m_nItemRarity;
        ID3D11ShaderResourceView* texture = nullptr;

        for (const auto& it : paint_kits) {
            auto pPaintKit = it.m_value;
            if (!pPaintKit)
                continue;

            const uint64_t skinKey = Helper_GetAlternateIconKeyForWeaponPaintWearItem(temp_item.item_def, pPaintKit->PaintKitId(), 0);

            if (skin_image_path.FindByKey(skinKey)) {

                std::string skin_path = tfm::format(xorstr_("panorama/images/%s_png.vtex_c"), skin_image_path.FindByKey(skinKey).value().path_image);
                auto skin_vtex = VTexParser::Load(skin_path.c_str());

                if (skin_path.find(xorstr_("newcs2")) != std::string::npos)
                    continue;
                c_paintKits dumped_skin;
                dumped_skin.name = sdk::m_localize->FindSafe(pPaintKit->PaintKitDescriptionTag());
                dumped_skin.id = pPaintKit->PaintKitId();
                dumped_skin.skin_key = skinKey;
                dumped_skin.rarity = pPaintKit->PaintKitRarity();
                dumped_skin.image = nullptr;
                dumped_skin.path_image = skin_path.c_str();
                create_texture(skin_vtex.w, skin_vtex.h, reinterpret_cast<unsigned char*>(skin_vtex.data.data()), &dumped_skin.image);
                texture = dumped_skin.image;
                temp_item.paintkits.reserve(15000);
                temp_item.paintkits.emplace_back(dumped_skin);
            }
        }

        if (!temp_item.paintkits.empty()) {
            std::sort(temp_item.paintkits.begin(), temp_item.paintkits.end(), [](const c_paintKits& a, const c_paintKits& b) {
                return a.rarity > b.rarity;
                });

        }
        temp_item.texture_id = texture;

        item_glove.emplace_back(temp_item);

    }

    // Copy only unique glove types to item_glove_default
    for (const auto& glove : item_glove) {
        if (!glove_type_exists(item_glove_default, glove.item_def)) {
            item_glove_default.emplace_back(glove);
        }
    }

    for (auto item : items)
    {
        if (!item.m_value->IsCustomPlayer() && !item.m_value->IsKnife(true, "") && !item.m_value->IsWeapon() && !item.m_value->IsGlove(false, "") )
            continue;

        if (!item.m_value->path_image)
            continue;

        std::string path = tfm::format(xorstr_("panorama/images/%s_png.vtex_c"), item.m_value->path_image);
        int item_def = item.m_value->m_nDefIndex;
        std::string name = sdk::m_localize->FindSafe(item.m_value->m_pszItemBaseName);
        auto vtex = VTexParser::Load(path.c_str());

        int rarity = -1;

        if (item.m_value->IsCustomPlayer())
            rarity = item.m_value->m_nItemRarity;

        gui_skins temp_item{};
        temp_item.item_def = item_def;
        temp_item.paint_name = "";
        temp_item.name = name;
        temp_item.skin_key = 0;
        temp_item.paint_id = -1;
        temp_item.rarity = rarity;
        
        create_texture(vtex.w, vtex.h, reinterpret_cast<unsigned char*>(vtex.data.data()), &temp_item.texture_id);

        int type_ = 0;

        if (item.m_value->IsCustomPlayer())
            type_ = type_item::type_agent;
        else if (item.m_value->IsKnife(true, ""))
            type_ = type_item::type_knife;
        else if (item.m_value->IsWeapon())
            type_ = type_item::type_weapons;
        else if (item.m_value->IsGlove(false, ""))
            type_ = type_item::type_glove;



        temp_item.type = type_;
        temp_item.path_image = path;
        temp_item.is_initilization = true;

        // Add to items_list if not already added
        if (type_ == type_item::type_weapons && uniqueTypes.find(type_item::type_weapons) == uniqueTypes.end()) {
            items_list.emplace_back(temp_item);
            uniqueTypes.insert(type_item::type_weapons);
        }
        if (type_ == type_item::type_knife && uniqueTypes.find(type_item::type_knife) == uniqueTypes.end()) {
            items_list.emplace_back(temp_item);
            uniqueTypes.insert(type_item::type_knife);
        }
        if (type_ == type_item::type_glove && uniqueTypes.find(type_item::type_glove) == uniqueTypes.end()) {
            items_list.emplace_back(temp_item);
            uniqueTypes.insert(type_item::type_glove);
        }




        for (const auto& it : paint_kits) {
            auto pPaintKit = it.m_value;
            if (!pPaintKit)
                continue;

            const uint64_t skinKey = Helper_GetAlternateIconKeyForWeaponPaintWearItem(item_def, pPaintKit->id, 0);

            if (skin_image_path.FindByKey(skinKey)) {

                std::string skin_path = tfm::format(xorstr_("panorama/images/%s_png.vtex_c"), skin_image_path.FindByKey(skinKey).value().path_image);
                // auto skin_vtex = VTexParser::Load(skin_path.c_str());

                if (skin_path.find(xorstr_("newcs2")) != std::string::npos)
                    continue;

                c_paintKits dumped_skin;
                dumped_skin.name = sdk::m_localize->FindSafe(pPaintKit->PaintKitDescriptionTag());
                dumped_skin.id = pPaintKit->id;
                dumped_skin.skin_key = skinKey;
                dumped_skin.rarity = pPaintKit->PaintKitRarity();
                dumped_skin.image = nullptr;
                dumped_skin.path_image = skin_path.c_str();
                //  create_texture(skin_vtex.w, skin_vtex.h, reinterpret_cast<unsigned char*>(skin_vtex.data.data()), &dumped_skin.image);
                temp_item.paintkits.reserve(15000);
                temp_item.paintkits.emplace_back(dumped_skin);

            }
        }

        if (!temp_item.paintkits.empty()) {
            std::sort(temp_item.paintkits.begin(), temp_item.paintkits.end(), [](const c_paintKits& a, const c_paintKits& b) {
                return a.rarity > b.rarity;
                });

        }

        if (item.m_value->IsGlove(true, ""))
        {
            item_glove.emplace_back(temp_item);
        }

        skins_preview.emplace_back(temp_item);
    }



    std::copy_if(skins_preview.begin(), skins_preview.end(), std::back_inserter(item_weapon), [](gui_skins skin) {
        return skin.type == type_item::type_weapons;
        });
    std::copy_if(skins_preview.begin(), skins_preview.end(), std::back_inserter(item_default), [](gui_skins skin) {
        return skin.paint_id == -1 && skin.type == type_item::type_weapons;
        });
    std::copy_if(skins_preview.begin(), skins_preview.end(), std::back_inserter(item_knife_default), [](gui_skins skin) {
        return skin.paint_id == -1 && skin.type == type_item::type_knife;
        });

    std::copy_if(skins_preview.begin(), skins_preview.end(), std::back_inserter(item_agent), [](gui_skins skin) {
        return skin.type == type_item::type_agent;
        });
    std::copy_if(skins_preview.begin(), skins_preview.end(), std::back_inserter(item_glove), [](gui_skins skin) {
        return skin.type == type_item::type_glove;
        });

    for (auto& keychain : key_chains)
    {
        std::string path = tfm::format(xorstr_("panorama/images/%s_png.vtex_c"), keychain.m_value->m_strInventoryImage);
        std::string name = sdk::m_localize->FindSafe(keychain.m_value->m_strNameLocToken);

        //auto skin_vtex = VTexParser::Load(path.c_str());

        gui_skins temp_keychain{};
        temp_keychain.item_def = keychain.m_value->m_nID;
        temp_keychain.rarity = keychain.m_value->m_nRarity;
        temp_keychain.name = name;

        temp_keychain.texture_id = nullptr;
        /*     create_texture(skin_vtex.w, skin_vtex.h, reinterpret_cast<unsigned char*>(skin_vtex.data.data()), &temp_keychain.image);*/

        temp_keychain.path_image = path;
        temp_keychain.type = type_keychains;

     
        item_key_chains.emplace_back(temp_keychain);
    }

    for (auto& sticker : sticker_kits)
    {
        std::string path = tfm::format(xorstr_("panorama/images/%s_png.vtex_c"), sticker.m_value->inventory_image);
        std::string name = sdk::m_localize->FindSafe(sticker.m_value->name);

        gui_skins temp_sticker{};
        temp_sticker.item_def = sticker.m_value->id;
        temp_sticker.rarity = sticker.m_value->rarity;
        temp_sticker.name = name;
        temp_sticker.texture_id = nullptr;

        temp_sticker.path_image = path;
        temp_sticker.type = type_sticker;

 

        item_stickers.emplace_back(temp_sticker);
    }
  

    _log(LOG_INFO) << "[skins] loaded all addons images";

    /*
    L_PRINT(LOG_INFO) << "item_default size:" << item_default.size();
    L_PRINT(LOG_INFO) << "item_weapon size:" << item_weapon.size();
    L_PRINT(LOG_INFO) << "item_knife_default size:" << item_knife_default.size();
    L_PRINT(LOG_INFO) << "skins_preview size:" << skins_preview.size();
    */
}
void Items::c_events::kill_feed(IGameEvent* pEvent)
{
}

void Items::c_events::round_start(IGameEvent* pEvent)
{
}

void Items::c_events::round_reset(IGameEvent* pEvent)
{
}
