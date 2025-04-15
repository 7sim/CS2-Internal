#pragma once
#include <cstdint>


class internal_metadata
{
public:
	intptr_t ptr_ = 0;
	intptr_t kUnknownFieldsTagMask = 1;
	intptr_t kPtrTagMask = kUnknownFieldsTagMask;
};
class CUtlBuffer
{
public:
	char _pad[0x80];

	CUtlBuffer(int a1 = 0, int size = 0, int a3 = 0);
	void ensure(int size);

	void PutString(const char* szString);
};

class CSerialize
{
public:
	bool serialize(void* data, CUtlBuffer buffer, int size);	
	void parse(std::uintptr_t* message, CUtlBuffer buffer, int size);
	void set_data(void* CRC, std::uintptr_t* message, void* size);
public:	
	internal_metadata _internal_metadata_;

};