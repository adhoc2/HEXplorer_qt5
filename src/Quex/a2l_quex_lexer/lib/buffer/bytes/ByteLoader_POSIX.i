/* vim: ft=c:
 * (C) Frank-Rene Schaefer */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_POSIX_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_POSIX_I

#include "a2l_quex_lexer\lib\buffer/bytes/ByteLoader_POSIX"
#include "a2l_quex_lexer\lib\quex/MemoryManager"
#include "a2l_quex_lexer\lib\definitions"



inline void                       a2l_quex_lexer_ByteLoader_POSIX_construct(a2l_quex_lexer_ByteLoader_POSIX* me, int fd);
inline a2l_quex_lexer_stream_position_t  a2l_quex_lexer_ByteLoader_POSIX_tell(a2l_quex_lexer_ByteLoader* me);
inline void                       a2l_quex_lexer_ByteLoader_POSIX_seek(a2l_quex_lexer_ByteLoader* me, 
                                                                        a2l_quex_lexer_stream_position_t Pos);
inline size_t                     a2l_quex_lexer_ByteLoader_POSIX_load(a2l_quex_lexer_ByteLoader* me, 
                                                                        void* buffer, const size_t ByteN, 
                                                                        bool*);
inline void                       a2l_quex_lexer_ByteLoader_POSIX_delete_self(a2l_quex_lexer_ByteLoader* me);
inline void                       a2l_quex_lexer_ByteLoader_POSIX_print_this(a2l_quex_lexer_ByteLoader* me);
inline bool                       a2l_quex_lexer_ByteLoader_POSIX_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                                                       const a2l_quex_lexer_ByteLoader* alter_ego_B);

inline a2l_quex_lexer_ByteLoader*    
a2l_quex_lexer_ByteLoader_POSIX_new(int fd)
{
    a2l_quex_lexer_ByteLoader_POSIX* me;

    if( fd == -1 ) return (a2l_quex_lexer_ByteLoader*)0;
    me = (a2l_quex_lexer_ByteLoader_POSIX*)quex::MemoryManager_allocate(sizeof(a2l_quex_lexer_ByteLoader_POSIX),
                                                           E_MemoryObjectType_BYTE_LOADER);
    if( ! me ) return (a2l_quex_lexer_ByteLoader*)0;
    a2l_quex_lexer_ByteLoader_POSIX_construct(me, fd);
    return &me->base;
}

inline a2l_quex_lexer_ByteLoader*    
a2l_quex_lexer_ByteLoader_POSIX_new_from_file_name(const char* FileName)
{
    a2l_quex_lexer_ByteLoader*  alter_ego;
    int                     fd = open(FileName, O_RDONLY);

    if( fd == -1 ) {
        return (a2l_quex_lexer_ByteLoader*)0;
    }
    alter_ego = a2l_quex_lexer_ByteLoader_POSIX_new(fd);
    if( ! alter_ego ) {
        return (a2l_quex_lexer_ByteLoader*)0;
    }

    /* ByteLoader from file name *must* be owned by lexical analyzer, 
     * to ensure automatic closure and deletion.                              */
    alter_ego->handle_ownership = E_Ownership_LEXICAL_ANALYZER;
    return alter_ego;
}

inline void
a2l_quex_lexer_ByteLoader_POSIX_construct(a2l_quex_lexer_ByteLoader_POSIX* me, int fd)
{
    /* IMPORTANT: fd must be set BEFORE call to constructor!
     *            Constructor does call 'tell()'                             */
    me->fd = fd;

    a2l_quex_lexer_ByteLoader_construct(&me->base,
                                    a2l_quex_lexer_ByteLoader_POSIX_tell,
                                    a2l_quex_lexer_ByteLoader_POSIX_seek,
                                    a2l_quex_lexer_ByteLoader_POSIX_load,
                                    a2l_quex_lexer_ByteLoader_POSIX_delete_self,
                                    a2l_quex_lexer_ByteLoader_POSIX_print_this,
                                    a2l_quex_lexer_ByteLoader_POSIX_compare_handle);

    /* A POSIX file handle is always in binary mode.                         */
    me->base.binary_mode_f = true;
}

