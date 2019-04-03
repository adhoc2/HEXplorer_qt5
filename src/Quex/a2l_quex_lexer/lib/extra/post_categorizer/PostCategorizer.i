/* -*- C++ -*- vim: set syntax=cpp: */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__EXTRA__POST_CATEGORIZER__POST_CATEGORIZER_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__EXTRA__POST_CATEGORIZER__POST_CATEGORIZER_I

#include "a2l_quex_lexer\lib\extra/post_categorizer/PostCategorizer"
#include "a2l_quex_lexer\lib\lexeme/basics"
#include "a2l_quex_lexer\lib\lexeme/converter-from-lexeme"
#include "a2l_quex_lexer\lib\quex/MemoryManager"
#include "a2l_quex_lexer\lib\quex/standard_functions"
#include "a2l_quex_lexer\lib\quex/debug_print"
#include "a2l_quex_lexer\lib\quex/asserts"




inline void
a2l_quex_lexer_PostCategorizer__enter(a2l_quex_lexer_Dictionary*    me, 
                                  const a2l_quex_lexer_lexatom_t*  EntryName, 
                                  a2l_quex_lexer_token_id_t        TokenID);
                          
inline void          
a2l_quex_lexer_PostCategorizer__insert(a2l_quex_lexer_Dictionary*    me, 
                                   const a2l_quex_lexer_lexatom_t*  EntryName, 
                                   a2l_quex_lexer_token_id_t        TokenID);
                          
inline a2l_quex_lexer_token_id_t 
a2l_quex_lexer_PostCategorizer__get_token_id(const a2l_quex_lexer_Dictionary*  me,
                                         const a2l_quex_lexer_lexatom_t*      Lexeme);

inline void          
a2l_quex_lexer_PostCategorizer__remove(a2l_quex_lexer_Dictionary*    me, 
                                   const a2l_quex_lexer_lexatom_t*  EntryName);
                          
inline void          
a2l_quex_lexer_PostCategorizer__clear(a2l_quex_lexer_Dictionary* me);
                          
inline void          
a2l_quex_lexer_PostCategorizer__clear_recursively(a2l_quex_lexer_Dictionary*      me, 
                                              a2l_quex_lexer_DictionaryNode*  branch);
inline int           
a2l_quex_lexer_PostCategorizer__compare(a2l_quex_lexer_DictionaryNode*  me, 
                                    a2l_quex_lexer_lexatom_t           FirstCharacter, 
                                    const a2l_quex_lexer_lexatom_t*    Remainder);

inline a2l_quex_lexer_DictionaryNode*  
a2l_quex_lexer_PostCategorizer__allocate_node(const a2l_quex_lexer_lexatom_t* Remainder);

inline void 
a2l_quex_lexer_PostCategorizer__free_node(a2l_quex_lexer_DictionaryNode* node);

inline a2l_quex_lexer_DictionaryNode*
a2l_quex_lexer_PostCategorizer__find(const a2l_quex_lexer_Dictionary* me, 
                                 const a2l_quex_lexer_lexatom_t*     EntryName);



inline  a2l_quex_lexer_DictionaryNode*  
a2l_quex_lexer_PostCategorizer__allocate_node(const a2l_quex_lexer_lexatom_t* Remainder)
{
    const size_t  RemainderL = a2l_quex_lexer_lexeme_length(Remainder);
    /* Allocate in one beat: base and remainder: 
     *
     *   [Base   |Remainder             ]
     *
     * Then bend the base->name_remainder to the Remainder part of the allocated
     * memory. Note, that this is not very efficient, since one should try to allocate
     * the small node objects and refer to the remainder only when necessary. This
     * would reduce cache misses.                                                      */
    const size_t   BaseSize      = sizeof(a2l_quex_lexer_DictionaryNode);
    /* Length + 1 == memory size (terminating zero) */
    const size_t   RemainderSize = sizeof(a2l_quex_lexer_lexatom_t) * (RemainderL + 1);
    uint8_t*       base          = 
                      (uint8_t*)
                      quex::MemoryManager_allocate(BaseSize + RemainderSize, 
                                                     E_MemoryObjectType_POST_CATEGORIZER_NODE);
    ((a2l_quex_lexer_DictionaryNode*)base)->name_remainder = (const a2l_quex_lexer_lexatom_t*)(base + BaseSize);
    return (a2l_quex_lexer_DictionaryNode*)base;
}

