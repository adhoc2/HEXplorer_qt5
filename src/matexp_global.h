#ifndef MATEXP_GLOBAL_H
#define MATEXP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MATEXP_LIBRARY)
#  define MATEXPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MATEXPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MATEXP_GLOBAL_H
