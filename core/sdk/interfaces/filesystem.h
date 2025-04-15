#pragma once
#include "../../utils/utils.h"
#include "..\handle\handle.hpp"
class IFileSystem
{
public:
    void Close(void* file_handle)
    {
        return utils::CallVFunc<void, 14U>(this, file_handle);
    }

    unsigned int Size(void* file)
    {
        return utils::CallVFunc<unsigned int, 18U>(this, file);
    }

    const char* unk(const char* file, std::uintptr_t* unk)
    {
        return utils::CallVFunc<const char*, 60U>(this, file, unk);
    }

    void* Open(const char* pFileName, const char* pOptions, const char* pathID = nullptr)
    {
        return utils::CallVFunc<void*, 78U>(this, pFileName, pOptions, pathID);
    }
  

    int Read(void* pOutput, int size, void* file)
    {
        return utils::CallVFunc<int, 11U>(this, pOutput, size, file);
    }
};