inline  void 
a2l_quex_lexer_PostCategorizer__free_node(a2l_quex_lexer_DictionaryNode* node)
{ 
    if( ! node ) return;
    
    quex::MemoryManager_free((void*)node, 
                               E_MemoryObjectType_POST_CATEGORIZER_NODE); 
}

inline a2l_quex_lexer_DictionaryNode* 
a2l_quex_lexer_DictionaryNode_new(a2l_quex_lexer_lexatom_t         FirstCharacter,
                              const a2l_quex_lexer_lexatom_t*  Remainder,
                              a2l_quex_lexer_token_id_t        TokenID)
{
    a2l_quex_lexer_DictionaryNode* me = a2l_quex_lexer_PostCategorizer__allocate_node(Remainder);

    me->name_first_character = FirstCharacter;
    me->name_remainder       = Remainder;
    me->token_id             = TokenID;
    me->lesser               = 0x0;
    me->greater              = 0x0;

    return me;
}

inline bool
a2l_quex_lexer_PostCategorizer_construct(a2l_quex_lexer_Dictionary* me)
{
    me->root = 0x0;

    return true;
}

inline void
a2l_quex_lexer_PostCategorizer_destruct(a2l_quex_lexer_Dictionary* me)
{
    a2l_quex_lexer_PostCategorizer__clear(me);
}

inline int
a2l_quex_lexer_PostCategorizer__compare(a2l_quex_lexer_DictionaryNode* me, 
                                    a2l_quex_lexer_lexatom_t          FirstCharacter, 
                                    const a2l_quex_lexer_lexatom_t*   Remainder)
    /* Returns: '0'   if both strings are the same
       '< 0' string 0 < string 1
       '> 0' string 0 > string 1           */
{
    const a2l_quex_lexer_lexatom_t* it0 = 0x0;
    const a2l_quex_lexer_lexatom_t* it1 = 0x0;

    if     ( FirstCharacter > me->name_first_character ) return 1;
    else if( FirstCharacter < me->name_first_character ) return -1;
    else {
        /* Implementation according to: P.J. Plauger, "The Standard C Library", 1992 */
        it0 = Remainder;
        it1 = me->name_remainder;
        for(; *it0 == *it1; ++it0, ++it1) {
            /* Both letters are the same and == 0?
             * => both reach terminall zero without being different. */
            if( *it0 == 0 ) return 0;
        }
        return (int)(*it0) - (int)(*it1);
    }
}

inline void
a2l_quex_lexer_PostCategorizer__enter(a2l_quex_lexer_Dictionary* me,
                                  const a2l_quex_lexer_lexatom_t*  EntryName, 
                                  const a2l_quex_lexer_token_id_t    TokenID)
{
    a2l_quex_lexer_lexatom_t           FirstCharacter = EntryName[0];
    const a2l_quex_lexer_lexatom_t*    Remainder = FirstCharacter == 0x0 ? 0x0 : EntryName + 1;
    a2l_quex_lexer_DictionaryNode*    node      = me->root;
    a2l_quex_lexer_DictionaryNode*    prev_node = 0x0;
    int                           result = 0;

    if( me->root == 0x0 ) {
        me->root = a2l_quex_lexer_DictionaryNode_new(FirstCharacter, Remainder, TokenID);
        return;
    }
    while( node != 0x0 ) {
        prev_node = node;
        result    = a2l_quex_lexer_PostCategorizer__compare(node, FirstCharacter, Remainder);
        if     ( result > 0 ) node = node->greater;
        else if( result < 0 ) node = node->lesser;
        else                  return; /* Node with that name already exists */
    }
    __quex_assert( prev_node != 0x0 );
    __quex_assert( result != 0 );

    if( result > 0 ) 
        prev_node->greater = a2l_quex_lexer_DictionaryNode_new(FirstCharacter, Remainder, TokenID);
    else 
        prev_node->lesser  = a2l_quex_lexer_DictionaryNode_new(FirstCharacter, Remainder, TokenID);
}

