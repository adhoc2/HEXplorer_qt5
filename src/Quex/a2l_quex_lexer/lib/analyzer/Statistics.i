#ifndef QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__STATISTICS_I
#define QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__STATISTICS_I

#include "a2l_quex_lexer\lib\analyzer/Statistics"

inline void
a2l_quex_lexer_statistics_state_count(const a2l_quex_lexer_statistics_state* S, a2l_quex_lexer_lexatom_t C)
{
    const a2l_quex_lexer_lexatom_t*  BeginP   = S->interval_list.boundary;
    const a2l_quex_lexer_lexatom_t*  EndP     = BeginP + S->interval_list.boundary_n;
    const a2l_quex_lexer_lexatom_t*  low      = BeginP;
    const a2l_quex_lexer_lexatom_t*  up       = EndP;
    const a2l_quex_lexer_lexatom_t*  iterator = (const a2l_quex_lexer_lexatom_t*)0x0; 

    /* Binary Search for the interval where 'C' belongs:
     * Find iterator so that: *(iterator-1) <= C < *(iterator) */
    while( up != low ) {
        iterator = low + ((up - low) >> 1);
        if( C < *iterator ) {
            if( iterator == BeginP )        break;
            else if( *(iterator - 1) <= C ) break; 
            up  = iterator;
        }
        else if( C >= *iterator ) { 
            if( iterator == EndP - 1 )      { iterator = EndP; break; }
            low = iterator;
        }
    }
    S->interval_list.counter[iterator - BeginP] += 1;
}

inline void
a2l_quex_lexer_statistics_save(const char* Filename)
{
    const a2l_quex_lexer_statistics_state*  BeginP = (const a2l_quex_lexer_statistics_state*)a2l_quex_lexer_statistics_state_list;
    const a2l_quex_lexer_statistics_state*  EndP   = a2l_quex_lexer_statistics_state_list_end;
    const a2l_quex_lexer_statistics_state*  s      = (const a2l_quex_lexer_statistics_state*)0x0;
    FILE*                               fh     = __QUEX_STD_fopen(Filename, "w");
    size_t                              i      = 0;

    if( fh == NULL ) return;

    for(s = BeginP; s != EndP; ++s) {
        __QUEX_STD_fprintf(fh, "{\nmode: %s;\nstate: %i; {\n", (const char*)s->mode_name, (int)s->state_index);
        for(i = 0; i != s->interval_list.boundary_n; ++i) {
            __QUEX_STD_fprintf(fh, "%i ", (int)s->interval_list.boundary[i]);
        }
        __QUEX_STD_fprintf(fh, ";\n");
        for(i = 0; i != s->interval_list.boundary_n + 1; ++i) {
            __QUEX_STD_fprintf(fh, "%i ", (int)s->interval_list.counter[i]);
        }
        __QUEX_STD_fprintf(fh, ";\n}\n");
    }

    __QUEX_STD_fclose(fh);
}

#endif /* QUEX_INCLUDE_GUARD_a2l_quex_lexer__ANALYZER__STATISTICS_I */


