#pragma once
#include "..\..\utils\utils.h"
class CNetworkStringTableContainer;
class ClassName
{
public:
	int64_t size; //0x0000
	class ClassData* class_data; //0x0008
}; //Size: 0x0010


class CNetworkGameClient
{
public:
	enum vtables
	{
		global_vars_ = 4,
		client_tick = 5,
		server_tick = 6,
		net_channel = 41,
		unk = 68,
		set_signon_state = 116,
		copy_new_entity = 123
	};
	CGlobalVarsBase* get_global_vars()
	{
		//xref: Curtime( %f )\nRealtime( %f )\n module: engine2
		return utils::CallVFunc<CGlobalVarsBase*, vtables::global_vars_>(this);
	}

	//# STR: "CL:  TransmitNetChannel"
	INetChannelInfo* get_net_channel()
	{
		return utils::CallVFunc<INetChannelInfo*, 59>(this, 0);
	}

	float get_client_interp() {
		return utils::CallVFunc<float, 61u>(this);
	}

	int get_client_tick()
	{
		return utils::CallVFunc<int, 5U>(this);
	}

	int get_server_tick()
	{
		return utils::CallVFunc<int, 6U>(this);
	}

	void get_prediction_tick(int* prediction_tick)
	{
		int unk{};
		return utils::CallVFunc<void, vtables::unk>(this, prediction_tick, &unk);
	}

	bool m_get_compression()
	{
		return *(bool*)(std::uintptr_t(this) + 0xF8);
	}
	void m_set_compression(bool value)
	{
		*(bool*)(std::uintptr_t(this) + 0xF8) = value;
	}
	void m_set_prediction(bool value)
	{
		*(bool*)(std::uintptr_t(this) + 0xDC) = value;
	}

	bool m_in_prediction()
	{
		return *(bool*)(std::uintptr_t(this) + 0xDC);
	}

	int m_LastCommandCreated()
	{
		return *(int*)(std::uintptr_t(this) + 0xD8);
	}
	//// #STR: "CL:  TransmitNetChannel"
	//    v3 = *(_QWORD *)(a1 + 24i64 * a2 + 0xE8);
	INetChannelInfo* get_net_channel2()
	{
		return *(INetChannelInfo**)(std::uintptr_t(this) + 0xE8);
	}

	float lerp()
	{
		return utils::CallVFunc<float, 62U>(this);
	}

	void set_lerp(float lerp)
	{
		*(float*)(std::uintptr_t(this) + 0x2855F0) = lerp;
	}

	int m_server_count()
	{
		return *(int*)(std::uintptr_t(this) + 0x258);
	}


	int m_command()
	{
		return *(int*)(std::uintptr_t(this) + 0x285E78);
	}

	void SetNetMsg(void* message_handle, void* message_data, int unk)
	{
		// xref: SetSignonState
		utils::CallVFunc<void, 68>( this, message_handle, message_data, unk);
	}

	bool IsConnected() {
		return utils::CallVFunc<bool, 13U>(this);
	}

	float GetClientInterpAmount(){
		return utils::CallVFunc<float, 62U>(this);
	}

	void FullUpdate(){
		utils::CallVFunc<void, 29U>(this, xorstr_("initial update"));
	}

	void Update() {
		int* deltaTickPtr = reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x244);
		*deltaTickPtr = -1;
	}

	int m_delta_tick(){
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x244);
    }
	int m_server_tick() {
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x36C);
	}

	void set_delta_tick(int delta_tick) {
		int* deltaTickPtr = reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x244);
		*deltaTickPtr = delta_tick;
	}

public:
	pad(0xE8); //0x0000
	INetChannelInfo* netchanell_info; // 0x00E8
	pad(0x8); // 0x00F0
	bool should_predict; // 0x00F8
	pad(0x7B); // 0x00F9
	int SomePrediction_tick; // 0x0174
	pad(0x104); // 0x0178
	int delta_tick; // 0x027C

};

class INetworkClientService
{
public:
	enum msg : int
	{
		unknown = 23,
		voice_channel = 48
	};

	// 0xA8 / unknownlib  name aka networkservice.dll
	CNetworkGameClient* GetNetworkClient() {
		return utils::CallVFunc<CNetworkGameClient*, 23>(this);
	}
};
