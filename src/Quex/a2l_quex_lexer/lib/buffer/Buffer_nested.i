/* vim:set ft=c: -*- C++ -*- */
#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_NESTED_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_NESTED_I

#include "a2l_quex_lexer\lib\quex/asserts"
#include "a2l_quex_lexer\lib\buffer/asserts"
#include "a2l_quex_lexer\lib\definitions"
#include "a2l_quex_lexer\lib\buffer/Buffer"
#include "a2l_quex_lexer\lib\buffer/Buffer_print.i"
#include "a2l_quex_lexer\lib\buffer/lexatoms/LexatomLoader"
#include "a2l_quex_lexer\lib\quex/MemoryManager"



inline void
a2l_quex_lexer_Buffer_adapt_to_new_memory_location_root(a2l_quex_lexer_Buffer* me,
                                                    a2l_quex_lexer_lexatom_t* old_memory_root,
                                                    a2l_quex_lexer_lexatom_t* new_memory_root,
                                                    ptrdiff_t          NewRootSize);

inline bool
a2l_quex_lexer_Buffer_nested_construct(a2l_quex_lexer_Buffer*        me,
                                   a2l_quex_lexer_Buffer*        nesting,
                                   a2l_quex_lexer_LexatomLoader* filler)
/* Construct 'me' buffer (-> memory split):
 *
 * Constructor takes over ownership over 'filler'. If construction fails,
 * the 'filler' is immediatedly deleted.
 *
 * To optimize memory usage and minimize the generation of new buffers in 
 * situations of extensive file inclusions, the current buffer's memory may
 * be split to generate the nested buffer's memory.
 *
 *                   nesting  .---------------------.
 *                   buffer   |0|a|b|c|d|0| | | | | |
 *                            '---------------------'
 *                   read_p -------'     |
 *                   end_p  -------------'
 *
 *                              /    split      \
 *                             /                 \
 *                                  
 *           nesting .-----------.     nested .---------.
 *            buffer |0|a|b|c|d|0|  +  buffer | | | | | |
 *                   '-----------'            '---------'
 *          read_p -------'     |
 *          end_p  -------------'
 *
 * NOTE: Loaded content is NEVER overwritten or split. This is a precaution
 *       for situations where byte loaders may not be able to reload content
 *       that has already been loaded (for example 'TCP socket' byte loaders).
 *
 * RETURNS: true,  if memory has been allocated and the 'nested' buffer is
 *                 ready to rumble.
 *          false, if memory allocation failed. 'nested' buffer is not 
 *                 functional.
 *                                                                            */
{
    /*         front           read_p      end_p                 back
     *           |               |           |                   |
     *          .-------------------------------------------------.
     *          |0|-|-|-|-|-|-|-|a|b|c|d|e|f|0| | | | | | | | | | |
     *          '-------------------------------------------------'
     *                                         :                 :
     *                                         '--- available ---'
     *                                                                        */
    ptrdiff_t           available_size =   nesting->content_space_end(nesting)
                                         - nesting->content_end(nesting);
    a2l_quex_lexer_lexatom_t*  memory;
    size_t              memory_size;
    E_Ownership         ownership;
    a2l_quex_lexer_Buffer*  nesting_buffer_p = (a2l_quex_lexer_Buffer*)0;


    a2l_quex_lexer_Buffer_assert_consistency(nesting);

    if( a2l_quex_lexer_Buffer_resources_absent(nesting) ) {
        goto ERROR_0;
    }
    else if( available_size < (ptrdiff_t)(QUEX_a2l_quex_lexer_SETTING_BUFFER_SIZE_MIN) ) {
        /* (1) AVAILABLE SIZE too SMALL
         *     => Try to move content, so that free space becomes available.  */                    
        available_size = a2l_quex_lexer_Buffer_nested_free_front(nesting);
    }

    if( available_size < (ptrdiff_t)QUEX_a2l_quex_lexer_SETTING_BUFFER_SIZE_MIN ) {
        /* (2) AVAILABLE SIZE still too SMALL
         *     => Allocate new memory for new buffer.                         */                    
        memory_size = (size_t)(QUEX_a2l_quex_lexer_SETTING_BUFFER_SIZE);
        memory      = (a2l_quex_lexer_lexatom_t*)quex::MemoryManager_allocate(
                                memory_size * sizeof(a2l_quex_lexer_lexatom_t), 
                                E_MemoryObjectType_BUFFER_MEMORY);
        if( ! memory ) {
            goto ERROR_0;
        }
        ownership        = E_Ownership_LEXICAL_ANALYZER;
        nesting_buffer_p = (a2l_quex_lexer_Buffer*)0;
    }
    else {
        /* (2) AVAILABLE SIZE in nesting buffer sufficient
         *     => Use free space for new buffer.                              */                    
        memory      = &nesting->content_end(nesting)[1];
        memory_size = (size_t)available_size;
        __quex_assert(0           != memory);
        __quex_assert(memory_size == (size_t)(nesting->end(nesting) - memory));

        nesting->_memory._back = nesting->content_end(nesting);
        ownership              = E_Ownership_INCLUDING_BUFFER;
        nesting_buffer_p       = nesting;
    }

    a2l_quex_lexer_Buffer_construct(me, filler, memory, memory_size, 
                                (a2l_quex_lexer_lexatom_t*)0, me->_fallback_n,
                                ownership, nesting_buffer_p);
    
    me->event = nesting->event;               /* Plain copy suffices. */

    a2l_quex_lexer_Buffer_assert_consistency(me);
    a2l_quex_lexer_Buffer_assert_consistency(nesting);
    return true;

ERROR_0:
    a2l_quex_lexer_Buffer_resources_absent_mark(me);
    return false;
}
    
