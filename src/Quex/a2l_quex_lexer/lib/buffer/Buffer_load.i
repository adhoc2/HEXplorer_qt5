/* vim:set ft=c: -*- C++ -*- */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_LOAD_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_LOAD_I

#include "a2l_quex_lexer\lib\buffer/asserts"
#include "a2l_quex_lexer\lib\definitions"
#include "a2l_quex_lexer\lib\buffer/Buffer"
#include "a2l_quex_lexer\lib\buffer/Buffer_print.i"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader"
#include "a2l_quex_lexer\lib\quex/asserts"
#include "a2l_quex_lexer\lib\quex/MemoryManager"



inline E_LoadResult
a2l_quex_lexer_Buffer_load_forward(a2l_quex_lexer_Buffer*  me,
                               a2l_quex_lexer_lexatom_t** position_register,
                               const size_t        PositionRegisterN)
/* Load a maximum amount of new content into the buffer from input stream. 
 * Pointers and indices are adapted, so that they relate to the same content 
 * in the input stream as before the call. 
 *
 *        .-----------------------------------------------------.
 *        |     BLOCKING wait for incoming stream content.      |
 *        | No return without content--EXCEPT at end of stream. |
 *        '-----------------------------------------------------'
 *
 * Before:    
 *            | . . . . . . . . .x.x.x.x.x.x.x.x.x.x.x| 
 *                              |<---- move size ---->|
 *
 * After:     |<- move distance |
 *
 *            |x.x.x.x.x.x.x.x.x.x.x|N.N.N.N.N.N.N. . | 
 *            |<--- move_size ----->|- loaded_n ->|
 *
 * RETURNS: 
 *          
 *     DONE              => Something has been loaded   (analysis MAY CONTINUE)
 *     OVERFLOW          => Current lexeme spans buffer.(analysis MUST STOP)
 *     ENCODING_ERROR    => Failed. conversion error    (analysis MUST STOP)
 *                          or, read pointer on buffer limit code,
 *                          but it was not a buffer limit.
 *     NO_MORE_DATA      => No more data available.     (analysis MUST STOP)
 *                                                                            */
{
    ptrdiff_t  move_distance;
    ptrdiff_t  loaded_n;
    bool       encoding_error_f = false;

    a2l_quex_lexer_Buffer_assert_consistency(me);

    if( me->_read_p != me->content_end(me)) {
        /* If the read pointer does not stand on the end of input pointer, then
         * the 'buffer limit code' at the read pointer is a bad lexatom.    
         * Buffer limit codes cannot be possibly be part of buffer content.  */
        return E_LoadResult_ENCODING_ERROR;
    }
    else if( ! me->filler || ! me->filler->byte_loader ) {
        a2l_quex_lexer_Buffer_register_eos(me, me->input.lexatom_index_begin + me->content_size(me));
        return E_LoadResult_NO_MORE_DATA;  /* No filler/loader => no load!   */
    }

    /* Move remaining content.
     * Maintain lexeme and fallback.                 
     * Adapt pointers.                                                       */
    move_distance = a2l_quex_lexer_Buffer_move_get_max_distance_towards_begin(me); 

    if(    0 == move_distance 
        && ! a2l_quex_lexer_Buffer_callbacks_on_cannot_move_towards_begin(me, &move_distance) ) {
        return E_LoadResult_OVERFLOW; 
    }

    if( ! a2l_quex_lexer_Buffer_move_and_load(me, position_register, PositionRegisterN, 
                                          move_distance, &encoding_error_f, &loaded_n) ) {
        return E_LoadResult_OVERFLOW;
    }
    __quex_debug_buffer_load(me, "LOAD FORWARD(exit)\n");
    a2l_quex_lexer_Buffer_assert_consistency(me);
    if     ( encoding_error_f ) return E_LoadResult_ENCODING_ERROR;
    else if( loaded_n )         return E_LoadResult_DONE;
    else                        return E_LoadResult_NO_MORE_DATA;
}

inline bool
a2l_quex_lexer_Buffer_load_forward_to_contain(a2l_quex_lexer_Buffer*        me, 
                                          a2l_quex_lexer_stream_position_t LexatomIndexToBeContained)
