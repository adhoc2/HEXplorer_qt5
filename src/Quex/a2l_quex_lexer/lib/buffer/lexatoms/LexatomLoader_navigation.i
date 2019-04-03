/* -*- C++ -*- vim: set syntax=cpp: */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_NAVIGATION_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_NAVIGATION_I
/* PURPOSE: LexatomLoader's seek: 
 *
 *        .--------------------------------------------------------------.
 *        |  Setting the lexatom index of the next lexatom to be filled  |
 *        |                         into the buffer.                     |
 *        '--------------------------------------------------------------'
 *
 * NOT TO CONFUSE with two other forms of seek:
 *
 *    -- Buffer's seek sets the '_read_p' of the analyzer to a lexatom
 *       that is to be treated next.
 *    -- a2l_quex_lexer_ByteLoader's seek sets the position in the low level input
 *       stream.
 * 
 * A LexatomLoader's seek has NO direct influence on the content or the pointers
 * of the Buffer object that it fills.
 *
 * The difference between the a2l_quex_lexer_ByteLoader's seeking (stream seeking) 
 * and the LexatomLoader's seeking is that the a2l_quex_lexer_ByteLoader understands 
 * the position independently of its meaning. The LexatomLoader already 
 * interprets the stream as 'lexatoms'. A a2l_quex_lexer_ByteLoader sets the stream 
 * to a particular byte position. A LexatomLoader prepare the input of a lexatom.
 *
 * (C) Frank-Rene Schaefer                                                   */

#include "a2l_quex_lexer\lib\definitions"
#include "a2l_quex_lexer\lib\buffer/Buffer"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader"
#include "a2l_quex_lexer\lib\buffer/Buffer_print"
#include "a2l_quex_lexer\lib\quex/MemoryManager"



inline void  a2l_quex_lexer_LexatomLoader_lexatom_index_reset_backup(a2l_quex_lexer_LexatomLoader* me, 
                                             a2l_quex_lexer_stream_position_t Backup_lexatom_index_next_to_fill, 
                                             ptrdiff_t                 BackupStomachByteN, 
                                             a2l_quex_lexer_stream_position_t BackupByteLoaderPosition);
inline a2l_quex_lexer_stream_position_t 
a2l_quex_lexer_LexatomLoader_lexatom_index_tell(a2l_quex_lexer_LexatomLoader* me)
{
    return me->lexatom_index_next_to_fill;
}


inline bool 
a2l_quex_lexer_LexatomLoader_lexatom_index_seek(a2l_quex_lexer_LexatomLoader*         me, 
                                             const a2l_quex_lexer_stream_position_t  CharacterIndex) 
/* LexatomLoader's seek sets the input position for the next lexatom load in
 * the stream. That is, it adapts:
 *
 *     'lexatom_index_next_to_fill = CharacterIndex' 
 *
 * and the byte loader is brought into a position so that this will happen.  
 *
 * RETURNS: true upon success, false else.                                   */
{ 
    ptrdiff_t                      backup_stomach_byte_n; 
    a2l_quex_lexer_stream_position_t      backup_byte_loader_position; 
    a2l_quex_lexer_stream_position_t      backup_lexatom_index_next_to_fill;
    a2l_quex_lexer_stream_position_t      target_byte_pos;

    if( me->lexatom_index_next_to_fill == CharacterIndex ) return true;

    /* Converter reports '-1' => unable to determine the number of bytes in
     *                           the stomach. 
     * => When backup position is to setup, start stepping from beginning.
     * (This is currently only an issue with ICU; IConv behaves well)        */
    backup_stomach_byte_n             = me->stomach_byte_n(me);
    backup_byte_loader_position       = me->byte_loader->tell(me->byte_loader);
    backup_lexatom_index_next_to_fill = me->lexatom_index_next_to_fill;

    if( me->byte_n_per_lexatom != -1 ) {
        /* LINEAR RELATION between lexatom index and stream position       
         * (It is not safe to assume that it can be computed)                */
        target_byte_pos =   CharacterIndex * me->byte_n_per_lexatom
                          + me->byte_loader->initial_position;

        me->byte_loader->seek(me->byte_loader, target_byte_pos);
        if( me->byte_loader->tell(me->byte_loader) != target_byte_pos ) {
            me->byte_loader->seek(me->byte_loader, backup_byte_loader_position);
            return false;
        }
        me->lexatom_index_next_to_fill = CharacterIndex;
        me->stomach_clear(me);
        return true;
    }

    /* STEPPING (WITHOUT COMPUTING) to the input position.                   */
    if( CharacterIndex < me->lexatom_index_next_to_fill ) {
        /* Character index lies backward, so stepping needs to start from 
         * the initial position.                                             */
        if( ! a2l_quex_lexer_LexatomLoader_lexatom_index_reset(me) ) {
            return false;
        }
    }

    /* step_forward_n_lexatoms() calls derived.load_lexatoms() 
     * which increments 'lexatom_index_next_to_fill'.                        */
    if( ! a2l_quex_lexer_LexatomLoader_lexatom_index_step_to(me, (ptrdiff_t)CharacterIndex) ) {
        a2l_quex_lexer_LexatomLoader_lexatom_index_reset_backup(me, 
                                                            backup_lexatom_index_next_to_fill, 
                                                            backup_stomach_byte_n, 
                                                            backup_byte_loader_position);
        return false;
    }
    __quex_assert(me->lexatom_index_next_to_fill == CharacterIndex);
    return true;
}