inline bool
a2l_quex_lexer_Buffer_nested_negotiate_extend(a2l_quex_lexer_Buffer*  me, 
                                          float               Factor)
/* Attempt to resize the current buffer to a size 's = Factor * current size'.
 * If that fails, try to access memory that of a sizes in between the 's' and 
 * the current sizes, i.e. 's = (s + current_size) / 2'. This is repeated until
 * either memory can be allocated or 's == current_size'. The latter indicates
 * failure. 
 *
 * RETURNS: true, in case if a chunk of size 's' with 
 *                'current_size < s <= Factor*current sizes'
 *                could be found.
 *          false, else.                                                      */
{
    a2l_quex_lexer_Buffer*  root         = a2l_quex_lexer_Buffer_nested_find_root(me);
    ptrdiff_t           current_size = me->end(me) - root->begin(root);
    /* Refuse negotiations where the requested amount of memory is greater
     * than the total addressable space divided by 16.
     * Addressable space = PTRDIFF_MAX * 2 => Max. size = PTRDIFF_MAX / 8     */
    const ptrdiff_t     MaxSize      = PTRDIFF_MAX >> 3;
    const ptrdiff_t     MinSize      = 4;
    ptrdiff_t           new_size     = (ptrdiff_t)((float)(QUEX_MAX(MinSize, QUEX_MIN(MaxSize, current_size))) * Factor);

    while( ! a2l_quex_lexer_Buffer_nested_extend(me, new_size - current_size) ) {
        new_size = (current_size + new_size) >> 1;
        if( new_size <= current_size ) {
            return false;
        }
    }
    return true;
}

