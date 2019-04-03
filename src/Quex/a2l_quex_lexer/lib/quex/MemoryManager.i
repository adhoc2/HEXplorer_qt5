/* -*- C++ -*- vim: set syntax=cpp: */

/* This file contains an implementation which can potentially be shared between
 * multiple different lexical analyzers. See 'multi.i' for further info.     */

#ifndef QUEX_INCLUDE_GUARD_LIB_QUEX__MEMORY_MANAGER_I
#define QUEX_INCLUDE_GUARD_LIB_QUEX__MEMORY_MANAGER_I



#include "a2l_quex_lexer\lib\quex/MemoryManager"
#include "a2l_quex_lexer\lib\quex/debug_print"
#include "a2l_quex_lexer\lib\quex/standard_functions"

namespace quex {

uint8_t*
MemoryManager_allocate(const size_t       ByteN, 
                                      E_MemoryObjectType Type)
{
    uint8_t* me = 0;
    (void)Type;

#   ifdef __cplusplus
    try                   { me = new uint8_t[ByteN]; } 
    catch(std::bad_alloc) { return (uint8_t*)0; }
#   else
    me = (uint8_t*)__QUEX_STD_malloc(ByteN);
#   endif

    QUEX_IF_ASSERTS_poison(me, &me[ByteN]);
    return me;
}

uint8_t*
MemoryManager_reallocate(void*              old_memory,
                                        const size_t       NewByteN, 
                                        E_MemoryObjectType Type)
/* Attempts to find a bigger chunk of memory for 'old_memory'--if possible 
 * at the same location, so that copying is not necessary.
 *
 * RETURNS: pointer == old_memory, if memory could be extended.
 *                     (old_memory REMAINS allocated)
 *                  == 0, if memory could neither be extended nor new 
 *                        memory could be allocated.
 *                     (old_memory REMAINS allocated)
 *                  != old_memory, if new memory has been allocated, 
 *                     content has been copied.
 *                     (old_memory is DEALLOCATED)                            */
{
    (void)Type;
#   ifdef QUEX_OPTION_ASSERTS
    /* (0) allocate new memory,
     * (1) copy memory,
     * (2) poison old memory,
     * (3) free old memory,
     * => Ensure that application does not rely on freed memory.              */
    uint8_t* new_memory = MemoryManager_allocate(NewByteN, Type);
    if( ! new_memory ) {
        return (uint8_t*)0;
    }
    else {
        __QUEX_STD_memcpy(new_memory, old_memory, NewByteN);
        /* Cannot make assumptions about the old's memory size, except >= 1.
         * => Poison one byte will at least trigger buffer limit code error.  */
        ((uint8_t*)old_memory)[0] = 0xFF;
        MemoryManager_free(old_memory, Type);
        return new_memory;
    }
#   else
    return (uint8_t*)__QUEX_STD_realloc(old_memory, NewByteN);
#   endif
}
       
void 
MemoryManager_free(void*              alter_ego, 
                               E_MemoryObjectType Type)  
{ 
    (void)Type;
    /* The de-allocator shall never be called for LexemeNull object.         */
    if( ! alter_ego ) return;
#   ifdef __cplusplus
    uint8_t* me = (uint8_t*)alter_ego;
    delete [] me;
#   else
    void* me = (void*)alter_ego;
    __QUEX_STD_free(me); 
#   endif
}

size_t
MemoryManager_insert(uint8_t* drain_begin_p,  uint8_t* drain_end_p,
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
MemoryManager_clone_string(const char* String)
{ 
    char* result;
   
    if( ! String ) {
        return (char*)0;
    }
   
    result = (char*)MemoryManager_allocate(
                                 sizeof(char)*(__QUEX_STD_strlen(String)+1),
                                 E_MemoryObjectType_INPUT_NAME);
    if( ! result ) {
        return (char*)0;
    }
    __QUEX_STD_strcpy(result, String);
    return result;
}

bool 
system_is_little_endian(void)
{
    union {
        long int multi_bytes;
        char     c[sizeof (long int)];
    } u;
    u.multi_bytes = 1;
    return u.c[sizeof(long int)-1] != 1;
}

void
print_relative_positions(const void*  Begin,       const void* End, 
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
strlcpy(char* dst, const char* src, size_t siz)
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

} /* close quex */
 
#endif /* QUEX_INCLUDE_GUARD_LIB_QUEX__MEMORY_MANAGER_I */


