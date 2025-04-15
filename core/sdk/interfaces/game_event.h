#pragma once
// used: cutllinkedlist
#include "..\..\utils\utils.h"
#include "..\handle\handle.hpp"
#include "../addresses/entity.h"
#include "..\sdk.h"
#include "..\datatypes\cutlstring\utlstringtoken.h"

using namespace utils;
#define STRINGTOKEN_MURMURHASH_SEED 0x31415926
uint32_t MurmurHash2(const void* key, int len, uint32_t seed);

struct EventHash_t
{
	EventHash_t(const char* szString);

	std::uint32_t m_uHash;
	const char* m_szString;
};

class IGameEvent
{
public:
	virtual ~IGameEvent() { }

	[[nodiscard]] const char* GetName() const
	{
		//   @ida: client.dll -> U8["48 8B CE FF 50 ? 48 8D 0D" + 0x5]   @XREF: "show_freeze_panel"
		return CallVFunc<const char*, 1U>(this);
	}

	[[nodiscard]] bool GetBool(const CUtlStringToken& keyToken, bool bDefault = false) const
	{
		return CallVFunc<bool, 6U>(this, &keyToken, bDefault);
	}



	[[nodiscard]] float GetInt(const char* szEvent,  int flDefault = 0) const
	{
		const EventHash_t uKeyHash = EventHash_t(szEvent);
		return CallVFunc<float, 7U>(this, uKeyHash, flDefault);
	}

	[[nodiscard]] std::uint64_t GetUint64(const CUtlStringToken& keyToken, std::uint64_t ullDefault = 0ULL) const
	{
		return CallVFunc<std::uint64_t, 8U>(this, &keyToken, ullDefault);
	}


	[[nodiscard]] float GetFloat(const char* szEvent, const float flDefault = 0.0f) const
	{
		//   @ida CGameEvent::GetFloat_Old(): client.dll -> ABS["E8 ? ? ? ? 0F 28 D8 89 5C 24 20" + 0x1]
		//   @ida: client.dll -> U8["4C 8B 60 ? 4D 8B C6" + 0x3] / sizeof(std::uintptr_t)   @XREF: "theta"
		const EventHash_t uKeyHash = EventHash_t(szEvent);
		return CallVFunc<float, 9U>(this, uKeyHash, flDefault);
	}

	[[nodiscard]] const char* GetString(const CUtlStringToken& keyToken, const char* szDefault = "") const
	{
		//   @ida: client.dll -> U8["48 8B 78 ? 48 8D 4D 80" + 0x3] / sizeof(std::uintptr_t)   @XREF: "weapon"
		return CallVFunc<const char*, 10U>(this, &keyToken, szDefault);
	}

	[[nodiscard]] const wchar_t* GetWString(const CUtlStringToken& keyToken, const wchar_t* wszDefault = L"") const
	{
		return CallVFunc<const wchar_t*, 11U>(this, &keyToken, wszDefault);
	}

	[[nodiscard]] const void* GetPtr(const CUtlStringToken& keyToken) const
	{
		return CallVFunc<const void*, 12U>(this, &keyToken);
	}

	int GetPlayerIndex(const CUtlStringToken& keyToken)
	{
		//   @ida: client.dll -> U8["4C 8B 70 ? 4C 89 7C 24" + 0x3] / sizeof(std::uintptr_t)   @XREF: "attacker", "assister", "userid"
		int nOutIndex;
		CallVFunc<void, 15U>(this, &nOutIndex, &keyToken);
		return nOutIndex;
	}


	// xref: client.dll & 4C 8B A8 80 00 00 00 ("killer")
	inline CCSPlayerController* get_player_controller(const std::string_view token_name) noexcept {
		CUtlStringToken token(token_name.data());
		return CallVFunc<CCSPlayerController*, 16U>(this, &token);

	}
	CCSPlayerPawn* GetPlayerPawn(const CUtlStringToken& keyToken)
	{
		//   @XREF: "_pawn"
		//   @ida CGameEvent::GetPlayerPawn(const char*): server.dll -> ABS["E8 ? ? ? ? 48 85 C0 74 0B 48 8B C8 E8 ? ? ? ? 4C 8B F0 41 8B 46 08" + 0x1]   @XREF: "userid"
		return CallVFunc<CCSPlayerPawn*, 17U>(this, &keyToken);
	}