inline bool
a2l_quex_lexer_Buffer_nested_extend(a2l_quex_lexer_Buffer*  me, ptrdiff_t  SizeAdd)
/* Allocates a chunk of memory that is 'SizeAdd' greater than the current size.
 * If 'SizeAdd' is negative a smaller chunk is allocated. However, if the 
 * resulting size is insufficient to hold the buffer's content, the function
 * refuses to operate. 
 *
 * The new chunk is allocated with 'E_Ownership_LEXICAL_ANALYZER', such that 
 * the memory is de-allocated upon destruction.
 *
 * RETURNS: true, in case of success.
 *          false, else.                                                      */
{
    a2l_quex_lexer_lexatom_t*  new_memory_root_p;
    ptrdiff_t           required_size;
    ptrdiff_t           new_size;
    a2l_quex_lexer_Buffer*  root = me;
    a2l_quex_lexer_Buffer*  focus = me;
    bool                verdict_f = false;
    
    a2l_quex_lexer_Buffer_assert_consistency(me);

    /* The 'Buffer_extend()' function cannot be called for an buffer which is
     * currently including, i.e. has dependent buffers! It can only be called
     * for the currently working buffer.                                      */
    root              = a2l_quex_lexer_Buffer_nested_find_root(me);
    a2l_quex_lexer_Buffer_assert_consistency(root);

    /* required: content + 2 lexatoms for border.                             */
    required_size     = me->content_end(me) - root->begin(root) + 1;
    new_size          = me->end(me) - root->begin(root) + SizeAdd;

    if( SizeAdd <= 0 || required_size >= new_size ) {
        return false;
    }

    /* Buffers must be informed about change early! Reallocation is not 
     * predictable. When memory extension fails and new memory is copied, then
     * nothing can be guaranteed about old content.                           */
    for(focus = me; focus ; focus = focus->_memory.including_buffer) {
        a2l_quex_lexer_Buffer_callbacks_on_buffer_before_change(focus);
    }

    new_memory_root_p = (a2l_quex_lexer_lexatom_t*)quex::MemoryManager_reallocate(
                                                (void*)root->begin(root),
                                                sizeof(a2l_quex_lexer_lexatom_t) * (size_t)new_size,
                                                E_MemoryObjectType_BUFFER_MEMORY);

    if( ! new_memory_root_p ) {
        /* Old memory object IS NOT DE-ALLOCATED.                             */
        verdict_f = false;
    }
    else if( new_memory_root_p == root->begin(root) ) {
        /* Old memory object IS NOT REPLACED--CONTENT AT SAME ADDRESS.        */
        me->_memory._back    = &new_memory_root_p[new_size-1];
        me->_memory._back[0] = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER;
        verdict_f = true;
    }
    else {
        a2l_quex_lexer_Buffer_adapt_to_new_memory_location_root(me, 
                                                            root->begin(root),
                                                            new_memory_root_p, 
                                                            new_size);

        root->_memory.ownership = E_Ownership_LEXICAL_ANALYZER;
        verdict_f = true;
    }
    a2l_quex_lexer_Buffer_assert_consistency(me);
    return verdict_f;
}

inline bool
a2l_quex_lexer_Buffer_nested_migrate(a2l_quex_lexer_Buffer*  me,
                                 a2l_quex_lexer_lexatom_t*  memory,
                                 ptrdiff_t           MemoryLexatomN,
                                 E_Ownership         Ownership) 
/* Migrate the content of the current buffer to a new memory space. In case
 * that the buffer is nested in an including buffer, the root of all included
 * buffers is moved. 
 *
 * The new memory *might* be smaller than the current memory, as long as the
 * content itself is less or equal the size of the newly provided memory.
 *
 * If this function fails, the caller is responsible for the de-allocation of
 * the memory.
 *
 * RETURNS: true, if migration was successful.
 *          false, if newly allocated memory is too small.                    */
{
    a2l_quex_lexer_Buffer* root;
    a2l_quex_lexer_Buffer* focus;
    a2l_quex_lexer_lexatom_t* old_memory_root_p;
    a2l_quex_lexer_lexatom_t* old_content_end_p = me->content_end(me);
    ptrdiff_t          required_size;
    bool               verdict_f = false;

    a2l_quex_lexer_Buffer_assert_consistency(me);

    __quex_assert(old_content_end_p >= me->begin(me));
    __quex_assert(old_content_end_p <= me->content_space_end(me));

    root              = a2l_quex_lexer_Buffer_nested_find_root(me);
    a2l_quex_lexer_Buffer_assert_consistency(root);

    old_memory_root_p = root->begin(root);
    /* required: content + 2 lexatoms for border.                             */
    required_size     = old_content_end_p - &old_memory_root_p[1] + 2;

    if( required_size > MemoryLexatomN ) {
        verdict_f = false;
    }
    else {
        /* Copy content to the new habitat.                                   */
        __QUEX_STD_memcpy((void*)&memory[0], 
                          (void*)&old_memory_root_p[0],
                          (size_t)required_size * sizeof(a2l_quex_lexer_lexatom_t));

        for(focus = me; focus ; focus = focus->_memory.including_buffer) {
            a2l_quex_lexer_Buffer_callbacks_on_buffer_before_change(focus);
        }

        /* Adapt this and all nesting buffers to new memory location.         */
        a2l_quex_lexer_Buffer_adapt_to_new_memory_location_root(me, old_memory_root_p,
                                                            &memory[0], MemoryLexatomN);

        if( root->_memory.ownership == E_Ownership_LEXICAL_ANALYZER ) {
            quex::MemoryManager_free(old_memory_root_p, E_MemoryObjectType_BUFFER_MEMORY);
        }
        root->_memory.ownership = Ownership;
        /* Limit codes for '_front' and '_end' have been set during 'memcpy'. */
        me->_memory._back[0]  = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER;
        verdict_f = true;
    }

    a2l_quex_lexer_Buffer_assert_consistency(me);
    return verdict_f;
}