/* Load new content into the buffer, so that a specific lexatom index is 
 * contained inside the buffer. In underlying seek procedure failed, it is
 * tried to restore the buffer to a state prior to this call.
 *
 *        .-----------------------------------------------------.
 *        |     BLOCKING wait for incoming stream content.      |
 *        | No return without content--EXCEPT at end of stream. |
 *        '-----------------------------------------------------'
 * 
 * Before:    
 *            | . . . . . . . . .x.x.x.x.x.x.x.x.x.x.x| 
 *
 * After:                                .--------------- LexatomIndex..
 *            |x.x.x.x.x.x.x.x.x.x.x|N.N.N.N.N.N.N. . |   
 *            |- move_size -------->|- loaded_n ->|
 *                                                             
 *
 * RETURNS:  true, in case of success.
 *           false, in case of FAILURE.
 *
 * FAILURE:  In case of error in underlying lexatom/byte loader, if it is not 
 *           possible to restore the previous content of the buffer the buffer is 
 *           *disabled*. 
 *
 *        => In case of 'return false', call 'a2l_quex_lexer_Buffer_dysfunctional */
{
    a2l_quex_lexer_stream_position_t    lexatom_index_to_be_contained = LexatomIndexToBeContained;
    a2l_quex_lexer_stream_position_t    lexatom_index_end = (a2l_quex_lexer_stream_position_t)-1;
    ptrdiff_t                    load_request_n;
    ptrdiff_t                    loaded_n;
    intmax_t                     move_distance;
    bool                         end_of_stream_f  = false;
    bool                         encoding_error_f = false;
    a2l_quex_lexer_BufferInvariance  bi;

    a2l_quex_lexer_Buffer_assert_consistency(me);

    a2l_quex_lexer_BufferInvariance_construct(&bi, me);

    /* Move existing content in the buffer to appropriate position.           */
    move_distance = a2l_quex_lexer_Buffer_move_get_distance_forward_to_contain(me, 
                                             &lexatom_index_to_be_contained);

    /* Even if 'move_distance = 0' call the 'move and load' for consistency.  */
    (void)a2l_quex_lexer_Buffer_move_and_load(me, (a2l_quex_lexer_lexatom_t**)0, 0,
                                          move_distance, &encoding_error_f, 
                                          &loaded_n);

    lexatom_index_end = me->input.lexatom_index_begin + me->content_size(me);

    if(    LexatomIndexToBeContained == me->input.lexatom_index_end_of_stream 
        && LexatomIndexToBeContained == lexatom_index_end ) {
        return true;
    }
    if( LexatomIndexToBeContained < lexatom_index_end ) {
        return true;
    }
    else {
        load_request_n = a2l_quex_lexer_Buffer_move_towards_begin_undo(me, &bi, move_distance);

        loaded_n       = a2l_quex_lexer_LexatomLoader_load(me->filler, me->content_begin(me), 
                                                       load_request_n, me->input.lexatom_index_begin,
                                                       &end_of_stream_f, &encoding_error_f);
        if( loaded_n != load_request_n ) {
            /* Error: buffer is dysfunctional.                                */
            a2l_quex_lexer_Buffer_dysfunctional_set(me);
        }
        else {
            /* Ensure, that the buffer limit code is restored.                */
            me->input.end_p[0] = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER;
        }
        return false;
    }
}