inline void
a2l_quex_lexer_PostCategorizer__remove(a2l_quex_lexer_Dictionary*  me,
                                   const a2l_quex_lexer_lexatom_t*   EntryName)
{
    int                         result = 0;
    a2l_quex_lexer_lexatom_t           FirstCharacter = EntryName[0];
    const a2l_quex_lexer_lexatom_t*    Remainder = FirstCharacter == 0x0 ? 0x0 : EntryName + 1;
    a2l_quex_lexer_DictionaryNode*  node   = 0x0;
    a2l_quex_lexer_DictionaryNode*  parent = 0x0;
    a2l_quex_lexer_DictionaryNode*  found  = me->root;

    __quex_assert( found != 0x0 );
    while( 1 + 1 == 2 ) {
        result = a2l_quex_lexer_PostCategorizer__compare(found, FirstCharacter, Remainder);

        /* result == 0: found's name == EntryName 
         * On 'break': If found == root then parent = 0x0 which triggers a special treatment. */
        if( result == 0 ) break;

        parent = found;

        if     ( result > 0 )  found = found->greater;
        else if( result < 0 ) found = found->lesser;

        if( found == 0x0 ) return; /* Not found name with that name */
    };
    /* Found a node with 'EntryName' */

    /* Remove node and re-order tree */
    if( ! parent ) {
        if( found->lesser ) {
            for(node = found->lesser; node->greater != 0x0; node = node->greater );
            node->greater = found->greater;
            me->root      = found->lesser;
        } else {
            me->root      = found->greater;
        }
    }
    else if( found == parent->lesser ) {
        /* (1) 'found' is the 'lesser' child of the parent:
         *
         *                 (parent's greater tree)
         *                /
         *        (parent)        (greater tree)
         *               \       /
         *                (found)
         *                       \
         *                        (lesser tree)
         *
         *     All subnodes of (greater tree) are greater than all subnodes in (lesser tree).
         *     => (i) mount (lesser tree) to the least node of (greater tree).                
         *     Anything in the subtree of 'found' is lesser than anything in 'parent's 
         *     greater tree.
         *     => (ii) mount (greater tree) to the least node of the (parent's greater tree). */
        /* parent != 0x0, see above */
        if( found->greater ) {
            for(node = found->greater; node->lesser != 0x0; node = node->lesser );
            node->lesser   = found->lesser;
            parent->lesser = found->greater;
        } else {
            parent->lesser = found->lesser;
        }

    } else {
        /* (2) 'found' is the 'greater' child of the parent:
         *
         *     (i)  mount (greater tree) to the greatest node of (greater tree).                  
         *     (ii) mount (lesser tree) to the greatest node of the (parent's lesser tree). */
        /* parent != 0x0, see above */
        if( found->lesser ) {
            for(node = found->lesser; node->greater != 0x0; node = node->greater );
            node->greater   = found->greater;
            parent->greater = found->lesser;
        } else {
            parent->greater = found->greater;
        }
    }
    a2l_quex_lexer_PostCategorizer__free_node(found);
}

inline a2l_quex_lexer_DictionaryNode*
a2l_quex_lexer_PostCategorizer__find(const a2l_quex_lexer_Dictionary*  me, 
                                 const a2l_quex_lexer_lexatom_t*      EntryName)
{
    a2l_quex_lexer_lexatom_t           FirstCharacter = EntryName[0];
    const a2l_quex_lexer_lexatom_t*    Remainder      = FirstCharacter == 0x0 ? 0x0 : EntryName + 1;
    a2l_quex_lexer_DictionaryNode*  node           = me->root;

    while( node != 0x0 ) {
        int result = a2l_quex_lexer_PostCategorizer__compare(node, FirstCharacter, Remainder);

        if     ( result > 0 ) node = node->greater;
        else if( result < 0 ) node = node->lesser;
        else                  return node;
    }
    return 0x0;
}

inline void
a2l_quex_lexer_PostCategorizer__clear_recursively(a2l_quex_lexer_Dictionary*      me, 
                                             a2l_quex_lexer_DictionaryNode*  branch)
{
    __quex_assert(branch);

    if( branch->lesser )  a2l_quex_lexer_PostCategorizer__clear_recursively(me, branch->lesser);
    if( branch->greater ) a2l_quex_lexer_PostCategorizer__clear_recursively(me, branch->greater);
    a2l_quex_lexer_PostCategorizer__free_node(branch);
}

inline a2l_quex_lexer_token_id_t 
a2l_quex_lexer_PostCategorizer__get_token_id(const a2l_quex_lexer_Dictionary*  me,
                                         const a2l_quex_lexer_lexatom_t*      Lexeme)
{
    a2l_quex_lexer_DictionaryNode* found = a2l_quex_lexer_PostCategorizer__find(me, Lexeme);
    if( ! found ) return  QUEX_TKN_TERMINATION;
    return found->token_id;
}

inline void
a2l_quex_lexer_PostCategorizer__clear(a2l_quex_lexer_Dictionary* me)
{
    if( ! me->root ) return;
    a2l_quex_lexer_PostCategorizer__clear_recursively(me, me->root);
    me->root = (a2l_quex_lexer_DictionaryNode*)0;
}

