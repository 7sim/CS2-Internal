#pragma once

#include "../sdk.h"
#include <cstdint>

class CCSPlayerController;

enum prediction_reason
{
    client_command_tick,
    demo_preentity,
    demo_simulation,
    postnetupdate,
    server_starved_and_added_usercmds,
    client_frame_simulate
};

enum signon_state_t
{
    SIGNONSTATE_NONE,
    SIGNONSTATE_CHALLENGE,
    SIGNONSTATE_CONNECTED,
    SIGNONSTATE_NEW,
    SIGNONSTATE_PRESPAWN,
    SIGNONSTATE_SPAWN,
    SIGNONSTATE_FULL,
    SIGNONSTATE_CHANGELEVEL
};

class CPrediction
{
public:
   inline void update(int reason, int delta_tick) {
       using function_t = void(__fastcall*)(CPrediction*, int, int);
       auto func = reinterpret_cast<function_t>(utils::FindPattern(xorstr_("client.dll"), xorstr_("40 55 56 41 56 48 81 EC 80")));

       return func(this, reason, delta_tick);
    }

public:
    char gap0[48];
    int reason;
    bool InPrediction;
    bool EnginePause;
    char gap36[6];
    int incoming_acknowledged;
    std::uint64_t delta_tick;
    std::uint64_t slot;
    CCSPlayerController* ControllerEntity;
    char gap58[4];
    int int5C;
    char gap60[32];
    bool bFirstPrediction;
    char pad90[15];
    int storage_entity_size;
    char gap94[4];
    void* storage_entitys;
    char gapA0[48];
    int count_penisov;
    char gapD4[4];
    void* position;
    char gapE0[8];
    float real_time;
    char gapEC[12];
    int max_client;
    char gapFC[12];
    std::uintptr_t* unk_class;
    float current_time;
    char gap11C[12];
    int tick_count;
    char gap12C[4];
    char gap130[2];
    char byte132;
    char gap133[3];
    bool HasUpdate;
};