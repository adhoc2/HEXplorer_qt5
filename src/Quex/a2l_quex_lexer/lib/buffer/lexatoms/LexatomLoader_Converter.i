/* -*- C++ -*-  vim: set syntax=cpp:
 * (C) 2007-2015 Frank-Rene Schaefer  */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_CONVERTER_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_CONVERTER_I

#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader_Converter"
#include "a2l_quex_lexer\lib\quex/MemoryManager"




inline void
a2l_quex_lexer_LexatomLoader_Converter_construct(a2l_quex_lexer_LexatomLoader_Converter* me, 
                                             a2l_quex_lexer_ByteLoader*              byte_loader,
                                             a2l_quex_lexer_Converter*               converter,
                                             size_t                              RawMemorySize);

inline void   
a2l_quex_lexer_LexatomLoader_Converter_stomach_clear(a2l_quex_lexer_LexatomLoader* alter_ego);

inline void   
a2l_quex_lexer_LexatomLoader_Converter_destruct_self(a2l_quex_lexer_LexatomLoader* alter_ego);

inline ptrdiff_t   
a2l_quex_lexer_LexatomLoader_Converter_stomach_byte_n(a2l_quex_lexer_LexatomLoader* alter_ego);

inline size_t 
a2l_quex_lexer_LexatomLoader_Converter_load_lexatoms(a2l_quex_lexer_LexatomLoader*  alter_ego,
                                                 a2l_quex_lexer_lexatom_t*         RegionBeginP, 
                                                 const size_t               N,
                                                 bool*                      end_of_stream_f,
                                                 bool*                      encoding_error_f);
inline void 
a2l_quex_lexer_LexatomLoader_Converter_fill_prepare(a2l_quex_lexer_LexatomLoader*  alter_ego,
                                                a2l_quex_lexer_Buffer*         buffer,
                                                void**                     begin_p,
                                                const void**               end_p);

inline void 
a2l_quex_lexer_LexatomLoader_Converter_get_fill_boundaries(a2l_quex_lexer_LexatomLoader*  alter_ego,
                                                       a2l_quex_lexer_Buffer*         buffer,
                                                       void**       begin_p, 
                                                       const void** end_p);

inline ptrdiff_t 
a2l_quex_lexer_LexatomLoader_Converter_fill_finish(a2l_quex_lexer_LexatomLoader*   alter_ego,
                                              a2l_quex_lexer_lexatom_t*       insertion_p,
                                              const a2l_quex_lexer_lexatom_t* BufferEnd,
                                              const void*                ContentEnd);

inline E_LoadResult
a2l_quex_lexer_LexatomLoader_call_converter(a2l_quex_lexer_LexatomLoader_Converter* me,
                                        a2l_quex_lexer_lexatom_t**                 insertion_p,
                                        a2l_quex_lexer_lexatom_t*                  RegionBeginP,
                                        const a2l_quex_lexer_lexatom_t*            RegionEndP);
inline void
a2l_quex_lexer_LexatomLoader_remove_spurious_BOM(a2l_quex_lexer_LexatomLoader_Converter* me,
                                            a2l_quex_lexer_lexatom_t**                  buffer_insertion_p,
                                            a2l_quex_lexer_lexatom_t*                   RegionBeginP);

inline void 
a2l_quex_lexer_LexatomLoader_Converter_print_this(a2l_quex_lexer_LexatomLoader* alter_ego);

inline void   
a2l_quex_lexer_RawBuffer_init(a2l_quex_lexer_RawBuffer* me, 
                          uint8_t* Begin, size_t SizeInBytes);
inline void 
a2l_quex_lexer_RawBuffer_move_away_passed_content(a2l_quex_lexer_RawBuffer*  me);

inline bool 
a2l_quex_lexer_RawBuffer_load(a2l_quex_lexer_RawBuffer*   me,
                          a2l_quex_lexer_ByteLoader*  byte_loader, 
                          bool*                   end_of_stream_f);

