#pragma once
#include <Windows.h>
#include <cstdint>

#include "..\..\..\utils\utils.h"

template <class T>
class RepFieldContainer
{
    uint64_t unk_field_;
    int current_size_;
    int capacity_;

    struct InnerContainer
    {
        int max_;
        T** elements( ) {
            return reinterpret_cast< T** >( reinterpret_cast< uintptr_t >( this ) + 0x8 );
        }
    }*container;

public:
    inline int& size( ) {
        return current_size_;
    }
    inline int& current_size() {
        return current_size_;
    }
    inline int& allocated_size() {
        return container->max_;
    }
    inline int& capacity( ) {
        return capacity_;
    }

    inline int& max_size( ) {
        return container->max_;
    }

    inline void clear( ) {
        current_size_ = 0;
    }

    template<class T>
    inline T* add( T* element )
    {
        static const auto sub_C92EF0 = utils::FindPattern( xorstr_( "client.dll" ), xorstr_( "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 48 8B FA 48 8B 49 ? 48 85 C9 74 ? 8B 01" ) );
        return reinterpret_cast< T * ( __fastcall* )( RepFieldContainer*, T* ) >( sub_C92EF0 )( this, element );
    }

    inline uint64_t unk_field( ) {
        return unk_field_;
    }

    inline T*& operator[]( int i ) {
        return container->elements( )[ i ];
    }

    inline operator bool( ) {
        return container != nullptr;
    }
};
