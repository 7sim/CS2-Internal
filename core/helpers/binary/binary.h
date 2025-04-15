#pragma once
#define CS_VERSION 2
#include "..\..\utils\base64\base64.h"
namespace C::BIN
{
	/* @section: [internal] */
	/// write single variable to the buffer
	/// @returns: number of written bytes
	inline std::size_t WriteBuffer(std::uint8_t* pBuffer, const VariableObject_t& variable)
	{
		std::uint8_t* pBufferCurrent = pBuffer;

		*reinterpret_cast<uint32_t*>(pBufferCurrent) = variable.uNameHash;
		pBufferCurrent += sizeof(uint32_t);

		*reinterpret_cast<uint32_t*>(pBufferCurrent) = variable.uTypeHash;
		pBufferCurrent += sizeof(uint32_t);

		switch (variable.uTypeHash)
		{
		case hash_32_fnv1a_const("bool"):
		case hash_32_fnv1a_const("int"):
		case hash_32_fnv1a_const("unsigned int"):
		case hash_32_fnv1a_const("float"):
		case hash_32_fnv1a_const("Color_t"):
		case hash_32_fnv1a_const("char[]"):
		{
			memcpy(pBufferCurrent, variable.GetStorage<const std::uint8_t, false>(), variable.nStorageSize);
			pBufferCurrent += variable.nStorageSize;
			break;
		}
		case hash_32_fnv1a_const("bool[]"):
		case hash_32_fnv1a_const("int[]"):
		case hash_32_fnv1a_const("unsigned int[]"):
		case hash_32_fnv1a_const("float[]"):
		case hash_32_fnv1a_const("char[][]"):
		{
			// write size
			*reinterpret_cast<std::size_t*>(pBufferCurrent) = variable.nStorageSize;
			pBufferCurrent += sizeof(std::size_t);

			// write data
			memcpy(pBufferCurrent, variable.GetStorage<const std::uint8_t, false>(), variable.nStorageSize);
			pBufferCurrent += variable.nStorageSize;
			break;
		}
		default:
		{
			[[maybe_unused]] bool bFoundUserType = false;
			const std::uint8_t* pVariableStorage = variable.GetStorage<const std::uint8_t, false>();

			// lookup for user-defined data type
			for (const UserDataType_t& userType : vecUserTypes)
			{
				if (userType.uTypeHash == variable.uTypeHash)
				{
					// write size
					*reinterpret_cast<std::size_t*>(pBufferCurrent) = variable.GetSerializationSize();
					pBufferCurrent += sizeof(std::size_t);

					// write data
					// @test: it would be so fucking neatful if we could rework this to proceed recursive call instead
					for (const UserDataMember_t& userMember : userType.vecMembers)
					{
						*reinterpret_cast<uint32_t*>(pBufferCurrent) = userMember.uNameHash;
						pBufferCurrent += sizeof(uint32_t);

						*reinterpret_cast<uint32_t*>(pBufferCurrent) = userMember.uTypeHash;
						pBufferCurrent += sizeof(uint32_t);

						memcpy(pBufferCurrent, pVariableStorage + userMember.uBaseOffset, userMember.nDataSize);
						pBufferCurrent += userMember.nDataSize;
					}

					bFoundUserType = true;
					break;
				}
			}

			break;
		}
		}

		const std::size_t nWriteCount = (pBufferCurrent - pBuffer);
		return nWriteCount;
	}

