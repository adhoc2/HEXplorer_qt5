/****************************************************************************/
/* Copyright (c) 2002-2009, Cherrystone Software Labs. All rights reserved. */
/*                                                                          */
/*                                                                          */
/* This software is furnished under a license and may be used and copied    */
/* only in accordance with the terms of such license and with the           */
/* inclusion of the above copyright notice. This software or any other      */
/* copies thereof may not be provided or otherwise made available to any    */
/* other person. No title to and ownership of the  software is hereby       */
/* transferred.                                                             */
/*                                                                          */
/* The information in this software is subject to change without notice     */
/* and should not be construed as a commitment by Cherrystone Software Labs */
/*                                                                          */
/* Cherrystone Software Labs assumes no responsibility for the use or       */
/* reliability of this software.                                            */
/*                                                                          */
/* CHERRYSTONE SOFTWARE LABS MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT   */
/* THE SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING    */
/* BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,            */
/* FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.                   */
/* CHERRYSTONE SOFTWARE LABS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED   */
/* BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS         */
/* SOFTWARE OR ITS DERIVATIVES.                                             */
/*                                                                          */
/****************************************************************************/


#if !defined(ESAAPI_H)
#define	ESAAPI_H

#include "memapi.h"

#if defined(EVEREST_DEBUG)
/*
** Bits for the ESA Logging
*/

#define	ESA_LOGGING_MEMORYAPI		(1 << 0)
#define	ESA_LOGGING_LOCKS		(1 << 2)
#endif


/*
** For Microsoft based COM application. Convert all occurrences of
** malloc/calloc, etc to the ESA names 
*/

#if !defined(ESA_KERNEL) && defined(ESACOM) && !defined(UNIX)
#define	malloc	ESAmalloc
#define	realloc	ESArealloc
#define	calloc	ESAcalloc
#define	free	ESAfree
#define	_msize	ESAmsize
#define	_expand ESAexpand
#endif



/*
** An initialization function to force the linking in of ESA
*/

#if defined(__cplusplus )
extern "C"
#endif
MSDECLSPECEXPORT unsigned long	MSCDECL ESAinitialize();



#if !defined(ESA_KERNEL) && defined(ESACOM)
/*
** Planning on using ESA in a Microsoft COM based application? Then you'll 
** need to compile this file with /DESACOM
*/

#if defined(__cplusplus )
#define	EXTERNAL	extern "C"
#else
#define	EXTERNAL	extern
#endif


#if defined(ESACOM)
EXTERNAL	void*	MSCDECL	ESAmalloc(size_t);
EXTERNAL	void*	MSCDECL	ESArealloc(void*, size_t);
EXTERNAL	void*	MSCDECL	ESAcalloc(size_t,size_t);
EXTERNAL	void	MSCDECL	ESAfree(void*);
EXTERNAL	void*	MSCDECL	ESAexpand(void*, size_t);
EXTERNAL	int	MSCDECL	ESAmsize(void*);
#endif


#endif
#endif
