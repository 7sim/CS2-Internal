#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include "../helpers/crt/crt.h"
#include "../../deps/fnv1a/fnv1a.h"
#include "../../deps/json/json.h"
#include "../../deps/xorstr/xorstr.h"
#include "../defs.h"
#include "../helpers/log.h"
#include "configs/settings.h"
#include "..\sdk\datatypes\cutlbuffer\utlbuffer.h"
#include "..\sdk\datatypes\ckeyvalues\keyvalues.h"
#include "..\helpers\import\lazy_importer.hpp"
#pragma region common_architecture_specific
#if defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__i386) || defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__)
#define CS_ARCH_X86
#elif defined(__LP64__) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define CS_ARCH_X64
#else
static_assert(false, "could not determine the target architecture, consider define it manually!");
#endif
#define CS_CLASS_NO_ASSIGNMENT(CLASS)	\
CLASS& operator=(CLASS&&) = delete;		\
CLASS& operator=(const CLASS&) = delete;

// explicitly delete any class initializer to prevent attempts on using them
#define CS_CLASS_NO_INITIALIZER(CLASS)	\
CS_CLASS_NO_CONSTRUCTOR(CLASS)			\
CS_CLASS_NO_ASSIGNMENT(CLASS)
#pragma region common_compiler_specific
#ifdef _MSC_VER
#define CS_COMPILER_MSC
#endif
#ifdef __clang__ // @note: clang-cl have defined both 'CS_COMPILER_CLANG' and 'CS_COMPILER_MSC'
#define CS_COMPILER_CLANG
#endif

#ifdef __has_builtin
#define CS_HAS_BUILTIN(BUILTIN) __has_builtin(BUILTIN)
#else
#define CS_HAS_BUILTIN(BUILTIN) 0
#endif

#ifdef CS_COMPILER_MSC
// treat "discarding return value of function with 'nodiscard' attribute" warning as error
#pragma warning(error: 4834)
#endif

#ifdef CS_COMPILER_CLANG
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

#if defined(CS_COMPILER_MSC) || defined(CS_COMPILER_CLANG)
#define CS_NAKED __declspec(naked)
#endif

// define to enable logging output to console
#ifdef _DEBUG
#define CS_LOG_CONSOLE
// define to enable logging output to file
#define CS_LOG_FILE
#endif



// define  to enable additional run-time checks
#ifdef _DEBUG
#define CS_PARANOID
#endif

/*
 * define to search all possible occurrences for pattern and log if pattern isn't unique
 * - useful for keeping patterns up to date and preventing possible inconsistent behavior
 */
#ifdef _DEBUG
#define CS_PARANOID_PATTERN_UNIQUENESS
#endif

 /*
  * define to overwrite configuration file formatter implementation
  */
#define CS_CONFIGURATION_BINARY
  // name of the default configuration file
#define CS_CONFIGURATION_DEFAULT_FILE_NAME L"default"

// define to force disable behavior based on "Run-Time Type Information", even if available
//#define CS_NO_RTTI

// @todo: use #warning instead of static asserts when c++23 comes out

#pragma region common_architecture_specific
#if defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__i386) || defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__)
#define CS_ARCH_X86
#elif defined(__LP64__) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define CS_ARCH_X64
#else
static_assert(false, "could not determine the target architecture, consider define it manually!");
#endif


#ifndef CS_DEBUG_BREAK
#if defined(CS_COMPILER_MSC)
#define CS_DEBUG_BREAK() __debugbreak()
#elif defined(CS_COMPILER_CLANG)
#define CS_DEBUG_BREAK() __builtin_debugtrap()
#else
static_assert(false, "it is expected you to define CS_DEBUG_BREAK() into something that will break in a debugger!");
#define CS_DEBUG_BREAK()
#endif
#endif

#ifdef _DEBUG
#define debug(EXPRESSION) static_cast<void>(!!(EXPRESSION) || (_log(LOG_INFO) << #EXPRESSION << " failed in " << __FILE__ << " line " << __LINE__, 0))
#else
#define debug(EXPRESSION) static_cast<void>(0)
#endif

#pragma region common_compiler_specific
#ifdef _MSC_VER
#define CS_COMPILER_MSC
#endif
#ifdef __clang__ // @note: clang-cl have defined both 'CS_COMPILER_CLANG' and 'CS_COMPILER_MSC'
#define CS_COMPILER_CLANG
#endif

#ifdef __has_builtin
#define CS_HAS_BUILTIN(BUILTIN) __has_builtin(BUILTIN)
#else
#define CS_HAS_BUILTIN(BUILTIN) 0
#endif

#ifdef CS_COMPILER_MSC
// treat "discarding return value of function with 'nodiscard' attribute" warning as error
#pragma warning(error: 4834)
#endif

#ifdef CS_COMPILER_CLANG
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

#if defined(CS_COMPILER_MSC) || defined(CS_COMPILER_CLANG)
#define CS_NAKED __declspec(naked)
#endif

// @todo: platform dependent / but currently we shouldn't give fuck on it
#define CS_CDECL __cdecl
#define CS_STDCALL __stdcall
#define CS_FASTCALL __fastcall
#define CS_THISCALL __thiscall
#define CS_VECTORCALL __vectorcall
#pragma endregion