inline a2l_quex_lexer_LexatomLoader*
a2l_quex_lexer_LexatomLoader_Converter_new(a2l_quex_lexer_ByteLoader* byte_loader,
                                       a2l_quex_lexer_Converter*  converter)
{ 
    a2l_quex_lexer_LexatomLoader_Converter*  me;
    /* UTF-8 char can be 6 bytes long => min. size of translation buffer.    */
    __quex_assert(QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_LOADER_CONVERTER_BUFFER_SIZE >= 6);  

    if( ! converter ) {
        return (a2l_quex_lexer_LexatomLoader*)0;
    }
#   if 0 /* No longer a constraint:                                          */
    else if( byte_loader && ! byte_loader->binary_mode_f ) {
        /* Binary mode is ABSOLUTELY REQUIRED for converters, otherwise the 
         * positioning with respect to the raw buffer becomes unreliable.    */
        QUEX_DEBUG_PRINT("! LexatomLoader_Converter_new: QUEX_NAME(ByteLoader )is not in binary mode. !\n");
        QUEX_DEBUG_PRINT("! Has file been opened in binary mode?                          !\n");
        return (a2l_quex_lexer_LexatomLoader*)0;
    }
#   endif

    /* The 'LexatomLoader_Converter' is the same host for all converters.
     * Converters are pointed to by 'converter',                             */
    me = (a2l_quex_lexer_LexatomLoader_Converter*) \
          quex::MemoryManager_allocate(sizeof(a2l_quex_lexer_LexatomLoader_Converter),
                                         E_MemoryObjectType_BUFFER_FILLER);
    if( ! me) {
        return (a2l_quex_lexer_LexatomLoader*)0;
    }

    a2l_quex_lexer_LexatomLoader_Converter_construct(me, byte_loader, converter, 
                                                 QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_LOADER_CONVERTER_BUFFER_SIZE);

    return &me->base;

}

inline void
a2l_quex_lexer_LexatomLoader_Converter_construct(a2l_quex_lexer_LexatomLoader_Converter* me, 
                                             a2l_quex_lexer_ByteLoader*              byte_loader,
                                             a2l_quex_lexer_Converter*               converter,
                                             size_t                              RawMemorySize)
{
    /* A linear relationship between stream position and lexatom index 
     * requires that: (1) The input stream is in 'binary mode'. That is, the 
     * stream position is proportional to the number of bytes that lie 
     * behind. (2) The input codec is of fixed size, i.e. 
     * converter->byte_n_per_lexatom != -1.                                */ 
    ptrdiff_t   byte_n_per_lexatom = byte_loader && byte_loader->binary_mode_f ? 
                                     converter->byte_n_per_lexatom : -1;
    uint8_t*    raw_memory;

    /* NO LONGER VALID: "Binary mode is absolutely required for converters,
     *                   otherwise the positioning with respect to the raw 
     *                   buffer becomes unreliable."                         */
    /* __quex_assert( ! byte_loader || byte_loader->binary_mode_f );         */

    a2l_quex_lexer_LexatomLoader_setup(&me->base,
                                  a2l_quex_lexer_LexatomLoader_Converter_load_lexatoms,
                                  a2l_quex_lexer_LexatomLoader_Converter_stomach_byte_n,
                                  a2l_quex_lexer_LexatomLoader_Converter_stomach_clear,
                                  a2l_quex_lexer_LexatomLoader_Converter_destruct_self,
                                  a2l_quex_lexer_LexatomLoader_Converter_fill_prepare,
                                  a2l_quex_lexer_LexatomLoader_Converter_fill_finish,
                                  a2l_quex_lexer_LexatomLoader_Converter_get_fill_boundaries,
                                  a2l_quex_lexer_LexatomLoader_Converter_print_this,
                                  byte_loader,
                                  byte_n_per_lexatom);

    /* Initialize the conversion operations                                  */
    me->converter = converter;
    me->converter->virginity_f = true;

    /* Initialize the raw buffer that holds the plain bytes of the input file
     * (setup to trigger initial reload)                                     */
    raw_memory = quex::MemoryManager_allocate(RawMemorySize, 
                                                E_MemoryObjectType_BUFFER_RAW);
    a2l_quex_lexer_RawBuffer_init(&me->raw_buffer, raw_memory, RawMemorySize);

    QUEX_ASSERT_RAW_BUFFER(&me->raw_buffer);
}

