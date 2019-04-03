/* -*- C++ -*- vim: set syntax=cpp:
 * (C) 2005-2010 Frank-Rene Schaefer
 * ABSOLUTELY NO WARRANTY              */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__ASSERTS_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__ASSERTS_I

#include "a2l_quex_lexer\lib\buffer/Buffer"

#if defined(QUEX_OPTION_ASSERTS)



inline void  a2l_quex_lexer_assert_lexeme_valid_basic(const char*        FileName, 
                                                       size_t             LineN, 
                                                       a2l_quex_lexer_Buffer* buffer);
inline void  a2l_quex_lexer_assert_lexeme_valid(const char*        FileName, 
                                                 size_t             LineN, 
                                                 a2l_quex_lexer_Buffer* buffer);

inline void
a2l_quex_lexer_assert_lexeme_valid_basic(const char* FileName, size_t LineN, a2l_quex_lexer_Buffer* buffer)
{
    if( buffer->_read_p < buffer->begin(buffer) ) {              
        QUEX_DEBUG_PRINT2("%s:%i:\n", FileName, (int)LineN);     
        QUEX_ERROR_EXIT("Lexeme out of buffer bounds.\n");       
    } else if( buffer->_read_p >= buffer->end(buffer) ) {        
        QUEX_DEBUG_PRINT2("%s:%i:\n", FileName, (int)LineN);     
        QUEX_ERROR_EXIT("Lexeme out of buffer bounds.\n");       
    }
}

inline void
a2l_quex_lexer_assert_lexeme_valid(const char* FileName, size_t LineN, a2l_quex_lexer_Buffer* buffer)
{
    a2l_quex_lexer_assert_lexeme_valid_basic(FileName, LineN, buffer);                                                
    if( *(buffer->_read_p) != (a2l_quex_lexer_lexatom_t)0 ) {                             
        QUEX_DEBUG_PRINT2("%s:%i:\n", FileName, (int)LineN);                               
        QUEX_ERROR_EXIT("Tried to access lexeme while terminating zero was not set.\n"     
                        "This happens, for example, if navigation commands (undo, seek,\n" 
                        "seek_forward, or seek_backward) are used **before** accessing\n"  
                        "the lexeme via 'Lexeme' or 'LexemeBegin'.\n");                    
    } 
}

inline a2l_quex_lexer_lexatom_t*  
a2l_quex_lexer_access_Lexeme(const char* FileName, size_t LineN, a2l_quex_lexer_Buffer* buffer)
{
    a2l_quex_lexer_assert_lexeme_valid(FileName, LineN, buffer); 
    return buffer->_lexeme_start_p;
}

inline a2l_quex_lexer_lexatom_t*  
a2l_quex_lexer_access_LexemeBegin(const char* FileName, size_t LineN, a2l_quex_lexer_Buffer* buffer)
{
    a2l_quex_lexer_assert_lexeme_valid_basic(FileName, LineN, buffer); /* IMPORTANT: THAT IS THE DIFFERENCE TO 'accessLexeme()' */
    return buffer->_lexeme_start_p;
}

inline size_t                
a2l_quex_lexer_access_LexemeL(const char* FileName, size_t LineN, a2l_quex_lexer_Buffer* buffer)
{
    a2l_quex_lexer_assert_lexeme_valid_basic(FileName, LineN, buffer);
    return (size_t)(buffer->_read_p - buffer->_lexeme_start_p);
}

inline a2l_quex_lexer_lexatom_t*  
a2l_quex_lexer_access_LexemeEnd(const char* FileName, size_t LineN, a2l_quex_lexer_Buffer* buffer)
{
    a2l_quex_lexer_assert_lexeme_valid_basic(FileName, LineN, buffer);
    return buffer->_read_p;
}



#endif

#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__ASSERTS_I */
