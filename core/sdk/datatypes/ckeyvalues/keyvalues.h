#pragma once
struct KV3ID_t
{
	const char* szName;
	std::uint64_t unk0;
	std::uint64_t unk1;
};

class CKeyValues3
{
public:
	char _pad[0x100];
	std::uint64_t uKey;
	void* pValue;
	char _pad1[0x8];

	void LoadFromBuffer(const char* szString);
	bool LoadKV3(CUtlBuffer* buffer);
	void LoadFromFile(const char* szString);

	CKeyValues3* create_material_from_resource();
};
