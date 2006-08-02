#ifndef __PARSER_H__
#define __PARSER_H__

/* parser.h -- HTML Parser

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2006/08/02 16:19:16 $ 
    $Revision: 1.11 $ 

*/

#include "forward.h"

Bool CheckNodeIntegrity(Node *node);

/*
 used to determine how attributes
 without values should be printed
 this was introduced to deal with
 user defined tags e.g. Cold Fusion
*/
Bool IsNewNode(Node *node);

void CoerceNode(TidyDocImpl* doc, Node *node, TidyTagId tid, Bool obsolete, Bool expected);

/* extract a node and its children from a markup tree */
Node *RemoveNode(Node *node);

/* remove node from markup tree and discard it */
Node *DiscardElement( TidyDocImpl* doc, Node *element);

/* insert node into markup tree as the firt element
 of content of element */
void InsertNodeAtStart(Node *element, Node *node);

/* insert node into markup tree as the last element
 of content of "element" */
void InsertNodeAtEnd(Node *element, Node *node);

/* insert node into markup tree before element */
void InsertNodeBeforeElement(Node *element, Node *node);

/* insert node into markup tree after element */
void InsertNodeAfterElement(Node *element, Node *node);

Node *TrimEmptyElement( TidyDocImpl* doc, Node *element );
Node* DropEmptyElements(TidyDocImpl* doc, Node* node);


/* assumes node is a text node */
Bool IsBlank(Lexer *lexer, Node *node);

Bool IsJavaScript(Node *node);

/*
  HTML is the top level element
*/
void ParseDocument( TidyDocImpl* doc );



/*
  XML documents
*/
Bool XMLPreserveWhiteSpace( TidyDocImpl* doc, Node *element );

void ParseXMLDocument( TidyDocImpl* doc );

#endif /* __PARSER_H__ */
