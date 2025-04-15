#include "..\cutlbuffer\utlbuffer.h"
#include "keyvalues.h"
#include "..\..\..\utils\utils.h"
#include "..\..\sdk.h"
void CKeyValues3::LoadFromBuffer(const char* szString)
{
	CUtlBuffer buffer(0, (c_run_time::StringLength(szString) + 10), 1);
	buffer.PutString(szString);
	LoadKV3(&buffer);
}

bool CKeyValues3::LoadKV3(CUtlBuffer* buffer)
{
	using fnLoadKeyValues = bool(__fastcall*)(CKeyValues3*, void*, CUtlBuffer*, KV3ID_t*, void*, void*, void*, void*, const char*);
	static const fnLoadKeyValues oLoadKeyValues = reinterpret_cast<fnLoadKeyValues>(utils::GetAbsoluteAddress(utils::FindPattern("tier0.dll", ("E8 ? ? ? ? EB 36 8B 43 10")), 0x1, 0x0));

#ifdef CS_PARANOID
	debug(oLoadKeyValues != nullptr);
#endif

	const char* szName = ("");
	KV3ID_t kv3ID = KV3ID_t(("generic"), 0x41B818518343427E, 0xB5F447C23C0CDF8C);
	return oLoadKeyValues(this, nullptr, buffer, &kv3ID, nullptr, nullptr, nullptr, nullptr, (""));
}

CKeyValues3* CKeyValues3::create_material_from_resource()
{
	using fnSetTypeKV3 = CKeyValues3 * (__fastcall*)(CKeyValues3*, unsigned int, unsigned int);
	static const fnSetTypeKV3 oSetTypeKV3 = reinterpret_cast<fnSetTypeKV3>(utils::FindPattern("client.dll", xorstr_("40 53 48 83 EC 30 48 8B D9 49")));


	CKeyValues3* pKeyValue = new CKeyValues3[0x10];
	return oSetTypeKV3(pKeyValue, 1U, 6U);
}