#pragma once

// used: callvfunc
#include "..\..\utils\utils.h"

class CEconItemDefinition;
class CAttributeString
{
public:
	char pad[0x10];
	std::string* data2;
	std::string* data;
};

class CSchemaAttributeType_String
{

};

class CEconItemAttributeDefinition
{
public:
	char pad[0x18];
	CSchemaAttributeType_String* m_pSchemaAttributeType_String;
};
inline constexpr uint64_t Helper_GetAlternateIconKeyForWeaponPaintWearItem(
	uint16_t nDefIdx, uint32_t nPaintId, uint32_t nWear) {
	return (nDefIdx << 16) + (nPaintId << 2) + nWear;
}

struct AlternateIconData_t {
	const char* path_image;
	const char* large_path_image;

private:
	char pad0[0x8];  // no idea
	char pad1[0x8];  // no idea

};
class CPaintKit {
public:

	int id;
	const char* name;
	const char* description_string;
	const char* description_name;
	char pad0[0x8];  // no idea
	char pad1[0x8];  // no idea
	char pad2[0x8];  // no idea
	char pad3[0x8];  // no idea
	char pad4[0x4];  // no idea
	int rarity;

	int64_t PaintKitId() {
		return *reinterpret_cast<int64_t*>((uintptr_t)(this));
	}

	const char* PaintKitName() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x8);
	}

	const char* PaintKitDescriptionString() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x10);
	}

	const char* PaintKitDescriptionTag() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x18);
	}

	int32_t PaintKitRarity() {
		return *reinterpret_cast<int32_t*>((uintptr_t)(this) + 0x44);
	}

	bool UsesOldModel() {
		return *reinterpret_cast<bool*>((uintptr_t)(this) + 0xB2);
	}
};
#include "../../sdk/datatypes/cutlhash/utlmap.h"

class CSkiterKit {
public:
	int id;
	int rarity;
	const char* name;
	const char* description_name;
	const char* item_name;
	const char* material_path;
	const char* material_path_no_drips;
	const char* inventory_image;
	char pad[0x18];
	float rotate_start;
	float rotate_end;
	float scale_min;
	float scale_max;
	const char* path_image;
	void* unk;
	const char* path_image_large;
	char pad2[0x20];
};

struct CKeyChain
{
	int32_t  m_nID;
	int32_t  m_nRarity;
	int32_t  m_nDisplaySeed;
	const char* m_strName;
	const char* m_strNameLocToken;
	const char* m_strLocDescription;
	const char* m_strPedestalDisplayModel;
	const char* m_strInventoryImage;
	char gap38[0x40];
};

class CEconItemSchema {
public:
	CEconItemAttributeDefinition* get_attribute_definition_by_name(const char* name);

	auto GetAttributeDefinitionInterface(int iAttribIndex) {
		return utils::CallVFunc<void*, 27U>(this, iAttribIndex);
	}

	auto& GetSortedItemDefinitionMap() {
		return *reinterpret_cast<CUtlMap<int, CEconItemDefinition*>*>(
			(uintptr_t)(this) + 0x128);
	}

	auto& GetAlternateIconsMap() {
		return *reinterpret_cast<CUtlMap<uint64_t, AlternateIconData_t>*>(
			(uintptr_t)(this) + 0x278);
	}

	auto& GetPaintKits() {
		return *reinterpret_cast<CUtlMap<int, CPaintKit*>*>((uintptr_t)(this) +
			0x2F0);
	}
	auto& GetStickers() {
		return *reinterpret_cast<CUtlMap<int, CSkiterKit*>*>((uintptr_t)(this) + 0x318);
	}
	auto& GetKeychains() {
		return *reinterpret_cast<CUtlMap<int, CKeyChain*>*>((uintptr_t)(this) + 0x4D0);
	}
};

