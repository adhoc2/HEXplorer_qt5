/* -*- C++ -*-  vim:set syntax=cpp: 
 *
 * (C) 2004-2010 Frank-Rene Schaefer
 * ABSOLUTELY NO WARRANTY              */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__MODE_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__MODE_I

#include "a2l_quex_lexer\lib\definitions"



inline void
a2l_quex_lexer_Mode_uncallable_analyzer_function(a2l_quex_lexer* me)
{ 
    __quex_assert(0); 
    (void)me;
    return; 
}



inline void
a2l_quex_lexer_Mode_on_entry_exit_null_function(a2l_quex_lexer* me, const a2l_quex_lexer_Mode* TheMode) 
{ (void)me; (void)TheMode; }

inline bool
a2l_quex_lexer_ModeStack_construct(a2l_quex_lexer_ModeStack* me, const size_t N)
{
    me->begin = (const a2l_quex_lexer_Mode**)quex::MemoryManager_allocate(
                                N * sizeof(a2l_quex_lexer_Mode*),
                                E_MemoryObjectType_MODE_STACK);
    if( ! me->begin ) {
        a2l_quex_lexer_ModeStack_resources_absent_mark(me);
        return false;
    }
    else {
        me->end        = &me->begin[0];
        me->memory_end = &me->begin[N];
        return true;
    }
}

inline void
a2l_quex_lexer_ModeStack_destruct(a2l_quex_lexer_ModeStack* me)
{
    if( me->begin ) {
        quex::MemoryManager_free((void*)&me->begin[0],
                                   E_MemoryObjectType_MODE_STACK);
    }
    a2l_quex_lexer_ModeStack_resources_absent_mark(me);
}

inline void
a2l_quex_lexer_ModeStack_resources_absent_mark(a2l_quex_lexer_ModeStack* me)
{
    me->begin      = (const a2l_quex_lexer_Mode**)0;
    me->end        = (const a2l_quex_lexer_Mode**)0;
    me->memory_end = (const a2l_quex_lexer_Mode**)0;
}

inline bool
a2l_quex_lexer_ModeStack_resources_absent(a2l_quex_lexer_ModeStack* me)
{
    return    me->end        == (const a2l_quex_lexer_Mode**)0
           && me->memory_end == (const a2l_quex_lexer_Mode**)0;
}

inline void
a2l_quex_lexer_ModeStack_print(a2l_quex_lexer_ModeStack* me)
{
    const a2l_quex_lexer_Mode** iterator = 0x0;
    if( a2l_quex_lexer_ModeStack_resources_absent(me) ) {
        QUEX_DEBUG_PRINT("<uninitialized>\n");
    }
    else {
        QUEX_DEBUG_PRINT("{\n");
        QUEX_DEBUG_PRINT1("    size:    %i;\n",
                          (int)(me->memory_end - me->begin));
        QUEX_DEBUG_PRINT("    content: [");
        if( me->end > me->memory_end || me->end < me->begin ) {
            QUEX_DEBUG_PRINT("<pointer corrupted>");
        }
        else {
            for(iterator=&me->end[-1]; iterator >= me->begin; --iterator) {
                QUEX_DEBUG_PRINT1("%s, ", (*iterator)->name);
            }
        }
        QUEX_DEBUG_PRINT("]\n");
        QUEX_DEBUG_PRINT("  }\n");
    }
}



#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__MODE_I */
