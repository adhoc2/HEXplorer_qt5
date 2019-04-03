/* -*- C++ -*- vim: set syntax=cpp: 
 * PURPOSE: Token Queue 
 *
 * A token queue is a queue where read and write cycles are separate.
 * That is, when the queue is filled, it is not read until the 
 * filling terminated. Then, the read does not terminate before there
 * is no more token left.
 *
 * Wrap-arround is neither necessary nor meaningful!
 *
 * (C) 2004-2017 Frank-Rene Schaefer                                          */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__TOKEN__TOKEN_QUEUE_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__TOKEN__TOKEN_QUEUE_I

#include "a2l_quex_lexer\lib\definitions"
#include "a2l_quex_lexer\lib\quex/asserts"
#include "a2l_quex_lexer\lib\quex/MemoryManager"

/* NOTE: a2l_quex_lexer_Token must be defined at this place!                       */

#include "a2l_quex_lexer\lib\token/TokenQueue"



inline void             
a2l_quex_lexer_TokenQueue_push_core(a2l_quex_lexer_TokenQueue* me, a2l_quex_lexer_token_id_t Id);

inline bool
a2l_quex_lexer_TokenQueue_construct(a2l_quex_lexer_TokenQueue* me, 
                                a2l_quex_lexer*    lexer,
                                const size_t           N)
/* me:     The token queue.
 * Memory: Pointer to memory of token queue, 0x0 --> no initial memory.
 * N:      Number of token objects that the array can carry.                  */
{
    a2l_quex_lexer_Token*   iterator   = 0x0;
    a2l_quex_lexer_Token*   memory     = (a2l_quex_lexer_Token*)quex::MemoryManager_allocate(
                                             N * sizeof(a2l_quex_lexer_Token),
                                             E_MemoryObjectType_TOKEN_ARRAY);
    a2l_quex_lexer_Token*   memory_end = &memory[N];

    if( ! memory ) {
        a2l_quex_lexer_TokenQueue_resources_absent_mark(me);
        return false;
    }

    __quex_assert(memory != 0x0);

    /* Call placement new (plain constructor) for all tokens in chunk.        */
    for(iterator = memory; iterator != memory_end; ++iterator) {
        a2l_quex_lexer_Token_construct(iterator);
    }
    a2l_quex_lexer_TokenQueue_init(me, memory, memory_end); 
    me->the_lexer = lexer;
    return true;
}

inline void
a2l_quex_lexer_TokenQueue_reset(a2l_quex_lexer_TokenQueue* me) 
{                                                    
    me->read_iterator  = (a2l_quex_lexer_Token*)me->begin; 
    me->write_iterator = (a2l_quex_lexer_Token*)me->begin; 
}

inline void
a2l_quex_lexer_TokenQueue_init(a2l_quex_lexer_TokenQueue* me, 
                           a2l_quex_lexer_Token*       Memory, 
                           a2l_quex_lexer_Token*       MemoryEnd) 
{
    me->begin = Memory;                           
    me->end   = MemoryEnd;                        
    a2l_quex_lexer_TokenQueue_reset(me);                                
}

inline void
a2l_quex_lexer_TokenQueue_resources_absent_mark(a2l_quex_lexer_TokenQueue* me) 
{
    me->begin          = (a2l_quex_lexer_Token*)0;                           
    me->end            = (a2l_quex_lexer_Token*)0;
    me->read_iterator  = (a2l_quex_lexer_Token*)0; 
    me->write_iterator = (a2l_quex_lexer_Token*)0; 
    me->the_lexer      = (a2l_quex_lexer*)0; 
}

inline bool
a2l_quex_lexer_TokenQueue_resources_absent(a2l_quex_lexer_TokenQueue* me) 
{
    return    me->begin          == (a2l_quex_lexer_Token*)0                           
           && me->end            == (a2l_quex_lexer_Token*)0
           && me->read_iterator  == (a2l_quex_lexer_Token*)0 
           && me->write_iterator == (a2l_quex_lexer_Token*)0
           && me->the_lexer      == (a2l_quex_lexer*)0;
}

inline void
a2l_quex_lexer_TokenQueue_destruct(a2l_quex_lexer_TokenQueue* me)
{
    a2l_quex_lexer_Token* iterator = 0x0;
    /* Call explicit destructors for all tokens in array                      */
    for(iterator = me->begin; iterator != me->end; ++iterator) {
        a2l_quex_lexer_Token_destruct(iterator);
    }

    quex::MemoryManager_free((void*)&me->begin[0],
                               E_MemoryObjectType_TOKEN_ARRAY);

    /* The memory chunk for the token queue itself is located inside the
     * analyzer object. Thus, no explicit free is necessary. In case of user
     * managed token queue memory the user takes care of the deletion.        */
    a2l_quex_lexer_TokenQueue_resources_absent_mark(me);
}

inline void   
a2l_quex_lexer_TokenQueue_remainder_get(a2l_quex_lexer_TokenQueue* me,
                                    a2l_quex_lexer_Token**      begin,
                                    a2l_quex_lexer_Token**      end)
{
    *begin = me->read_iterator;
    *end   = me->write_iterator;
    a2l_quex_lexer_TokenQueue_reset(me);
}

inline void 
a2l_quex_lexer_TokenQueue_memory_get(a2l_quex_lexer_TokenQueue* me,
                                 a2l_quex_lexer_Token**      memory,
                                 size_t*                n)
{
    *memory = me->begin;
    *n      = (size_t)(me->end - me->begin);
}

inline bool 
a2l_quex_lexer_TokenQueue_is_full(a2l_quex_lexer_TokenQueue* me) 
{ return me->write_iterator >= me->end; }

