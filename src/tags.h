#ifndef __TAGS_H__
#define __TAGS_H__

/* tags.h -- recognize HTML tags

  (c) 1998-2003 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: hoehrmann $ 
    $Date: 2003/03/30 23:57:25 $ 
    $Revision: 1.4 $ 

  The HTML tags are stored as 8 bit ASCII strings.
  Use lookupw() to find a tag given a wide char string.

*/

#include "forward.h"

#define TAG_HASHSIZE 357

typedef void (Parser)( TidyDocImpl* doc, Node *node, uint mode );
typedef void (CheckAttribs)( TidyDocImpl* doc, Node *node );

/*
 Tag dictionary node
*/

/* types of tags that the user can define */
#define tagtype_empty     1
#define tagtype_inline    2
#define tagtype_block     4
#define tagtype_pre       8

struct _Dict
{
    TidyTagId       id;
    tmbstr          name;
    uint            versions;
    uint            model;
    Parser*         parser;
    CheckAttribs*   chkattrs;
    Dict*           next;
};

struct _TidyTagImpl
{
    Dict* xml_tags;                /* placeholder for all xml tags */
    Dict* declared_tag_list;       /* User declared tags */
};

typedef struct _TidyTagImpl TidyTagImpl;

/* choose what version to use for new doctype */
int HTMLVersion( TidyDocImpl* doc );


/* interface for finding tag by name */
const Dict* LookupTagDef( TidyTagId tid );
Bool    FindTag( TidyDocImpl* doc, Node *node );
Parser* FindParser( TidyDocImpl* doc, Node *node );
void    DefineTag( TidyDocImpl* doc, int tagType, ctmbstr name );
void    FreeDeclaredTags( TidyDocImpl* doc, int tagType ); /* 0 to free all */

TidyIterator   GetDeclaredTagList( TidyDocImpl* doc );
Dict*          GetNextDeclaredDict( TidyDocImpl* doc, TidyIterator* iter );
ctmbstr        GetNextDeclaredTag( TidyDocImpl* doc, int tagType,
                                   TidyIterator* iter );

void InitTags( TidyDocImpl* doc );
void FreeTags( TidyDocImpl* doc );


/* Parser methods for tags */

Parser ParseHTML;
Parser ParseHead;
Parser ParseTitle;
Parser ParseScript;
Parser ParseFrameSet;
Parser ParseNoFrames;
Parser ParseBody;
Parser ParsePre;
Parser ParseList;
Parser ParseLI;
Parser ParseDefList;
Parser ParseBlock;
Parser ParseInline;
Parser ParseEmpty;
Parser ParseTableTag;
Parser ParseColGroup;
Parser ParseRowGroup;
Parser ParseRow;
Parser ParseSelect;
Parser ParseOptGroup;
Parser ParseText;
Parser ParseObject;
Parser ParseMap;

/* Attribute checking methods */

CheckAttribs CheckAttributes;
CheckAttribs CheckHR;
CheckAttribs CheckIMG;
CheckAttribs CheckAnchor;
CheckAttribs CheckLINK;
CheckAttribs CheckMap;
CheckAttribs CheckAREA;
CheckAttribs CheckTABLE;
CheckAttribs CheckTableCell;
CheckAttribs CheckCaption;
CheckAttribs CheckSCRIPT;
CheckAttribs CheckSTYLE;
CheckAttribs CheckHTML;
CheckAttribs CheckFORM;
CheckAttribs CheckMETA;


/* 0 == TidyElem_UNKNOWN */
#define TagId(node)        ((node) && (node)->tag ? (node)->tag->id : TidyElem_UNKNOWN)
#define TagIsId(node, tid) ((node) && (node)->tag && (node)->tag->id == tid)

Bool nodeIsText( Node* node );
Bool nodeIsElement( Node* node );

Bool nodeHasText( TidyDocImpl* doc, Node* node );

/* Compare & result to operand.  If equal, then all bits
** requested are set.
*/
Bool nodeMatchCM( Node* node, uint contentModel );

/* True if any of the bits requested are set.
*/
Bool nodeHasCM( Node* node, uint contentModel );

Bool nodeCMIsBlock( Node* node );
Bool nodeCMIsInline( Node* node );
Bool nodeCMIsEmpty( Node* node );


Bool nodeIsHeader( Node* node );     /* H1, H2, ..., H6 */
uint nodeHeaderLevel( Node* node );  /* 1, 2, ..., 6 */