#pragma region common_implementation_specific
#define _CS_INTERNAL_CONCATENATE(LEFT, RIGHT) LEFT##RIGHT
#define _CS_INTERNAL_STRINGIFY(NAME) #NAME
#define _CS_INTERNAL_UNPARENTHESIZE(...) __VA_ARGS__

// convert plain text to string
#define CS_STRINGIFY(NAME) _CS_INTERNAL_STRINGIFY(NAME)
// concatenate plain text
#define CS_CONCATENATE(LEFT, RIGHT) _CS_INTERNAL_CONCATENATE(LEFT, RIGHT)
// unparenthesize variadic arguments
#define CS_UNPARENTHESIZE(...) _CS_INTERNAL_UNPARENTHESIZE(__VA_ARGS__)

// calculate elements count of fixed-size C array
#define CS_ARRAYSIZE(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

// calculate the offset of a struct member variable, in bytes
#if defined(_CRT_USE_BUILTIN_OFFSETOF) || CS_HAS_BUILTIN(__builtin_offsetof)
#define CS_OFFSETOF(STRUCT, MEMBER) __builtin_offsetof(STRUCT, MEMBER)
#else
#define CS_OFFSETOF(STRUCT, MEMBER) reinterpret_cast<std::size_t>(std::addressof(static_cast<STRUCT*>(nullptr)->MEMBER))
#endif

#pragma endregion


#define TIME_TO_TICKS( t ) ( int )( ( t ) / g::interval + 0.5f )
#define TICKS_TO_TIME( t ) ( ( t ) * g::interval )
#pragma region memory_definitions
#pragma warning(push)
#pragma warning(disable: 6255) // '_alloca' indicates failure by raising a stack overflow exception. consider using '_malloca' instead
#define MEM_STACKALLOC(SIZE) _malloca(SIZE)
#pragma warning(pop)
#define MEM_STACKFREE(MEMORY) static_cast<void>(0)

#define pad(SIZE)										\
private:												\
	char CS_CONCATENATE(pad_0, __COUNTER__)[SIZE]; \
public:
#pragma endregion

namespace utils
{
	bool Init( );

	void CreateConsole( LPCWSTR console_title );
	void DestroyConsole( );

	inline int( __stdcall* SetWindowGrab )( void*, int ) = nullptr;
	inline int( __stdcall* WarpMouseInWindow )( void*, float, float ) = nullptr;
	inline bool(__stdcall* fnGetRelativeMouseMode)(void) = nullptr;
	inline void(__stdcall* SetRelativeMouseMode)(void*, bool) = nullptr;

	inline void(__fastcall* ensure_buffer_capacity)(CUtlBuffer*, int) = nullptr;
	inline CUtlBuffer* (__fastcall* construct_utl_buffer)(CUtlBuffer*, int, int, int) = nullptr;
	inline void(__fastcall* put_utl_buffer_string)(CUtlBuffer*, const char*);
	inline std::int64_t(__fastcall* create_material)(void*, void*, const char*, void*, unsigned int, unsigned int);
	inline bool(__fastcall* load_key_values3)(CKeyValues3*, void*, const char*, const KV3ID_t*, const char*);

	template <typename T = void*>
	inline T GetVFunc( const void* thisptr, std::size_t nIndex )
	{
		return ( *static_cast< T* const* >( thisptr ) )[ nIndex ];
	}

	template <typename T, std::size_t nIndex, class CBaseClass, typename... Args_t>
	static inline T CallVFunc( CBaseClass* thisptr, Args_t... argList )
	{
		using VirtualFn_t = T( __thiscall* )( const void*, decltype( argList )... );
		return ( *reinterpret_cast< VirtualFn_t* const* >( reinterpret_cast< std::uintptr_t >( thisptr ) ) )[ nIndex ]( thisptr, argList... );
	}

	template<typename T = void*>
	inline T CallVFunc( void* class_ptr, int index )
	{
		void** __vfptr = *reinterpret_cast< void*** >( class_ptr );
		return reinterpret_cast< T >( __vfptr[ index ] );
	}

	template <typename T = std::uint8_t>
	T* GetAbsoluteAddress( T* relative_address, int pre_offset = 0x0, int post_offset = 0x0 )
	{
		relative_address += pre_offset;
		relative_address += sizeof( std::int32_t ) + *reinterpret_cast< std::int32_t* >( relative_address );
		relative_address += post_offset;
		return relative_address;
	}

	inline std::uint8_t* ResolveRelativeAddress( std::uint8_t* address, std::uint32_t rva_offset, std::uint32_t rip_offset )
	{
		std::uint32_t rva = *reinterpret_cast< std::uint32_t* >( address + rva_offset );
		std::uint64_t rip = reinterpret_cast< std::uint64_t >( address ) + rip_offset;

		return reinterpret_cast< std::uint8_t* >( rva + rip );
	}


	inline std::uint8_t* ToAbsolute( std::uint8_t* at )
	{
		const int relative = *reinterpret_cast< int* >( at );
		return at + relative + sizeof( int );
	}

	std::uint8_t* FindPattern( const char* module_name, const std::string& byte_sequence );
	[[nodiscard]] std::uint8_t* FindPatternEx(const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask = nullptr);
	void* base_handle(const wchar_t* wszModuleName);
	const wchar_t* GetModuleBaseFileName(const void* hModuleBase, const bool bGetFullPath);
	void* export_address(const void* hModuleBase, const char* szProcedureName);
}