inline a2l_quex_lexer_Buffer*
a2l_quex_lexer_Buffer_nested_find_root(a2l_quex_lexer_Buffer* me)
/* A buffer may be nested in an including buffer. This function walks down
 * the path of nesting until the root of all including buffers is found.
 *
 * RETURNS: Pointer to root buffer object.                                */
{
    a2l_quex_lexer_Buffer* focus = me;
    for(; focus->_memory.including_buffer; 
        focus = focus->_memory.including_buffer) {
        __quex_assert(focus->_memory.ownership == E_Ownership_INCLUDING_BUFFER);
    }
    __quex_assert(focus->_memory.ownership != E_Ownership_INCLUDING_BUFFER);
    return focus;
}

inline ptrdiff_t
a2l_quex_lexer_Buffer_nested_free_front(a2l_quex_lexer_Buffer* me)
/* Shrink all nesting buffers to a minimum and reset all pointers accordingly.
 *
 * RETURNS: Free space at the end of 'me'.                                    */
{
    a2l_quex_lexer_Buffer* it;
    a2l_quex_lexer_Buffer* previous = (a2l_quex_lexer_Buffer*)0;
    ptrdiff_t          move_distance = 0;
    ptrdiff_t          nesting_freed = 0;

    /* Iterate over all nesting buffers starting from root up to 'me'.
     * Shrink each buffer, adapt its 'back pointer' and the 'front pointer'
     * of the included buffer.                                                */
    for(it = a2l_quex_lexer_Buffer_nested_find_root(me); true ; 
        it = a2l_quex_lexer_Buffer_nested_get(it, me)) {
        move_distance = a2l_quex_lexer_Buffer_move_get_max_distance_towards_begin(it);

        if( previous ) {
            previous->_memory._back   -= nesting_freed;
            it->_memory._front        -= nesting_freed;
            previous->_memory._back[0] = (a2l_quex_lexer_lexatom_t)(QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER);
            it->_memory._front[0]      = (a2l_quex_lexer_lexatom_t)(QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER);
        }

        a2l_quex_lexer_Buffer_move_towards_begin(it, move_distance + nesting_freed,
                                             (a2l_quex_lexer_lexatom_t**)0, 0);
        /* Asserts in 'Buffer_move_towards_begin()' => FallbackN maintained!  */

        /* Adapt pointers added 'move_distance + nesting_freed' to lexatom
         * index. Must subtract 'nesting_freed'.                              */
        __quex_assert(it->input.lexatom_index_begin >= nesting_freed);
        it->input.lexatom_index_begin -= nesting_freed;

        if( it == me ) {
            break;
        }

        nesting_freed = it->content_space_end(it) - it->content_end(it);
        previous      = it;
    } 

    a2l_quex_lexer_Buffer_assert_consistency(me);
    return me->content_space_end(me) - me->content_end(me);
}
    
inline void
a2l_quex_lexer_Buffer_adapt_to_new_memory_location_root(a2l_quex_lexer_Buffer* me,
                                                    a2l_quex_lexer_lexatom_t* old_memory_root,
                                                    a2l_quex_lexer_lexatom_t* new_memory_root,
                                                    ptrdiff_t          NewRootSize)
