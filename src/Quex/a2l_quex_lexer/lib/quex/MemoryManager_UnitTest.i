/* -*- C++ -*- vim: set syntax=cpp: 
 *
 * PURPOSE: Implementation of the MemoryManager for Unit Tests.
 *          
 * Some information about allocation and freeing of memory is collected in 
 * 'MemoryManager_UnitTest_t'.                                                        
 *
 * (C) Frank-Rene Schaefer                                                    */
#ifndef QUEX_INCLUDE_GUARD_LIB_QUEX__MEMORY_MANAGER_UNIT_TEST_I
#define QUEX_INCLUDE_GUARD_LIB_QUEX__MEMORY_MANAGER_UNIT_TEST_I

#include "a2l_quex_lexer\lib\quex/compatibility/stdint.h"
#include "a2l_quex_lexer\lib\quex/MemoryManager"

typedef struct {
    int allocation_n;
    int allocated_byte_n;

    int reallocated_byte_n;
    int reallocated_refusal_n;

    int free_n;

    int allocation_addmissible_f;

    int reallocate_limit_byte_n;
    int reallocate_verbose_f;

    int forbid_ByteLoader_f;
    int forbid_LexatomLoader_f;
    int forbid_BufferMemory_f;
    int forbid_InputName_f;
} MemoryManager_UnitTest_t;

/* Object must be defined in unit test!                                       */
extern MemoryManager_UnitTest_t MemoryManager_UnitTest;

uint8_t*
quex_MemoryManager_allocate(const size_t       ByteN, 
                            E_MemoryObjectType Type)
{
    uint8_t*  me;

    if( ! MemoryManager_UnitTest.allocation_addmissible_f ) {
        return (uint8_t*)0;
    }
    switch( Type ) {
    case E_MemoryObjectType_BYTE_LOADER:
        if( MemoryManager_UnitTest.forbid_ByteLoader_f ) return (uint8_t*)0;
        else                                             break;
    case E_MemoryObjectType_BUFFER_FILLER:
        if( MemoryManager_UnitTest.forbid_LexatomLoader_f ) return (uint8_t*)0;
        else                                                break;
    case E_MemoryObjectType_BUFFER_MEMORY:
        if( MemoryManager_UnitTest.forbid_BufferMemory_f ) return (uint8_t*)0;
        else                                               break;
    case E_MemoryObjectType_INPUT_NAME:
        if( MemoryManager_UnitTest.forbid_InputName_f ) return (uint8_t*)0;
        else                                            break;
    case E_MemoryObjectType_BUFFER:
    case E_MemoryObjectType_BUFFER_RAW:
    case E_MemoryObjectType_CONVERTER:
    case E_MemoryObjectType_MEMENTO:
    case E_MemoryObjectType_POST_CATEGORIZER_NODE:
    case E_MemoryObjectType_TEXT:
    case E_MemoryObjectType_TOKEN_ARRAY:
    default:
        break;
    }
    me = (uint8_t*)__QUEX_STD_malloc((size_t)ByteN);

    (void)Type;
#   ifdef QUEX_OPTION_ASSERTS
    __QUEX_STD_memset((void*)me, 0xFF, ByteN);
#   endif

    MemoryManager_UnitTest.allocation_n     += 1;
    MemoryManager_UnitTest.allocated_byte_n += ByteN;

#   ifdef QUEX_OPTION_UNIT_TEST_MEMORY_MANAGER_VERBOSE_EXT
    QUEX_DEBUG_PRINT2("allocate: { adr: ((%p)); size: %i; }\n", (void*)me, (int)ByteN);
#   endif
    return me;
}

uint8_t*
quex_MemoryManager_reallocate(void*              old_memory,
                                       const size_t       NewByteN, 
                                       E_MemoryObjectType Type)
{
    void* result_p;

    if( NewByteN > (size_t)MemoryManager_UnitTest.reallocate_limit_byte_n ) {
        MemoryManager_UnitTest.reallocated_refusal_n += 1;
        if( MemoryManager_UnitTest.reallocate_verbose_f ) {
            QUEX_DEBUG_PRINT1("Reallocate: refuse %i;\n", (int)NewByteN);
        }
        return (uint8_t*)0;
    }
    else {
        MemoryManager_UnitTest.reallocated_byte_n += NewByteN;
        result_p = __QUEX_STD_realloc(old_memory, NewByteN);
        if( MemoryManager_UnitTest.reallocate_verbose_f ) {
            QUEX_DEBUG_PRINT1("Reallocate: done %i;\n", (int)NewByteN);
        }
        return (uint8_t*)result_p;
    }
}
       
void 
quex_MemoryManager_free(void*              alter_ego, 
                                 E_MemoryObjectType Type)  
{ 
    void* me = (void*)alter_ego;
    (void)Type;

#   ifdef QUEX_OPTION_UNIT_TEST_MEMORY_MANAGER_VERBOSE_EXT
    QUEX_DEBUG_PRINT1("free:     { adr: ((%p)); }\n", (void*)me);
#   endif

    /* The de-allocator shall never be called for LexemeNull object.         */
    if( me ) {
        __QUEX_STD_free(me); 
    }
    MemoryManager_UnitTest.free_n       += 1;
}

size_t
quex_MemoryManager_insert(uint8_t* drain_begin_p,  uint8_t* drain_end_p,
                                   uint8_t* source_begin_p, uint8_t* source_end_p)