inline bool
a2l_quex_lexer_LexatomLoader_lexatom_index_reset(a2l_quex_lexer_LexatomLoader* me)
/* Set the lexatom index position to '0' and the byte loader to the initial
 * position. The 'stomach' of derived buffer fillers is cleared, so that 
 * filling may start from the beginning.                                     
 * 
 * RETURNS: true, if initial position has been reached.
 *          false, if positioning failed.                                    */
{
    if( me->byte_loader ) {
        me->byte_loader->seek(me->byte_loader, me->byte_loader->initial_position);
        if( me->byte_loader->tell(me->byte_loader) != me->byte_loader->initial_position ) {
            QUEX_ERROR_EXIT("QUEX_NAME(ByteLoader )failed to seek to initial position.\n");
            return false;
        }
    }
    me->lexatom_index_next_to_fill = 0;
    me->stomach_clear(me);
    return true;
}

inline void
a2l_quex_lexer_LexatomLoader_lexatom_index_reset_backup(a2l_quex_lexer_LexatomLoader* me, 
                                                    a2l_quex_lexer_stream_position_t Backup_lexatom_index_next_to_fill, 
                                                    ptrdiff_t                 BackupStomachByteN, 
                                                    a2l_quex_lexer_stream_position_t BackupByteLoaderPosition)
/* Reset a previous state of the LexatomLoader and its a2l_quex_lexer_ByteLoader.            */
{
    a2l_quex_lexer_stream_position_t backup_byte_pos;

    if( BackupStomachByteN == -1 ) {
        /* Since it was not possible to determine the number of bytes in
         * the converter's stomach, the backup position must be reached 
         * by starting from the begining.                                    */
        if( ! a2l_quex_lexer_LexatomLoader_lexatom_index_reset(me) ) {
            return;
        }
        else if( ! a2l_quex_lexer_LexatomLoader_lexatom_index_step_to(me, (ptrdiff_t)Backup_lexatom_index_next_to_fill) ) {
            QUEX_ERROR_EXIT("LexatomLoader failed to seek previously reached lexatom.\n");
        }
    }
    else {
        backup_byte_pos = BackupByteLoaderPosition - BackupStomachByteN;
        me->byte_loader->seek(me->byte_loader, backup_byte_pos);
        me->stomach_clear(me);
        me->lexatom_index_next_to_fill = Backup_lexatom_index_next_to_fill;
    }
}

inline bool 
a2l_quex_lexer_LexatomLoader_lexatom_index_step_to(a2l_quex_lexer_LexatomLoader*       me,
                                               const a2l_quex_lexer_stream_position_t TargetCI)
/* From the given 'lexatom_index_next_to_fill' (i.e. the return value of 
 * 'input_lexatom_tell()') step forward to lexatom index 'TargetCI'. This 
 * function is used to reach a target input position in cases where computing 
 * is impossible.
 *
 * THIS FUNCTION DOES NOT BACKUP OR RE-INIT ANYTHING.
 * => Must be done upon failure in the caller's function.
 *
 * RETURNS: true - success; false - else.                                    */
{ 
    const a2l_quex_lexer_stream_position_t ChunkSize = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_LOADER_SEEK_BUFFER_SIZE;
    a2l_quex_lexer_lexatom_t               chunk[QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_LOADER_SEEK_BUFFER_SIZE];
    a2l_quex_lexer_stream_position_t       remaining_n      = TargetCI - me->lexatom_index_next_to_fill;
    bool                            end_of_stream_f  = false;
    bool                            encoding_error_f = false;
    size_t                          loaded_n;

    __quex_assert(QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_LOADER_SEEK_BUFFER_SIZE >= 1);

    for(; remaining_n > ChunkSize; remaining_n -= ChunkSize ) {
        loaded_n = me->derived.load_lexatoms(me, &chunk[0], (size_t)ChunkSize, 
                                             &end_of_stream_f, &encoding_error_f);
        me->lexatom_index_next_to_fill += (a2l_quex_lexer_stream_position_t)loaded_n;
        if( ChunkSize > (a2l_quex_lexer_stream_position_t)loaded_n ) {
            return false;
        }
    }
    if( remaining_n ) {
        loaded_n = me->derived.load_lexatoms(me, &chunk[0], (size_t)remaining_n, 
                                             &end_of_stream_f, &encoding_error_f);
        me->lexatom_index_next_to_fill += (a2l_quex_lexer_stream_position_t)loaded_n;
        if( remaining_n > (a2l_quex_lexer_stream_position_t)loaded_n ) {
            return false;
        }
    }
   
    __quex_assert(me->lexatom_index_next_to_fill == TargetCI);
    return true;
}




#endif /*   QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_NAVIGATION_I */
