/* -*- C++ -*- vim:set syntax=cpp:
 * (C) 2005-2010 Frank-Rene Schaefer
 * ABSOLUTELY NO WARRANTY                      */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__IMPLEMENTATIONS_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__IMPLEMENTATIONS_I

#if ! defined(__cplusplus)

#include "a2l_quex_lexer\lib\analyzer/asserts.i"
#include "a2l_quex_lexer\lib\buffer/asserts.i"

#include "a2l_quex_lexer\a2l_quex_lexer-token.i"
#include "a2l_quex_lexer\lib\token/TokenQueue.i"
#include "a2l_quex_lexer\lib\token/receiving.i"

#include "a2l_quex_lexer\lib\analyzer/member/mode-handling.i"
#include "a2l_quex_lexer\lib\analyzer/member/misc.i"
#include "a2l_quex_lexer\lib\analyzer/member/navigation.i"

#include "a2l_quex_lexer\lib\analyzer/struct/constructor.i"
#include "a2l_quex_lexer\lib\analyzer/struct/include-stack.i"
#include "a2l_quex_lexer\lib\analyzer/struct/reset.i"

#include "a2l_quex_lexer\lib\buffer/Buffer.i"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader.i"
#include "a2l_quex_lexer\lib\buffer/bytes/ByteLoader.i"

#include "a2l_quex_lexer\lib\analyzer/Mode.i"

#include "a2l_quex_lexer\lib\lexeme/basics.i"
#include "a2l_quex_lexer\lib\analyzer/Counter.i"

#endif

#include "a2l_quex_lexer\lib\quex/MemoryManager.i"
#include "a2l_quex_lexer\lib\lexeme/converter-from-lexeme.i"



a2l_quex_lexer_lexatom_t   a2l_quex_lexer_LexemeNull = (a2l_quex_lexer_lexatom_t)0;




#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__IMPLEMENTATIONS_I */
