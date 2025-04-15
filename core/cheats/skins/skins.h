#pragma once

#include "..\..\utils\render\render.h"

#include "..\..\utils\utils.h"
#include "..\..\sdk\sdk.h"
#include "..\..\math\elements\qangle.h"
#include "..\..\math\elements\vector.h"
#include "..\..\math\elements\transform.h"

#include "..\..\utils\configs\config.h"
#include "..\..\..\deps\imgui\imgui.h"
#include "..\..\..\deps\imgui\imgui_internal.h"
#include <d3d11.h>
#include <unordered_set>

class IGameEvent;
class CEconItem;
class C_BaseModelEntity;
class CCSPlayerInventory;
class CCSPlayerPawn;
class CCSPlayerController;
class CEconItemDefinition;
class C_BaseViewModel;
class C_EconItemView;
enum type_item : int
{
    type_weapons,
    type_agent,
    type_glove,
    type_knife,
    type_sticker,
    type_keychains
};

struct gui_stickers {
    int item_def;
    std::string name;
    ID3D11ShaderResourceView* image;
    std::string path_image;
    int rarity;


};
struct c_paintKits {
    int id;
    int rarity;
    const char* name;
    uint64_t skin_key;
    std::string path_image;
    bool is_initilization;
    ID3D11ShaderResourceView* image = nullptr;
    int item_id;
};

struct gui_skins {
    int item_def;
    std::string paint_name;
    std::string name;
    uint64_t skin_key;
    int paint_id;
    int rarity;
    ID3D11ShaderResourceView* texture_id;
    int type;
    std::string path_image;
    bool is_initilization;
    int paint_seed;
     std::vector<c_paintKits> paintkits;
    int paint_wear;
    std::uint64_t id;

    c_paintKits* current_paint;
    std::vector<gui_stickers> stickers;
    std::unordered_map<int, gui_stickers> added_stickers;

};

struct config_skins {
    int item_def;
    std::string paint_name;
    std::string name;
    int paint_id;
    int rarity;
    
    int paint_seed;
    int paint_wear;
    std::uint64_t item_id;

    std::vector<c_paintKits> paint_kits;
};

namespace Items
{

  

    class c_events {
    public:
        void kill_feed(IGameEvent* pEvent);
        void round_start(IGameEvent* pEvent);
        void round_reset(IGameEvent* pEvent);

    };

    class c_gloves {
    public:
        enum material_identifier : uint32_t
        {
            material_magic_number__gloves = 0xF143B82A,
            material_magic_number__unknown1 = 0x1b52829c,
            material_magic_number__unknown2 = 0xa6ebe9b9,
            material_magic_number__unknown3 = 0x423b2ed4,
            material_magic_number__unknown4 = 0xc8d7255e
        };
        struct material_record
        {
            pad(0x4);
            material_identifier  	identifer;
            uint32_t					ui32_handle;
            pad(0x4);
            uint32_t					ui32_type_index;
        };
 
        struct glove_information {
            int item_id;
            uint64_t item_high_id;
            uint64_t item_low_id;
            int item_def_id;
        }m_added_gloves;

        struct material {
            material_record* p_mat_records;
            uint32_t ui32_count;
        };

        int glove_frame = 0;

        void glove(CCSPlayerInventory* pInventory, CCSPlayerPawn* pLocalPawn, C_BaseViewModel* pViewModel, C_EconItemView* GlovesItem, CEconItemDefinition* GlovesDefinition);

    };
    const auto gloves = std::make_unique<c_gloves>();

    class c_agent_model {
    public:
    
        struct agent_info {
            uint64_t id;
            const char* name;
            const char* mdl_name;
        };

        void set_agent(CCSPlayerInventory* pInventory, CCSPlayerController* pLocalPawn);
    };
    const auto agent = std::make_unique<c_agent_model>();




	class c_skins {
	public:

        void parse();
        void fsn(int stage);
        void on_knife(int stage);
        void on_weapon(void* a2);
        void on_glove(int stage);

        void remove();
        void add(CEconItem* pItem);
        void equip_item(int team, int slot, uint64_t itemID);
        void set_model(C_BaseModelEntity* pEntity, const char*& model);
        void create_texture(int image_width, int image_height,
            unsigned char* image_data,
            ID3D11ShaderResourceView** out_srv);
        void parse_paintkits(int defidx);
	};
	const auto skins = std::make_unique<c_skins>();

}

struct knife_information {
    int skin_id;
    uint64_t item_id;
    uint32_t account_id;
    uint64_t item_def_id;
    uint32_t item_low_id;
    uint32_t item_high_id;
    const char* name;
    const char* model_name;
};
struct weapon_information {
    uint64_t item_id;
    uint32_t item_high_id;
    uint32_t item_low_id;
    uint64_t item_def_id;
    uint32_t account_id;
    int skin_id;
};
extern std::unordered_map<int, weapon_information>  m_weapon_skins;
extern knife_information m_added_knife;

extern bool update;
extern bool update_gloves;
extern bool update_knife;
extern bool full_update;
extern int update_stlye;
extern int weapons_update_stlye;
extern  std::vector<gui_skins> skins_preview;
extern std::vector<gui_skins> item_default;
extern std::vector<gui_skins> item_knife_default;
extern std::vector<gui_skins> item_agent;
extern std::vector<gui_skins> item_glove;
extern std::vector<gui_skins> skin_preview;
extern std::unordered_set<int> uniqueTypes;
extern std::vector<gui_skins> item_weapon;
extern std::vector<gui_skins> items_list;
extern std::vector<gui_skins> item_glove_default;
extern std::unordered_map<int ,std::vector< c_paintKits>> m_paint_kits;
extern std::vector<c_paintKits> added_skins;
extern std::vector<EconItem_t> added_item_list;
extern std::vector<gui_skins> item_stickers;
extern std::vector<gui_skins> item_stickers_default;
extern std::vector<gui_stickers> item_medals;
extern std::vector<gui_skins> item_key_chains;