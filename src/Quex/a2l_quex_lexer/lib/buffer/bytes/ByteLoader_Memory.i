/* vim: set ft=c:
 * (C) Frank-Rene Schaefer */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_MEMORY_I
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_MEMORY_I

#include "a2l_quex_lexer\lib\quex/MemoryManager"
#include "a2l_quex_lexer\lib\buffer/bytes/ByteLoader_Memory"
#include <malloc.h> // DEBUG



inline void                       a2l_quex_lexer_ByteLoader_Memory_construct(a2l_quex_lexer_ByteLoader_Memory* me,
                                                                              const uint8_t*                BeginP,
                                                                              const uint8_t*                EndP);
inline a2l_quex_lexer_stream_position_t  a2l_quex_lexer_ByteLoader_Memory_tell(a2l_quex_lexer_ByteLoader* me);
inline void                       a2l_quex_lexer_ByteLoader_Memory_seek(a2l_quex_lexer_ByteLoader*    me, 
                                                                         a2l_quex_lexer_stream_position_t Pos);
inline size_t                     a2l_quex_lexer_ByteLoader_Memory_load(a2l_quex_lexer_ByteLoader* me, 
                                                                         void*                  buffer, 
                                                                         const size_t           ByteN, 
                                                                         bool*                  end_of_stream_f);
inline void                       a2l_quex_lexer_ByteLoader_Memory_delete_self(a2l_quex_lexer_ByteLoader* me);
inline void                       a2l_quex_lexer_ByteLoader_Memory_print_this(a2l_quex_lexer_ByteLoader* me);
inline bool                       a2l_quex_lexer_ByteLoader_Memory_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                                                                   const a2l_quex_lexer_ByteLoader* alter_ego_B);

inline a2l_quex_lexer_ByteLoader*    
a2l_quex_lexer_ByteLoader_Memory_new(const uint8_t*  BeginP,
                                 const uint8_t*  EndP)
{
    a2l_quex_lexer_ByteLoader_Memory* me;
   
    if( ! BeginP || ! EndP ) return (a2l_quex_lexer_ByteLoader*)0;

    me = (a2l_quex_lexer_ByteLoader_Memory*)quex::MemoryManager_allocate(sizeof(a2l_quex_lexer_ByteLoader_Memory),
                                                                       E_MemoryObjectType_BYTE_LOADER);
    if( ! me ) return (a2l_quex_lexer_ByteLoader*)0;

    a2l_quex_lexer_ByteLoader_Memory_construct(me, BeginP, EndP);

    return &me->base;
}

inline a2l_quex_lexer_ByteLoader*    
a2l_quex_lexer_ByteLoader_Memory_new_from_file_name(const char* FileName)
{
    size_t                  size;
    uint8_t*                begin_p;
    a2l_quex_lexer_ByteLoader*  me = (a2l_quex_lexer_ByteLoader*)0;

    /* Determine size of file                                                */;
    FILE* fh = fopen(FileName, "rb"); 
    if( ! fh ) return me;
        
    if( fseek(fh, 0, SEEK_END) ) {
        fclose(fh);
        return me;
    }
    size = (size_t)ftell(fh);

    if( fseek(fh, 0, SEEK_SET) ) {
        fclose(fh);
        return me;
    }

    /* Load the file's content into some memory.                             */
    begin_p = (uint8_t*)quex::MemoryManager_allocate(size, E_MemoryObjectType_BUFFER_MEMORY);
    if( size != fread(begin_p, 1, size, fh) ) {
        quex::MemoryManager_free(begin_p, E_MemoryObjectType_BUFFER_MEMORY);
        return me;
    }

    /* Call the main new operator to allocate the byte loader.               */
    me = a2l_quex_lexer_ByteLoader_Memory_new(begin_p, &begin_p[size]);
    if( ! me ) {
        quex::MemoryManager_free(begin_p, E_MemoryObjectType_BUFFER_MEMORY);
    }
    else {
        /* Mark memory ownership => destructor deletes it.                   */
        ((a2l_quex_lexer_ByteLoader_Memory*)me)->memory_ownership = E_Ownership_LEXICAL_ANALYZER;
    }

    return me;
}

inline void
a2l_quex_lexer_ByteLoader_Memory_construct(a2l_quex_lexer_ByteLoader_Memory* me, 
                                       const uint8_t*                BeginP,
                                       const uint8_t*                EndP)
{
    __quex_assert(EndP >= BeginP);
    me->byte_array.begin_p  = BeginP;
    me->byte_array.position = BeginP;
    me->byte_array.end_p    = EndP;
    a2l_quex_lexer_ByteLoader_construct(&me->base,
                         a2l_quex_lexer_ByteLoader_Memory_tell,
                         a2l_quex_lexer_ByteLoader_Memory_seek,
                         a2l_quex_lexer_ByteLoader_Memory_load,
                         a2l_quex_lexer_ByteLoader_Memory_delete_self,
                         a2l_quex_lexer_ByteLoader_Memory_print_this,
                         a2l_quex_lexer_ByteLoader_Memory_compare_handle);
    me->base.binary_mode_f = true;
    me->memory_ownership   = E_Ownership_EXTERNAL; /* Default */
}

