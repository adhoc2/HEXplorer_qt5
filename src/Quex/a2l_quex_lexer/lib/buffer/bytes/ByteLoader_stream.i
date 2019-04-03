/* vim: set ft=c:
 * (C) Frank-Rene Schaefer */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_STREAM_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_STREAM_I
#ifdef __cplusplus

#include <fstream>
#include <sstream>
#include "a2l_quex_lexer\lib\buffer/bytes/ByteLoader_stream"



template <class StreamType> inline a2l_quex_lexer_stream_position_t  a2l_quex_lexer_ByteLoader_stream_tell(a2l_quex_lexer_ByteLoader* me);
template <class StreamType> inline void                       a2l_quex_lexer_ByteLoader_stream_seek(a2l_quex_lexer_ByteLoader* me, 
                                                                                          a2l_quex_lexer_stream_position_t Pos);
template <class StreamType> inline size_t                     a2l_quex_lexer_ByteLoader_stream_load(a2l_quex_lexer_ByteLoader* me, void* buffer, const size_t ByteN, bool*);
template <class StreamType> inline void                       a2l_quex_lexer_ByteLoader_stream_delete_self(a2l_quex_lexer_ByteLoader* me);
template <class StreamType> inline void                       a2l_quex_lexer_ByteLoader_stream_print_this(a2l_quex_lexer_ByteLoader* alter_ego);
template <class StreamType> inline bool                       a2l_quex_lexer_ByteLoader_stream_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                                                                                               const a2l_quex_lexer_ByteLoader* alter_ego_B);

template <class StreamType> inline a2l_quex_lexer_ByteLoader*
a2l_quex_lexer_ByteLoader_stream_new(StreamType* sh)
{
    a2l_quex_lexer_ByteLoader_stream<StreamType>* me;

    if( ! sh ) return (a2l_quex_lexer_ByteLoader*)0;

    me = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)quex::MemoryManager_allocate(sizeof(a2l_quex_lexer_ByteLoader_stream<StreamType>),
                                                                        E_MemoryObjectType_BYTE_LOADER);

    if( ! me ) return (a2l_quex_lexer_ByteLoader*)0;

    a2l_quex_lexer_ByteLoader_stream_construct(me, sh);
    return &me->base;
}

inline a2l_quex_lexer_ByteLoader*    
a2l_quex_lexer_ByteLoader_stream_new_from_file_name(const char* FileName)
{
    std::ifstream*         sh = new std::ifstream(FileName, std::ios_base::binary | std::ios::in);
    a2l_quex_lexer_ByteLoader* alter_ego;
    if( ! sh || ! *sh ) {
        return (a2l_quex_lexer_ByteLoader*)0;
    }
    alter_ego = a2l_quex_lexer_ByteLoader_stream_new<std::ifstream>(sh);
    if( ! alter_ego ) {
        return (a2l_quex_lexer_ByteLoader*)0;
    }

    /* ByteLoader from file name *must* be owned by lexical analyzer, 
     * to ensure automatic closure and deletion.                              */
    alter_ego->handle_ownership = E_Ownership_LEXICAL_ANALYZER;
    return alter_ego;
}

template <class StreamType> inline void
a2l_quex_lexer_ByteLoader_stream_construct(a2l_quex_lexer_ByteLoader_stream<StreamType>* me, 
                                       StreamType*                               sh)
{
    /* IMPORTANT: input_handle must be set BEFORE call to constructor!
     *            Constructor does call 'tell()'                             */
    me->input_handle = sh;

    a2l_quex_lexer_ByteLoader_construct(&me->base,
                         a2l_quex_lexer_ByteLoader_stream_tell<StreamType>,
                         a2l_quex_lexer_ByteLoader_stream_seek<StreamType>,
                         a2l_quex_lexer_ByteLoader_stream_load<StreamType>,
                         a2l_quex_lexer_ByteLoader_stream_delete_self<StreamType>,
                         a2l_quex_lexer_ByteLoader_stream_print_this<StreamType>,
                         a2l_quex_lexer_ByteLoader_stream_compare_handle<StreamType>);
    me->base.element_size = sizeof(typename StreamType::char_type);
}

