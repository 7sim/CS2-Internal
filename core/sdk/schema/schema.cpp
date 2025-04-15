#include "schema.h"

// used: [stl] vector
#include <vector>
// used: [stl] find_if
#include <algorithm>
#include "../../utils/utils.h"

// used: ischemasystem
#include "..\..\sdk\interfaces\schemasystem.h"


// used: dbg
#include "../../helpers/log.h"


// used: [win] shgetknownfolderpath
#include <Shlobj.h>
#include <shlobj_core.h>


#include "../../helpers/crt/crt.h"
#include "../../helpers/sprint/stb_sprintf.h"

#include "../sdk.h"
#define STB_SPRINTF_IMPLEMENTATION

bool GetWorkingPath(wchar_t* wszDestination)
{
	const wchar_t* wszModuleName = utils::GetModuleBaseFileName(static_cast<HMODULE>(g::m_module), true);
	c_run_time::StringCopy(wszDestination, wszModuleName);

	// remove the module name
	if (wchar_t* pwszLastSlash = c_run_time::StringCharR(wszDestination, L'\\'); pwszLastSlash != nullptr)
		*pwszLastSlash = L'\0';

	c_run_time::StringCat(wszDestination, L"\\.cs2c\\");
	// create directory if it doesn't exist
	if (!::CreateDirectoryW(wszDestination, nullptr))
	{
		if (const DWORD dwError = ::GetLastError(); dwError != ERROR_ALREADY_EXISTS)
		{
			return false;
		}
	}

	return true;
}
struct SchemaData_t
{
	uint32_t uHashedFieldName = 0x0ULL;
	std::uint32_t uOffset = 0x0U;
};

static std::vector<SchemaData_t> vecSchemaData;

bool SCHEMA::Setup(const wchar_t* wszFileName, const char* szModuleName)
{

	//wchar_t wszDumpFilePath[MAX_PATH];
	//if (!GetWorkingPath(wszDumpFilePath))
	//	return false;

	//c_run_time::StringCat(wszDumpFilePath, wszFileName);

//	HANDLE hOutFile = ::CreateFileW(wszDumpFilePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	//if (hOutFile == INVALID_HANDLE_VALUE)
	//	return false;

	// @todo: maybe remove this redundant? and put it inside CRT::String_t c'tor
//	const std::time_t time = std::time(nullptr);
	//std::tm timePoint;
//	localtime_s(&timePoint, &time);

	//c_run_time::String_t<64> szTimeBuffer(xorstr_("[%d-%m-%Y %T] cs2 | schema dump\n\n"), &timePoint);

	// write current date, time and info
	//::WriteFile(hOutFile, szTimeBuffer.Data(), szTimeBuffer.Length(), nullptr, nullptr);

	CSchemaSystemTypeScope* pTypeScope = sdk::m_schema_system->FindTypeScopeForModule(szModuleName);
	if (pTypeScope == nullptr)
		return false;

	const int nTableSize = pTypeScope->hashClasses.Count();

	// allocate memory for elements
	UtlTSHashHandle_t* pElements = new UtlTSHashHandle_t[nTableSize + 1U];
	const auto nElements = pTypeScope->hashClasses.GetElements(0, nTableSize, pElements);

	for (int i = 0; i < nElements; i++)
	{
		const UtlTSHashHandle_t hElement = pElements[i];

		if (hElement == 0)
			continue;

		CSchemaClassBinding* pClassBinding = pTypeScope->hashClasses[hElement];
		if (pClassBinding == nullptr)
			continue;

		SchemaClassInfoData_t* pDeclaredClassInfo;
		pTypeScope->FindDeclaredClass(&pDeclaredClassInfo, pClassBinding->szBinaryName);

		if (pDeclaredClassInfo == nullptr)
			continue;

		if (pDeclaredClassInfo->nFieldSize == 0)
			continue;

		//c_run_time::String_t<MAX_PATH> szClassBuffer(xorstr_("class %s\n"), pDeclaredClassInfo->szName);
		//::WriteFile(hOutFile, szClassBuffer.Data(), szClassBuffer.Length(), nullptr, nullptr);

		for (auto j = 0; j < pDeclaredClassInfo->nFieldSize; j++)
		{
			SchemaClassFieldData_t* pFields = pDeclaredClassInfo->pFields;
			c_run_time::String_t<MAX_PATH> szFieldClassBuffer(xorstr_("%s->%s"), pClassBinding->szBinaryName, pFields[j].szName);
			// store field info
			vecSchemaData.emplace_back(hash_32_fnv1a_const(szFieldClassBuffer.Data()), pFields[j].nSingleInheritanceOffset);

			//c_run_time::String_t<MAX_PATH> szFieldBuffer(xorstr_("    %s %s = 0x%X\n"), pFields[j].pSchemaType->szName, pFields[j].szName, pFields[j].nSingleInheritanceOffset);
			// write field info
			//::WriteFile(hOutFile, szFieldBuffer.Data(), szFieldBuffer.Length(), nullptr, nullptr);
		}
#ifdef _DEBUG
	//	CS2_LOG("dumped  (name: %s total: %i) \n", pDeclaredClassInfo->szName, pDeclaredClassInfo->nFieldSize);
#endif
	}

	// free allocated memory
	delete[] pElements;

	// close file
//	::CloseHandle(hOutFile);

	return true;
}