	void SetBool(const CUtlStringToken& keyToken, const bool bValue)
	{
		//   @ida CGameEvent::SetBool(const char*, bool): server.dll -> ABS["E8 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 C0 48 8B D3 48 8B 01 FF 50 38 48 8B 46 10" + 0x1]   @XREF: "canbuy"
		CallVFunc<void, 20U>(this, &keyToken, bValue);
	}

	void SetInt(const CUtlStringToken& keyToken, const int nValue)
	{
		//   @ida: server.dll -> ["48 8D 4D A7 4C 8B B0 ? ? ? ? FF" + 0x7] / sizeof(std::uintptr_t)
		CallVFunc<void, 21U>(this, &keyToken, nValue);
	}

	void SetUint64(const CUtlStringToken& keyToken, const std::uint64_t ullValue)
	{
		CallVFunc<void, 22U>(this, &keyToken, ullValue);
	}

	void SetFloat(const CUtlStringToken& keyToken, const float flValue)
	{
		//   @ida: server.dll -> ["48 8B B0 ? ? ? ? 33 D2 44 89 6C 24" + 0x3] / sizeof(std::uintptr_t)   @XREF: "inferno_expire"
		CallVFunc<void, 23U>(this, &keyToken, flValue);
	}

	void SetString(const CUtlStringToken& keyToken, const char* szValue)
	{
		//   @ida: server.dll -> ["48 8D 4D A7 48 8B B8 ? ? ? ? 33" + 0x7] / sizeof(std::uintptr_t)   @XREF: "weapon"
		CallVFunc<void, 24U>(this, &keyToken, szValue);
	}

	void SetWString(const CUtlStringToken& keyToken, const wchar_t* wszValue)
	{
		CallVFunc<void, 25U>(this, &keyToken, wszValue);
	}

	void SetPtr(const CUtlStringToken& keyToken, const void* pValue)
	{
		CallVFunc<void, 26U>(this, &keyToken, pValue);
	}

	void SetPlayerIndex(const CUtlStringToken& keyToken, const int nIndex)
	{
		//   @ida: server.dll -> ["48 8B B8 ? ? ? ? E8 ? ? ? ? 48 8D 05 ? ? ? ? 44 8B C3 48 8D 55 97 48 89 45 9F 48 8B CE FF D7 48 8D 15" + 0x3] / sizeof(std::uintptr_t)   @XREF: "attacker_pawn"
		CallVFunc<void, 27U>(this, &keyToken, nIndex);
	}

	void SetPlayerController(const CUtlStringToken& keyToken, CCSPlayerController* pPlayerController)
	{
		CallVFunc<void, 28U>(this, &keyToken, pPlayerController);
	}

	void SetPlayerPawn(const CUtlStringToken& keyToken, CCSPlayerPawn* pPlayerPawn)
	{
		//   @XREF: "_pawn"
		CallVFunc<void, 29U>(this, &keyToken, pPlayerPawn);
	}

public:
	std::byte pad0[0x60]; // 0x08

	std::int64_t get_int(const std::string_view event_name) noexcept {
		// client.dll; 48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 30 48 8B 01 41 8B F0 4C 8B F1 41 B0 01 48 8D 4C 24 20 48 8B DA 48 8B 78
		using function_t = std::int64_t(__fastcall*)(void*, const char*);
		static function_t fn = reinterpret_cast<function_t>(FindPattern(xorstr_("client.dll"), "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 30 48 8B 01 41 8B F0 4C 8B F1 41 B0 01 48 8D 4C 24 20 48 8B DA 48 8B 78"));
		debug(fn != nullptr);
		return fn(this, event_name.data());
	}

	float get_float(const std::string_view event_name) noexcept {
		// client.dll; 48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 30 48 8B 01 41 8B F0 4C 8B F1 41 B0 01 48 8D 4C 24 20 48 8B DA 48 8B 78
		using function_t = std::int64_t(__fastcall*)(void*, const char*);
		static function_t fn = reinterpret_cast<function_t>(FindPattern(xorstr_("client.dll"), "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B 01 48 8B F1 0F 29 74 24 30 48 8D 4C 24 20 0F 28 F2 48 8B DA 48 8B B8"));
		debug(fn != nullptr);
		return static_cast<float>(fn(this, event_name.data()));
	}

