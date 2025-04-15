#pragma once
#include <cstdint>
#include <utility>

#include "..\..\utils\utils.h"
#include "..\..\sdk\datatypes\cutlvectors\utlvector.h"

class CCSPlayerInventory {
public:
    static CCSPlayerInventory* GetInstance();

    auto SOCreated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent) {
        return utils::CallVFunc<void, 0u>(this, owner, pObject, eEvent);
    }

    auto SOUpdated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent) {
        return utils::CallVFunc<void, 1u>(this, owner, pObject, eEvent);

    }

    auto SODestroyed(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent) {
        return utils::CallVFunc<void, 2u>(this, owner, pObject);
    }

    auto GetItemInLoadout(int iClass, int iSlot) {
        return utils::CallVFunc<C_EconItemView*, 8u>(this, iClass, iSlot);
    }

    void SOUpdateInventory() {
        return utils::CallVFunc<void, 17u>(this);

    }
    bool AddEconItem(CEconItem* pItem);
    void RemoveEconItem(CEconItem* pItem);
    std::pair<uint64_t, uint32_t> GetHighestIDs();
    C_EconItemView* GetItemViewForItem(uint64_t itemID);
    CEconItem* GetSOCDataForItem(uint64_t itemID);

    auto GetOwner() {
        return *reinterpret_cast<SOID_t*>((uintptr_t)(this) + 0x10);
    }

    auto& GetItemVector() {
        return *reinterpret_cast<CUtlVector<C_EconItemView*>*>(
            (uintptr_t)(this) + 0x20);
    }
};
