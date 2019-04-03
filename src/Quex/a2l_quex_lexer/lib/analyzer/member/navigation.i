/* -*- C++ -*- vim: set syntax=cpp:
 * (C) 2005-2010 Frank-Rene Schaefer
 * ABSOLUTELY NO WARRANTY              */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__MEMBER__NAVIGATION_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__MEMBER__NAVIGATION_I

#include "a2l_quex_lexer\lib\definitions"



inline size_t  
a2l_quex_lexer_MF_tell(a2l_quex_lexer* me)
{
    /* No 'undo terminating zero' -- we do not change the lexeme pointer here. */
    return (size_t)a2l_quex_lexer_Buffer_tell(&me->buffer);
}

inline void    
a2l_quex_lexer_MF_seek(a2l_quex_lexer* me, const size_t CharacterIndex)
{
    QUEX_a2l_quex_lexer_LEXEME_TERMINATING_ZERO_UNDO(&me->buffer);
    a2l_quex_lexer_Buffer_seek(&me->buffer, (ptrdiff_t)CharacterIndex);
}

inline void    
a2l_quex_lexer_MF_seek_forward(a2l_quex_lexer* me, const size_t CharacterN)
{
    QUEX_a2l_quex_lexer_LEXEME_TERMINATING_ZERO_UNDO(&me->buffer);
    a2l_quex_lexer_Buffer_seek_forward(&me->buffer, (ptrdiff_t)CharacterN);
}

inline void    
a2l_quex_lexer_MF_seek_backward(a2l_quex_lexer* me, const size_t CharacterN)
{
    QUEX_a2l_quex_lexer_LEXEME_TERMINATING_ZERO_UNDO(&me->buffer);
    a2l_quex_lexer_Buffer_seek_backward(&me->buffer, (ptrdiff_t)CharacterN);
}

inline void  
a2l_quex_lexer_MF_undo(a2l_quex_lexer* me)
{
       me->counter._line_number_at_end   = me->counter._line_number_at_begin;
     me->counter._column_number_at_end = me->counter._column_number_at_begin;

    QUEX_a2l_quex_lexer_LEXEME_TERMINATING_ZERO_UNDO(&me->buffer);

    me->buffer._read_p = me->buffer._lexeme_start_p;

    QUEX_a2l_quex_lexer_LEXEME_TERMINATING_ZERO_SET(&me->buffer);
}

inline void  
a2l_quex_lexer_MF_undo_n(a2l_quex_lexer* me, size_t DeltaN_Backward)
{
       me->counter._line_number_at_end   = me->counter._line_number_at_begin;
     me->counter._column_number_at_end = me->counter._column_number_at_begin;

    QUEX_a2l_quex_lexer_LEXEME_TERMINATING_ZERO_UNDO(&me->buffer);

    me->buffer._read_p -= (ptrdiff_t)DeltaN_Backward;

    QUEX_a2l_quex_lexer_LEXEME_TERMINATING_ZERO_SET(&me->buffer);

    __quex_assert(me->buffer._read_p >= me->buffer._lexeme_start_p);
}



#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__MEMBER__NAVIGATION_I */