std::uint32_t SCHEMA::GetOffset(const uint32_t uHashedFieldName)
{
	if (const auto it = std::ranges::find_if(vecSchemaData, [uHashedFieldName](const SchemaData_t& data)
		{ return data.uHashedFieldName == uHashedFieldName; });
		it != vecSchemaData.end())
		return it->uOffset;

	CS2_LOG("\nfailed to find offset for field with hash: %i", uHashedFieldName);
	debug(false); // schema field not found
	return 0U;
}

// @todo: optimize this, this is really poorly do and can be done much better?
std::uint32_t SCHEMA::GetForeignOffset(const char* szModulenName, const uint32_t uHashedClassName, const uint32_t uHashedFieldName)
{
	CSchemaSystemTypeScope* pTypeScope = sdk::m_schema_system->FindTypeScopeForModule(szModulenName);
	if (pTypeScope == nullptr)
		return false;

	const int nTableSize = pTypeScope->hashClasses.Count();
	// allocate memory for elements
	UtlTSHashHandle_t* pElements = new UtlTSHashHandle_t[nTableSize + 1U];
	const auto nElements = pTypeScope->hashClasses.GetElements(0, nTableSize, pElements);
	std::uint32_t uOffset = 0x0;

	for (int i = 0; i < nElements; i++)
	{
		const UtlTSHashHandle_t hElement = pElements[i];

		if (hElement == 0)
			continue;

		CSchemaClassBinding* pClassBinding = pTypeScope->hashClasses[hElement];
		if (pClassBinding == nullptr)
			continue;

		SchemaClassInfoData_t* pDeclaredClassInfo;
		pTypeScope->FindDeclaredClass(&pDeclaredClassInfo, pClassBinding->szBinaryName);

		if (pDeclaredClassInfo == nullptr)
			continue;

		if (pDeclaredClassInfo->nFieldSize == 0)
			continue;

		for (auto j = 0; j < pDeclaredClassInfo->nFieldSize; j++)
		{
			SchemaClassFieldData_t* pFields = pDeclaredClassInfo->pFields;
			if (pFields == nullptr)
				continue;

			SchemaClassFieldData_t field = pFields[j];
			if (hash_32_fnv1a_const(pClassBinding->szBinaryName) == uHashedClassName && hash_32_fnv1a_const(field.szName) == uHashedFieldName)
				uOffset = field.nSingleInheritanceOffset;
		}
	}

	return uOffset;
}