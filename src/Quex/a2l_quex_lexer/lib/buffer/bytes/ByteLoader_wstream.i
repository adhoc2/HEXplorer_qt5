/* vim: set ft=c:
 * (C) Frank-Rene Schaefer */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_WSTREAM_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_WSTREAM_I

#if defined(__cplusplus) && ! defined(QUEX_OPTION_WCHAR_T_DISABLED_EXT)

#include <fstream>
#include <sstream>



inline void                       a2l_quex_lexer_ByteLoader_wstream_construct(a2l_quex_lexer_ByteLoader_wstream* me, 
                                                                               std::wistream*                 sh);
inline a2l_quex_lexer_stream_position_t  a2l_quex_lexer_ByteLoader_wstream_tell(a2l_quex_lexer_ByteLoader* me);
inline void                       a2l_quex_lexer_ByteLoader_wstream_seek(a2l_quex_lexer_ByteLoader* me, 
                                                                          a2l_quex_lexer_stream_position_t Pos);
inline size_t                     a2l_quex_lexer_ByteLoader_wstream_load(a2l_quex_lexer_ByteLoader* me, 
                                                                          void*                  buffer, 
                                                                          const size_t           ByteN, 
                                                                          bool*                  end_of_stream_f);
inline void                       a2l_quex_lexer_ByteLoader_wstream_delete_self(a2l_quex_lexer_ByteLoader* me);
inline void                       a2l_quex_lexer_ByteLoader_wstream_print_this(a2l_quex_lexer_ByteLoader* me);
inline bool                       a2l_quex_lexer_ByteLoader_wstream_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                                                                    const a2l_quex_lexer_ByteLoader* alter_ego_B);

inline a2l_quex_lexer_ByteLoader*
a2l_quex_lexer_ByteLoader_wstream_new(std::wistream* sh)
{
    a2l_quex_lexer_ByteLoader_wstream* me;

    if( ! sh ) return (a2l_quex_lexer_ByteLoader*)0;

    me = (a2l_quex_lexer_ByteLoader_wstream*)quex::MemoryManager_allocate(sizeof(a2l_quex_lexer_ByteLoader_wstream),
                                                                       E_MemoryObjectType_BYTE_LOADER);

    if( ! me ) return (a2l_quex_lexer_ByteLoader*)0;

    a2l_quex_lexer_ByteLoader_wstream_construct(me, sh);
    return &me->base;
}

inline a2l_quex_lexer_ByteLoader*    
a2l_quex_lexer_ByteLoader_wstream_new_from_file_name(const char* FileName)
{
    std::wifstream*        sh = new std::wifstream(FileName, std::ios_base::binary | std::ios::in);
    a2l_quex_lexer_ByteLoader* alter_ego;

    if( ! sh || ! *sh ) {
        return (a2l_quex_lexer_ByteLoader*)0;
    }
    alter_ego = a2l_quex_lexer_ByteLoader_wstream_new(sh);
    if( ! alter_ego ) {
        return (a2l_quex_lexer_ByteLoader*)0;
    }

    /* ByteLoader from file name *must* be owned by lexical analyzer, 
     * to ensure automatic closure and deletion.                              */
    alter_ego->handle_ownership = E_Ownership_LEXICAL_ANALYZER;
    return alter_ego;
}

inline void
a2l_quex_lexer_ByteLoader_wstream_construct(a2l_quex_lexer_ByteLoader_wstream* me, 
                                        std::wistream*                 sh)
{
    /* IMPORTANT: input_handle must be set BEFORE call to constructor!
     *            Constructor does call 'tell()'                             */
    me->input_handle = sh;

    a2l_quex_lexer_ByteLoader_construct(&me->base,
                         a2l_quex_lexer_ByteLoader_wstream_tell,
                         a2l_quex_lexer_ByteLoader_wstream_seek,
                         a2l_quex_lexer_ByteLoader_wstream_load,
                         a2l_quex_lexer_ByteLoader_wstream_delete_self,
                         a2l_quex_lexer_ByteLoader_wstream_print_this,
                         a2l_quex_lexer_ByteLoader_wstream_compare_handle);
    me->base.element_size = sizeof(wchar_t);
}

