#include "particles.h"
#include "../../utils/hook/hooks.h"

CGameParticleManager* CParticles::GetGameParticleManager()
{
	using fnGetParticleSystem = CGameParticleManager * (__fastcall*)();
	fnGetParticleSystem oGetGameParticleManager = reinterpret_cast<fnGetParticleSystem>(utils::GetAbsoluteAddress(utils::FindPattern("client.dll", xorstr_("E8 ? ? ? ? 41 B0 01 8B D3")), 0x1));
	return oGetGameParticleManager();
}

int* CGameParticleManager::CacheParticleEffect(unsigned int* pEffectIndex, const char* szName)
{

    return hooks::m_cache_particle_effect(this, pEffectIndex, szName, 8, 0ll, 0ll, 0ll, 0);
}

bool CGameParticleManager::CreateParticleEffect(unsigned int nEffectIndex, int nUnknown, void* pData, int nUnknown1)
{
    return hooks::m_create_particle_effect(this, nEffectIndex, nUnknown, pData, nUnknown1);
}

bool CGameParticleManager::UnknownFunction(int nEffectIndex, unsigned int nUnknown, const CStrongHandle<CParticleSnapshot>* pParticleSnapshot)
{
    return hooks::m_unknown_particle_effect(this, nEffectIndex, nUnknown, pParticleSnapshot);
}