/* Caller *must* ensure that 'NewRootSize' can hold all the content.          */
{
    a2l_quex_lexer_Buffer* focus = (a2l_quex_lexer_Buffer*)0;
    a2l_quex_lexer_lexatom_t* new_memory;
    ptrdiff_t          buffer_size;
    /* NOT: a2l_quex_lexer_Buffer_assert_consistency(me)
     *      Because, memory and its content may have been moved!              */
    
    /* Migration impossible, if the memory is not large enough for content.   */
    __quex_assert(me->content_end(me) - old_memory_root <= NewRootSize);

    /* Adapt this and all nesting buffers to new memory location.             */
    for(focus = me; focus ; focus = focus->_memory.including_buffer) {

        new_memory  = &new_memory_root[focus->begin(focus) - old_memory_root];
        buffer_size = focus->size(focus);
        a2l_quex_lexer_Buffer_adapt_to_new_memory_location(focus, new_memory, buffer_size);
    }
    __quex_assert(me->content_end(me) <= &new_memory_root[NewRootSize-1]);
    me->_memory._back    = &new_memory_root[NewRootSize - 1];
    me->_memory._back[0] = QUEX_a2l_quex_lexer_SETTING_BUFFER_LEXATOM_BUFFER_BORDER;
    a2l_quex_lexer_Buffer_assert_consistency(me);
}

inline bool
a2l_quex_lexer_Buffer_adapt_to_new_memory_location(a2l_quex_lexer_Buffer* me,
                                               a2l_quex_lexer_lexatom_t* new_memory_base,
                                               ptrdiff_t          NewSize)
/* Adapt all content to a new memory base and ownership. 
 *
 * -- This function is not concerned with memory management, etc. everything is
 *    supposed to be setup/destructed previously.
 *
 * -- This function does not consider the buffer nesting. It solely treats 
 *    the memory of 'me' itself.
 *
 * -- It is assumed, that new memory has the SAME size as the current.
 *
 * RETURN: true, if success.
 *         false, if new memory is too small to hold content.
 *                                                                            */
{
    ptrdiff_t  offset_end_p          = me->content_end(me) - me->begin(me);
    ptrdiff_t  offset_read_p         = me->_read_p         - me->begin(me);
    ptrdiff_t  offset_lexeme_start_p = me->_lexeme_start_p - me->begin(me);

    __quex_assert(   (0                            != me->_memory.including_buffer) 
                  == (E_Ownership_INCLUDING_BUFFER == me->_memory.ownership));
    __quex_assert(offset_end_p          < NewSize);
    __quex_assert(offset_read_p         < NewSize);
    __quex_assert(offset_lexeme_start_p < NewSize);
    /* Required buffer size: content + 2 lexatoms for borders.                */
    __quex_assert(me->size(me) <= NewSize);

    a2l_quex_lexer_BufferMemory_construct(&me->_memory, new_memory_base, (size_t)NewSize,
                                      me->_memory.ownership, 
                                      me->_memory.including_buffer); 

    a2l_quex_lexer_Buffer_init_content_core(me, 
                                        me->input.lexatom_index_begin,
                                        me->input.lexatom_index_end_of_stream,
                                        &new_memory_base[offset_end_p]);
    a2l_quex_lexer_Buffer_init_analyzis_core(me, 
    /* ReadP                          */ &new_memory_base[offset_read_p],
    /* LexatomStartP                  */ &new_memory_base[offset_lexeme_start_p],
    /* LexatomAtLexemeStart           */ me->_lexatom_at_lexeme_start,
    /* LexatomBeforeLexemeStart       */ 
    /*                                */  (a2l_quex_lexer_lexatom_t)0, /* ignored */
    /* BackupLexatomIndexOfReadP      */ me->_backup_lexatom_index_of_lexeme_start_p);

    a2l_quex_lexer_Buffer_assert_consistency(me);
    return true;
}

inline a2l_quex_lexer_Buffer* 
a2l_quex_lexer_Buffer_nested_get(a2l_quex_lexer_Buffer* me, 
                             a2l_quex_lexer_Buffer* tail)
/* RETURNS: Buffer which is included by 'me'.
 *          'tail' in case 'me' does not inculude a buffer. 
 *
 * The 'tail' must be provided because only the including buffer is 
 * provided. Finding the nested buffer means going backward until the 
 * buffer's included buffer is equal to 'me'                              */
{
    a2l_quex_lexer_Buffer* focus;

    for(focus = tail; focus->_memory.including_buffer != me; 
        focus = focus->_memory.including_buffer) {
        if( ! focus->_memory.including_buffer ) {
            return tail;
        }
    }
    
    /* HERE: 'focus' is directly included by 'me', or 'focus' == tail, 
     *       if it fails.                                                 */
    return focus;
}



#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__BUFFER_NESTED_I */
