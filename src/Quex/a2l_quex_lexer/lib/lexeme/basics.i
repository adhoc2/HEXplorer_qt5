/* -*- C++ -*- vim:set syntax=cpp: 
 * (C) Frank-Rene Schaefer    
 * ABSOLUTELY NO WARRANTY                                                     */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__LEXEME__BASICS_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__LEXEME__BASICS_I

#include "a2l_quex_lexer\lib\quex/standard_functions"
#include "a2l_quex_lexer\lib\quex/MemoryManager"



extern a2l_quex_lexer_lexatom_t a2l_quex_lexer_LexemeNull;

inline size_t 
a2l_quex_lexer_lexeme_length(const a2l_quex_lexer_lexatom_t* Str)
{
    const a2l_quex_lexer_lexatom_t* iterator = Str;
    while( *iterator ) ++iterator; 
    return (size_t)(iterator - Str);
}

inline a2l_quex_lexer_lexatom_t*
a2l_quex_lexer_lexeme_clone(const a2l_quex_lexer_lexatom_t* BeginP,
                        size_t                   Length)
{
    a2l_quex_lexer_lexatom_t* result;

    if( ! BeginP || ! *BeginP ) {
        return &a2l_quex_lexer_LexemeNull;
    }
    
    result = (a2l_quex_lexer_lexatom_t*)quex::MemoryManager_allocate(
                   sizeof(a2l_quex_lexer_lexatom_t) * (Length + 1),
                   E_MemoryObjectType_TEXT);

    if( result ) {
        __QUEX_STD_memcpy((void*)result, (void*)BeginP, 
                          sizeof(a2l_quex_lexer_lexatom_t) * (Length + 1));
    }
    else {
        result = &a2l_quex_lexer_LexemeNull; 
    }
    return result;
}

inline size_t 
a2l_quex_lexer_lexeme_compare(const a2l_quex_lexer_lexatom_t* it0, 
                          const a2l_quex_lexer_lexatom_t* it1)
{
    for(; *it0 == *it1; ++it0, ++it1) {
        /* Both letters are the same and == 0?
         * => both reach terminall zero without being different.              */
        if( *it0 == 0 ) return 0;
    }
    return (size_t)(*it0) - (size_t)(*it1);
}



#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__LEXEME_BASICS_I */