#define nodeIsHTML( node )       TagIsId( node, TidyElem_HTML )
#define nodeIsHEAD( node )       TagIsId( node, TidyElem_HEAD )
#define nodeIsTITLE( node )      TagIsId( node, TidyElem_TITLE )
#define nodeIsBASE( node )       TagIsId( node, TidyElem_BASE )
#define nodeIsMETA( node )       TagIsId( node, TidyElem_META )
#define nodeIsBODY( node )       TagIsId( node, TidyElem_BODY )
#define nodeIsFRAMESET( node )   TagIsId( node, TidyElem_FRAMESET )
#define nodeIsFRAME( node )      TagIsId( node, TidyElem_FRAME )
#define nodeIsIFRAME( node )     TagIsId( node, TidyElem_IFRAME )
#define nodeIsNOFRAMES( node )   TagIsId( node, TidyElem_NOFRAMES )
#define nodeIsHR( node )         TagIsId( node, TidyElem_HR )
#define nodeIsH1( node )         TagIsId( node, TidyElem_H1 )
#define nodeIsH2( node )         TagIsId( node, TidyElem_H2 )
#define nodeIsPRE( node )        TagIsId( node, TidyElem_PRE )
#define nodeIsLISTING( node )    TagIsId( node, TidyElem_LISTING )
#define nodeIsP( node )          TagIsId( node, TidyElem_P )
#define nodeIsUL( node )         TagIsId( node, TidyElem_UL )
#define nodeIsOL( node )         TagIsId( node, TidyElem_OL )
#define nodeIsDL( node )         TagIsId( node, TidyElem_DL )
#define nodeIsDIR( node )        TagIsId( node, TidyElem_DIR )
#define nodeIsLI( node )         TagIsId( node, TidyElem_LI )
#define nodeIsDT( node )         TagIsId( node, TidyElem_DT )
#define nodeIsDD( node )         TagIsId( node, TidyElem_DD )
#define nodeIsTABLE( node )      TagIsId( node, TidyElem_TABLE )
#define nodeIsCAPTION( node )    TagIsId( node, TidyElem_CAPTION )
#define nodeIsTD( node )         TagIsId( node, TidyElem_TD )
#define nodeIsTH( node )         TagIsId( node, TidyElem_TH )
#define nodeIsTR( node )         TagIsId( node, TidyElem_TR )
#define nodeIsCOL( node )        TagIsId( node, TidyElem_COL )
#define nodeIsCOLGROUP( node )   TagIsId( node, TidyElem_COLGROUP )
#define nodeIsBR( node )         TagIsId( node, TidyElem_BR )
#define nodeIsA( node )          TagIsId( node, TidyElem_A )
#define nodeIsLINK( node )       TagIsId( node, TidyElem_LINK )
#define nodeIsB( node )          TagIsId( node, TidyElem_B )
#define nodeIsI( node )          TagIsId( node, TidyElem_I )
#define nodeIsSTRONG( node )     TagIsId( node, TidyElem_STRONG )
#define nodeIsEM( node )         TagIsId( node, TidyElem_EM )
#define nodeIsBIG( node )        TagIsId( node, TidyElem_BIG )
#define nodeIsSMALL( node )      TagIsId( node, TidyElem_SMALL )
#define nodeIsPARAM( node )      TagIsId( node, TidyElem_PARAM )
#define nodeIsOPTION( node )     TagIsId( node, TidyElem_OPTION )
#define nodeIsOPTGROUP( node )   TagIsId( node, TidyElem_OPTGROUP )
#define nodeIsIMG( node )        TagIsId( node, TidyElem_IMG )
#define nodeIsMAP( node )        TagIsId( node, TidyElem_MAP )
#define nodeIsAREA( node )       TagIsId( node, TidyElem_AREA )
#define nodeIsNOBR( node )       TagIsId( node, TidyElem_NOBR )
#define nodeIsWBR( node )        TagIsId( node, TidyElem_WBR )
#define nodeIsFONT( node )       TagIsId( node, TidyElem_FONT )
#define nodeIsLAYER( node )      TagIsId( node, TidyElem_LAYER )
#define nodeIsSPACER( node )     TagIsId( node, TidyElem_SPACER )
#define nodeIsCENTER( node )     TagIsId( node, TidyElem_CENTER )
#define nodeIsSTYLE( node )      TagIsId( node, TidyElem_STYLE )
#define nodeIsSCRIPT( node )     TagIsId( node, TidyElem_SCRIPT )
#define nodeIsNOSCRIPT( node )   TagIsId( node, TidyElem_NOSCRIPT )
#define nodeIsFORM( node )       TagIsId( node, TidyElem_FORM )
#define nodeIsTEXTAREA( node )   TagIsId( node, TidyElem_TEXTAREA )
#define nodeIsBLOCKQUOTE( node ) TagIsId( node, TidyElem_BLOCKQUOTE )
#define nodeIsAPPLET( node )     TagIsId( node, TidyElem_APPLET )
#define nodeIsOBJECT( node )     TagIsId( node, TidyElem_OBJECT )
#define nodeIsDIV( node )        TagIsId( node, TidyElem_DIV )
#define nodeIsSPAN( node )       TagIsId( node, TidyElem_SPAN )
#define nodeIsINPUT( node )      TagIsId( node, TidyElem_INPUT )
#define nodeIsQ( node )          TagIsId( node, TidyElem_Q )
#define nodeIsLABEL( node )      TagIsId( node, TidyElem_LABEL )
#define nodeIsH3( node )         TagIsId( node, TidyElem_H3 )
#define nodeIsH4( node )         TagIsId( node, TidyElem_H4 )
#define nodeIsH5( node )         TagIsId( node, TidyElem_H5 )
#define nodeIsH6( node )         TagIsId( node, TidyElem_H6 )
#define nodeIsADDRESS( node )    TagIsId( node, TidyElem_ADDRESS )
#define nodeIsXMP( node )        TagIsId( node, TidyElem_XMP )
#define nodeIsSELECT( node )     TagIsId( node, TidyElem_SELECT )
#define nodeIsBLINK( node )      TagIsId( node, TidyElem_BLINK )
#define nodeIsMARQUEE( node )    TagIsId( node, TidyElem_MARQUEE )
#define nodeIsEMBED( node )      TagIsId( node, TidyElem_EMBED )
#define nodeIsBASEFONT( node )   TagIsId( node, TidyElem_BASEFONT )
#define nodeIsISINDEX( node )    TagIsId( node, TidyElem_ISINDEX )
#define nodeIsS( node )          TagIsId( node, TidyElem_S )
#define nodeIsSTRIKE( node )     TagIsId( node, TidyElem_STRIKE )
#define nodeIsU( node )          TagIsId( node, TidyElem_U )
#define nodeIsMENU( node )       TagIsId( node, TidyElem_MENU )


#endif /* __TAGS_H__ */
