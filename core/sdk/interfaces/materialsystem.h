#pragma once
// used: pad
#include "..\..\utils\utils.h"
#include "..\..\sdk\datatypes\cutlvectors\utlvector.h"
#include "..\..\sdk\datatypes\cutlhandles\strong_handle.hpp"
#include "..\..\sdk\datatypes\color.h"
#include "../handle/handle.hpp"

#include "..\..\sdk\addresses\entity.h"
#include "..\..\math\elements\matrix.h"
class CMaterial2
{
public:
	virtual const char* GetName() = 0;
	virtual const char* GetShareName() = 0;
};

// idk
struct MaterialKeyVar_t
{
	std::uint64_t uKey;
	const char* szName;

	MaterialKeyVar_t(std::uint64_t uKey, const char* szName) :
		uKey(uKey), szName(szName) { }

	MaterialKeyVar_t(const char* szName, bool bShouldFindKey = false) :
		szName(szName)
	{
		uKey = bShouldFindKey ? FindKey(szName) : 0x0;
	}

	std::uint64_t FindKey(const char* szName)
	{
		using fnFindKeyVar = std::uint64_t(CS_FASTCALL*)(const char*, unsigned int, int);
		static auto oFindKeyVar = reinterpret_cast<fnFindKeyVar>(utils::FindPattern("particles.dll", xorstr_("48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA")));

#ifdef CS_PARANOID
		assert(oFindKeyVar != nullptr);
#endif

		// idk those enum flags, just saw it called like that soooo yea
		return oFindKeyVar(szName, 0x12, 0x31415926);
	}
};

class CSceneObject
{
public:
	char pad_0000[184]; //0x0000
	uint8_t r; //0x00B8
	uint8_t g; //0x00B9
	uint8_t b; //0x00BA
	uint8_t a; //0x00BB
	char pad_00BC[196]; //0x00BC
}; //Size: 0x0180

class CObjectInfo
{
	pad(0xB0);
	int nId;
};


class CBaseSceneData
{
public:
	char pad_0000[0x18]; // 0x0000
	void* scene_object; // 0x0018
	CMaterial2* material; // 0x0020
	CMaterial2* material2; // 0x0028
	char pad_0030[0x20]; // 0x0030
	uint8_t r; // 0x0040
	uint8_t g; // 0x0041
	uint8_t b; // 0x0042
	uint8_t a; // 0x0043
	char pad_0044[0x14]; // 0x0044
public:
	void set_color(Color_t new_color) {
		constexpr std::uintptr_t offset = 0x50;
		*reinterpret_cast<Color_t*>(reinterpret_cast<std::uintptr_t>(this) + offset) = new_color;
	}
}; //Size: 0x0068

class C_AggregateSceneObjectData
{
private:
	char pad_0000[0x38]; // 0x0
public:
	std::uint8_t r; // 0x38
	std::uint8_t g; // 0x39
	std::uint8_t b; // 0x3A
private:
	char pad_0038[0x9];
};

class C_AggregateSceneObject
{
private:
	char pad_0000[0x120];
public:
	int m_nCount; // 0x120
private:
	char pad_0120[0x4];
public:
	C_AggregateSceneObjectData* m_pData; // 0x128
};

class C_SceneLightObject
{
public:
	char pad_0000[0xE4]; // 0x0
	float r; // 0xE4
	float g; // 0xE4
	float b; // 0xE4
};

#pragma pack(push, 1)
class CSkeletonInstance;

class CSceneAnimatableObject {
public:
	CBaseHandle GetOwner() const {
		if (!this )
			return CBaseHandle();

		return *(CBaseHandle*)((std::uintptr_t)this + 0xB8);
	}	

	
	CSkeletonInstance* GetSkeletonInstance2() {
		constexpr std::uintptr_t offset = 0x108;
		return *reinterpret_cast<CSkeletonInstance**>(reinterpret_cast<std::uintptr_t>(this) + offset);
	}
	// incomplete.
};  // Size: 0x118
#pragma pack(pop)