inline ptrdiff_t   
a2l_quex_lexer_LexatomLoader_Converter_stomach_byte_n(a2l_quex_lexer_LexatomLoader*  alter_ego)
/* RETURNS: Number of bytes that were read from the input stream, but remained
 *                 inside the 'stomach' without being filled into the drain.
 *          '-1'   reports that the LexatomLoader cannot tell how many bytes are
 *                 in the stomach.                                           */
{
    a2l_quex_lexer_LexatomLoader_Converter* me = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;
    ptrdiff_t  byte_n;
    ptrdiff_t  converter_byte_n;

    byte_n = me->raw_buffer.fill_end_p - me->raw_buffer.next_to_convert_p;

    /* me->converter->stomach_byte_n   == 0   => converter does NOT keep any 
     *                                           bytes in stomach.
     * me->converter->stomach_byte_n() == -1  => converter CANNOT tell how 
     *                                           many bytes in stomach.      */
    if( me->converter->stomach_byte_n ) {
        converter_byte_n = me->converter->stomach_byte_n(me->converter);
        if( converter_byte_n == - 1) return (ptrdiff_t)-1;
        byte_n += converter_byte_n;
    }

    return byte_n;
}

inline void   
a2l_quex_lexer_LexatomLoader_Converter_stomach_clear(a2l_quex_lexer_LexatomLoader* alter_ego)
{
    a2l_quex_lexer_LexatomLoader_Converter* me = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;
    a2l_quex_lexer_RawBuffer_init(&me->raw_buffer, 0, 0);
    a2l_quex_lexer_Converter_reset(me->converter);
}

inline void   
a2l_quex_lexer_LexatomLoader_Converter_destruct_self(a2l_quex_lexer_LexatomLoader* alter_ego)
/* destruct_self: Free resources occupied by 'me' BUT NOT 'myself'.
 * delete_self:   Free resources occupied by 'me' AND 'myself'.              */
{ 
    a2l_quex_lexer_LexatomLoader_Converter* me = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;

    if( ! me ) return;

    QUEX_ASSERT_RAW_BUFFER(&me->raw_buffer);

    if( me->converter ) {
        me->converter->delete_self(me->converter); 
    }

    quex::MemoryManager_free((void*)me->raw_buffer.begin,
                               E_MemoryObjectType_BUFFER_RAW); 
}

inline size_t 
a2l_quex_lexer_LexatomLoader_Converter_load_lexatoms(a2l_quex_lexer_LexatomLoader* alter_ego,
                                                 a2l_quex_lexer_lexatom_t*        RegionBeginP, 
                                                 const size_t              N,
                                                 bool*                     end_of_stream_f,
                                                 bool*                     encoding_error_f)
