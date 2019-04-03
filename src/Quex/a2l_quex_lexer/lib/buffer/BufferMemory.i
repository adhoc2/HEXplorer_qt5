/* vim:set ft=c: -*- C++ -*- */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_MEMORY_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_MEMORY_I

#include "a2l_quex_lexer\lib\quex/asserts"
#include "a2l_quex_lexer\lib\buffer/asserts"
#include "a2l_quex_lexer\lib\definitions"
#include "a2l_quex_lexer\lib\buffer/Buffer"
#include "a2l_quex_lexer\lib\buffer/Buffer_print.i"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader"
#include "a2l_quex_lexer\lib\quex/MemoryManager"



inline void 
a2l_quex_lexer_BufferMemory_construct(a2l_quex_lexer_BufferMemory*  me, 
                                  a2l_quex_lexer_lexatom_t*        Memory, 
                                  const size_t              Size,
                                  E_Ownership               Ownership,
                                  a2l_quex_lexer_Buffer*        IncludingBuffer) 
{
    __quex_assert(E_Ownership_is_valid(Ownership));    

    if( Memory ) {
        me->_front    = Memory;
        me->_back     = &Memory[Size-1];
        me->ownership = Ownership;
        me->_front[0] = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER;
        me->_back[0]  = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER;
    } else {
        me->_front    = (a2l_quex_lexer_lexatom_t*)0;
        me->_back     = me->_front;
        me->ownership = Ownership;
    }
    
    if( Ownership == E_Ownership_INCLUDING_BUFFER ) { 
        __quex_assert(0 != IncludingBuffer); 
    }
    else { 
        __quex_assert(0 == IncludingBuffer); 
        __quex_assert(me != &IncludingBuffer->_memory); 
    }

    me->including_buffer = IncludingBuffer;
}

inline void 
a2l_quex_lexer_BufferMemory_destruct(a2l_quex_lexer_BufferMemory* me) 
/* Does not set 'me->_front' to zero, if it is not deleted. Thus, the user
 * may detect wether it needs to be deleted or not.                           */
{
    if( me->_front ) {
        switch( me->ownership ) {
        case E_Ownership_LEXICAL_ANALYZER:
            __quex_assert(0 == me->including_buffer);
            quex::MemoryManager_free((void*)me->_front, 
                                       E_MemoryObjectType_BUFFER_MEMORY);
            break;
        case E_Ownership_INCLUDING_BUFFER:
            __quex_assert(0 != me->including_buffer);
            __quex_assert(me->_front == me->including_buffer->end(me->including_buffer));
            me->including_buffer->_memory._back = me->_back;
        default: 
            break;
        }
    }
    a2l_quex_lexer_BufferMemory_resources_absent_mark(me);
}


inline void 
a2l_quex_lexer_BufferMemory_resources_absent_mark(a2l_quex_lexer_BufferMemory* me) 
/* Marks memory absent. If previously the ownership was 'E_Ownership_EXTERNAL'.
 * then the concerned memory is no longer referred by this buffer.            */
{
    /* 'me->_front == 0' prevents 'MemoryManager_free()'                      */
    me->_front = me->_back = (a2l_quex_lexer_lexatom_t*)0;
    me->ownership        = E_Ownership_LEXICAL_ANALYZER;
    me->including_buffer = (a2l_quex_lexer_Buffer*)0;
}

inline bool 
a2l_quex_lexer_BufferMemory_resources_absent(a2l_quex_lexer_BufferMemory* me) 
{
    /* Ownership is irrelevant.                                               */
    return    (me->_front == me->_back) 
           && (me->_front == (a2l_quex_lexer_lexatom_t*)0)
           && (me->including_buffer == (a2l_quex_lexer_Buffer*)0);
}

inline size_t          
a2l_quex_lexer_BufferMemory_size(a2l_quex_lexer_BufferMemory* me)
{ return (size_t)(me->_back - me->_front + 1); }

inline bool
a2l_quex_lexer_BufferMemory_check_chunk(const a2l_quex_lexer_lexatom_t* Front, 
                                    size_t                   Size, 
                                    const a2l_quex_lexer_lexatom_t* EndOfFileP) 
{
    const a2l_quex_lexer_lexatom_t* Back = &Front[Size-1];

    if( ! Front ) {
        return (Size == 0 && ! EndOfFileP) ? true : false; 
    }
    else if(   Size < 3  
            || ! EndOfFileP
            || EndOfFileP    <= Front 
            || EndOfFileP    >  Back  
            || Front[0]      != QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER    
            || Back[0]       != QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER 
            || EndOfFileP[0] != QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER ) {
        return false;
    }
    else  {
        return true;
    }
}

inline void
a2l_quex_lexer_BufferMemory_print_this(a2l_quex_lexer_BufferMemory* me)
{
    QUEX_DEBUG_PRINT("    memory: {\n");
    QUEX_DEBUG_PRINT1("      ownership: %s;\n", E_Ownership_NAME(me->ownership));
    QUEX_DEBUG_PRINT1("      begin:     ((%p)) (== _front);\n",       (void*)me->_front);
    QUEX_DEBUG_PRINT1("      end:       begin + %i (== _back + 1);\n", (int)(me->_back + 1 - me->_front));
    QUEX_DEBUG_PRINT("    }\n");
}



#endif /*  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_MEMORY_I */