inline E_LoadResult   
a2l_quex_lexer_Buffer_load_backward(a2l_quex_lexer_Buffer* me)
/* Load *previous* stream content into the buffer so that the analyzer can 
 * continue in backward direction.
 *
 *        .-----------------------------------------------------.
 *        |     BLOCKING wait for incoming stream content.      |
 *        | No return without content--EXCEPT at end of stream. |
 *        '-----------------------------------------------------'
 *
 * RETURNS: 
 *          
 *     DONE              => Something has been loaded   (analysis MAY CONTINUE)
 *     NO_MORE_DATA      => Begin of stream reached.    (analysis MUST STOP)
 *     ENCODING_ERROR    => Failed. Conversion error.   (analysis MUST STOP)
 *     FAILURE           => General load failure.       (analysis MUST STOP)
 *                         
 * In case of 'FAILURE', the buffer is set into a dysfunctional state. I may
 * not be used any longer!
 *_____________________________________________________________________________
 * NO ADAPTIONS OF POST-CONTEXT POSITIONS. Reason: Backward analysis appears
 * only in the following two cases.
 *  
 *  (1) When checking for a pre-condition. This does not involve pre-contexts.
 * 
 *  (2) When tracing back along a 'pseudo-ambigous post context'. However,
 *      the stretch from 'end-of-core' pattern to 'end-of-post context' lies
 *      completely in between 'lexeme start' to 'read '. Thus, one never has
 *      to go farther back then the buffer's begin.                          */
{
    ptrdiff_t           move_distance;
    ptrdiff_t           loaded_n;
    ptrdiff_t           load_request_n;
    bool                encoding_error_f = false;

    a2l_quex_lexer_Buffer_assert_consistency(me);

    __quex_debug_buffer_load(me, "BACKWARD(entry)\n");
    if( me->_read_p != me->begin(me) ) {
        /* If the read pointer does not stand on 'front', then the buffer limit
         * code is a bad character, Buffer limit codes are not supposed to     
         * appear in buffer content.                                         */
        return E_LoadResult_ENCODING_ERROR;
    }
    else if( me->input.lexatom_index_begin == 0 ) {
        return E_LoadResult_NO_MORE_DATA; /* Begin of stream.                */
    }
    else if( ! me->filler || ! me->filler->byte_loader ) {
        return E_LoadResult_NO_MORE_DATA; /* No filler/loader => no loading! */
    }
    else if( ! a2l_quex_lexer_ByteLoader_seek_is_enabled(me->filler->byte_loader) ) {
        return E_LoadResult_NO_MORE_DATA; /* Stream cannot go backwards.     */
    }

    move_distance = a2l_quex_lexer_Buffer_move_get_max_distance_towards_end(me);

    if( 0 == move_distance ) {
        /* Earlier, it has been checked that 'lexatom_index_begin != 0'
         * => move distance == 0 definitely means 'error'.                    */
        a2l_quex_lexer_Buffer_dysfunctional_set(me);
        return E_LoadResult_FAILURE;
    }
    loaded_n = a2l_quex_lexer_Buffer_move_and_load_backward(me, move_distance, &encoding_error_f, &load_request_n);

    if( encoding_error_f ) {
        return E_LoadResult_ENCODING_ERROR;
    }
    else if( loaded_n != move_distance ) {
        /* Serious: previously loaded content could not be loaded again!     
         * => Buffer has now hole: 
         *    from _front[1+loaded_n] to Begin[move_distance]                 
         * The analysis can continue in forward direction, but not backwards.*/
        a2l_quex_lexer_Buffer_dysfunctional_set(me);
        return E_LoadResult_FAILURE;     
    }

    __quex_debug_buffer_load(me, "BACKWARD(exit)\n");
    a2l_quex_lexer_Buffer_assert_consistency(me);
    return E_LoadResult_DONE;     
}

inline bool
a2l_quex_lexer_Buffer_load_backward_to_contain(a2l_quex_lexer_Buffer*        me, 
                                           a2l_quex_lexer_stream_position_t NewCharacterIndexBegin)
/* Load new content into the buffer BACKWARDS, so that a specific lexatom 
 * index is contained inside the buffer. In underlying seek procedure failed, 
 * it is tried to restore the buffer to a state prior to this call.
 *
 *        .-----------------------------------------------------.
 *        |     BLOCKING wait for incoming stream content.      |
 *        | No return without content--EXCEPT at end of stream. |
 *        '-----------------------------------------------------'
 *
 * RETURN: true, in case of success.
 *         false, if the region could not be be filled.
 *                => something is seriously wrong.                            */
{
    /* a2l_quex_lexer_lexatom_t* end_p    = me->content_space_end(me); */
    ptrdiff_t loaded_n;
    ptrdiff_t load_request_n;
    ptrdiff_t move_distance;
    bool      encoding_error_f = false;

    __quex_assert(NewCharacterIndexBegin >= 0);
    __quex_assert(me->input.lexatom_index_begin >= NewCharacterIndexBegin);

    /* (1) Move away content, so that previous content can be reloaded.      */
    move_distance  = (ptrdiff_t)(me->input.lexatom_index_begin - NewCharacterIndexBegin);

    loaded_n = a2l_quex_lexer_Buffer_move_and_load_backward(me, move_distance, 
                                                        &encoding_error_f, 
                                                        &load_request_n);
                           
    /* (3) In case of error, the stream must have been corrupted. Previously
     *     present content is not longer available. Continuation impossible.  */
    if( loaded_n != load_request_n ) {
        return false;
    }
    return true;
}