	void* get_player_pawn_from_id(const std::string_view event_name) noexcept {
		// client.dll; 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 01 48 8B F1 41 B0 01 48 8D 4C 24 20 48 8B FA 48 8B 98
		using function_t = void* (__fastcall*)(void*, const char*, std::int64_t);
		static function_t fn = reinterpret_cast<function_t>(FindPattern(xorstr_("client.dll"), "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 01 48 8B F1 41 B0 01 48 8D 4C 24 20 48 8B FA 48 8B 98"));
		debug(fn != nullptr);
		return fn(this, event_name.data(), 0);
	}

	void* get_pointer_from_id(const std::string_view event_name) noexcept {
		// used: "userid", "attackerid"
		std::int64_t id{ };
		{
			// client.dll; 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 48 8B 01 49
			using function_t = std::int64_t(__fastcall*)(void*, std::int64_t*, const char*);
			static function_t fn = reinterpret_cast<function_t>(FindPattern(xorstr_("client.dll"), "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 01 48 8B F1 41 B0 01 48 8D 4C 24 20 48 8B FA 48 8B 98"));
			debug(fn != nullptr);
			fn(this, &id, event_name.data());
		}

		if (id == -1)
			return { };

		// xref: "player_disconnect"
		// client.dll; E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8B D8 48 85 C9
		using function_t2 = void* (__fastcall*)(std::int64_t);
		static function_t2 fn = reinterpret_cast<function_t2>(FindPattern(xorstr_("client.dll"), "E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8B D8 48 85 C9"));
		debug(fn != nullptr);

		return fn(id);
	}

};
static_assert(sizeof(IGameEvent) == 0x68);



// todo: you can use this instead of hooking like you did it in csgo (input ur listeners), but that's a task for the reader.
// client.dll; 48 89 05 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8D 8B E0 00 00 00
struct event_manager_t {
	void init_event( /*C_CSPlayerPawnBase*/ void* player_pawn_base,
		const std::string_view event_name, void* unk = nullptr) noexcept {
		// (*(*g_pGameEventManager + 24i64))(g_pGameEventManager, a1 + 4680, "round_end", 0i64);
		return CallVFunc< void, 6U>(this, player_pawn_base, event_name.data(), unk);
	}
};


class IGameEventListener {
public:
	virtual ~IGameEventListener() {}
	virtual void fire_game_event(IGameEvent* ev) = 0;
};

class IGameEventManager
{
public:
	enum
	{
		SERVERSIDE = 0, // this is a server side listener, event logger etc
		CLIENTSIDE, // this is a client side listenet, HUD element etc
		CLIENTSTUB, // this is a serverside stub for a remote client listener (used by engine only)
		SERVERSIDE_OLD, // legacy support for old server event listeners
		CLIENTSIDE_OLD // legacy support for old client event listeners
	};

	virtual ~IGameEventManager() { }
	/// load game event descriptions from a file e.g. "resource\gameevents.res"
	///  @Returns: count of loaded game events
	int load(const char* szFileName)
	{
		return CallVFunc<int, 1U>(this, szFileName);
	}

	// remove all and anything
	void reset()
	{
		CallVFunc<void, 2U>(this);
	}

	/// add a listener for a particular event
	///  @Returns: true if listener was successfully added for given event or was already there, false otherwise
	bool add(IGameEventListener* pListener, const char* szName, bool bServerSide = false)
	{
		return CallVFunc<bool, 3U>(this, pListener, szName, bServerSide);
	}

	///  @Returns: true if given listener is listens to given event, false otherwise
	bool find(IGameEventListener* pListener, const char* szName)
	{
		return CallVFunc<bool, 4U>(this, pListener, szName);
	}

	// remove a listener
	void remove(IGameEventListener* pListener)
	{
		CallVFunc<void, 5U>(this, pListener);
	}

};


class CEvent : public IGameEventListener
{
public:
	void Initialize();
	void Destroy();

	void fire_game_event(IGameEvent* pEvent);

	void bullet_impact(IGameEvent* event);

};

inline auto game_events = std::make_unique<CEvent>();