/* Loads content into the raw buffer, convert it and write it to the engine's
 * buffer. The region where to write into the engine's buffer expands from
 * 'RegionBeginP' to 'N' lexatoms after it.                                
 *
 * RETURNS: Number of loaded lexatoms into the given region.               */
{
    a2l_quex_lexer_LexatomLoader_Converter* me = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;
    a2l_quex_lexer_RawBuffer*               raw = &me->raw_buffer;
    a2l_quex_lexer_lexatom_t*                  buffer_insertion_p = RegionBeginP;
    const a2l_quex_lexer_lexatom_t*            BufferRegionEnd    = &RegionBeginP[N];
    E_LoadResult                        load_result;
    bool                                raw_load_complete_f;
    bool                                raw_end_of_stream_f;
#   if 0
    int                              i;
    a2l_quex_lexer_lexatom_t*               buffer_insertion_begin_p;
#   endif
    (void)encoding_error_f;
    (void)raw;

    __quex_assert(0 != me->converter);
    __quex_assert(0 != alter_ego); 
    __quex_assert(0 != RegionBeginP); 
    QUEX_ASSERT_RAW_BUFFER(raw);

    /* NOT: QUEX_IF_ASSERTS_poison(RegionBeginP, &RegionBeginP[N]);
     * The buffer must remain intact, in case that not all is loaded.        */

    /* Some converters keep some content internally. So, it is a more general
     * solution to convert first and reload new bytes upon need.             */
    load_result = a2l_quex_lexer_LexatomLoader_call_converter(me, &buffer_insertion_p, 
                                                          RegionBeginP,
                                                          BufferRegionEnd);

    /* Convert, as long as the following two hold:
     *  (i)  Drain is not totally filled.
     *  (ii) Loading of 'to-be-converted bytes' received enough to fill the
     *       raw buffer to its limits. The contrary indicates that there is 
     *       an transmission interuption, or even and end-of-stream. In both
     *       cases, the analyser may continue, before the next try.          */
    raw_end_of_stream_f = false;
    raw_load_complete_f = true;
    while( load_result == E_LoadResult_INCOMPLETE && raw_load_complete_f ) {
        __quex_assert(buffer_insertion_p < BufferRegionEnd);  /* '==' break  */

        if( ! raw_end_of_stream_f ) {
            raw_load_complete_f = a2l_quex_lexer_RawBuffer_load(&me->raw_buffer, 
                                                            me->base.byte_loader,
                                                            &raw_end_of_stream_f);
        } else  {
            raw_load_complete_f = false;
        }

        /* next_to_convert_p == raw->fill_end_p => nothing happens.          */
        load_result = a2l_quex_lexer_LexatomLoader_call_converter(me, &buffer_insertion_p, 
                                                              RegionBeginP,
                                                              BufferRegionEnd);
    }

    __quex_assert(BufferRegionEnd >= buffer_insertion_p);
    /* NOT: QUEX_IF_ASSERTS_poison(buffer_insertion_p, BufferRegionEnd);
     *      Buffer MUST be left as is, in case of ERROR!                     */

    switch( load_result ) {
        case E_LoadResult_COMPLETE:
            break;
        case E_LoadResult_INCOMPLETE:
            /* Some ByteLoader-s (socket based ones, for example) may not be
             * able to fill the whole raw buffer, but still the end of stream
             * is not reached. Only, if the raw buffer detected end of stream
             * the end of stream can be claimed.                             */
            if( raw_end_of_stream_f ) {
                *end_of_stream_f = true;
            }
            /* __quex_assert(raw->next_to_convert_p == raw->fill_end_p);     */
            /* Nothing can be loaded; Everything is converted.               */

            break;
        case E_LoadResult_ENCODING_ERROR:
            *encoding_error_f = true;
            break;

        case E_LoadResult_NO_MORE_DATA:
            /* A converter does not load--when called, there should be data.
             * => Cannot complain 'NO_MORE_DATA' (end of stream).            */
        default:
            __quex_assert(false);
    }

    /* 'buffer_insertion_p' was updated by 'convert' and points behind the 
     * last byte that was converted.                                         */ 
    return (size_t)(buffer_insertion_p - RegionBeginP);
}

inline void 
a2l_quex_lexer_LexatomLoader_Converter_fill_prepare(a2l_quex_lexer_LexatomLoader*  alter_ego,
                                                a2l_quex_lexer_Buffer*         buffer,
                                                void**                     begin_p,
                                                const void**               end_p)
{
    (void)buffer;
    a2l_quex_lexer_LexatomLoader_Converter* me = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;

    a2l_quex_lexer_RawBuffer_move_away_passed_content(&me->raw_buffer);

    alter_ego->derived.get_fill_boundaries(alter_ego, buffer, begin_p, end_p);
}

inline void 
a2l_quex_lexer_LexatomLoader_Converter_get_fill_boundaries(a2l_quex_lexer_LexatomLoader*  alter_ego,
                                                       a2l_quex_lexer_Buffer*         buffer,
                                                       void**                     begin_p, 
                                                       const void**               end_p)
{
    a2l_quex_lexer_LexatomLoader_Converter* me = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;
    (void)buffer;

    *begin_p = (void*)me->raw_buffer.fill_end_p; 
    *end_p   = (void*)me->raw_buffer.memory_end;
}

inline ptrdiff_t 
a2l_quex_lexer_LexatomLoader_Converter_fill_finish(a2l_quex_lexer_LexatomLoader* alter_ego,
                                              a2l_quex_lexer_lexatom_t*         RegionBeginP,
                                              const a2l_quex_lexer_lexatom_t*   RegionEndP,
                                              const void*                FilledEndP_raw)