inline void    
a2l_quex_lexer_ByteLoader_POSIX_delete_self(a2l_quex_lexer_ByteLoader* alter_ego)
{
    a2l_quex_lexer_ByteLoader_POSIX* me = (a2l_quex_lexer_ByteLoader_POSIX*)(alter_ego);

    if( me->fd && me->base.handle_ownership == E_Ownership_LEXICAL_ANALYZER ) {
        close(me->fd);
    }
    quex::MemoryManager_free(me, E_MemoryObjectType_BYTE_LOADER);
}

inline a2l_quex_lexer_stream_position_t    
a2l_quex_lexer_ByteLoader_POSIX_tell(a2l_quex_lexer_ByteLoader* alter_ego)            
{ 
    a2l_quex_lexer_ByteLoader_POSIX* me = (a2l_quex_lexer_ByteLoader_POSIX*)(alter_ego);
    /* Use 'lseek(current position + 0)' to get the current position.        */
    return (a2l_quex_lexer_stream_position_t)lseek(me->fd, 0, SEEK_CUR); 
}

inline void    
a2l_quex_lexer_ByteLoader_POSIX_seek(a2l_quex_lexer_ByteLoader*    alter_ego, 
                                 a2l_quex_lexer_stream_position_t Pos) 
{ 
    a2l_quex_lexer_ByteLoader_POSIX* me = (a2l_quex_lexer_ByteLoader_POSIX*)(alter_ego);
    lseek(me->fd, (long)Pos, SEEK_SET); 
}

inline size_t  
a2l_quex_lexer_ByteLoader_POSIX_load(a2l_quex_lexer_ByteLoader* alter_ego, 
                                 void*                  buffer, 
                                 const size_t           ByteN, 
                                 bool*                  end_of_stream_f) 
/* The POSIX interface does not allow to detect end of file upon reading.
 * The caller will realize end of stream by a return of zero bytes.          */
{ 
    a2l_quex_lexer_ByteLoader_POSIX* me = (a2l_quex_lexer_ByteLoader_POSIX*)(alter_ego);
    ssize_t                      n  = read(me->fd, buffer, ByteN); 
    /* Theoretically, a last 'terminating zero' might be send over socket 
     * connections. Make sure, that this does not appear in the stream.      */
    if( n && ((uint8_t*)buffer)[n-1] == 0x0 ) {
        --n;
    }
    *end_of_stream_f = false;
    return (size_t)n;
}

inline bool  
a2l_quex_lexer_ByteLoader_POSIX_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                           const a2l_quex_lexer_ByteLoader* alter_ego_B) 
/* RETURNS: true  -- if A and B point to the same POSIX object.
 *          false -- else.                                                   */
{ 
    const a2l_quex_lexer_ByteLoader_POSIX* A = (a2l_quex_lexer_ByteLoader_POSIX*)(alter_ego_A);
    const a2l_quex_lexer_ByteLoader_POSIX* B = (a2l_quex_lexer_ByteLoader_POSIX*)(alter_ego_B);

    return A->fd == B->fd;
}

inline void                       
a2l_quex_lexer_ByteLoader_POSIX_print_this(a2l_quex_lexer_ByteLoader* alter_ego)
{
    a2l_quex_lexer_ByteLoader_POSIX* me = (a2l_quex_lexer_ByteLoader_POSIX*)(alter_ego);

    QUEX_DEBUG_PRINT("        type:             POSIX;\n");
    QUEX_DEBUG_PRINT1("        file_descriptor:  ((%i));\n", (int)me->fd);
    QUEX_DEBUG_PRINT("        end_of_stream_f:  <no means to detect>;\n");
}



#endif /*  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_POSIX_I */