class CEconItemSystem {
public:
	auto GetEconItemSchema() {
		return *reinterpret_cast<CEconItemSchema**>((uintptr_t)(this) + 0x8);
	}
};
enum EClientFrameStage : int
{
	FRAME_UNDEFINED = -1,
	FRAME_START,
	// a network packet is being received
	FRAME_NET_UPDATE_START,
	// data has been received and we are going to start calling postdataupdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// data has been received and called postdataupdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// received all packets, we can now do interpolation, prediction, etc
	FRAME_NET_UPDATE_END,
	// start rendering the scene
	FRAME_RENDER_START = 9,
	// finished rendering the scene
	FRAME_RENDER_END,
	FRAME_NET_FULL_FRAME_UPDATE_ON_REMOVE
};

enum flow : int {
	FLOW_OUTGOING = 0,
	FLOW_INCOMING = 1,
};

class INetChannelInfo {
public:
	float latency(flow flow) {
		return utils::CallVFunc<int, 12U>(this, flow);
	}

	double get_network_latency()
	{
		return utils::CallVFunc<double, 10U>(this);
	}

	double get_engine_latency()
	{
		// xref: StartLagcompensation
		return utils::CallVFunc<double, 11>(this);
		
	}

	float get_avg_latency(int type)
	{
		// xref: %4.0f ms : %s\n
		return utils::CallVFunc<float, 15>(this, type);

	}
};

class CDebugOverlay
{
public:
	void add_box(const Vector_t& end, const Vector_t& mins, const Vector_t& maxs, const Vector_t& a3, Color_t clr)
	{
		utils::CallVFunc<CDebugOverlay*, 48U>(this, end, mins, maxs, a3, clr.r, clr.g, clr.b, clr.a, static_cast<double>(6.f));
	}

	void add_line(const Vector_t& vecOrigin, const Vector_t& vecDest, const Color_t& Color, const bool bNoDethTest, const double flDuration)
	{
		utils::CallVFunc<CDebugOverlay*, 12U>(this, vecOrigin, vecDest, Color, bNoDethTest, flDuration);
	}

};

class ISource2Client {
public:
	auto GetEconItemSystem() {
		return utils::CallVFunc<CEconItemSystem*, 123U>(this);
	}
	auto GetDebugOverlay() {
		return utils::CallVFunc<CDebugOverlay*, 163U>(this);
	}


};



class c_networked_client_info
{
	std::byte pad_001[0x4];

public:
	int m_render_tick;
	float m_render_tick_fraction;
	int m_player_tick_count;
	float m_player_tick_fraction;

private:
	std::byte pad_002[0x4];

public:
	struct
	{
	private:
		std::byte pad_022[0xC];

	public:
		Vector_t m_eye_pos;
	}*m_local_data;

private:
	std::byte pad_003[0x8];
};
class IEngineClient
{
public:


	int maxClients()
	{
		return utils::CallVFunc<int, 34U>(this);
	}

	bool in_game()
	{
		return utils::CallVFunc<bool, 35U>(this);
	}

	bool connected()
	{
		return utils::CallVFunc<bool, 36U>(this);
	}

	INetChannelInfo* get_net_channel_info(int split_screen_slot) {
		return utils::CallVFunc<INetChannelInfo*, 37U>(this, split_screen_slot);
	}
	//8B 0D ?? ?? ?? ?? 4C 8B C2 B8
	c_networked_client_info* get_networked_client_info()
	{
		c_networked_client_info client_info;
		utils::CallVFunc<c_networked_client_info*, 178U>(this, &client_info);
		return &client_info;
	}
	[[nodiscard]] const char* GetLevelName()
	{
		return utils::CallVFunc<const char*, 56U>(this);
	}

	[[nodiscard]] const char* GetLevelNameShort()
	{
		return utils::CallVFunc<const char*, 57U>(this);
	}

	[[nodiscard]] const char* GetProductVersionString()
	{
		return utils::CallVFunc<const char*, 81U>(this);
	}
	int get_local_player() {
		int nIndex = -1;
		utils::CallVFunc<void, 49U>(this, std::ref(nIndex), 0);
		return nIndex + 1;
	}
public:
	inline bool is_valid() {
		return connected() && in_game();
	}
	

};