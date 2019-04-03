/* -*- C++ -*- vim: set syntax=cpp: 
 *
 * PURPOSE:
 *   
 *   Quex allows to connect to different unicode converters. Not all of those converters
 *   are necessarily installed on every system. However, each converter library provides
 *   header files which need to be included to use the library. There must be a mechanism
 *   to prevent the inclusion of converter headers that the user does not provide.
 *   
 *   (C) 2009-2018 Frank-Rene Schaefer
 *
 *   ABSOLUTELY NO WARRANTY                                                                 */
#ifndef  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__CONVERTER__ICU__SPECIAL_HEADERS_H
#define  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__CONVERTER__ICU__SPECIAL_HEADERS_H
   
#include "a2l_quex_lexer\lib\buffer/lexatoms/converter/Converter"


extern "C" {
#include <stdio.h>
#include <assert.h>
#include <string.h>
}



#include "unicode/utypes.h"   /* Basic ICU data types */
#include "unicode/ucnv.h"     /* C   Converter API    */
#include "unicode/ustring.h"  /* some more string fcns*/
#include "unicode/uchar.h"    /* char names           */
#include "unicode/uloc.h"
#include "unicode/uclean.h"

#endif /*  QUEX_INCLUDE_GUARD_a2l_quex_lexer__BUFFER__LEXATOMS__CONVERTER__ICU__SPECIAL_HEADERS_H */
