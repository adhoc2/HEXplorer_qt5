/* -*- C++ -*-  vim: set syntax=cpp:
 * (C) 2007-2015 Frank-Rene Schaefer  */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_CONVERTER_RAW_BUFFER_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_CONVERTER_RAW_BUFFER_I

#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader_Converter"
#include "a2l_quex_lexer\lib\quex/MemoryManager"



inline void   
a2l_quex_lexer_RawBuffer_init(a2l_quex_lexer_RawBuffer* me, 
                          uint8_t*              Begin, 
                          size_t                SizeInBytes)
/* Initialize raw buffer. 
 * (1) Begin != 0 => Assign memory. 
 * (2) Begin == 0 => Only reset pointers, so buffer is 'empty'.              */
{
    if( Begin ) {
        me->begin      = Begin;
        me->memory_end = &Begin[(ptrdiff_t)SizeInBytes];
    }
    me->fill_end_p        = &me->begin[0];
    me->next_to_convert_p = &me->begin[0];            /* --> trigger reload. */

    QUEX_IF_ASSERTS_poison(me->begin, me->memory_end);
}

inline void 
a2l_quex_lexer_RawBuffer_move_away_passed_content(a2l_quex_lexer_RawBuffer*  me)
/* Consider any content in the raw buffer from begin to 'next_to_convert_p' as
 * passed and useless. Thus, move what comes behind to the beginning of the 
 * buffer. Adapt:
 *
 *     -- '.fill_end_p'
 *     -- '.next_to_convert_p'
 *
 * The relation of '.next_to_convert_p' and '.next_to_convert_character_index' 
 * remains unaffected. The pointer still points to the same character index. */
{
    uint8_t*   move_begin_p;
    ptrdiff_t  move_size;
    ptrdiff_t  move_distance;
   
    __quex_assert(me->next_to_convert_p <= me->fill_end_p);
    QUEX_ASSERT_RAW_BUFFER(me);

    move_begin_p  = me->next_to_convert_p;
    move_size     = me->fill_end_p - me->next_to_convert_p;
    move_distance = me->next_to_convert_p - me->begin;

    if( ! move_distance ) {
        return;
    }
    else if( move_size ) {
        __QUEX_STD_memmove((void*)me->begin, (void*)move_begin_p, (size_t)move_size);
    }

    me->next_to_convert_p  = me->begin; 
    me->fill_end_p        -= move_distance;

    QUEX_IF_ASSERTS_poison(me->fill_end_p, me->memory_end);
    QUEX_ASSERT_RAW_BUFFER(me);
}

inline bool 
a2l_quex_lexer_RawBuffer_load(a2l_quex_lexer_RawBuffer*  me,
                          a2l_quex_lexer_ByteLoader* byte_loader, 
                          bool*                  end_of_stream_f)  
/* Try to fill the me buffer to its limits with data from the file.  The
 * filling starts from its current position, thus the remaining bytes to be
 * translated are exactly the number of bytes in the buffer.                 
 *
 * '*end_of_stream_f' = true, if the ByteLoader reported 'END OF STREAM',
 *                            or if it returned ZERO BYTES.
 *                      false, else.
 * 
 * NOTE: Some ByteLoaders (such as the POSIX ByteLoader) are not capable of 
 *       telling whether the end of stream occurred. 
 *       
 *           END OF STREAM             NOT => 'end_of_stream_f = true'
 *
 *       But, 
 *
 *           'end_of_stream_f == true'     => END OF STREAM 
 *                                                                           */
{
    uint8_t*  fill_begin_p;
    size_t    load_request_n;
    size_t    loaded_byte_n;

    QUEX_ASSERT_RAW_BUFFER(me);

    /* Move content that has not yet been converted to the buffer's begin.   */
    a2l_quex_lexer_RawBuffer_move_away_passed_content(me);

    fill_begin_p    = me->fill_end_p;
    load_request_n  = (size_t)(me->memory_end - fill_begin_p);
    /* load(): Blocks until either bytes are received, or the end-of-stream
     *         occurs. In the latter case zero bytes are received. The end-
     *         of-stream, may also be detected in other cases--as hint.      */
    loaded_byte_n   = byte_loader->load(byte_loader, fill_begin_p, load_request_n, 
                                        end_of_stream_f);
    me->fill_end_p  = &fill_begin_p[loaded_byte_n];

    if( ! loaded_byte_n ) {
        *end_of_stream_f = true;
    }

    QUEX_ASSERT_RAW_BUFFER(me);
    return loaded_byte_n == load_request_n;
}

inline void 
a2l_quex_lexer_RawBuffer_print_this(a2l_quex_lexer_RawBuffer*  me)
{
    QUEX_DEBUG_PRINT("      raw_buffer: {\n");
    /* QUEX_DEBUG_PRINT("        ownership:         %s;\n", E_Ownership_NAME(me->ownership)); */
    QUEX_DEBUG_PRINT3("        buffer:            { begin: ((%p)) end: ((%p)) size: %i; }\n",
                      (void*)me->begin, (void*)me->memory_end, 
                      (int)(me->memory_end - me->begin));
    QUEX_DEBUG_PRINT("        fill_end_p:        ");
    quex::print_relative_positions((void*)me->begin, (void*)me->memory_end,
                                             sizeof(uint8_t), (void*)me->fill_end_p);
    QUEX_DEBUG_PRINT("\n");
    QUEX_DEBUG_PRINT("        next_to_convert_p: ");
    quex::print_relative_positions((void*)me->begin, (void*)me->memory_end,
                                             sizeof(uint8_t), (void*)me->next_to_convert_p);
    QUEX_DEBUG_PRINT("\n");
    QUEX_DEBUG_PRINT("      }\n");
}



#endif /*  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_CONVERTER_RAW_BUFFER_I */