// the naming is incorrect but i dont care atm
class CMeshData
{
public:
	void SetShaderType(const char* szShaderName)
	{
		// @ida: #STR: shader, spritecard.vfx
		using fnSetMaterialShaderType = void(CS_FASTCALL*)(void*, MaterialKeyVar_t, const char*, int);
		static auto oSetMaterialShaderType = reinterpret_cast<fnSetMaterialShaderType>(utils::FindPattern("particles.dll", ("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 0F B6 01 45 0F B6 F9 8B 2A 4D 8B E0 4C 8B 72 ? 48 8B F9 C0 E8 ? 24 ? 3C ? 74 ? 41 B0 ? B2 ? E8 ? ? ? ? 0F B6 07 33 DB C0 E8 ? 24 ? 3C ? 75 ? 48 8B 77 ? EB ? 48 8B F3 4C 8D 44 24 ? C7 44 24 ? ? ? ? ? 48 8D 54 24 ? 89 6C 24 ? 48 8B CE 4C 89 74 24 ? E8 ? ? ? ? 8B D0 83 F8 ? 75 ? 45 33 C9 89 6C 24 ? 4C 8D 44 24 ? 4C 89 74 24 ? 48 8B D7 48 8B CE E8 ? ? ? ? 8B D0 0F B6 0F C0 E9 ? 80 E1 ? 80 F9 ? 75 ? 48 8B 4F ? EB ? 48 8B CB 8B 41 ? 85 C0 74 ? 48 8D 59 ? 83 F8 ? 76 ? 48 8B 1B 48 63 C2 4D 85 E4")));

#ifdef CS_PARANOID
		debug(oSetMaterialShaderType != nullptr);
#endif

		MaterialKeyVar_t shaderVar(0x162C1777, ("shader"));
		oSetMaterialShaderType(this, shaderVar, szShaderName, 0x18);
	}

	void SetMaterialFunction(const char* szFunctionName, int nValue)
	{
		using fnSetMaterialFunction = void(__fastcall*)(void*, MaterialKeyVar_t, int, int);
		static auto oSetMaterialFunction = reinterpret_cast<fnSetMaterialFunction>(utils::FindPattern("particles.dll", ("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 0F B6 01 45 0F B6 F9 8B 2A 48 8B F9")));

#ifdef CS_PARANOID
		debug(oSetMaterialFunction != nullptr);
#endif

		MaterialKeyVar_t functionVar(szFunctionName, true);
		oSetMaterialFunction(this, functionVar, nValue, 0x18);
	}
private:
	pad(0x18); // 0x0
public:
	CSceneAnimatableObject* pSceneAnimatableObject; // 0x18
	CMaterial2* pMaterial; // 0x20
	CMaterial2* pMaterialCopy; // 0x20
private:
	pad(0x10); // 0x28
public:
	CObjectInfo* pObjectInfo;
private:
	pad(0x8); // 0x48
public:
	Color_t colValue; // 0x50
};

class IMaterialSystem2
{
public:
	CMaterial2*** FindOrCreateFromResource(CMaterial2*** pOutMaterial, const char* szMaterialName)
	{
		return utils::CallVFunc<CMaterial2***, 14U>(this, pOutMaterial, szMaterialName);
	}

	CMaterial2** CreateMaterial(CMaterial2*** pOutMaterial, const char* szMaterialName, CMeshData* pData)
	{
		return utils::CallVFunc<CMaterial2**, 29U>(this, pOutMaterial, szMaterialName, pData, 0, 0, 0, 0, 0, 1);
	}

	void SetCreateDataByMaterial(const void* pData, CMaterial2*** const pInMaterial)
	{
		return utils::CallVFunc<void, 37U>(this, pInMaterial, pData);
	}

	void SetColor(void* data, Color_t color)
	{
		*(byte*)((uintptr_t)data + 0x40) = color.r;
		*(byte*)((uintptr_t)data + 0x41) = color.g;
		*(byte*)((uintptr_t)data + 0x42) = color.b;
		*(byte*)((uintptr_t)data + 0x43) = color.a;
	}

};

