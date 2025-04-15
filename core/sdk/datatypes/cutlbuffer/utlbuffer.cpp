#include "..\..\..\utils\utils.h"
#include "utlbuffer.h"
#include "..\..\..\utils\hook\hooks.h"
#include "..\..\sdk\sdk.h"
CUtlBuffer::CUtlBuffer(int a1, int size, int a3)
{
	utils::construct_utl_buffer(this, a1, size, a3);
}

void CUtlBuffer::ensure(int size)
{
	utils::ensure_buffer_capacity(this, size);
}	
void CUtlBuffer::PutString(const char* szString)
{

	utils::put_utl_buffer_string(this, szString);
}

bool CSerialize::serialize(void * data, CUtlBuffer buffer, int size) {
	debug(hooks::m_serialize != nullptr);

	return hooks::m_serialize(0, buffer, size);
}

void CSerialize::parse(std::uintptr_t* message, CUtlBuffer buffer, int size) {
	debug(hooks::m_parse != nullptr);
	hooks::m_parse(message, buffer, size);
}

void CSerialize::set_data(void* CRC, std::uintptr_t* message, void* size) {
	debug(hooks::m_merge != nullptr);
	hooks::m_merge(CRC, message, size);
}

