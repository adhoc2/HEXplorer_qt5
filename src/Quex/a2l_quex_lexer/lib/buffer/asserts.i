/* -*- C++ -*- vim: set syntax=cpp: */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__ASSERTS_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__ASSERTS_I

#include "a2l_quex_lexer\lib\buffer/asserts"
#include "a2l_quex_lexer\lib\buffer/Buffer"

#if defined(QUEX_OPTION_ASSERTS)



inline void
a2l_quex_lexer_BUFFER_ASSERT_INVARIANCE_SETUP(a2l_quex_lexer_BufferInvariance* bi, 
                                          a2l_quex_lexer_Buffer*           me)
{
    a2l_quex_lexer_BufferInvariance_construct(bi, me);
    a2l_quex_lexer_Buffer_assert_consistency(me);
}

inline void
a2l_quex_lexer_BUFFER_ASSERT_INVARIANCE_VERIFY(a2l_quex_lexer_BufferInvariance* bi, 
                                           a2l_quex_lexer_Buffer*           me)     
{
    a2l_quex_lexer_Buffer_assert_consistency(me);                   
    a2l_quex_lexer_BufferInvariance_assert(bi, me, false);
}

inline void
a2l_quex_lexer_BUFFER_ASSERT_INVARIANCE_VERIFY_SAME(a2l_quex_lexer_BufferInvariance* bi, 
                                                a2l_quex_lexer_Buffer*           me)     
{
    a2l_quex_lexer_Buffer_assert_consistency(me);                   
    a2l_quex_lexer_BufferInvariance_assert(bi, me, true);
}


inline void
a2l_quex_lexer_Buffer_assert_pointers_in_range(const a2l_quex_lexer_Buffer* B)                                      
/* Check whether _read_p and _lexeme_start_p are in ther appropriate range. */
{                                                                                    
    __quex_assert( (B) != 0x0 );                                                     
    if( ! (*B)._memory._front && ! (*B)._memory._back ) {                    
        return;
    }

    __quex_assert((*B).begin(B)        <  (*B).content_space_end(B));                     
    __quex_assert((*B).content_end(B)  >= (*B).content_begin(B));          
    __quex_assert((*B).content_end(B)  <= (*B).content_space_end(B));               

    __quex_assert((*B)._read_p         >= (*B).begin(B));                
    __quex_assert((*B)._read_p         <= (*B).content_end(B));              
    __quex_assert((*B)._lexeme_start_p >= (*B).begin(B));                
    __quex_assert((*B)._lexeme_start_p <= (*B).content_end(B));              
}

inline void
a2l_quex_lexer_Buffer_assert_limit_codes_in_place(const a2l_quex_lexer_Buffer* B)                                            
{
    if( ! (*B)._memory._front && ! (*B)._memory._back ) {                    
        return;
    }
    __quex_assert((*B).begin(B)[0]              == QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER);    
    __quex_assert((*B).content_space_end(B)[0]  == QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER);    
    __quex_assert((*B).content_end(B)[0]        == QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER);   
}

inline void
a2l_quex_lexer_Buffer_assert_consistency(const a2l_quex_lexer_Buffer* B)                                            
{                                                                                    
    const a2l_quex_lexer_Buffer* focus;
    if( ! B ) return;
    __quex_assert(   B->input.lexatom_index_begin == -1
                  || B->input.lexatom_index_begin >= 0);
    __quex_assert(   B->input.lexatom_index_end_of_stream == -1 
                  || B->input.lexatom_index_end_of_stream >= B->input.lexatom_index_begin);
    a2l_quex_lexer_Buffer_assert_pointers_in_range(B);                                              
    a2l_quex_lexer_Buffer_assert_limit_codes_in_place(B);

    if( B->_memory.ownership == E_Ownership_INCLUDING_BUFFER ) {
        __quex_assert(0 != B->_memory.including_buffer);
        /* No cyclic nesting of buffers.                                      */
        for(focus = B; 0 != focus->_memory.including_buffer; ) { 
            __quex_assert(focus->_memory.ownership == E_Ownership_INCLUDING_BUFFER);
            focus = focus->_memory.including_buffer;
            __quex_assert(focus != B);
        }
        /* NOT:
         *     __quex_assert(&including_buffer->_memory._back[1] == &front[0]);
         * BECAUSE: (1) Pointer adaption happens from back to front.
         *              => consistency could not be checked during adaption.
         *          (2) Future versions may store more in the buffer region.  */
    }
    else {
        __quex_assert(0 == B->_memory.including_buffer);
    }
    a2l_quex_lexer_Buffer_member_functions_assert(B);
}

inline void
a2l_quex_lexer_Buffer_member_functions_assert(const a2l_quex_lexer_Buffer* me)
{
    __quex_assert(me->fill                == a2l_quex_lexer_Buffer_fill);
    __quex_assert(me->fill_prepare        == a2l_quex_lexer_Buffer_fill_prepare);
    __quex_assert(me->fill_finish         == a2l_quex_lexer_Buffer_fill_finish);

    __quex_assert(me->begin               == a2l_quex_lexer_Buffer_memory_begin);
    __quex_assert(me->end                 == a2l_quex_lexer_Buffer_memory_end);
    __quex_assert(me->size                == a2l_quex_lexer_Buffer_memory_size);

    __quex_assert(me->content_space_end   == a2l_quex_lexer_Buffer_memory_content_space_end);
    __quex_assert(me->content_space_size  == a2l_quex_lexer_Buffer_memory_content_space_size);

    __quex_assert(me->content_begin       == a2l_quex_lexer_Buffer_memory_content_begin);
    __quex_assert(me->content_end         == a2l_quex_lexer_Buffer_memory_content_end);
    __quex_assert(me->content_size        == a2l_quex_lexer_Buffer_memory_content_size);
}

inline void
a2l_quex_lexer_Buffer_assert_no_lexatom_is_buffer_border(const a2l_quex_lexer_lexatom_t* Begin, 
                                                     const a2l_quex_lexer_lexatom_t* End)
{
    const a2l_quex_lexer_lexatom_t* iterator = 0x0;
    __quex_assert(Begin <= End);

    for(iterator = Begin; iterator != End; ++iterator) {
        if( *iterator != QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER ) continue;

        if( iterator == Begin ) {
            QUEX_ERROR_EXIT("Buffer limit code character appeared as first character in buffer.\n"
                            "This is most probably a load failure.\n");
        } else {
            QUEX_ERROR_EXIT("Buffer limit code character appeared as normal text content.\n");
        }
    }
}



#endif

#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__ASSERTS_I */