/* Inserts as many bytes as possible into the array from 'drain_begin_p'
 * to 'drain_end_p'. The source of bytes starts at 'source_begin_p' and
 * ends at 'source_end_p'.
 *
 * RETURNS: Number of bytes that have been copied.                           */
{
    /* Determine the insertion size.                                         */
    const size_t DrainSize = (size_t)(drain_end_p  - drain_begin_p);
    size_t       size      = (size_t)(source_end_p - source_begin_p);
    __quex_assert(drain_end_p  >= drain_begin_p);
    __quex_assert(source_end_p >= source_begin_p);

    if( DrainSize < size ) size = DrainSize;

    /* memcpy() might fail if the source and drain domain overlap! */
#   ifdef QUEX_OPTION_ASSERTS 
    if( drain_begin_p > source_begin_p ) __quex_assert(drain_begin_p >= source_begin_p + size);
    else                                 __quex_assert(drain_begin_p <= source_begin_p - size);
#   endif
    __QUEX_STD_memcpy(drain_begin_p, source_begin_p, size);

    return size;
}

char*
quex_MemoryManager_clone_string(const char* String)
{ 
    char* result;
   
    if( ! String ) {
        return (char*)0;
    }
    if( ! MemoryManager_UnitTest.allocation_addmissible_f ) {
        return (char*)0;
    }
   
    result = (char*)quex_MemoryManager_allocate(
                                 sizeof(char)*(__QUEX_STD_strlen(String)+1),
                                 E_MemoryObjectType_INPUT_NAME);
    if( ! result ) {
        return (char*)0;
    }
    __QUEX_STD_strcpy(result, String);
    return result;
}

bool 
quex_system_is_little_endian(void)
{
    union {
        long int multi_bytes;
        char     c[sizeof (long int)];
    } u;
    u.multi_bytes = 1;
    return u.c[sizeof(long int)-1] != 1;
}

void
quex_print_relative_positions(const void*  Begin,       const void* End, 
                                     size_t       ElementSize, const void* P)
/* Begin       = pointer to first element of buffer.
 * End         = pointer behind last element of buffer.
 * ElementSize = size in bytes of each element of buffer.
 * P           = pointer for which relative position is to be printed.        */
{
    const uint8_t* BytePBegin = (const uint8_t*)Begin;
    const uint8_t* BytePEnd   = (const uint8_t*)End;
    const uint8_t* ByteP      = (const uint8_t*)P;

    if( P == 0x0 ) {
        QUEX_DEBUG_PRINT("<void>;");
    }
    else if( P < Begin ) {
        QUEX_DEBUG_PRINT1("begin - %i <beyond boarder>;", 
                          (int)(BytePBegin - ByteP) / (int)ElementSize);
    }
    else if( P >= End ) {
        QUEX_DEBUG_PRINT1("end + %i <beyond boarder>;", 
                          (int)(ByteP - BytePEnd) / (int)ElementSize);
    }
    else {
        QUEX_DEBUG_PRINT2("begin + %i, end - %i;", 
                          (int)(ByteP - BytePBegin) / (int)ElementSize, 
                          (int)(BytePEnd - ByteP) / (int)ElementSize);
    }
}

ptrdiff_t
quex_strlcpy(char* dst, const char* src, size_t siz)
{
    /* Copy src to string dst of size siz.  At most siz-1 characters
     * will be copied.  Always NUL terminates (unless siz == 0).
     *
     * RETURNS: strlen(src); if retval >= siz, truncation occurred.           
     *
     * Original 'strlcpy' returns 'size_t', however 'ptrdiff_t' is fitter for
     * pointer arithmetic
     *
     * 'strlcpy()' is copied from BSD sources since it is not part of the 
     * standard C library. The following copyright/license notice concerns only 
     * the body of this function:
     *_________________________________________________________________________
     * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>          *
     * Permission to use, copy, modify, and distribute this software for any  *
     * purpose with or without fee is hereby granted, provided that the above *
     * copyright notice and this permission notice appear in all copies.      *
     *________________________________________________________________________*/

    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0) break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';/* NUL-terminate dst */
        while (*s++);
    }

    return (s - src - 1);/* count does n ot include NUL */
}

#ifdef __cplusplus
namespace quex {

    uint8_t* MemoryManager_allocate(const size_t ByteN, E_MemoryObjectType MT)
    { return quex_MemoryManager_allocate(ByteN, MT); }

    uint8_t* MemoryManager_reallocate(void*              old_memory,
                                      const size_t       NewByteN, 
                                      E_MemoryObjectType Type)
    { return quex_MemoryManager_reallocate(old_memory, NewByteN, Type); }

    void   MemoryManager_free(void* Obj, E_MemoryObjectType MT)
    { return quex_MemoryManager_free(Obj, MT); }

    size_t MemoryManager_insert(uint8_t* drain_begin_p,  uint8_t* drain_end_p,
                                     uint8_t* source_begin_p, uint8_t* source_end_p)
    { return quex_MemoryManager_insert(drain_begin_p, drain_end_p, source_begin_p, source_end_p); }

    char*  MemoryManager_clone_string(const char* String)
    { return quex_MemoryManager_clone_string(String); }

    bool   system_is_little_endian(void)
    { return quex_system_is_little_endian(); }

    void   print_relative_positions(const void* Begin,       const void* End, 
                                            size_t      ElementSize, const void* P)
    { return quex_print_relative_positions(Begin, End, ElementSize, P); }

    ptrdiff_t strlcpy(char* Dest, const char* Src, size_t MaxN)
    { return quex_strlcpy(Dest, Src, MaxN); }

} /* namespace Quex */
#endif
 
#endif /*  QUEX_INCLUDE_GUARD_LIB_QUEX__MEMORY_MANAGER_I */


