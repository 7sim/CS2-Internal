#pragma once
#include "..\cutlmemory\utlmemory.h"

// used: memorymove
#include "..\..\..\..\deps\fnv1a\fnv1a.h"
#define STRINGTOKEN_MURMURHASH_SEED 0x31415926

#pragma pack(push, 8)
class CUtlStringToken
{
public:
	explicit CUtlStringToken(const char* szKeyName)
	{
		uHashCode = hash_32_fnv1a_const(szKeyName, STRINGTOKEN_MURMURHASH_SEED);
		szDebugName = szKeyName;
	}

	constexpr CUtlStringToken(const uint32_t uHashCode, const char* szKeyName) :
		uHashCode(uHashCode), szDebugName(szKeyName) { }

	inline bool operator==(const CUtlStringToken& other) const
	{
		return (other.uHashCode == uHashCode);
	}

	inline bool operator!=(const CUtlStringToken& other) const
	{
		return (other.uHashCode != uHashCode);
	}

	inline bool operator<(const CUtlStringToken& other) const
	{
		return (uHashCode < other.uHashCode);
	}

public:
	uint32_t uHashCode = 0U; // 0x00
	const char* szDebugName = nullptr; // 0x08 //   @Todo: for some reason retards keep this even for non-debug builds, it can be changed later
};
#pragma pack(pop)

// helper to create a string token at compile-time
inline consteval CUtlStringToken MakeStringToken(const char* szKeyName)
{
	return { hash_32_fnv1a_const(szKeyName, STRINGTOKEN_MURMURHASH_SEED), szKeyName };
}
