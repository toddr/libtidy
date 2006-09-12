#ifndef __FORWARD_H__
#define __FORWARD_H__

/* forward.h -- Forward declarations for major Tidy structures

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2006/09/12 15:14:44 $ 
    $Revision: 1.4 $ 

  Avoids many include file circular dependencies.

  Try to keep this file down to the minimum to avoid
  cross-talk between modules.

  Header files include this file.  C files include tidy-int.h.

*/

#include "platform.h"
#include "tidy.h"

#define TYDYAPPEND(str1,str2) str1##str2
#define TY_(str) TYDYAPPEND(prvTidy,str)

struct _StreamIn;
typedef struct _StreamIn StreamIn;

struct _StreamOut;
typedef struct _StreamOut StreamOut;

struct _TidyDocImpl;
typedef struct _TidyDocImpl TidyDocImpl;


struct _Dict;
typedef struct _Dict Dict;

struct _Attribute;
typedef struct _Attribute Attribute;

struct _AttVal;
typedef struct _AttVal AttVal;

struct _Node;
typedef struct _Node Node;

struct _IStack;
typedef struct _IStack IStack;

struct _Lexer;
typedef struct _Lexer Lexer;



#endif /* __FORWARD_H__ */