	/// read single variable from the buffer
	/// @returns: number of read bytes
	inline std::size_t ReadBuffer(std::uint8_t* pBuffer, VariableObject_t& variable)
	{
		std::uint8_t* pBufferCurrent = pBuffer;

		// @todo: instead of overwriting those, check are them same? or alternatively, make caller of this method do not initialize variable
		variable.uNameHash = *reinterpret_cast<uint32_t*>(pBufferCurrent);
		pBufferCurrent += sizeof(uint32_t);

		variable.uTypeHash = *reinterpret_cast<uint32_t*>(pBufferCurrent);
		pBufferCurrent += sizeof(uint32_t);

		switch (variable.uTypeHash)
		{
		case hash_32_fnv1a_const("char[]"):
			debug((c_run_time::StringLength(reinterpret_cast<const char*>(pBufferCurrent)) + 1U) * sizeof(char) <= variable.nStorageSize); // string length mismatched
			[[fallthrough]];
		case hash_32_fnv1a_const("bool"):
		case hash_32_fnv1a_const("int"):
		case hash_32_fnv1a_const("unsigned int"):
		case hash_32_fnv1a_const("float"):
		case hash_32_fnv1a_const("Color_t"):
		{
			variable.SetStorage(pBufferCurrent);
			pBufferCurrent += variable.nStorageSize;
			break;
		}
		case hash_32_fnv1a_const("bool[]"):
		case hash_32_fnv1a_const("int[]"):
		case hash_32_fnv1a_const("unsigned int[]"):
		case hash_32_fnv1a_const("float[]"):
		case hash_32_fnv1a_const("char[][]"):
		{
			// read size
			[[maybe_unused]] const std::size_t nSourceStorageSize = *reinterpret_cast<std::size_t*>(pBufferCurrent);
			pBufferCurrent += sizeof(std::size_t);

			// read data
			debug(nSourceStorageSize <= variable.nStorageSize); // source size is bigger than destination size
			variable.SetStorage(pBufferCurrent);
			pBufferCurrent += nSourceStorageSize;
			break;
		}
		default:
		{
			[[maybe_unused]] bool bFoundUserType = false;
			std::uint8_t* pVariableStorage = variable.GetStorage<std::uint8_t, false>();

			// lookup for user-defined data type
			for (const UserDataType_t& userType : vecUserTypes)
			{
				if (userType.uTypeHash == variable.uTypeHash)
				{
					// read size
					const std::size_t nSourceSerializationSize = *reinterpret_cast<std::size_t*>(pBufferCurrent);
					pBufferCurrent += sizeof(std::size_t);

					const std::uint8_t* pBufferFirstMember = pBufferCurrent;

					// read data
					// @test: it would be so fucking neatful if we could rework this to proceed recursive call instead
					for (const UserDataMember_t& userMember : userType.vecMembers)
					{
						const uint32_t uMemberNameHash = *reinterpret_cast<uint32_t*>(pBufferCurrent);
						pBufferCurrent += sizeof(uint32_t);

						const uint32_t uMemberTypeHash = *reinterpret_cast<uint32_t*>(pBufferCurrent);
						pBufferCurrent += sizeof(uint32_t);

						// verify source and destination variable name and data type
						if (uMemberNameHash != userMember.uNameHash || uMemberTypeHash != userMember.uTypeHash)
						{
							// @todo: instead we can skip up to the next matched name and type hashes variable and continue read from there
							debug(false); // source and destination structure mismatch

							// skip rest of the variables due to the impossibility of determining their size
							pBufferCurrent += nSourceSerializationSize - (pBufferCurrent - pBufferFirstMember);
							break;
						}

						memcpy(pVariableStorage + userMember.uBaseOffset, pBufferCurrent, userMember.nDataSize);
						pBufferCurrent += userMember.nDataSize;
					}

					bFoundUserType = true;
					break;
				}
			}

			debug(bFoundUserType); // value type is not defined
			break;
		}
		}

		const std::size_t nReadCount = (pBufferCurrent - pBuffer);
		debug(nReadCount == sizeof(uint32_t[2]) + variable.GetSerializationSize()); // count of actually read bytes mismatch to serialization size
		return nReadCount;
	}

	/// search for single variable in the buffer
	/// @returns: pointer to the buffer at position of found variable on success, null otherwise
	inline std::uint8_t* FindBuffer(const std::uint8_t* pBufferStart, const std::size_t nBufferSize, const VariableObject_t& variable)
	{
		// convert hashes to bytes
		const uint32_t uNameTypeHash[2] = { variable.uNameHash, variable.uTypeHash };

		return utils::FindPatternEx(pBufferStart, nBufferSize, reinterpret_cast<const std::uint8_t*>(uNameTypeHash), sizeof(uint32_t[2]));

	}

