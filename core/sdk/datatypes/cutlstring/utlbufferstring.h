#pragma once


#include "..\cutlmemory\utlmemory.h"
#include "..\..\sdk.h"
#include "..\..\interfaces\memory.h"
template <size_t max_size, bool allow_heap_allocation>
class c_buffer_string_growable;

/*
        Main idea of c_buffer_string is to provide the base class for the c_buffer_string_growable wich implements stack allocation
        with the ability to convert to the heap allocation if allowed.

        Example usage of c_buffer_string_growable class:

        * Basic buffer allocation:
        ```
                c_buffer_string_growable<256> buff;
                buff.Insert(0, "Hello World!");
                printf("Result: %s\n", buff.Get());
        ```
        additionaly the heap allocation of the buffer could be disabled, by providing ``allow_heap_allocation`` template argument,
        by disabling heap allocation, if the buffer capacity is not enough to perform the operation, the app would exit with an Assert;

        * Additional usage:
        c_buffer_string::IsStackAllocated() - could be used to check if the buffer is stack allocated;
        c_buffer_string::IsHeapAllocated() - could be used to check if the buffer is heap allocated;
        c_buffer_string::Get() - would return a pointer to the data, or an empty string if it's not allocated.

        * Additionaly current length of the buffer could be read via c_buffer_string::GetTotalNumber()
        and currently allocated amount of bytes could be read via c_buffer_string::GetAllocatedNumber()

        * Most, if not all the functions would ensure the buffer capacity and enlarge it when needed,
        in case of stack allocated buffers, it would switch to heap allocation instead.
*/

class c_buffer_string {
protected:
    // You shouldn't be initializing this class, use c_buffer_string_growable instead.
    c_buffer_string() { }

public:
    enum EAllocationOption_t {
        UNK1 = -1,
        UNK2 = 0,
        UNK3 = (1 << 1),
        UNK4 = (1 << 8),
        UNK5 = (1 << 9),
        ALLOW_HEAP_ALLOCATION = (1 << 31)
    };

    enum EAllocationFlags_t {
        LENGTH_MASK = (1 << 30) - 1,
        FLAGS_MASK = ~LENGTH_MASK,

        STACK_ALLOCATION_MARKER = (1 << 30),
        HEAP_ALLOCATION_MARKER = (1 << 31)
    };

    // Casts to c_buffer_string_growable. Very dirty solution until someone figures out the sane one.
    template <size_t max_size = 8, bool allow_heap_allocation = true, typename T = c_buffer_string_growable<max_size, allow_heap_allocation>>
    T* to_growable() {
        return (T*)this;
    }
};

template <size_t max_size, bool allow_heap_allocation = true>
class c_buffer_string_growable : public c_buffer_string {
    friend class c_buffer_string;
public:
    c_buffer_string_growable() : m_nTotalCount(0), m_nAllocated(STACK_ALLOCATION_MARKER | (max_size & LENGTH_MASK)) {
        memset(m_Memory.m_szString, 0, sizeof(m_Memory.m_szString));
        if (allow_heap_allocation) {
            m_nAllocated |= ALLOW_HEAP_ALLOCATION;
        }
    }

    c_buffer_string_growable(const c_buffer_string_growable& other) : m_nTotalCount(0), m_nAllocated(STACK_ALLOCATION_MARKER | (max_size & LENGTH_MASK)) {
        memset(m_Memory.m_szString, 0, sizeof(m_Memory.m_szString));
        if (allow_heap_allocation) {
            m_nAllocated |= ALLOW_HEAP_ALLOCATION;
        }
        MoveFrom(const_cast<c_buffer_string_growable&>(other));
    }

    ~c_buffer_string_growable() {
        if (IsHeapAllocated() && m_Memory.m_pString) {
            free((void*)m_Memory.m_pString);
        }
    }

    inline c_buffer_string_growable& operator=(const c_buffer_string_growable& src) {
        MoveFrom(const_cast<c_buffer_string_growable&>(src));
        return *this;
    }

    inline int GetAllocatedNumber() const {
        return m_nAllocated & LENGTH_MASK;
    }

    inline int GetTotalNumber() const {
        return m_nTotalCount & LENGTH_MASK;
    }

    inline bool IsStackAllocated() const {
        return (m_nAllocated & STACK_ALLOCATION_MARKER) != 0;
    }

    inline bool IsHeapAllocated() const {
        return (m_nTotalCount & HEAP_ALLOCATION_MARKER) != 0;
    }

    inline bool IsInputStringUnsafe(const char* pData) const {
        return ((void*)pData >= this && (void*)pData < &this[1]) ||
            (GetAllocatedNumber() != 0 && pData >= Get() && pData < (Get() + GetAllocatedNumber()));
    }

    inline const char* Get() const {
        if (IsStackAllocated()) {
            return m_Memory.m_szString;
        }
        else if (GetAllocatedNumber() != 0) {
            return m_Memory.m_pString;
        }

        return "";
    }

    inline void Clear() {
        if (GetAllocatedNumber() != 0) {
            if (IsStackAllocated())
                m_Memory.m_szString[0] = '\0';
            else
                m_Memory.m_pString[0] = '\0';
        }
        m_nTotalCount &= ~LENGTH_MASK;
    }
private:
    int m_nTotalCount;
    int m_nAllocated;

    union {
        char* m_pString;
        char m_szString[max_size];
    } m_Memory;
};