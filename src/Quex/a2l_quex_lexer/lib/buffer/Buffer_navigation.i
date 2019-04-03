/* vim: set ft=c:
 * 
 * PURPOSE: Buffer's seek: 
 *  
 *    .---------------------------------------------------------------.
 *    |  Setting the '_read_p' to a specific position in the stream.  |
 *    '---------------------------------------------------------------'
 *
 * This is the type of 'seek' used in the user interface's seek functions.
 *
 * NOT TO CONFUSE with two other forms of seek:
 *
 *    -- LexatomLoader's seek sets the input position of the next 
 *       lexatom to be loaded into the buffer.
 *    -- a2l_quex_lexer_ByteLoader's seek sets the position in the low level input
 *       stream.
 *
 * A 'seek' always implies that the following happens:
 *
 *                      _lexeme_start_p = _read_p  
 * 
 * The two stored lexatoms will be assigned after seeking as
 *
 *       _lexatom_at_lexeme_start     = _read_p[0]
 *       _lexatom_before_lexeme_start = _read_p[-1]
 * 
 * If the read pointer stands at the beginning of the file, then
 *
 *       _lexatom_before_lexeme_start = newline
 *
 * It is crucial to understand the difference between 'stream seeking' and 
 * 'buffer seeking'. Stream seeking determines the next position in the input
 * stream from where content is loaded into the buffer. Buffer seeking sets
 * the input pointer '_read_p' to a particular position. The position-1 where 
 * it points contains the next lexatom to be read during analysis.           
 *
 * (C) Frank-Rene Schaefer                                                    */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_NAVIGATION_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_NAVIGATION_I



inline a2l_quex_lexer_stream_position_t  
a2l_quex_lexer_Buffer_input_lexatom_index_begin(a2l_quex_lexer_Buffer* me)
/* Determine lexatom index of first lexatom in the buffer.                    */
{
    __quex_assert(me->input.lexatom_index_begin >= 0);
    return me->input.lexatom_index_begin;
}

inline bool  
a2l_quex_lexer_Buffer_seek_forward(a2l_quex_lexer_Buffer* me, const ptrdiff_t CharacterN)
/* Move '_read_p' forwards by 'CharacterN'. This may involve reload in 
 * forward direction. 
 *
 * Seeking error => Buffer is completely left as is. In particular no change
 *                  to '_read_p' or '_lexeme_start_p'. 
 * 
 * RETURNS: True -- if positioning was successful,
 *          False -- else.                                                    */
{
    a2l_quex_lexer_stream_position_t  CharacterIndexAtReadP =   me->input.lexatom_index_begin
                                                       + (me->_read_p - me->content_begin(me));
    a2l_quex_lexer_stream_position_t  target = CharacterIndexAtReadP + CharacterN;

    if( ! CharacterN ) {
        return true;
    }
    else if( target < a2l_quex_lexer_Buffer_input_lexatom_index_end(me) ) {
        me->_read_p += CharacterN;
        /* => &me->_read_p[-1] inside buffer.                                */
    }
    else if(    me->input.lexatom_index_end_of_stream != -1 
             && target > me->input.lexatom_index_end_of_stream ) {
        return false;
    }
    else {
        /* Character index at read_p = lexatom index at begin + offset     */
        if( ! a2l_quex_lexer_Buffer_load_forward_to_contain(me, target) ) {
            a2l_quex_lexer_Buffer_assert_consistency(me);
            return false;
        }
        me->_read_p = &me->content_begin(me)[target - me->input.lexatom_index_begin];
    }
    me->_lexeme_start_p = me->_read_p;

    return a2l_quex_lexer_Buffer_finish_seek_based_on_read_p(me);
}

inline bool  
a2l_quex_lexer_Buffer_seek_backward(a2l_quex_lexer_Buffer* me, 
                                const ptrdiff_t    CharacterN)