	/* @section: main */
	inline bool SaveVariable(const wchar_t* wszFilePath, const VariableObject_t& variable)
	{
		const HANDLE hFileInOut = ::CreateFileW(wszFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFileInOut == INVALID_HANDLE_VALUE)
			return false;

		DWORD dwFileSize;
		if (dwFileSize = ::GetFileSize(hFileInOut, nullptr); dwFileSize == INVALID_FILE_SIZE)
		{
			::CloseHandle(hFileInOut);
			return false;
		}

		std::uint8_t* pBuffer = static_cast<std::uint8_t*>(HeapAlloc(GetProcessHeap(), 0,dwFileSize));
		if (!::ReadFile(hFileInOut, pBuffer, dwFileSize, nullptr, nullptr))
		{
			::CloseHandle(hFileInOut);

			HeapFree(GetProcessHeap(), 0,pBuffer);
			return false;
		}

		VariableObject_t version = { hash_32_fnv1a_const("version"), hash_32_fnv1a_const("int"), CS_VERSION };
		ReadBuffer(pBuffer, version);

		BOOL bWritten = FALSE;

		// check have we found a variable
		if (std::uint8_t* pVariableData = FindBuffer(pBuffer, dwFileSize, variable); pVariableData != nullptr)
		{
			const std::size_t nOverwriteBytesCount = WriteBuffer(pVariableData, variable);

			// overwrite variable in the file
			if (::SetFilePointer(hFileInOut, pVariableData - pBuffer, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
				bWritten = ::WriteFile(hFileInOut, pVariableData, nOverwriteBytesCount, nullptr, nullptr);
		}
		// or we need to create new
		else
		{
			// write missing variable to the end of file
			std::uint8_t* pTemporaryBuffer = static_cast<std::uint8_t*>(MEM_STACKALLOC(sizeof(uint32_t[2]) + variable.GetSerializationSize()));
			const std::size_t nWriteBytesCount = WriteBuffer(pTemporaryBuffer, variable);

			bWritten = ::WriteFile(hFileInOut, pTemporaryBuffer, nWriteBytesCount, nullptr, nullptr);
			MEM_STACKFREE(pTemporaryBuffer);
		}
		::CloseHandle(hFileInOut);

		HeapFree(GetProcessHeap(), 0,pBuffer);
		return bWritten;
	}

	inline bool LoadVariable(const wchar_t* wszFilePath, VariableObject_t& variable)
	{
		const HANDLE hFileInOut = ::CreateFileW(wszFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFileInOut == INVALID_HANDLE_VALUE)
			return false;

		DWORD dwFileSize;
		if (dwFileSize = ::GetFileSize(hFileInOut, nullptr); dwFileSize == INVALID_FILE_SIZE)
		{
			::CloseHandle(hFileInOut);
			return false;
		}

		std::uint8_t* pBuffer = static_cast<std::uint8_t*>(HeapAlloc(GetProcessHeap(), 0,dwFileSize));
		if (!::ReadFile(hFileInOut, pBuffer, dwFileSize, nullptr, nullptr))
		{
			::CloseHandle(hFileInOut);

			HeapFree(GetProcessHeap(), 0,pBuffer);
			return false;
		}

		VariableObject_t version = { hash_32_fnv1a_const("version"), hash_32_fnv1a_const("int"), CS_VERSION };
		ReadBuffer(pBuffer, version);

		// search a variable in the file
		std::uint8_t* pVariableData = FindBuffer(pBuffer, dwFileSize, variable);
		const bool bFoundVariable = (pVariableData != nullptr);

		// check have we found the variable
		if (bFoundVariable)
			ReadBuffer(pVariableData, variable);
		// otherwise serialize it
		else
		{
			if (*version.GetStorage<int>() < CS_VERSION)
			{
				// write missing variable to the end of file
				std::uint8_t* pTemporaryBuffer = static_cast<std::uint8_t*>(MEM_STACKALLOC(sizeof(uint32_t[2]) + variable.GetSerializationSize()));
				const std::size_t nWriteBytesCount = WriteBuffer(pTemporaryBuffer, variable);

				::WriteFile(hFileInOut, pTemporaryBuffer, nWriteBytesCount, nullptr, nullptr);
				MEM_STACKFREE(pTemporaryBuffer);

				// overwrite version
				if (::SetFilePointer(hFileInOut, sizeof(uint32_t[2]), nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
				{
					constexpr int iLastVersion = CS_VERSION;
					::WriteFile(hFileInOut, &iLastVersion, version.GetSerializationSize(), nullptr, nullptr);
				}
			}
			else
				debug(false); // version of configuration is greater than cheat, or version is same but configuration missing variable, consider update 'CS_VERSION' cheat version
		}
		::CloseHandle(hFileInOut);

		HeapFree(GetProcessHeap(), 0,pBuffer);
		return bFoundVariable;
	}

	inline bool RemoveVariable(const wchar_t* wszFilePath, const VariableObject_t& variable)
	{
		const HANDLE hFileInOut = ::CreateFileW(wszFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFileInOut == INVALID_HANDLE_VALUE)
			return false;

		DWORD dwFileSize;
		if (dwFileSize = ::GetFileSize(hFileInOut, nullptr); dwFileSize == INVALID_FILE_SIZE)
		{
			::CloseHandle(hFileInOut);
			return false;
		}

		std::uint8_t* pBuffer = static_cast<std::uint8_t*>(HeapAlloc(GetProcessHeap(), 0,dwFileSize));
		if (!::ReadFile(hFileInOut, pBuffer, dwFileSize, nullptr, nullptr))
		{
			::CloseHandle(hFileInOut);

			HeapFree(GetProcessHeap(), 0,pBuffer);
			return false;
		}
		const std::uint8_t* pBufferEnd = pBuffer + dwFileSize;

		VariableObject_t version = { hash_32_fnv1a_const("version"), hash_32_fnv1a_const("int"), CS_VERSION };
		ReadBuffer(pBuffer, version);

		// search a variable in the file
		std::uint8_t* pVariableData = FindBuffer(pBuffer, dwFileSize, variable);
		bool bRemovedVariable = false;

		// check have we found the variable
		if (pVariableData != nullptr)
		{
			const std::size_t nVariableDataLength = sizeof(uint32_t[2]) + variable.GetSerializationSize();
			const std::uint8_t* pVariableDataEnd = pVariableData + nVariableDataLength;

			// shift bytes left with overlapping data of variable to remove
			memmove(pVariableData, pVariableDataEnd, pBufferEnd - pVariableDataEnd);

			// truncate file size
			if (::SetFilePointer(hFileInOut, -static_cast<LONG>(nVariableDataLength), nullptr, FILE_END) != INVALID_SET_FILE_POINTER)
				bRemovedVariable = true;
		}
		::CloseHandle(hFileInOut);

		HeapFree(GetProcessHeap(), 0,pBuffer);
		return bRemovedVariable;
	}
	inline bool SaveFile(const wchar_t* wszFilePath)
	{
		const HANDLE hFileOut = ::CreateFileW(wszFilePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFileOut == INVALID_HANDLE_VALUE)
			return false;

		VariableObject_t version = { hash_32_fnv1a_const("version"), hash_32_fnv1a_const("int"), 2 };

		// pre-calculate buffer size for all variables to avoid reallocation
		std::size_t nBufferSize = sizeof(uint32_t[2]) + version.GetSerializationSize();
		for (const auto& variable : vecVariables)
			nBufferSize += sizeof(uint32_t[2]) + variable.GetSerializationSize();

		// since we know final size, reserve disk space for it
		if (::SetFilePointer(hFileOut, nBufferSize, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
		{
			::SetEndOfFile(hFileOut);

			if (::SetFilePointer(hFileOut, 0L, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
				return false;
		}

		std::uint8_t* pBuffer = static_cast<std::uint8_t*>(HeapAlloc(GetProcessHeap(), 0, nBufferSize));
		const std::uint8_t* pBufferEnd = pBuffer + nBufferSize;

		std::uint8_t* pCurrentBuffer = pBuffer;

		// put current cheat build number
		pCurrentBuffer += WriteBuffer(pBuffer, version);

		for (auto& variable : vecVariables)
		{
			debug(pCurrentBuffer <= pBufferEnd); // allocated buffer can't hold more variables
			pCurrentBuffer += WriteBuffer(pCurrentBuffer, variable);
		}

		// Base64 encode the buffer
		std::string encodedData = base64_encode(std::string(reinterpret_cast<char*>(pBuffer), nBufferSize));

		// Write Base64 encoded data to the file
		const BOOL bWritten = ::WriteFile(hFileOut, encodedData.c_str(), static_cast<DWORD>(encodedData.size()), nullptr, nullptr);
		::CloseHandle(hFileOut);

		HeapFree(GetProcessHeap(), 0, pBuffer);
		return bWritten;
	}
	inline bool LoadFile(const wchar_t* wszFilePath)
	{
		const HANDLE hFileInOut = ::CreateFileW(wszFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFileInOut == INVALID_HANDLE_VALUE)
			return false;

		DWORD dwFileSize;
		if (dwFileSize = ::GetFileSize(hFileInOut, nullptr); dwFileSize == INVALID_FILE_SIZE)
		{
			::CloseHandle(hFileInOut);
			return false;
		}

		char* pBuffer = static_cast<char*>(HeapAlloc(GetProcessHeap(), 0, dwFileSize));
		if (!::ReadFile(hFileInOut, pBuffer, dwFileSize, nullptr, nullptr))
		{
			::CloseHandle(hFileInOut);
			HeapFree(GetProcessHeap(), 0, pBuffer);
			return false;
		}

		// Base64 decode the buffer to get original data
		std::string decodedData = base64_decode(std::string(pBuffer, dwFileSize));

		HeapFree(GetProcessHeap(), 0, pBuffer);

		// Now, treat the decoded data as a byte buffer and process it as usual
		std::uint8_t* pDecodedBuffer = reinterpret_cast<std::uint8_t*>(decodedData.data());
		const std::uint8_t* pBufferEnd = pDecodedBuffer + decodedData.size();

		VariableObject_t version = { hash_32_fnv1a_const("version"), hash_32_fnv1a_const("int"), CS_VERSION };
		ReadBuffer(pDecodedBuffer, version);

		for (auto& variable : vecVariables)
		{
			std::uint8_t* pVariableData = FindBuffer(pDecodedBuffer, decodedData.size(), variable);

			// check if variable is not found
			if (pVariableData == nullptr)
			{
				if (*version.GetStorage<int>() < CS_VERSION)
				{
					// write missing variable to the end of file
					if (::SetFilePointer(hFileInOut, 0L, nullptr, FILE_END) != INVALID_SET_FILE_POINTER)
					{
						std::uint8_t* pTemporaryBuffer = static_cast<std::uint8_t*>(MEM_STACKALLOC(sizeof(uint32_t[2]) + variable.GetSerializationSize()));
						const std::size_t nWriteBytesCount = WriteBuffer(pTemporaryBuffer, variable);

						::WriteFile(hFileInOut, pTemporaryBuffer, nWriteBytesCount, nullptr, nullptr);
						MEM_STACKFREE(pTemporaryBuffer);

						// overwrite version
						if (::SetFilePointer(hFileInOut, sizeof(uint32_t[2]), nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
						{
							constexpr int iLastVersion = CS_VERSION;
							::WriteFile(hFileInOut, &iLastVersion, version.GetSerializationSize(), nullptr, nullptr);
						}
					}
				}
				continue;
			}

			ReadBuffer(pVariableData, variable);
		}

		::CloseHandle(hFileInOut);
		return true;
	}
}