inline bool 
a2l_quex_lexer_TokenQueue_is_empty(a2l_quex_lexer_TokenQueue* me)
{ return me->read_iterator == me->write_iterator; }

inline void             
a2l_quex_lexer_TokenQueue_push(a2l_quex_lexer_TokenQueue* me,
                           a2l_quex_lexer_token_id_t     Id)
/* Push a token and set only its token identifier.                           */
{

    a2l_quex_lexer_TokenQueue_push_core(me, Id);
}

inline void             
a2l_quex_lexer_TokenQueue_push_core(a2l_quex_lexer_TokenQueue* me,
                                a2l_quex_lexer_token_id_t     Id)
{
    if( a2l_quex_lexer_TokenQueue_is_full(me) ) {
        me->the_lexer->error_code = E_Error_Token_QueueOverflow;
        return;
    }
    a2l_quex_lexer_TokenQueue_assert_before_sending(me);  

       me->write_iterator->_line_n   = me->the_lexer->counter._line_number_at_begin;
     me->write_iterator->_column_n = me->the_lexer->counter._column_number_at_begin;

    me->write_iterator->id = Id;              
    ++(me->write_iterator);       
}


inline bool             
a2l_quex_lexer_TokenQueue_push_text(a2l_quex_lexer_TokenQueue* me,
                                a2l_quex_lexer_token_id_t     Id,
                                a2l_quex_lexer_lexatom_t*     BeginP,
                                a2l_quex_lexer_lexatom_t*     EndP)
/* Push a token and set its 'text' member.                                    */
{
    bool ownership_transferred_to_token_f = false;
    a2l_quex_lexer_TokenQueue_assert_before_sending(me);
    ownership_transferred_to_token_f = a2l_quex_lexer_Token_take_text(me->write_iterator, BeginP, EndP);
    a2l_quex_lexer_TokenQueue_push(me, Id);
    return ownership_transferred_to_token_f;
}







inline void             
a2l_quex_lexer_TokenQueue_push_repeated(a2l_quex_lexer_TokenQueue* me,
                                    a2l_quex_lexer_token_id_t     Id,
                                    size_t                 RepetitionN)
/* Push a repeated token by 'RepetitionN' times. This is only addmissible for
 * TokenId-s specified in the 'repeated_token' section of the '.qx' file.     */
{
    a2l_quex_lexer_TokenQueue_assert_before_sending(me);  
    (void)me; (void)Id; (void)RepetitionN;
    __quex_assert(RepetitionN != 0);        

    __quex_assert((const char*)0 == "Token type does not support token repetition.");
    a2l_quex_lexer_TokenQueue_set_token_TERMINATION(me);
}

inline a2l_quex_lexer_Token* 
a2l_quex_lexer_TokenQueue_pop(a2l_quex_lexer_TokenQueue* me)
{
    __quex_assert(a2l_quex_lexer_TokenQueue_begin(me) != 0x0);

    if( a2l_quex_lexer_TokenQueue_is_empty(me) ) {        
        return (a2l_quex_lexer_Token*)0;
    }
    /* Tokens are in queue --> take next token from queue                    */ 
    return me->read_iterator++;
}


inline a2l_quex_lexer_Token* a2l_quex_lexer_TokenQueue_begin(a2l_quex_lexer_TokenQueue* me)
{ return me->begin; }

inline a2l_quex_lexer_Token* a2l_quex_lexer_TokenQueue_back(a2l_quex_lexer_TokenQueue* me)
{ return me->end - 1; }

inline a2l_quex_lexer_Token* a2l_quex_lexer_TokenQueue_last_token(a2l_quex_lexer_TokenQueue* me)
{ return me->write_iterator == me->begin ? (a2l_quex_lexer_Token*)0 : &me->write_iterator[-1]; }

inline size_t a2l_quex_lexer_TokenQueue_available_n(a2l_quex_lexer_TokenQueue* me) 
{ return (size_t)(me->end - me->write_iterator); }

inline void
a2l_quex_lexer_TokenQueue_set_token_TERMINATION(a2l_quex_lexer_TokenQueue* me) 
/* Reset entire token queue and set the token 'TERMINATION'. This should
 * only be called in case of a detected error.                                */
{
    a2l_quex_lexer_TokenQueue_reset(me);

    a2l_quex_lexer_TokenQueue_push_text(me, QUEX_TKN_TERMINATION, 
                                    (a2l_quex_lexer_lexatom_t*)0, (a2l_quex_lexer_lexatom_t*)0);


}

inline bool             
a2l_quex_lexer_TokenQueue_assert_before_sending(a2l_quex_lexer_TokenQueue* me)
{                                                                              
    if( ! a2l_quex_lexer_TokenQueue_assert_after_sending(me) ) {
        return false;
    } 
    /* End of token queue has not been reached.                       */          
    __quex_assert((me)->write_iterator != (me)->end);                             
    /* No token sending after 'TERMINATION'.                          */          
    __quex_assert(   (me)->write_iterator         == (me)->begin                  
                  || (me)->write_iterator[-1].id !=  QUEX_TKN_TERMINATION ); 
    return true;
} 

inline bool             
a2l_quex_lexer_TokenQueue_assert_after_sending(a2l_quex_lexer_TokenQueue* me)
{                                                                     
    __quex_assert((me)->begin != 0x0);                                   
    __quex_assert((me)->read_iterator  >= (me)->begin);                  
    __quex_assert((me)->write_iterator >= (me)->read_iterator);          
    /* If the following breaks, then the given queue size was to small*/ 
    if( (me)->write_iterator > (me)->end ) {                            
        QUEX_ERROR_EXIT("Error: Token queue overflow.");                 
        return false;
    }                                                                    
    return true;
}



#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__TOKEN__TOKEN_QUEUE_I */
