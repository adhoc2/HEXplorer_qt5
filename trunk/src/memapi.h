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

#if !defined(_ESA_MEMAPI_H)
#define	_ESA_MEMAPI_H



#if defined(WIN32)
#define	MSDECLSPECIMPORT		__declspec(dllexport)
#define	MSDECLSPECEXPORT		__declspec(dllexport)
#define	MSCDECL				__cdecl
#else
#define	MSDECLSPECIMPORT
#define	MSDECLSPECEXPORT
#define	MSCDECL
#endif

#if defined(UNIX)
#define	MALLOC		malloc
#define	CALLOC		calloc
#define	REALLOC		realloc
#define	FREE		free
#else
#define	MALLOC		ESAmalloc
#define	CALLOC		ESAcalloc
#define	REALLOC		ESArealloc
#define	FREE		ESAfree
#endif


/*
** Bits for ESAinitialize() to return to the user
*/

#define	ESA_LIBRARY_STDC		0x1
#define	ESA_LIBRARY_MSVCRT		0x2
#define	ESA_LIBRARY_MSVCR70		0x4
#define	ESA_LIBRARY_MSVCR71		0x8
#define	ESA_LIBRARY_MSVCR80		0x10
#define	ESA_LIBRARY_MSVCR90		0x20
#define	ESA_FUNCTION_MALLOC		0x40
#define	ESA_FUNCTION_CALLOC		0x80
#define	ESA_FUNCTION_REALLOC		0x100
#define	ESA_FUNCTION_FREE		0x200
#define	ESA_FUNCTION_MSIZE		0x400
#define	ESA_FUNCTION_EXPAND		0x800
#define	ESA_FUNCTION_NEW		0x1000
#define	ESA_FUNCTION_DELETE		0x2000


/*
** ESA Trace Counters
*/

#define	ESA_TRACE_COUNTERS		68



/*
** The ESA memory statistics API
*/

typedef struct	_esamemusage
{
	int	npools;		/* number of pools */
	int	nslabs;		/* number of slabs */
	int	nxblocks;	/* NOT USED */
	long	xreserved;	/* NOT USED */
	long	reserved;	/* reserved bytes */
	long	allocated;	/* allocated bytes */
} esamemusage_t;

typedef struct _evcounters
{
	unsigned long long	counter[ESA_TRACE_COUNTERS];
} ESAEventCounters;



#if !defined(ESA_KERNEL)

#if defined(__cplusplus)
extern "C"
{
#endif

MSDECLSPECIMPORT int MSCDECL
ESAGetThreadMemUsage(esamemusage_t *pmu);

#if defined(__cplusplus)
}
#endif


#if defined(__cplusplus)
extern "C"
{
#endif

MSDECLSPECIMPORT int MSCDECL
ESAGetMemUsage(esamemusage_t *tmu);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

MSDECLSPECIMPORT int MSCDECL
ESALogging(unsigned int, unsigned int);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

MSDECLSPECIMPORT int MSCDECL
ESAFlushLog();

#if defined(__cplusplus)
}
#endif


#if defined(EVEREST_DEBUG)
/*
** The Event Counter API
*/

#if defined(__cplusplus)
extern "C"
{
#endif

MSDECLSPECIMPORT int MSCDECL
ESAGetTraceCounters(ESAEventCounters*);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

MSDECLSPECIMPORT int MSCDECL
ESAClearTraceCounters();

#if defined(__cplusplus)
}
#endif
#endif

#endif
#endif