template <class StreamType> inline void
a2l_quex_lexer_ByteLoader_stream_delete_self(a2l_quex_lexer_ByteLoader* alter_ego)
{
    a2l_quex_lexer_ByteLoader_stream<StreamType>* me = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)alter_ego;

    if( me->input_handle && me->base.handle_ownership == E_Ownership_LEXICAL_ANALYZER ) {
        delete me->input_handle;
    }
    quex::MemoryManager_free(me, E_MemoryObjectType_BYTE_LOADER);
}

/* The 'char_type' of a stream determines the atomic size of elements which are
 * read from the stream. It is unrelated to a2l_quex_lexer_lexatom_t which
 * determines the size of a buffer element.                                  */
template <class StreamType> inline a2l_quex_lexer_stream_position_t    
a2l_quex_lexer_ByteLoader_stream_tell(a2l_quex_lexer_ByteLoader* alter_ego)            
{ 
    a2l_quex_lexer_ByteLoader_stream<StreamType>*       me = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)alter_ego;
    const a2l_quex_lexer_stream_position_t      CharSize = \
         (a2l_quex_lexer_stream_position_t)sizeof(typename StreamType::char_type);
    std::streampos                       Position = me->input_handle->tellg();

    return (a2l_quex_lexer_stream_position_t)(Position * CharSize); 
}

template <class StreamType> inline void    
a2l_quex_lexer_ByteLoader_stream_seek(a2l_quex_lexer_ByteLoader*    alter_ego, 
                                  a2l_quex_lexer_stream_position_t Pos) 
{ 
    a2l_quex_lexer_ByteLoader_stream<StreamType>*       me = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)alter_ego;
    const a2l_quex_lexer_stream_position_t      CharSize = \
         (a2l_quex_lexer_stream_position_t)sizeof(typename StreamType::char_type);
    std::streampos                       Target   = (std::streampos)(Pos / CharSize);

    me->input_handle->clear();                    /* Clear any iostate flag. */
    me->input_handle->seekg(Target); 
}

template <class StreamType> inline size_t  
a2l_quex_lexer_ByteLoader_stream_load(a2l_quex_lexer_ByteLoader*  alter_ego, 
                                  void*        buffer, 
                                  const size_t ByteN, 
                                  bool*        end_of_stream_f) 
{ 
    a2l_quex_lexer_ByteLoader_stream<StreamType>*    me = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)alter_ego;
    const a2l_quex_lexer_stream_position_t   CharSize = \
         (a2l_quex_lexer_stream_position_t)sizeof(typename StreamType::char_type);

    if( ! ByteN ) return (size_t)0;

    me->input_handle->read((typename StreamType::char_type*)buffer, 
                           (std::streamsize)(ByteN / CharSize)); 

    const size_t loaded_char_n = (size_t)(me->input_handle->gcount());

    *end_of_stream_f = me->input_handle->eof();

    if( (! *end_of_stream_f) && me->input_handle->fail() ) {
        throw std::runtime_error("Fatal error during stream reading.");
    }

    /* std::fprintf(stdout, "tell 1 = %i, loaded_char_n = %i\n", (long)(me->input_handle->tellg()), loaded_char_n);*/
    return (size_t)(loaded_char_n * CharSize);
}

template <class StreamType> inline bool  
a2l_quex_lexer_ByteLoader_stream_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                            const a2l_quex_lexer_ByteLoader* alter_ego_B) 
/* RETURNS: true  -- if A and B point to the same StreamType object.
 *          false -- else.                                                   */
{ 
    const a2l_quex_lexer_ByteLoader_stream<StreamType>* A = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)(alter_ego_A);
    const a2l_quex_lexer_ByteLoader_stream<StreamType>* B = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)(alter_ego_B);

    return A->input_handle == B->input_handle;
}

template <class StreamType> void
a2l_quex_lexer_ByteLoader_stream_print_this(a2l_quex_lexer_ByteLoader* alter_ego)
{
    a2l_quex_lexer_ByteLoader_stream<StreamType>*    me = (a2l_quex_lexer_ByteLoader_stream<StreamType>*)alter_ego;

    QUEX_DEBUG_PRINT("        type:             istream;\n");
    QUEX_DEBUG_PRINT1("        stream:           ((%p));\n", (const void*)me->input_handle);
    if( me->input_handle ) {
        QUEX_DEBUG_PRINT1("        end_of_stream:    %s;\n", E_Boolean_NAME(me->input_handle->eof()));
    }
}



#endif /* __cplusplus                                             */
#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_STREAM_I */