inline void
a2l_quex_lexer_ByteLoader_wstream_delete_self(a2l_quex_lexer_ByteLoader* alter_ego)
{
    a2l_quex_lexer_ByteLoader_wstream* me = (a2l_quex_lexer_ByteLoader_wstream*)alter_ego;

    if( me->input_handle && me->base.handle_ownership == E_Ownership_LEXICAL_ANALYZER ) {
        delete me->input_handle;
    }
    quex::MemoryManager_free(me, E_MemoryObjectType_BYTE_LOADER);
}

/* The 'char_type' of a stream determines the atomic size of elements which are
 * read from the stream. It is unrelated to a2l_quex_lexer_lexatom_t which
 * determines the size of a buffer element.                                  */
inline a2l_quex_lexer_stream_position_t    
a2l_quex_lexer_ByteLoader_wstream_tell(a2l_quex_lexer_ByteLoader* alter_ego)            
{ 
    a2l_quex_lexer_ByteLoader_wstream*    me       = (a2l_quex_lexer_ByteLoader_wstream*)alter_ego;
    const a2l_quex_lexer_stream_position_t   CharSize = sizeof(wchar_t);
    std::streampos                    Position = me->input_handle->tellg();

    return (a2l_quex_lexer_stream_position_t)(Position * CharSize); 
}

inline void    
a2l_quex_lexer_ByteLoader_wstream_seek(a2l_quex_lexer_ByteLoader*    alter_ego, 
                                  a2l_quex_lexer_stream_position_t Pos) 
{ 
    a2l_quex_lexer_ByteLoader_wstream*    me       = (a2l_quex_lexer_ByteLoader_wstream*)alter_ego;
    const a2l_quex_lexer_stream_position_t   CharSize = sizeof(wchar_t);
    std::streampos                    Target   = (std::streampos)(Pos / CharSize);

    me->input_handle->clear();                    /* Clear any iostate flag. */
    me->input_handle->seekg(Target); 
}

inline size_t  
a2l_quex_lexer_ByteLoader_wstream_load(a2l_quex_lexer_ByteLoader*  alter_ego, 
                                  void*        buffer, 
                                  const size_t ByteN, 
                                  bool*        end_of_stream_f) 
{ 
    a2l_quex_lexer_ByteLoader_wstream*    me = (a2l_quex_lexer_ByteLoader_wstream*)alter_ego;
    const a2l_quex_lexer_stream_position_t   CharSize = sizeof(wchar_t);
    if( ! ByteN ) return (size_t)0;

    me->input_handle->read((wchar_t*)buffer, (std::streamsize)(ByteN / CharSize)); 

    const size_t loaded_char_n = (size_t)(me->input_handle->gcount());

    *end_of_stream_f = me->input_handle->eof();

    if( (! *end_of_stream_f) && me->input_handle->fail() ) {
        throw std::runtime_error("Fatal error during stream reading.");
    }

    /* std::fprintf(stdout, "tell 1 = %i, loaded_char_n = %i\n", (long)(me->input_handle->tellg()), loaded_char_n);*/
    return (size_t)(loaded_char_n * CharSize);
}

inline bool  
a2l_quex_lexer_ByteLoader_wstream_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                            const a2l_quex_lexer_ByteLoader* alter_ego_B) 
/* RETURNS: true  -- if A and B point to the same std::wistream object.
 *          false -- else.                                                   */
{ 
    const a2l_quex_lexer_ByteLoader_wstream* A = (a2l_quex_lexer_ByteLoader_wstream*)(alter_ego_A);
    const a2l_quex_lexer_ByteLoader_wstream* B = (a2l_quex_lexer_ByteLoader_wstream*)(alter_ego_B);

    return A->input_handle == B->input_handle;
}

inline void
a2l_quex_lexer_ByteLoader_wstream_print_this(a2l_quex_lexer_ByteLoader* alter_ego)
{
    a2l_quex_lexer_ByteLoader_wstream*   me = (a2l_quex_lexer_ByteLoader_wstream*)alter_ego;

    QUEX_DEBUG_PRINT1("        stream:           ((%p));\n", (const void*)me->input_handle);
    QUEX_DEBUG_PRINT("        type:             wistream;\n");
    QUEX_DEBUG_PRINT1("        stream:           ((%p));\n", (const void*)me->input_handle);
    if( me->input_handle ) {
        QUEX_DEBUG_PRINT1("        end_of_stream:    %s;\n", E_Boolean_NAME(me->input_handle->eof()));
    }
}


#endif 
#endif /*  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_WSTREAM_I         */