/* Move '_read_p' backwards by 'CharacterN'. This may involve reload in
 * backward direction.                                                   
 *
 * Seeking error => Buffer is completely left as is. In particular no change
 *                  to '_read_p' or '_lexeme_start_p'. 
 * 
 * RETURNS: True  -- if positioning was successful, 
 *          False -- else.                                                   */
{
    a2l_quex_lexer_stream_position_t  CharacterIndexAtReadP =   me->input.lexatom_index_begin
                                                       + (me->_read_p - me->content_begin(me));
    a2l_quex_lexer_stream_position_t  target           = CharacterIndexAtReadP - CharacterN;
    const ptrdiff_t            ContentSpaceSize = me->content_space_size(me); 
    a2l_quex_lexer_stream_position_t  new_lexatom_index_begin;
    ptrdiff_t                  offset;

    if( ! CharacterN ) {
        return true;
    }
    else if( target > me->input.lexatom_index_begin ) {
        /* => &me->_read_p[-1] inside buffer.                                */
        me->_read_p -= CharacterN;
    }
    else {
        /* offset = desired distance from begin to 'read_p'.                 */
        offset                  = (ptrdiff_t)QUEX_MIN((a2l_quex_lexer_stream_position_t)(ContentSpaceSize >> 1), target);
        new_lexatom_index_begin = target - offset;

        if( ! a2l_quex_lexer_Buffer_load_backward_to_contain(me, new_lexatom_index_begin) ) {
            /* QUEX_ERROR_EXIT() initiated inside above function.            */
            return false;
        }
        me->_read_p = &me->content_begin(me)[offset];
    }

    return a2l_quex_lexer_Buffer_finish_seek_based_on_read_p(me);
}

inline a2l_quex_lexer_stream_position_t  
a2l_quex_lexer_Buffer_tell(a2l_quex_lexer_Buffer* me)
/* RETURNS: lexatom index which corresponds to the position of the input
 *          pointer.                                                         */
{
    const a2l_quex_lexer_stream_position_t Delta = me->_read_p - me->content_begin(me);
    return Delta + me->input.lexatom_index_begin;
}

inline bool    
a2l_quex_lexer_Buffer_seek(a2l_quex_lexer_Buffer*              me, 
                       const a2l_quex_lexer_stream_position_t LexatomIndex)
/* Set the _read_p according to a lexatom index of the input. 
 *
 * RETURNS: 'true' in case of success.
 *          'false', else.
 *
 * FAILURE: Due to a errors in seek-operations of the input stream, this may 
 *          totally fail. Then, check 'a2l_quex_lexer_Buffer_dysfunctional'.      */
{
    const a2l_quex_lexer_stream_position_t lexatom_index_read_p = a2l_quex_lexer_Buffer_tell(me);
    bool  verdict_f = false;

    if( LexatomIndex > lexatom_index_read_p ) {
        verdict_f = a2l_quex_lexer_Buffer_seek_forward(me, (ptrdiff_t)(LexatomIndex - lexatom_index_read_p));
    }
    else if( LexatomIndex < lexatom_index_read_p ) {
        verdict_f = a2l_quex_lexer_Buffer_seek_backward(me,(ptrdiff_t)(lexatom_index_read_p - LexatomIndex));
    }
    return verdict_f;
}

inline bool
a2l_quex_lexer_Buffer_finish_seek_based_on_read_p(a2l_quex_lexer_Buffer* me)
{
    a2l_quex_lexer_lexatom_t* BeginP    = me->content_begin(me);
    bool               verdict_f = true;

    if( me->_read_p > me->content_end(me) ) {
        me->_read_p = me->content_end(me);
        verdict_f   = false;
    }
    else if( me->_read_p < BeginP ) {
        me->_read_p = BeginP;
        verdict_f   = false;
    }

    me->_lexeme_start_p = me->_read_p;
    a2l_quex_lexer_Buffer_assert_consistency(me);
    return verdict_f;
}


#endif                  /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_NAVIGATION_I */
