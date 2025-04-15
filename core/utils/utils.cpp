#include "utils.h"

#include <Windows.h>
// used: memory api
#include <memory>
// used: std::vector
#include <vector>
#include <span>
#include "..\helpers\pe64.h"
#include "..\utils\render\ui\menu.h"
#include "..\utils\hook\hooks.h"
bool utils::Init( )
{
	bool success = true;

	// get module basses
	const HMODULE sdl3_base = GetModuleHandleA( xorstr_( "SDL3.dll" ) );
	const HMODULE dbghelp_base = GetModuleHandleA( xorstr_( "dbghelp.dll" ) );
	const HMODULE tier0_base = GetModuleHandleA( xorstr_( "tier0.dll" ) );

	// sanity
	if ( sdl3_base == nullptr || dbghelp_base == nullptr || tier0_base == nullptr )
		return false;
	_log(LOG_INFO) << "1";
	

	construct_utl_buffer = reinterpret_cast<decltype(construct_utl_buffer)>(GetProcAddress(tier0_base, xorstr_("??0CUtlBuffer@@QEAA@HHH@Z")));
	debug(construct_utl_buffer != nullptr);

	_log(LOG_INFO) << "5";

	ensure_buffer_capacity = reinterpret_cast<decltype(ensure_buffer_capacity)>(GetProcAddress(tier0_base, xorstr_("?EnsureCapacity@CUtlBuffer@@QEAAXH@Z")));
	debug(ensure_buffer_capacity != nullptr);

	_log(LOG_INFO) << "6";

	put_utl_buffer_string = reinterpret_cast<decltype(put_utl_buffer_string)>(GetProcAddress(tier0_base, xorstr_("?PutString@CUtlBuffer@@QEAAXPEBD@Z")));
	debug(put_utl_buffer_string != nullptr);

	SetWindowGrab = reinterpret_cast<decltype(SetWindowGrab)>(GetProcAddress(sdl3_base, "SDL_SetWindowMouseGrab"));
	success &= (SetWindowGrab != nullptr);

	_log(LOG_INFO) << "7";
	WarpMouseInWindow = reinterpret_cast<decltype(WarpMouseInWindow)>(GetProcAddress(sdl3_base, "SDL_WarpMouseInWindow"));
	success &= (WarpMouseInWindow != nullptr);

	create_material = reinterpret_cast<decltype(create_material)>(FindPattern("materialsystem2.dll", xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05")));
	debug(create_material != nullptr);

	_log(LOG_INFO) << "8";

	load_key_values3 = reinterpret_cast<decltype(load_key_values3)>(GetProcAddress(tier0_base, xorstr_("?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEBDAEBUKV3ID_t@@2@Z")));
	debug(load_key_values3 != nullptr);

	_log(LOG_INFO) << "9";

	// return status
	return success;
}


void utils::CreateConsole( LPCWSTR console_title )
{
#ifdef CS2_CONSOLE
	// aloccate console memory
	AllocConsole( );

	// give the console a name
	SetConsoleTitleW( console_title );

	// redirect output to console
	freopen_s( reinterpret_cast< FILE** > stdout, xorstr_( "CONOUT$" ), xorstr_( "w" ), stdout );
#endif
}

void utils::DestroyConsole( )
{
#ifdef CS2_CONSOLE
	// stop redirecting output to console
	fclose( stdout );

#endif
}

std::uint8_t* utils::FindPattern( const char* module_name, const std::string& byte_sequence )
{
	// retrieve the handle to the specified module
	const HMODULE module = GetModuleHandleA( module_name );
	if ( module == nullptr )
		return nullptr;

	// retrieve the DOS header of the module
	const auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( module );
	if ( dos_header->e_magic != IMAGE_DOS_SIGNATURE )
		return nullptr;

	// retrieve the NT headers of the module
	const auto nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS >( reinterpret_cast< std::uint8_t* >( module ) + dos_header->e_lfanew );
	if ( nt_headers->Signature != IMAGE_NT_SIGNATURE )
		return nullptr;

	// get the size and base address of the code section
	DWORD m_size = nt_headers->OptionalHeader.SizeOfCode;
	std::uint8_t* m_base = reinterpret_cast< std::uint8_t* >( module ) + nt_headers->OptionalHeader.BaseOfCode;

	using SeqByte_t = std::pair< std::uint8_t, bool >;

	std::string str { };
	std::vector< std::pair< std::uint8_t, bool > > byte_sequence_vec { };
	std::stringstream stream { byte_sequence };

	// parse the byte sequence string into a vector of byte sequence elements
	while ( stream >> str )
	{
		// wildcard byte
		if ( str[ 0u ] == '?' )
		{
			byte_sequence_vec.emplace_back( 0u, true );
			continue;
		}

		// invalid hex digit, skip this byte
		if ( !std::isxdigit( str[ 0u ] ) || !std::isxdigit( str[ 1u ] ) )
			continue;

		byte_sequence_vec.emplace_back( static_cast< std::uint8_t >( std::strtoul( str.data( ), nullptr, 16 ) ), false );
	}

	// end pointer of the code section
	const auto end = reinterpret_cast< std::uint8_t* >( m_base + m_size );

	// search for the byte sequence within the code section
	const auto ret = std::search( reinterpret_cast< std::uint8_t* >( m_base ), end, byte_sequence_vec.begin( ), byte_sequence_vec.end( ),
		[ ] ( const std::uint8_t byte, const std::pair< std::uint8_t, bool >& seq_byte )
		{
			return std::get< bool >( seq_byte ) || byte == std::get< std::uint8_t >( seq_byte );
		} );

	// byte sequence found, return the pointer
	if ( ret )
		return ret;
#ifdef _DEBUG

	// failed to find byte sequence, log error and return
	L_PRINT(LOG_WARNING) << "failed to find pattern:" << byte_sequence.c_str() << " | inside: " << module_name ;

#endif
	return nullptr;
}

// @todo: msvc poorly optimizes this, it looks even better w/o optimization at all
std::uint8_t* utils::FindPatternEx(const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask)
{
	std::uint8_t* pCurrentAddress = const_cast<std::uint8_t*>(pRegionStart);
	const std::uint8_t* pRegionEnd = pRegionStart + nRegionSize - nByteCount;
	const bool bIsMaskUsed = (szByteMask != nullptr);

	while (pCurrentAddress < pRegionEnd)
	{
		// check the first byte before entering the loop, otherwise if there two consecutive bytes of first byte in the buffer, we may skip both and fail the search
		if ((bIsMaskUsed && *szByteMask == '?') || *pCurrentAddress == *arrByteBuffer)
		{
			if (nByteCount == 1)
				return pCurrentAddress;

			// compare the least byte sequence and continue on wildcard or skip forward on first mismatched byte
			std::size_t nComparedBytes = 0U;
			while ((bIsMaskUsed && szByteMask[nComparedBytes + 1U] == '?') || pCurrentAddress[nComparedBytes + 1U] == arrByteBuffer[nComparedBytes + 1U])
			{
				// check does byte sequence match
				if (++nComparedBytes == nByteCount - 1U)
					return pCurrentAddress;
			}

			// skip non suitable bytes
			pCurrentAddress += nComparedBytes;
		}

		++pCurrentAddress;
	}

	return nullptr;
}

void* utils::base_handle(const wchar_t* wszModuleName)
{
	const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));

	if (wszModuleName == nullptr)
		return pPEB->ImageBaseAddress;

	void* pModuleBase = nullptr;
	for (LIST_ENTRY* pListEntry = pPEB->Ldr->InMemoryOrderModuleList.Flink; pListEntry != &pPEB->Ldr->InMemoryOrderModuleList; pListEntry = pListEntry->Flink)
	{
		const _LDR_DATA_TABLE_ENTRY* pEntry = CONTAINING_RECORD(pListEntry, _LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (pEntry->FullDllName.Buffer != nullptr && c_run_time::StringCompare(wszModuleName, pEntry->BaseDllName.Buffer) == 0)
		{
			pModuleBase = pEntry->DllBase;
			break;
		}
	}


	return pModuleBase;
}

const wchar_t* utils::GetModuleBaseFileName(const void* hModuleBase, const bool bGetFullPath)
{
	const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));

	if (hModuleBase == nullptr)
		hModuleBase = pPEB->ImageBaseAddress;

	::EnterCriticalSection(pPEB->LoaderLock);

	const wchar_t* wszModuleName = nullptr;
	for (LIST_ENTRY* pListEntry = pPEB->Ldr->InMemoryOrderModuleList.Flink; pListEntry != &pPEB->Ldr->InMemoryOrderModuleList; pListEntry = pListEntry->Flink)
	{
		const _LDR_DATA_TABLE_ENTRY* pEntry = CONTAINING_RECORD(pListEntry, _LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (pEntry->DllBase == hModuleBase)
		{
			wszModuleName = bGetFullPath ? pEntry->FullDllName.Buffer : pEntry->BaseDllName.Buffer;
			break;
		}
	}

	::LeaveCriticalSection(pPEB->LoaderLock);

	return wszModuleName;
}
void* utils::export_address(const void* hModuleBase, const char* szProcedureName)
{
	const auto pBaseAddress = static_cast<const std::uint8_t*>(hModuleBase);

	const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	const auto pINH = reinterpret_cast<const IMAGE_NT_HEADERS64*>(pBaseAddress + pIDH->e_lfanew);
	if (pINH->Signature != IMAGE_NT_SIGNATURE)
		return nullptr;

	const IMAGE_OPTIONAL_HEADER64* pIOH = &pINH->OptionalHeader;
	const std::uintptr_t nExportDirectorySize = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	const std::uintptr_t uExportDirectoryAddress = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	if (nExportDirectorySize == 0U || uExportDirectoryAddress == 0U)
	{
		return nullptr;
	}

	const auto pIED = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(pBaseAddress + uExportDirectoryAddress);
	const auto pNamesRVA = reinterpret_cast<const std::uint32_t*>(pBaseAddress + pIED->AddressOfNames);
	const auto pNameOrdinalsRVA = reinterpret_cast<const std::uint16_t*>(pBaseAddress + pIED->AddressOfNameOrdinals);
	const auto pFunctionsRVA = reinterpret_cast<const std::uint32_t*>(pBaseAddress + pIED->AddressOfFunctions);

	// Perform binary search to find the export by name
	std::size_t nRight = pIED->NumberOfNames, nLeft = 0U;
	while (nRight != nLeft)
	{
		// Avoid INT_MAX/2 overflow
		const std::size_t uMiddle = nLeft + ((nRight - nLeft) >> 1U);
		const int iResult = c_run_time::StringCompare(szProcedureName, reinterpret_cast<const char*>(pBaseAddress + pNamesRVA[uMiddle]));

		if (iResult == 0)
		{
			const std::uint32_t uFunctionRVA = pFunctionsRVA[pNameOrdinalsRVA[uMiddle]];


			// Check if it's a forwarded export
			if (uFunctionRVA >= uExportDirectoryAddress && uFunctionRVA - uExportDirectoryAddress < nExportDirectorySize)
			{
				// Forwarded exports are not supported
				break;
			}

			return const_cast<std::uint8_t*>(pBaseAddress) + uFunctionRVA;
		}

		if (iResult > 0)
			nLeft = uMiddle + 1;
		else
			nRight = uMiddle;
	}

	// Export not found
	return nullptr;
}