inline bool
a2l_quex_lexer_Buffer_move_and_load(a2l_quex_lexer_Buffer*  me, 
                                a2l_quex_lexer_lexatom_t** position_register,
                                size_t              PositionRegisterN,
                                ptrdiff_t           move_distance, 
                                bool*               encoding_error_f, 
                                ptrdiff_t*          loaded_n)
/* Move buffer content towards begin and load as much content as possible in
 * the front.
 *
 *        .-----------------------------------------------------.
 *        |     BLOCKING wait for incoming stream content.      |
 *        | No return without content--EXCEPT at end of stream. |
 *        '-----------------------------------------------------'
 *
 * ADAPTS:  'encoding_error_f == true' if an encoding error has been detected.
 *          'loaded_n' containing the number of loaded lexatoms.
 *
 * RETURNS: true, in case of success,
 *          false, if there was no free-space to load.                        */
{
    a2l_quex_lexer_stream_position_t   load_lexatom_index;
    ptrdiff_t                   free_space;
    bool                        end_of_stream_f  = false;

    a2l_quex_lexer_Buffer_move_towards_begin(me, move_distance,
                                         position_register, PositionRegisterN); 
    free_space = me->content_space_end(me) - me->content_end(me);

    if( 0 == free_space ) {
        return false; 
    }

    load_lexatom_index  =   me->input.lexatom_index_begin 
                          + (me->content_end(me) - me->content_begin(me));

    *loaded_n = a2l_quex_lexer_LexatomLoader_load(me->filler, me->content_end(me), 
                                              free_space, load_lexatom_index, 
                                              &end_of_stream_f, encoding_error_f);

    if( (! *loaded_n) || end_of_stream_f ) { /* End of stream detected.       */
        me->input.lexatom_index_end_of_stream = load_lexatom_index + *loaded_n;
    }

    me->input.end_p    = &me->input.end_p[*loaded_n];
    me->input.end_p[0] = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER;
    return true;
}

inline ptrdiff_t
a2l_quex_lexer_Buffer_move_and_load_backward(a2l_quex_lexer_Buffer* me, 
                                         ptrdiff_t          MoveDistance,
                                         bool*              encoding_error_f, 
                                         ptrdiff_t*         load_request_n)
/* Move buffer content towards end and load as much content as possible in
 * the back.
 *
 *        .-----------------------------------------------------.
 *        |     BLOCKING wait for incoming stream content.      |
 *        | No return without content--EXCEPT at end of stream. |
 *        '-----------------------------------------------------'
 *
 * ADAPTS:  'encoding_error_f == true' if an encoding error has been detected.
 *          'load_request_n' containing the number of requested lexatoms to be 
 *                           loaded.
 *
 * RETURNS: Number of loaded lexatoms.                                        */
{
    bool end_of_stream_f  = false;

    (void)a2l_quex_lexer_Buffer_move_towards_end(me, MoveDistance);

    *load_request_n = QUEX_MIN(me->content_space_end(me) - me->content_begin(me), 
                               MoveDistance);

    return a2l_quex_lexer_LexatomLoader_load(me->filler, me->content_begin(me), *load_request_n,
                                         me->input.lexatom_index_begin,
                                         &end_of_stream_f, encoding_error_f);
}



#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader.i"
#include "a2l_quex_lexer\lib\buffer/BufferMemory.i"

#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_LOAD_I */

