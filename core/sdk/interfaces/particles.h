#pragma once
// used: cutllinkedlist
#include "..\..\utils\utils.h"
#include "..\handle\handle.hpp"
#include "../addresses/entity.h"
#include "..\sdk.h"
#include "..\datatypes\cutlstring\utlstringtoken.h"

class CParticleSnapshot
{
public:
    void Draw(int nCount, void* pData)
    {
        utils::CallVFunc<void, 1U>(this, nCount, pData);
    }
};

// The interface we are getting is actually CParticleSystemMgr which inherrits IParticleSystemMgr
class IParticleSystemMgr
{
public:
    // Third argument here is a CUtlString*, but I honestly can't be bothered to construct an empty CUtlString so this will have to do
    void CreateSnapshot(CStrongHandle<CParticleSnapshot>* pOutParticleSnapshot)
    {
        __int64 nUnknown = 0;
        utils::CallVFunc<void, 42U>(this, pOutParticleSnapshot, &nUnknown);
    }

    // This does the same as calling the Draw function from particle snapshot at index 1
    void Draw(CStrongHandle<CParticleSnapshot>* pParticleSnapshot, int nCount, void* pData)
    {
        utils::CallVFunc<void, 43U>(this, pParticleSnapshot, nCount, pData);
    }
};

class CGameParticleManager
{
public:

    // inline int*(__fastcall* fnCacheParticleEffect)(CGameParticleManager*, unsigned int*, const char*, int, __int64, __int64, __int64, int) = nullptr;
    // client.dll @ 4C 8B DC 53 48 83 EC 60 48 8B 84 24
    int* CacheParticleEffect(unsigned int* pEffectIndex, const char* szName);
    // This function does not create a particle effect, it simply changes it's attributes
    // inline bool(__fastcall* fnCreateParticleEffect)(CGameParticleManager*, unsigned int, int, void*, int) = NULL;
    // client.dll @ 48 83 EC 58 F3 41 0F 10 51 ? 0F 28 05
    bool CreateParticleEffect(unsigned int nEffectIndex, int nUnknown, void* pData, int nUnknown1);

    // inline bool(__fastcall* fnUnknownParticleFunction)(CGameParticleManager*, int, unsigned int, const CStrongHandle<CParticleSnapshot>*) = NULL;
    // client.dll @ 48 89 74 24 10 57 48 83 EC 30 4C 8B D9 49 8B F9 33 C9 41 8B F0 83 FA FF 0F
    bool UnknownFunction(int nEffectIndex, unsigned int nUnknown, const CStrongHandle<CParticleSnapshot>* pParticleSnapshot);

};

// This class changed slightly when molotov's where changed.
// I did not reverse this and getting positions and times as pointers like this is most certainly not a big brain move.
// credits: epoxi [https://www.unknowncheats.me/forum/members/5188035.html]
class CParticleData
{
public:
    Vector_t* m_vecPosition;
    pad(0x74);
    float* m_flTimes;
    pad(0x28);
    float* m_flTimes2;
    pad(0x98);
}; //Size: 0x0150

class particle_effect
{
public:
    const char* name{};
    char pad[0x30]{};
};

class particle_information
{
public:
    float time{};
    float width{};
    float unk2{};
};


struct particle_color
{
    float r;
    float g;
    float b;
};

class CParticles {
public:
  CGameParticleManager* GetGameParticleManager();     
};
const auto particles = std::make_unique<CParticles>();