/* Converts what has been filled into the 'raw_buffer' until 'FilledEndP
 * and stores it into the buffer.                                            */
{
    a2l_quex_lexer_LexatomLoader_Converter* me  = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;
    a2l_quex_lexer_RawBuffer*               raw = &me->raw_buffer;
    a2l_quex_lexer_lexatom_t*                  insertion_p = RegionBeginP;
    uint8_t*                            FilledEndP = (uint8_t*)FilledEndP_raw;
    E_LoadResult                        load_result;

    __quex_assert(FilledEndP >= raw->next_to_convert_p);
    __quex_assert(FilledEndP <= raw->memory_end);

    /* Assert triggers => FilledEndP points WRONGLY BEHIND terminating zero. 
     * (FilledEndP, may point to it, at max.)                                */
    __quex_assert(   FilledEndP     <= raw->next_to_convert_p 
                  || FilledEndP[-1] != QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER);

    raw->fill_end_p = FilledEndP;   
    QUEX_ASSERT_RAW_BUFFER(raw);

    load_result = a2l_quex_lexer_LexatomLoader_call_converter(me, &insertion_p, 
                                                          RegionBeginP,
                                                          RegionEndP);
    (void)load_result;
    
    QUEX_ASSERT_RAW_BUFFER(raw);
    return insertion_p - RegionBeginP;
}

inline E_LoadResult
a2l_quex_lexer_LexatomLoader_call_converter(a2l_quex_lexer_LexatomLoader_Converter* me,
                                        a2l_quex_lexer_lexatom_t**                 insertion_p,
                                        a2l_quex_lexer_lexatom_t*                  RegionBeginP,
                                        const a2l_quex_lexer_lexatom_t*            RegionEndP)
{
    a2l_quex_lexer_RawBuffer*  raw = &me->raw_buffer;
    E_LoadResult           load_result;

    load_result = me->converter->convert(me->converter, 
                                         &raw->next_to_convert_p, raw->fill_end_p,
                                         insertion_p, RegionEndP);

    /* A converter does not load => It cannot report 'end of stream'     */
    __quex_assert(   load_result == E_LoadResult_COMPLETE
                  || load_result == E_LoadResult_INCOMPLETE
                  || load_result == E_LoadResult_ENCODING_ERROR);
    
    if( *insertion_p > RegionBeginP ) {
        a2l_quex_lexer_LexatomLoader_remove_spurious_BOM(me, insertion_p, RegionBeginP);
        me->converter->virginity_f = false;
    }

    return load_result;
}

inline void
a2l_quex_lexer_LexatomLoader_remove_spurious_BOM(a2l_quex_lexer_LexatomLoader_Converter* me,
                                             a2l_quex_lexer_lexatom_t**                 buffer_insertion_p,
                                             a2l_quex_lexer_lexatom_t*                  RegionBeginP)
{
    uint32_t  first_lexatom;

    __quex_assert(*buffer_insertion_p > RegionBeginP);

    first_lexatom = (uint32_t)RegionBeginP[0];    /* avoid warning */
    if( first_lexatom != 0xFEFF ) return;

    if( ! me->converter->virginity_f ) {
        QUEX_ERROR_EXIT("Converter produced BOM upon not-first call to 'convert'\n"
                        "Better make sure that converter NEVER produces BOM.\n"
                        "(May be, by specifiying the endianness of 'FromCoding' or 'ToCoding')\n");
    }

    __QUEX_STD_memmove(RegionBeginP, &RegionBeginP[1], 
                       (size_t)(*buffer_insertion_p - &RegionBeginP[1]) * sizeof(a2l_quex_lexer_lexatom_t)); 

    *buffer_insertion_p = &(*buffer_insertion_p)[-1];
}

inline void 
a2l_quex_lexer_LexatomLoader_Converter_print_this(a2l_quex_lexer_LexatomLoader* alter_ego)
{
    a2l_quex_lexer_LexatomLoader_Converter* me  = (a2l_quex_lexer_LexatomLoader_Converter*)alter_ego;

    QUEX_DEBUG_PRINT("      type:                           converter;\n");
    a2l_quex_lexer_RawBuffer_print_this(&me->raw_buffer);
    a2l_quex_lexer_Converter_print_this(me->converter);
}



#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader.i"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader_Converter_RawBuffer.i"
#include "a2l_quex_lexer\lib\buffer/lexatoms/converter/Converter.i"

#endif /*  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__LEXATOM_LOADER_CONVERTER_I */
