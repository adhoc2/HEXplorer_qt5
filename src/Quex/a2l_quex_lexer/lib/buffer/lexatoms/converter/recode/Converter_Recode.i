/* -*- C++ -*-  vim: set syntax=cpp:
 * (C) 2007-2008 Frank-Rene Schaefer  */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__CONVERTER__RECODE__CONVERTER_RECODE_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__CONVERTER__RECODE__CONVERTER_RECODE_I

#include "a2l_quex_lexer\lib\quex/compatibility/stdint.h"
#include "a2l_quex_lexer\lib\quex/MemoryManager"



    inline void
    QuexConverter_ICU_open(QuexConverter* alter_ego, 
                           const char* FromCodingName, const char* ToCodingName)
    {
        QuexConverter_ICU* me = (QuexConverter_ICU*)alter_ego;
        __quex_assert(me != 0x0);

        me->from_handle = ucnv_open(FromCodingName, &me->status);

        if( ! U_SUCCESS(me->status) ) 
            QUEX_ERROR_EXIT("Input Coding not supported by ICU converter.");

        if( ToCodingName != 0x0 ) {
            me->to_handle = ucnv_open(ToCodingName, &me->status);
        } else {
            switch( sizeof(a2l_quex_lexer_lexatom_t) ) {
            case 4:  
                me->to_handle = ucnv_open("UTF32_PlatformEndian", &me->status); 
                break;
            case 2:  
                /* 2 byte encoding may use the 'direct converter for UChar'  */
                me->to_handle = 0x0; 
                break;
            case 1:  
                me->to_handle = ucnv_open("ISO-8859-1", &me->status); 
                break;
            default:
                QUEX_ERROR_EXIT("ICU character conversion: target coding different from unicode not yet supported.");
            }
        }

        me->pivot_iterator_begin = me->pivot_buffer;
        me->pivot_iterator_end   = me->pivot_buffer;
    }

    inline E_LoadResult
    QuexConverter_ICU_convert(QuexConverter*        alter_ego, 
                              uint8_t**             source, const uint8_t*              SourceEnd, 
                              a2l_quex_lexer_lexatom_t** drain,  const a2l_quex_lexer_lexatom_t*  DrainEnd)
    /* RETURNS: 'true'  if the drain was completely filled.
     *          'false' if the drain could not be filled completely and more 
     *                  source bytes are required.                           */
    {
        QuexConverter_ICU*    me = (QuexConverter_ICU*)alter_ego;

        __quex_assert(me != 0x0);
        me->status = U_ZERO_ERROR;

        if( me->to_handle == 0x0 ) {
            /* Convert according to a2l_quex_lexer_lexatom_t:
             *
             * NOTE: The author did not find a better way to do non-16bit conversion than
             *       converting 'normally' and then shifting according to the size
             *       of a2l_quex_lexer_lexatom_t. If you read these lines and know of a better
             *       method, please, let me know (email: fschaef@users.sourceforge.net).   
             *
             * NOTE: 'UChar' is defined to be wchar_t, if sizeof(wchar_t) == 2 byte, 
             *       otherwise it as defined as uint16_t.                        
             *
             * We need to cast to UChar, since otherwise the code would not compile for sizeof() != 2.
             * Nevertheless, in this case the code would never be executed.                            */
            __quex_assert( sizeof(a2l_quex_lexer_lexatom_t) == 2 );

            /* 16 bit --> nothing to be done */
            ucnv_toUnicode(me->from_handle, 
                           (UChar**)drain,       (const UChar*)DrainEnd,
                           (const char**)source, (const char*)SourceEnd, 
                           /* offsets */NULL,
                           /* flush = */FALSE,
                           &me->status);

            if( *drain == DrainEnd ) return E_LoadResult_OK;
            else                     return E_LoadResult_FAILURE;

        } else {
            ucnv_convertEx(me->to_handle, me->from_handle,
                           (char**)drain,        (const char*)DrainEnd,
                           (const char**)source, (const char*)SourceEnd,
                           me->pivot_buffer, 
                           &me->pivot_iterator_begin, &me->pivot_iterator_end, 
                           me->pivot_buffer + QUEX_a2l_quex_lexer_SETTING_ICU_PIVOT_BUFFER_SIZE,
                           /* reset = */FALSE, 
                           /* flush = */FALSE,
                           &me->status);

            if( *drain == DrainEnd ) return E_LoadResult_OK;
            else                     return E_LoadResult_FAILURE;
        }

        /*
        if( me->status == U_BUFFER_OVERFLOW_ERROR) {
            return false;
        }
        else {
            if( ! U_SUCCESS(me->status) ) {
                QUEX_ERROR_EXIT(u_errorName(me->status));
            }
            / * Are more source bytes needed to fill the drain buffer? If so we return 'false' * /
            if( *drain != DrainEnd && *source == SourceEnd ) return false;
            else                                             return true;
        }
        */
    }

    inline void 
    QuexConverter_ICU_stomach_clear(QuexConverter* alter_ego)
    {
        QuexConverter_ICU* me = (QuexConverter_ICU*)alter_ego;

        ucnv_reset(me->from_handle);
        if( me->to_handle ) ucnv_reset(me->to_handle);

        /* Reset the pivot buffer iterators */
        me->pivot_iterator_begin = me->pivot_buffer;
        me->pivot_iterator_end   = me->pivot_buffer;

        me->status = U_ZERO_ERROR;
    }

    inline void
    QuexConverter_ICU_delete_self(QuexConverter* alter_ego)
    {
        QuexConverter_ICU* me = (QuexConverter_ICU*)alter_ego;

        ucnv_close(me->from_handle);
        ucnv_close(me->to_handle);

        quex::MemoryManager_free((void*)me, E_MemoryObjectType_CONVERTER);

        /* There should be a way to call 'ucnv_flushCache()' as soon as all converters
         * are freed automatically.                                                       */
    }

    inline QuexConverter*
    QuexConverter_ICU_new()
    {
        QuexConverter_ICU*  me = \
             (a2l_quex_lexer_Converter_ICU*)quex::MemoryManager_allocate(sizeof(a2l_quex_lexer_Converter_ICU),
                                                                       E_MemoryObjectType_CONVERTER);

        me->base.open        = QuexConverter_ICU_open;
        me->base.convert     = QuexConverter_ICU_convert;
        me->base.delete_self = QuexConverter_ICU_delete_self;
        me->base.stomach_clear = QuexConverter_ICU_stomach_clear;

        me->to_handle   = 0x0;
        me->from_handle = 0x0;
        me->status      = U_ZERO_ERROR;

        return (QuexConverter*)me;
    }




#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader.i"

#endif /* __INCLUDE_GUARD__QUEX_BUFFER__CONVERTER_ICONV_I__ */
