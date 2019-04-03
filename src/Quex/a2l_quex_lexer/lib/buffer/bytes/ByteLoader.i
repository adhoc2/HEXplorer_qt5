/* vim: set ft=c:
 * (C) Frank-Rene Schaefer */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_I



inline a2l_quex_lexer_stream_position_t
a2l_quex_lexer_ByteLoader_tell(a2l_quex_lexer_ByteLoader* me);

inline void
a2l_quex_lexer_ByteLoader_seek(a2l_quex_lexer_ByteLoader* me, a2l_quex_lexer_stream_position_t Position);

inline size_t                    
a2l_quex_lexer_ByteLoader_load(a2l_quex_lexer_ByteLoader* me, void* begin_p, const size_t N, bool* end_of_stream_f);

inline void                    
a2l_quex_lexer_ByteLoader_print_this(a2l_quex_lexer_ByteLoader* me);

inline void
a2l_quex_lexer_ByteLoader_construct(a2l_quex_lexer_ByteLoader* me, 
                     a2l_quex_lexer_stream_position_t  (*tell)(a2l_quex_lexer_ByteLoader* me),
                     void                       (*seek)(a2l_quex_lexer_ByteLoader* me, a2l_quex_lexer_stream_position_t Pos),
                     size_t                     (*load)(a2l_quex_lexer_ByteLoader*, void*, const size_t, bool*),
                     void                       (*delete_self)(a2l_quex_lexer_ByteLoader*),
                     void                       (*print_this)(a2l_quex_lexer_ByteLoader*),
                     bool                       (*compare_handle)(const a2l_quex_lexer_ByteLoader*, 
                                                                  const a2l_quex_lexer_ByteLoader*))
{
    me->tell               = a2l_quex_lexer_ByteLoader_tell;
    me->seek               = a2l_quex_lexer_ByteLoader_seek;
    me->load               = a2l_quex_lexer_ByteLoader_load;
    me->print_this         = a2l_quex_lexer_ByteLoader_print_this;
    me->derived.tell       = tell;
    me->derived.seek       = seek;
    me->derived.load       = load;
    me->derived.print_this = print_this;
    me->delete_self        = delete_self;
    me->compare_handle     = compare_handle;
    me->on_nothing         = (bool  (*)(struct a2l_quex_lexer_ByteLoader_tag*, size_t, size_t))0;

    me->handle_ownership = E_Ownership_EXTERNAL; /* Default                  */

    me->initial_position = tell(me);

    me->binary_mode_f    = false;                /* Default: 'false' is SAFE */
    me->element_size     = 1;                    /* Default: 1 [byte]        */
}

inline a2l_quex_lexer_stream_position_t
a2l_quex_lexer_ByteLoader_tell(a2l_quex_lexer_ByteLoader* me)
{
    return me->derived.tell(me);
}

inline void
a2l_quex_lexer_ByteLoader_seek_disable(a2l_quex_lexer_ByteLoader* me)
{
    me->derived.seek = (void (*)(a2l_quex_lexer_ByteLoader*, a2l_quex_lexer_stream_position_t))0;
}

inline bool
a2l_quex_lexer_ByteLoader_seek_is_enabled(a2l_quex_lexer_ByteLoader* me)
{
    return me->derived.seek ? true : false;
}

inline void
a2l_quex_lexer_ByteLoader_seek(a2l_quex_lexer_ByteLoader* me, a2l_quex_lexer_stream_position_t Position)
{
    if     ( Position < me->initial_position ) return;
    else if( ! me->derived.seek )              return;
    me->derived.seek(me, Position);
}

inline size_t                    
a2l_quex_lexer_ByteLoader_load(a2l_quex_lexer_ByteLoader* me, void* begin_p, const size_t N, bool* end_of_stream_f)
/* RETURNS: != 0, if something could be loaded
 *          == 0, if nothing could be loaded further. End of stream (EOS).   
 *
 * Additionally, 'end_of_stream_f' may hint the end of the stream while still
 * some bytes have been loaded. 
 *
 *    *end_of_stream_f == true  => End of stream has been reached.
 *    *end_of_stream_f == false => No assumption if end of stream ore not.
 *
 * The first case is solely a hint to help the caller to act upon end of stream
 * before actually reading zero bytes. It may spare a unnecessary load-cycle
 * which ends up with no load at all.                                        */
{
    size_t    loaded_n;
    ptrdiff_t try_n = 0;
   
    *end_of_stream_f = false;

    if( ! N ) {
        return 0;
    }

    do {
        ++try_n;

        /* Try to load 'N' bytes.                                            */
        loaded_n = me->derived.load(me, begin_p, N, end_of_stream_f);
        if( loaded_n ) {
            /* If at least some bytes could be loaded, return 'success'.     */
            return loaded_n;
        }

    } while( me->on_nothing && me->on_nothing(me, (size_t)try_n, N) );

    /* If user's on nothing returns 'false' no further attemps to read.      */
    *end_of_stream_f = true;

    return 0;
}

inline bool
a2l_quex_lexer_ByteLoader_is_equivalent(const a2l_quex_lexer_ByteLoader* A, 
                                    const a2l_quex_lexer_ByteLoader* B)
/* RETURNS: true -- if A and B are equivalent.
 *          false -- else.                                                   */
{
    /* If two QUEX_NAME(ByteLoader )classes use the same 'load()' function, then they 
     * should not be different. For example, it does not make sense to have
     * two loaders implementing stdandard libraries 'fread()' interface.     
     *
     * Further, it is always safe to return 'false'.                         */
    if( A == NULL ) {
        if( B != NULL ) return false; 
        else            return true;
    }
    else if( B == NULL ) {
        return false;
    }
    else if( A->load != B->load ) {
        return false;
    }

    /* The 'compare_handle()' function can now safely cast the two pointers
     * to its pointer type.                                                  */
    return A->compare_handle(A, B);
}

inline void  
a2l_quex_lexer_ByteLoader_delete(a2l_quex_lexer_ByteLoader** me)
{
    if( ! *me )                   return;
    else if( (*me)->delete_self ) (*me)->delete_self(*me);
    (*me) = (a2l_quex_lexer_ByteLoader*)0;
}

inline void                    
a2l_quex_lexer_ByteLoader_print_this(a2l_quex_lexer_ByteLoader* me)
{
    QUEX_DEBUG_PRINT("      byte_loader: {\n");
    QUEX_DEBUG_PRINT1("        handle_ownership: %s;\n", E_Ownership_NAME(me->handle_ownership));
    QUEX_DEBUG_PRINT1("        binary_mode_f:    %s;\n", E_Boolean_NAME(me->binary_mode_f));
    QUEX_DEBUG_PRINT1("        element_size:     %i;\n", (int)me->element_size); 
    QUEX_DEBUG_PRINT1("        initial_position: %i;\n", (int)me->initial_position); 
    QUEX_DEBUG_PRINT1("        current_position: %i;\n", (int)me->tell(me)); 
    if( me->derived.print_this ) {
        me->derived.print_this(me);
    }
    QUEX_DEBUG_PRINT("      }\n");
}



#include "a2l_quex_lexer\lib\buffer/bytes/ByteLoader_FILE.i"
#include "a2l_quex_lexer\lib\buffer/bytes/ByteLoader_stream.i"
#include "a2l_quex_lexer\lib\buffer/bytes/ByteLoader_wstream.i"

#endif /*  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_I */
