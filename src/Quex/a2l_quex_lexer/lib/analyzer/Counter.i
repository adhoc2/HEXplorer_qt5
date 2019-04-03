#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__COUNTER_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__COUNTER_I

#include "a2l_quex_lexer\lib\definitions"
#include "a2l_quex_lexer\lib\analyzer/asserts"
#include "a2l_quex_lexer\lib\analyzer/Counter"



inline bool
a2l_quex_lexer_Counter_construct(a2l_quex_lexer_Counter* me)
{
       me->_line_number_at_begin   = (size_t)1;
       me->_line_number_at_end     = (size_t)1;
     me->_column_number_at_begin = (size_t)1;
     me->_column_number_at_end   = (size_t)1;

    
    return true;
}

inline void
a2l_quex_lexer_Counter_resources_absent_mark(a2l_quex_lexer_Counter* me)
{
     me->_line_number_at_begin               = (size_t)0;
     me->_line_number_at_end                 = (size_t)0;
     me->_column_number_at_begin           = (size_t)0;
     me->_column_number_at_end             = (size_t)0;
    
    
}

inline bool
a2l_quex_lexer_Counter_resources_absent(a2l_quex_lexer_Counter* me)
{
     if( me->_line_number_at_begin               != (size_t)0) return false;
     if( me->_line_number_at_end                 != (size_t)0) return false;
     if( me->_column_number_at_begin           != (size_t)0) return false;
     if( me->_column_number_at_end             != (size_t)0) return false;
    
    
    return true;
}


inline void 
a2l_quex_lexer_Counter_print_this(a2l_quex_lexer_Counter* me)
{
    

    QUEX_DEBUG_PRINT("  counter: ");
    if( a2l_quex_lexer_Counter_resources_absent(me) ) {
        QUEX_DEBUG_PRINT("<unitialized>\n");
        return;
    }
    QUEX_DEBUG_PRINT("{\n");


    QUEX_DEBUG_PRINT1("    _line_number_at_begin:   %i;\n", (int)me->_line_number_at_begin);
    QUEX_DEBUG_PRINT1("    _line_number_at_end:     %i;\n", (int)me->_line_number_at_end);


    QUEX_DEBUG_PRINT1("    _column_number_at_begin: %i;\n", (int)me->_column_number_at_begin);
    QUEX_DEBUG_PRINT1("    _column_number_at_end:   %i;\n", (int)me->_column_number_at_end);


    QUEX_DEBUG_PRINT("  }\n");
}




#ifdef QUEX_OPTION_ASSERTS
inline void
a2l_quex_lexer_Counter_assert_consistency(a2l_quex_lexer_Counter* me)
{

    /* The line number can never decrease.                                   */ 
    __quex_assert(me->_line_number_at_begin <= me->_line_number_at_end);            
    /* Line number remained the same => column number *must* have increased. */ 
    /* There is not pattern of a length less than 1                          */ 
    if(me->_line_number_at_begin == me->_line_number_at_end ) {                     
        __quex_assert(me->_column_number_at_begin < me->_column_number_at_end);     
    }                                                                                   


    /* If only column numbers are counted, then no assumptions can be made 
     * about increase or decrease. If a newline appears, for example, the 
     * column number may decrease.                                            */
}
#endif



#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__COUNTER_I */

