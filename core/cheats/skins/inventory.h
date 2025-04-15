#pragma once
#include "..\..\utils\utils.h"

class CCSPlayerInventory;

class CCSInventoryManager {
public:
    static CCSInventoryManager* GetInstance();

    bool EquipItemInLoadout(int iTeam, int iSlot, uint64_t iItemID);
    auto GetLocalInventory() {
        return utils::CallVFunc<CCSPlayerInventory*, 63u>(this);
    }

};