inline void    
a2l_quex_lexer_ByteLoader_Memory_delete_self(a2l_quex_lexer_ByteLoader* alter_ego)
{
    /* NOTE: The momory's ownership remains in the hand of the one who
     *       constructed this object.                                        */
    a2l_quex_lexer_ByteLoader_Memory* me = (a2l_quex_lexer_ByteLoader_Memory*)(alter_ego);

    if( me->memory_ownership == E_Ownership_LEXICAL_ANALYZER ) {
        quex::MemoryManager_free((void*)&me->byte_array.begin_p[0], 
                                   E_MemoryObjectType_BUFFER_MEMORY);
    }

    quex::MemoryManager_free(me, E_MemoryObjectType_BYTE_LOADER);

}

inline a2l_quex_lexer_stream_position_t    
a2l_quex_lexer_ByteLoader_Memory_tell(a2l_quex_lexer_ByteLoader* alter_ego)            
{ 
    const a2l_quex_lexer_ByteLoader_Memory* me = (a2l_quex_lexer_ByteLoader_Memory*)(alter_ego);

    return me->byte_array.position - me->byte_array.begin_p;
}

inline void    
a2l_quex_lexer_ByteLoader_Memory_seek(a2l_quex_lexer_ByteLoader* alter_ego, a2l_quex_lexer_stream_position_t Pos) 
{ 
    a2l_quex_lexer_ByteLoader_Memory* me = (a2l_quex_lexer_ByteLoader_Memory*)(alter_ego);

    if( Pos > me->byte_array.end_p -  me->byte_array.begin_p ) return;
    me->byte_array.position = &me->byte_array.begin_p[(ptrdiff_t)Pos];
}

inline size_t  
a2l_quex_lexer_ByteLoader_Memory_load(a2l_quex_lexer_ByteLoader* alter_ego, 
                                void*                    buffer, 
                                const size_t             ByteN, 
                                bool*                    end_of_stream_f) 
{ 
    a2l_quex_lexer_ByteLoader_Memory* me = (a2l_quex_lexer_ByteLoader_Memory*)(alter_ego);
    const ptrdiff_t               Remaining = me->byte_array.end_p - me->byte_array.position;
    ptrdiff_t                     copy_n;

    if( (size_t)Remaining < ByteN ) { copy_n = Remaining; *end_of_stream_f = true; }
    else                            { copy_n = ByteN; } 

    __QUEX_STD_memcpy((void*)buffer, (void*)me->byte_array.position, copy_n);
    me->byte_array.position += copy_n;
    return copy_n;
}

inline bool  
a2l_quex_lexer_ByteLoader_Memory_compare_handle(const a2l_quex_lexer_ByteLoader* alter_ego_A, 
                                            const a2l_quex_lexer_ByteLoader* alter_ego_B) 
/* RETURNS: true  -- if A and B point to the same Memory object.
 *          false -- else.                                                   */
{ 
    const a2l_quex_lexer_ByteLoader_Memory* A = (a2l_quex_lexer_ByteLoader_Memory*)(alter_ego_A);
    const a2l_quex_lexer_ByteLoader_Memory* B = (a2l_quex_lexer_ByteLoader_Memory*)(alter_ego_B);

    return    A->byte_array.begin_p  == B->byte_array.begin_p
           && A->byte_array.end_p    == B->byte_array.end_p
           && A->byte_array.position == B->byte_array.position;
}

inline void                       
a2l_quex_lexer_ByteLoader_Memory_print_this(a2l_quex_lexer_ByteLoader* alter_ego)
{
    a2l_quex_lexer_ByteLoader_Memory* me = (a2l_quex_lexer_ByteLoader_Memory*)(alter_ego);

    QUEX_DEBUG_PRINT("        type:             memory;\n");
    QUEX_DEBUG_PRINT1("        memory_ownership: %s;\n", E_Ownership_NAME(me->memory_ownership));
    QUEX_DEBUG_PRINT3("        byte_array:       { begin: ((%p)) end: ((%p)) size: %i; }\n",
                      (const void*)me->byte_array.begin_p, 
                      (const void*)me->byte_array.end_p, 
                      (int)(me->byte_array.end_p - me->byte_array.begin_p));
    QUEX_DEBUG_PRINT("        input_position:   ");
    quex::print_relative_positions(me->byte_array.begin_p, me->byte_array.end_p, 1, 
                                     (void*)me->byte_array.position);
    QUEX_DEBUG_PRINT("\n");
}



#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BYTES__BYTE_LOADER_MEMORY_I */