inline void
a2l_quex_lexer_PostCategorizer_resources_absent_mark(a2l_quex_lexer_Dictionary* me)
{ 
    me->root = (a2l_quex_lexer_DictionaryNode*)0;
}

inline bool
a2l_quex_lexer_PostCategorizer_resources_absent(a2l_quex_lexer_Dictionary* me)
{ 
    return me->root == (a2l_quex_lexer_DictionaryNode*)0;
}

inline void
a2l_quex_lexer_PostCategorizer__print_tree(a2l_quex_lexer_DictionaryNode* node, int Depth)
{
    int i = 0;
    if( ! node ) {
        for(i=0; i<Depth; ++i) QUEX_DEBUG_PRINT("        ");
        QUEX_DEBUG_PRINT("[EMPTY]\n");
        return;
    }

    a2l_quex_lexer_PostCategorizer__print_tree(node->greater, Depth + 1);

    for(i=0; i < Depth + 1; ++i) QUEX_DEBUG_PRINT("        ");
    QUEX_DEBUG_PRINT("/\n");

    for(i=0; i<Depth; ++i) QUEX_DEBUG_PRINT("        ");
    {
        uint8_t                  drain[256];
        uint8_t*                 drain_p     = &drain[0];
        uint8_t*                 remainder_p = (uint8_t*)0; 
        const a2l_quex_lexer_lexatom_t* source_p     = (a2l_quex_lexer_lexatom_t*)&node->name_first_character;
        const a2l_quex_lexer_lexatom_t* source_end_p = &source_p[1];

        /* Convert the first character                                       */
        a2l_quex_lexer_lexeme_nnzt_to_utf8(&source_p, source_end_p, &drain_p, &drain[256]);

        *drain_p++   = '\0';
        remainder_p  = drain_p;
        source_p     = (a2l_quex_lexer_lexatom_t*)node->name_remainder;
        source_end_p = &source_p[a2l_quex_lexer_lexeme_length((a2l_quex_lexer_lexatom_t*)source_p) + 1];

        /* Convert the remainder                                             */
        a2l_quex_lexer_lexeme_nnzt_to_utf8(&source_p, source_end_p, &drain_p, &drain[256]);

        QUEX_DEBUG_PRINT3("[%s]%s: %i\n", &drain[0], remainder_p, 
                          (int)node->token_id);
    }

    for(i=0; i<Depth + 1; ++i) QUEX_DEBUG_PRINT("        ");
    QUEX_DEBUG_PRINT("\\\n");

    a2l_quex_lexer_PostCategorizer__print_tree(node->lesser, Depth + 1);
}

inline void
a2l_quex_lexer_PostCategorizer_print_this(a2l_quex_lexer_Dictionary* me)
{
    a2l_quex_lexer_PostCategorizer__print_tree(me->root, 0);
}



inline 
a2l_quex_lexer_Dictionary::a2l_quex_lexer_Dictionary()
{ /* C/C++ Compability: Constructors/Destructors do nothing. */ }

inline
a2l_quex_lexer_Dictionary::~a2l_quex_lexer_Dictionary()
{ /* C/C++ Compability: Constructors/Destructors do nothing. */ }

inline void
a2l_quex_lexer_Dictionary::clear()
{ a2l_quex_lexer_PostCategorizer__clear(this); }

inline a2l_quex_lexer_token_id_t 
a2l_quex_lexer_Dictionary::get_token_id(const a2l_quex_lexer_lexatom_t* Lexeme) const
{ return a2l_quex_lexer_PostCategorizer__get_token_id(this, Lexeme); }

inline void
a2l_quex_lexer_Dictionary::remove(const a2l_quex_lexer_lexatom_t* EntryName)
{ a2l_quex_lexer_PostCategorizer__remove(this, EntryName); }

inline void
a2l_quex_lexer_Dictionary::enter(const a2l_quex_lexer_lexatom_t*  EntryName, 
                             const a2l_quex_lexer_token_id_t    TokenID)
{ a2l_quex_lexer_PostCategorizer__enter(this, EntryName, TokenID); }

inline void
a2l_quex_lexer_Dictionary::print_this()
{ a2l_quex_lexer_PostCategorizer_print_this(this); }





#include "a2l_quex_lexer\lib\lexeme/basics.i"

#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__EXTRA__POST_CATEGORIZER__POST_CATEGORIZER_I */
