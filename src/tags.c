/* tags.c -- recognize HTML tags

  (c) 1998-2003 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: hoehrmann $ 
    $Date: 2003/03/31 16:31:38 $ 
    $Revision: 1.28 $ 

  The HTML tags are stored as 8 bit ASCII strings.

*/

#include "tags.h"
#include "tidy-int.h"
#include "message.h"
#include "tmbstr.h"
#include "parser.h"  /* For FixId() */

static const Dict tag_defs[] =
{
  { TidyElem_UNKNOWN,    "unknown!",   0,                         (0),                                           NULL,          NULL           },
  { TidyElem_A,          "a",          VERS_ALL,                  (CM_INLINE),                                   ParseInline,   CheckAnchor    },
  { TidyElem_ABBR,       "abbr",       VERS_HTML40,               (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_ACRONYM,    "acronym",    VERS_HTML40,               (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_ADDRESS,    "address",    VERS_ALL,                  (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_ALIGN,      "align",      VERS_NETSCAPE,             (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_APPLET,     "applet",     VERS_LOOSE,                (CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM),         ParseBlock,    NULL           },
  { TidyElem_AREA,       "area",       (VERS_ALL)&~VERS_BASIC,    (CM_BLOCK|CM_EMPTY),                           ParseEmpty,    CheckAREA      },
  { TidyElem_B,          "b",          (VERS_ALL)&~VERS_BASIC,    (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_BASE,       "base",       VERS_ALL,                  (CM_HEAD|CM_EMPTY),                            ParseEmpty,    NULL           },
  { TidyElem_BASEFONT,   "basefont",   VERS_LOOSE,                (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyElem_BDO,        "bdo",        (VERS_HTML40)&~VERS_BASIC, (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_BGSOUND,    "bgsound",    VERS_MICROSOFT,            (CM_HEAD|CM_EMPTY),                            ParseEmpty,    NULL           },
  { TidyElem_BIG,        "big",        (VERS_FROM32)&~VERS_BASIC, (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_BLINK,      "blink",      VERS_PROPRIETARY,          (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_BLOCKQUOTE, "blockquote", VERS_ALL,                  (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_BODY,       "body",       VERS_ALL,                  (CM_HTML|CM_OPT|CM_OMITST),                    ParseBody,     NULL           },
  { TidyElem_BR,         "br",         VERS_ALL,                  (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyElem_BUTTON,     "button",     (VERS_HTML40)&~VERS_BASIC, (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_CAPTION,    "caption",    VERS_FROM32,               (CM_TABLE),                                    ParseInline,   CheckCaption   },
  { TidyElem_CENTER,     "center",     VERS_LOOSE,                (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_CITE,       "cite",       VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_CODE,       "code",       VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_COL,        "col",        VERS_HTML40,               (CM_TABLE|CM_EMPTY),                           ParseEmpty,    NULL           },
  { TidyElem_COLGROUP,   "colgroup",   VERS_HTML40,               (CM_TABLE|CM_OPT),                             ParseColGroup, NULL           },
  { TidyElem_COMMENT,    "comment",    VERS_MICROSOFT,            (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_DD,         "dd",         VERS_ALL,                  (CM_DEFLIST|CM_OPT|CM_NO_INDENT),              ParseBlock,    NULL           },
  { TidyElem_DEL,        "del",        (VERS_HTML40)&~VERS_BASIC, (CM_INLINE|CM_BLOCK|CM_MIXED),                 ParseInline,   NULL           },
  { TidyElem_DFN,        "dfn",        VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_DIR,        "dir",        VERS_LOOSE,                (CM_BLOCK|CM_OBSOLETE),                        ParseList,     NULL           },
  { TidyElem_DIV,        "div",        VERS_FROM32,               (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_DL,         "dl",         VERS_ALL,                  (CM_BLOCK),                                    ParseDefList,  NULL           },
  { TidyElem_DT,         "dt",         VERS_ALL,                  (CM_DEFLIST|CM_OPT|CM_NO_INDENT),              ParseInline,   NULL           },
  { TidyElem_EM,         "em",         VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_EMBED,      "embed",      VERS_NETSCAPE,             (CM_INLINE|CM_IMG|CM_EMPTY),                   ParseEmpty,    NULL           },
  { TidyElem_FIELDSET,   "fieldset",   (VERS_HTML40)&~VERS_BASIC, (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_FONT,       "font",       VERS_LOOSE,                (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_FORM,       "form",       VERS_ALL,                  (CM_BLOCK),                                    ParseBlock,    CheckFORM      },
  { TidyElem_FRAME,      "frame",      VERS_FRAMESET,             (CM_FRAMES|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyElem_FRAMESET,   "frameset",   VERS_FRAMESET,             (CM_HTML|CM_FRAMES),                           ParseFrameSet, NULL           },
  { TidyElem_H1,         "h1",         VERS_ALL,                  (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyElem_H2,         "h2",         VERS_ALL,                  (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyElem_H3,         "h3",         VERS_ALL,                  (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyElem_H4,         "h4",         VERS_ALL,                  (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyElem_H5,         "h5",         VERS_ALL,                  (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyElem_H6,         "h6",         VERS_ALL,                  (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyElem_HEAD,       "head",       VERS_ALL,                  (CM_HTML|CM_OPT|CM_OMITST),                    ParseHead,     NULL           },
  { TidyElem_HR,         "hr",         (VERS_ALL)&~VERS_BASIC,    (CM_BLOCK|CM_EMPTY),                           ParseEmpty,    CheckHR        },
  { TidyElem_HTML,       "html",       VERS_ALL,                  (CM_HTML|CM_OPT|CM_OMITST),                    ParseHTML,     CheckHTML      },
  { TidyElem_I,          "i",          (VERS_ALL)&~VERS_BASIC,    (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_IFRAME,     "iframe",     VERS_IFRAME,               (CM_INLINE),                                   ParseBlock,    NULL           },
  { TidyElem_ILAYER,     "ilayer",     VERS_NETSCAPE,             (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_IMG,        "img",        VERS_ALL,                  (CM_INLINE|CM_IMG|CM_EMPTY),                   ParseEmpty,    CheckIMG       },
  { TidyElem_INPUT,      "input",      VERS_ALL,                  (CM_INLINE|CM_IMG|CM_EMPTY),                   ParseEmpty,    NULL           },
  { TidyElem_INS,        "ins",        (VERS_HTML40)&~VERS_BASIC, (CM_INLINE|CM_BLOCK|CM_MIXED),                 ParseInline,   NULL           },
  { TidyElem_ISINDEX,    "isindex",    VERS_LOOSE,                (CM_BLOCK|CM_EMPTY),                           ParseEmpty,    NULL           },
  { TidyElem_KBD,        "kbd",        VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_KEYGEN,     "keygen",     VERS_NETSCAPE,             (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyElem_LABEL,      "label",      VERS_HTML40,               (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_LAYER,      "layer",      VERS_NETSCAPE,             (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_LEGEND,     "legend",     (VERS_HTML40)&~VERS_BASIC, (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_LI,         "li",         VERS_ALL,                  (CM_LIST|CM_OPT|CM_NO_INDENT),                 ParseBlock,    NULL           },
  { TidyElem_LINK,       "link",       VERS_ALL,                  (CM_HEAD|CM_EMPTY),                            ParseEmpty,    CheckLINK      },
  { TidyElem_LISTING,    "listing",    VERS_ALL,                  (CM_BLOCK|CM_OBSOLETE),                        ParsePre,      NULL           },
  { TidyElem_MAP,        "map",        (VERS_FROM32)&~VERS_BASIC, (CM_INLINE),                                   ParseBlock,    CheckMap       },
  { TidyElem_MARQUEE,    "marquee",    VERS_MICROSOFT,            (CM_INLINE|CM_OPT),                            ParseInline,   NULL           },
  { TidyElem_MENU,       "menu",       VERS_LOOSE,                (CM_BLOCK|CM_OBSOLETE),                        ParseList,     NULL           },
  { TidyElem_META,       "meta",       VERS_ALL,                  (CM_HEAD|CM_EMPTY),                            ParseEmpty,    CheckMETA      },
  { TidyElem_MULTICOL,   "multicol",   VERS_NETSCAPE,             (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_NOBR,       "nobr",       VERS_PROPRIETARY,          (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_NOEMBED,    "noembed",    VERS_NETSCAPE,             (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_NOFRAMES,   "noframes",   VERS_IFRAME,               (CM_BLOCK|CM_FRAMES),                          ParseNoFrames, NULL           },
  { TidyElem_NOLAYER,    "nolayer",    VERS_NETSCAPE,             (CM_BLOCK|CM_INLINE|CM_MIXED),                 ParseBlock,    NULL           },
  { TidyElem_NOSAVE,     "nosave",     VERS_NETSCAPE,             (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyElem_NOSCRIPT,   "noscript",   (VERS_HTML40)&~VERS_BASIC, (CM_BLOCK|CM_INLINE|CM_MIXED),                 ParseBlock,    NULL           },
  { TidyElem_OBJECT,     "object",     VERS_HTML40,               (CM_OBJECT|CM_HEAD|CM_IMG|CM_INLINE|CM_PARAM), ParseBlock,    NULL           },
  { TidyElem_OL,         "ol",         VERS_ALL,                  (CM_BLOCK),                                    ParseList,     NULL           },
  { TidyElem_OPTGROUP,   "optgroup",   (VERS_HTML40)&~VERS_BASIC, (CM_FIELD|CM_OPT),                             ParseOptGroup, NULL           },
  { TidyElem_OPTION,     "option",     VERS_ALL,                  (CM_FIELD|CM_OPT),                             ParseText,     NULL           },
  { TidyElem_P,          "p",          VERS_ALL,                  (CM_BLOCK|CM_OPT),                             ParseInline,   NULL           },
  { TidyElem_PARAM,      "param",      VERS_FROM32,               (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyElem_PLAINTEXT,  "plaintext",  VERS_ALL,                  (CM_BLOCK|CM_OBSOLETE),                        ParsePre,      NULL           },
  { TidyElem_PRE,        "pre",        VERS_ALL,                  (CM_BLOCK),                                    ParsePre,      NULL           },
  { TidyElem_Q,          "q",          VERS_HTML40,               (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_RB,         "rb",         VERS_XHTML11,              (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_RBC,        "rbc",        VERS_XHTML11,              (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_RP,         "rp",         VERS_XHTML11,              (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_RT,         "rt",         VERS_XHTML11,              (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_RTC,        "rtc",        VERS_XHTML11,              (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_RUBY,       "ruby",       VERS_XHTML11,              (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_S,          "s",          VERS_LOOSE,                (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_SAMP,       "samp",       VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_SCRIPT,     "script",     (VERS_FROM32)&~VERS_BASIC, (CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE),         ParseScript,   CheckSCRIPT    },
  { TidyElem_SELECT,     "select",     VERS_ALL,                  (CM_INLINE|CM_FIELD),                          ParseSelect,   NULL           },
  { TidyElem_SERVER,     "server",     VERS_NETSCAPE,             (CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE),         ParseScript,   NULL           },
  { TidyElem_SERVLET,    "servlet",    VERS_SUN,                  (CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM),         ParseBlock,    NULL           },
  { TidyElem_SMALL,      "small",      (VERS_FROM32)&~VERS_BASIC, (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_SPACER,     "spacer",     VERS_NETSCAPE,             (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyElem_SPAN,       "span",       VERS_FROM32,               (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_STRIKE,     "strike",     VERS_LOOSE,                (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_STRONG,     "strong",     VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_STYLE,      "style",      (VERS_FROM32)&~VERS_BASIC, (CM_HEAD),                                     ParseScript,   CheckSTYLE     },
  { TidyElem_SUB,        "sub",        (VERS_FROM32)&~VERS_BASIC, (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_SUP,        "sup",        (VERS_FROM32)&~VERS_BASIC, (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_TABLE,      "table",      VERS_FROM32,               (CM_BLOCK),                                    ParseTableTag, CheckTABLE     },
  { TidyElem_TBODY,      "tbody",      (VERS_HTML40)&~VERS_BASIC, (CM_TABLE|CM_ROWGRP|CM_OPT),                   ParseRowGroup, NULL           },
  { TidyElem_TD,         "td",         VERS_FROM32,               (CM_ROW|CM_OPT|CM_NO_INDENT),                  ParseBlock,    CheckTableCell },
  { TidyElem_TEXTAREA,   "textarea",   VERS_ALL,                  (CM_INLINE|CM_FIELD),                          ParseText,     NULL           },
  { TidyElem_TFOOT,      "tfoot",      (VERS_HTML40)&~VERS_BASIC, (CM_TABLE|CM_ROWGRP|CM_OPT),                   ParseRowGroup, NULL           },
  { TidyElem_TH,         "th",         VERS_FROM32,               (CM_ROW|CM_OPT|CM_NO_INDENT),                  ParseBlock,    CheckTableCell },
  { TidyElem_THEAD,      "thead",      (VERS_HTML40)&~VERS_BASIC, (CM_TABLE|CM_ROWGRP|CM_OPT),                   ParseRowGroup, NULL           },
  { TidyElem_TITLE,      "title",      VERS_ALL,                  (CM_HEAD),                                     ParseTitle,    NULL           },
  { TidyElem_TR,         "tr",         VERS_FROM32,               (CM_TABLE|CM_OPT),                             ParseRow,      NULL           },
  { TidyElem_TT,         "tt",         (VERS_ALL)&~VERS_BASIC,    (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_U,          "u",          VERS_LOOSE,                (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_UL,         "ul",         VERS_ALL,                  (CM_BLOCK),                                    ParseList,     NULL           },
  { TidyElem_VAR,        "var",        VERS_ALL,                  (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyElem_WBR,        "wbr",        VERS_PROPRIETARY,          (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyElem_XMP,        "xmp",        VERS_ALL,                  (CM_BLOCK|CM_OBSOLETE),                        ParsePre,      NULL           },
  { TidyElem_NEXTID,     "nextid",     VERS_HTML20,               (CM_HEAD|CM_EMPTY),                            ParseEmpty,    NULL           },

  /* this must be the final entry */
  { 0,                   NULL,         0,                         (0),                                           NULL,          NULL           }
};

/* choose what version to use for new doctype */
int HTMLVersion( TidyDocImpl* doc )
{
    int dtver = doc->lexer->doctype;
    uint versions = doc->lexer->versions;
    TidyDoctypeModes dtmode = cfg(doc, TidyDoctypeMode);

    Bool wantXhtml = !cfgBool(doc, TidyHtmlOut) &&
                     ( cfgBool(doc, TidyXmlOut) || doc->lexer->isvoyager );

    Bool wantHtml4 = dtmode==TidyDoctypeStrict || dtmode==TidyDoctypeLoose ||
                     dtver==VERS_HTML40_STRICT || dtver==VERS_HTML40_LOOSE;

    /* Prefer HTML 4.x for XHTML */
    if ( !wantXhtml && !wantHtml4 )
    {
        if ( versions & VERS_HTML32 )   /* Prefer 3.2 over 2.0 */
            return VERS_HTML32;

        if ( versions & VERS_HTML20 )
            return VERS_HTML20;
    }

    if ( wantXhtml && (versions & VERS_XHTML11) )
        return VERS_XHTML11;

    if ( versions & VERS_HTML40_STRICT )
        return VERS_HTML40_STRICT;

    if ( versions & VERS_HTML40_LOOSE )
        return VERS_HTML40_LOOSE;

    if ( versions & VERS_FRAMESET )
        return VERS_FRAMESET;

    /* Still here?  Try these again. */
    if ( versions & VERS_HTML32 )   /* Prefer 3.2 over 2.0 */
        return VERS_HTML32;

    if ( versions & VERS_HTML20 )
        return VERS_HTML20;

    return VERS_UNKNOWN;
}

static const Dict* lookup( TidyTagImpl* tags, ctmbstr s )
{
    Dict *np = NULL;
    if ( s )
    {
        const Dict *np = tag_defs + 1;  /* Skip Unknown */
        for ( /**/; np < tag_defs + N_TIDY_TAGS; ++np )
            if ( tmbstrcmp(s, np->name) == 0 )
                return np;

        for ( np = tags->declared_tag_list; np; np = np->next )
            if ( tmbstrcmp(s, np->name) == 0 )
                return np;
    }
    return NULL;
}


static void declare( TidyTagImpl* tags,
                     ctmbstr name, uint versions, uint model, 
                     Parser *parser, CheckAttribs *chkattrs )
{
    if ( name )
    {
        Dict* np = (Dict*) lookup( tags, name );
        if ( np == NULL )
        {
            np = (Dict*) MemAlloc( sizeof(Dict) );
            ClearMemory( np, sizeof(Dict) );

            np->name = tmbstrdup( name );
            np->next = tags->declared_tag_list;
            tags->declared_tag_list = np;
        }

        /* Make sure we are not over-writing predefined tags */
        if ( np->id == TidyElem_UNKNOWN )
        {
          np->versions = versions;
          np->model   |= model;
          np->parser   = parser;
          np->chkattrs = chkattrs;
        }
    }
}

/* public interface for finding tag by name */
Bool FindTag( TidyDocImpl* doc, Node *node )
{
    const Dict *np = NULL;
    if ( cfgBool(doc, TidyXmlTags) )
    {
        node->tag = doc->tags.xml_tags;
        return yes;
    }

    if ( node->element && (np = lookup(&doc->tags, node->element)) )
    {
        node->tag = np;
        return yes;
    }
    
    return no;
}

const Dict* LookupTagDef( TidyTagId tid )
{
  if ( tid > TidyElem_UNKNOWN && tid < N_TIDY_TAGS )
      return tag_defs + tid;
  return NULL;
}


Parser* FindParser( TidyDocImpl* doc, Node *node )
{
    const Dict* np = lookup( &doc->tags, node->element );
    if ( np )
        return np->parser;
    return NULL;
}

void DefineTag( TidyDocImpl* doc, int tagType, ctmbstr name )
{
    Parser* parser = NULL;
    uint cm = 0;
    uint vers = VERS_PROPRIETARY;

    switch (tagType)
    {
    case tagtype_empty:
        cm = CM_EMPTY|CM_NO_INDENT|CM_NEW;
        parser = ParseBlock;
        break;

    case tagtype_inline:
        cm = CM_INLINE|CM_NO_INDENT|CM_NEW;
        parser = ParseInline;
        break;

    case tagtype_block:
        cm = CM_BLOCK|CM_NO_INDENT|CM_NEW;
        parser = ParseBlock;
        break;

    case tagtype_pre:
        cm = CM_BLOCK|CM_NO_INDENT|CM_NEW;
        parser = ParsePre;
        break;
    }
    if ( cm && parser )
        declare( &doc->tags, name, vers, cm, parser, NULL );
}

TidyIterator   GetDeclaredTagList( TidyDocImpl* doc )
{
    return (TidyIterator) doc->tags.declared_tag_list;
}

ctmbstr        GetNextDeclaredTag( TidyDocImpl* doc, int tagType,
                                   TidyIterator* iter )
{
    ctmbstr name = NULL;
    Dict* curr = (Dict*) *iter;
    for ( curr; name == NULL && curr != NULL; curr = curr->next )
    {
        switch ( tagType )
        {
        case tagtype_empty:
            if ( curr->model & CM_EMPTY )
                name = curr->name;
            break;

        case tagtype_inline:
            if ( curr->model & CM_INLINE )
                name = curr->name;
            break;

        case tagtype_block:
            if ( (curr->model & CM_BLOCK) &&
                 curr->parser == ParseBlock )
                name = curr->name;
            break;
    
        case tagtype_pre:
            if ( (curr->model & CM_BLOCK) &&
                 curr->parser == ParsePre )
                name = curr->name;
            break;
        }
    }
    *iter = (TidyIterator) curr;
    return name;
}

void InitTags( TidyDocImpl* doc )
{
    Dict* xml;
    TidyTagImpl* tags = &doc->tags;
    ClearMemory( tags, sizeof(TidyTagImpl) );

    /* create dummy entry for all xml tags */
    xml = (Dict*) MemAlloc( sizeof(Dict) );
    ClearMemory( xml, sizeof(Dict) );
    xml->name = NULL;
    xml->versions = VERS_XML;
    xml->model = CM_BLOCK;
    xml->parser = NULL;
    xml->chkattrs = NULL;
    tags->xml_tags = xml;
}

/* By default, zap all of them.  But allow
** an single type to be specified.
*/
void FreeDeclaredTags( TidyDocImpl* doc, int tagType )
{
    TidyTagImpl* tags = &doc->tags;
    Dict *curr, *next = NULL, *prev = NULL;

    for ( curr=tags->declared_tag_list; curr; curr = next )
    {
        Bool deleteIt = yes;
        next = curr->next;
        switch ( tagType )
        {
        case tagtype_empty:
            deleteIt = ( curr->model & CM_EMPTY );
            break;

        case tagtype_inline:
            deleteIt = ( curr->model & CM_INLINE );
            break;

        case tagtype_block:
            deleteIt = ( (curr->model & CM_BLOCK) &&
                         curr->parser == ParseBlock );
            break;
    
        case tagtype_pre:
            deleteIt = ( (curr->model & CM_BLOCK) &&
                         curr->parser == ParsePre );
            break;
        }

        if ( deleteIt )
        {
          MemFree( curr->name );
          MemFree( curr );
          if ( prev )
            prev->next = next;
          else
            tags->declared_tag_list = next;
        }
        else
          prev = curr;
    }
}

void FreeTags( TidyDocImpl* doc )
{
    TidyTagImpl* tags = &doc->tags;
    FreeDeclaredTags( doc, 0 );

    MemFree( tags->xml_tags );

    /* get rid of dangling tag references */
    ClearMemory( tags, sizeof(TidyTagImpl) );
}


/* default method for checking an element's attributes */
void CheckAttributes( TidyDocImpl* doc, Node *node )
{
    AttVal *attval;
    for ( attval = node->attributes; attval != NULL; attval = attval->next )
        CheckAttribute( doc, node, attval );
}

/* methods for checking attributes for specific elements */

void CheckHR( TidyDocImpl* doc, Node *node )
{
    AttVal* av = GetAttrByName( node, "src" );
    CheckAttributes( doc, node );
    if ( av )
        ReportAttrError( doc, node, av, PROPRIETARY_ATTR_VALUE );
}

void CheckIMG( TidyDocImpl* doc, Node *node )
{
    Bool HasAlt = no;
    Bool HasSrc = no;
    Bool HasUseMap = no;
    Bool HasIsMap = no;
    Bool HasDataFld = no;

    AttVal *attval;
    for ( attval = node->attributes; attval != NULL; attval = attval->next )
    {
        const Attribute* dict = CheckAttribute( doc, node, attval );
        if ( dict )
        {
            TidyAttrId id = dict->id;
            if ( id == TidyAttr_alt )
                HasAlt = yes;
            else if ( id == TidyAttr_src )
                HasSrc = yes;
            else if ( id == TidyAttr_usemap )
                HasUseMap = yes;
            else if ( id == TidyAttr_ismap )
                HasIsMap = yes;
            else if ( id == TidyAttr_datafld )
                HasDataFld = yes;
            else if ( id == TidyAttr_width || id == TidyAttr_height )
                ConstrainVersion( doc, ~VERS_HTML20 );
        }
    }

    if ( !HasAlt )
    {
        if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
        {
            doc->badAccess |= MISSING_IMAGE_ALT;
            ReportMissingAttr( doc, node, "alt" );
        }
  
        if ( cfgStr(doc, TidyAltText) )
            AddAttribute( doc, node, "alt", cfgStr(doc, TidyAltText) );
    }

    if ( !HasSrc && !HasDataFld )
        ReportMissingAttr( doc, node, "src" );

    if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
    {
        if ( HasIsMap && !HasUseMap )
            ReportMissingAttr( doc, node, "ismap" );
    }
}

void CheckAnchor( TidyDocImpl* doc, Node *node )
{
    CheckAttributes( doc, node );
    FixId( doc, node );
}

void CheckMap( TidyDocImpl* doc, Node *node )
{
    CheckAttributes( doc, node );
    FixId( doc, node );
}

void CheckTableCell( TidyDocImpl* doc, Node *node )
{
    CheckAttributes( doc, node );

    /*
      HTML4 strict doesn't allow mixed content for
      elements with %block; as their content model
    */
    if ( GetAttrByName(node, "width") 
         || GetAttrByName(node, "height") )
        ConstrainVersion( doc, ~VERS_HTML40_STRICT );
}

void CheckCaption( TidyDocImpl* doc, Node *node )
{
    AttVal *attval;
    char *value = NULL;

    CheckAttributes( doc, node );

    for (attval = node->attributes; attval != NULL; attval = attval->next)
    {
        if ( tmbstrcasecmp(attval->attribute, "align") == 0 )
        {
            value = attval->value;
            break;
        }
    }

    if (value != NULL)
    {
        if ( tmbstrcasecmp(value, "left") == 0 ||
             tmbstrcasecmp(value, "right") == 0 )
            ConstrainVersion( doc, VERS_HTML40_LOOSE );
        else if ( tmbstrcasecmp(value, "top") == 0 ||
                  tmbstrcasecmp(value, "bottom") == 0 )
            ConstrainVersion( doc, ~(VERS_HTML20|VERS_HTML32) );
        else
            ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE );
    }
}

void CheckHTML( TidyDocImpl* doc, Node *node )
{
    AttVal *attval;
    AttVal *xmlns;

    xmlns = GetAttrByName(node, "xmlns");

    if ( xmlns != NULL && tmbstrcmp(xmlns->value, XHTML_NAMESPACE) == 0 )
    {
        Bool htmlOut = cfgBool( doc, TidyHtmlOut );
        doc->lexer->isvoyager = yes;                  /* Unless plain HTML */
        SetOptionBool( doc, TidyXhtmlOut, !htmlOut ); /* is specified, output*/
        SetOptionBool( doc, TidyXmlOut, !htmlOut );   /* will be XHTML. */

        /* adjust other config options, just as in config.c */
        if ( !htmlOut )
        {
            SetOptionBool( doc, TidyUpperCaseTags, no );
            SetOptionBool( doc, TidyUpperCaseAttrs, no );
        }
    }

    for (attval = node->attributes; attval != NULL; attval = attval->next)
    {
        CheckAttribute( doc, node, attval );
    }
}

void CheckAREA( TidyDocImpl* doc, Node *node )
{
    Bool HasAlt = no;
    Bool HasHref = no;
    AttVal *attval;

    for (attval = node->attributes; attval != NULL; attval = attval->next)
    {
        const Attribute* dict = CheckAttribute( doc, node, attval );
        if ( dict )
        {
          if ( dict->id == TidyAttr_alt )
              HasAlt = yes;
          else if ( dict->id == TidyAttr_href )
              HasHref = yes;
        }
    }

    if ( !HasAlt )
    {
        if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
        {
            doc->badAccess |= MISSING_LINK_ALT;
            ReportMissingAttr( doc, node, "alt" );
        }
    }

    if ( !HasHref )
        ReportMissingAttr( doc, node, "href" );
}

void CheckTABLE( TidyDocImpl* doc, Node *node )
{
    Bool HasSummary = no;
    AttVal *attval;

    for (attval = node->attributes; attval != NULL; attval = attval->next)
    {
        const Attribute* dict = CheckAttribute( doc, node, attval );
        if ( dict && dict->id == TidyAttr_summary )
            HasSummary = yes;
    }

    /* suppress warning for missing summary for HTML 2.0 and HTML 3.2 
    ** Now handled in HTMLVersionCompliance()
    ** if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
    ** {
    **     Lexer* lexer = doc->lexer;
    **     if ( !HasSummary 
    **          && lexer->doctype != VERS_HTML20
    **          && lexer->doctype != VERS_HTML32 )
    **     {
    **         doc->badAccess |= MISSING_SUMMARY;
    **         ReportMissingAttr( doc, node, "summary");
    **     }
    ** }
    */

    /* convert <table border> to <table border="1"> */
    if ( cfgBool(doc, TidyXmlOut) && (attval = GetAttrByName(node, "border")) )
    {
        if (attval->value == NULL)
            attval->value = tmbstrdup("1");
    }

    /* <table height="..."> is proprietary */
    if ( attval = GetAttrByName(node, "height") )
    {
        ReportAttrError( doc, node, attval, PROPRIETARY_ATTRIBUTE );
        ConstrainVersion( doc, VERS_PROPRIETARY );
    }
}

/* add missing type attribute when appropriate */
void CheckSCRIPT( TidyDocImpl* doc, Node *node )
{
    AttVal *lang, *type;
    char buf[16];

    CheckAttributes( doc, node );

    lang = GetAttrByName( node, "language" );
    type = GetAttrByName( node, "type" );

    if ( !type )
    {
        /*  ReportMissingAttr( doc, node, "type" );  */

        /* check for javascript */
        if ( lang )
        {
            tmbstrncpy( buf, lang->value, sizeof(buf) );
            buf[10] = '\0';

            if ( tmbstrncasecmp(buf, "javascript", 10) == 0 ||
                 tmbstrncasecmp(buf,    "jscript", 7) == 0 )
            {
                AddAttribute( doc, node, "type", "text/javascript" );
            }
            else if ( tmbstrcasecmp(buf, "vbscript") == 0 )
            {
                /* per Randy Waki 8/6/01 */
                AddAttribute( doc, node, "type", "text/vbscript" );
            }
        }
        else
            AddAttribute( doc, node, "type", "text/javascript" );
        type = GetAttrByName( node, "type" );
        ReportAttrError( doc, node, type, INSERTING_ATTRIBUTE );
    }
}


/* add missing type attribute when appropriate */
void CheckSTYLE( TidyDocImpl* doc, Node *node )
{
    AttVal *type = GetAttrByName( node, "type" );

    CheckAttributes( doc, node );

    if ( !type )
    {
        AddAttribute( doc, node, "type", "text/css" );
        type = GetAttrByName( node, "type" );
        ReportAttrError( doc, node, type, INSERTING_ATTRIBUTE );
    }
}

/* add missing type attribute when appropriate */
void CheckLINK( TidyDocImpl* doc, Node *node )
{
    AttVal *rel = GetAttrByName( node, "rel" );

    CheckAttributes( doc, node );

    if ( rel && rel->value &&
         tmbstrcmp(rel->value, "stylesheet") == 0 )
    {
        AttVal *type = GetAttrByName(node, "type");
        if (!type)
        {
            AddAttribute( doc, node, "type", "text/css" );
            type = GetAttrByName( node, "type" );
            ReportAttrError( doc, node, type, INSERTING_ATTRIBUTE );
        }
    }
}

/* reports missing action attribute */
void CheckFORM( TidyDocImpl* doc, Node *node )
{
    AttVal *action = GetAttrByName( node, "action" );
    CheckAttributes( doc, node );
    if (!action)
        ReportMissingAttr( doc, node, "action");
}

/* reports missing content attribute */
void CheckMETA( TidyDocImpl* doc, Node *node )
{
    AttVal *content = GetAttrByName( node, "content" );
    CheckAttributes( doc, node );
    if ( ! content )
        ReportMissingAttr( doc, node, "content" );
    /* name or http-equiv attribute must also be set */
}


Bool nodeIsText( Node* node )
{
  return ( node && node->type == TextNode );
}

Bool nodeHasText( TidyDocImpl* doc, Node* node )
{
  if ( doc && node )
  {
    uint ix;
    Lexer* lexer = doc->lexer;
    for ( ix = node->start; ix < node->end; ++ix )
    {
        /* whitespace */
        if ( !IsWhite( lexer->lexbuf[ix] ) )
            return yes;
    }
  }
  return no;
}

Bool nodeIsElement( Node* node )
{
  return ( node && 
           (node->type == StartTag || node->type == StartEndTag) );
}

/* Compare & result to operand.  If equal, then all bits
** requested are set.
*/
Bool nodeMatchCM( Node* node, uint contentModel )
{
  return ( node && node->tag && 
           (node->tag->model & contentModel) == contentModel );
}

/* True if any of the bits requested are set.
*/
Bool nodeHasCM( Node* node, uint contentModel )
{
  return ( node && node->tag && 
           (node->tag->model & contentModel) != 0 );
}

Bool nodeCMIsBlock( Node* node )
{
  return nodeHasCM( node, CM_BLOCK );
}
Bool nodeCMIsInline( Node* node )
{
  return nodeHasCM( node, CM_INLINE );
}
Bool nodeCMIsEmpty( Node* node )
{
  return nodeHasCM( node, CM_EMPTY );
}

Bool nodeIsHeader( Node* node )
{
    TidyTagId tid = TagId( node  );
    return ( tid && 
             tid == TidyElem_H1 ||
             tid == TidyElem_H2 ||
             tid == TidyElem_H3 ||        
             tid == TidyElem_H4 ||        
             tid == TidyElem_H5 ||
             tid == TidyElem_H6 );
}

uint nodeHeaderLevel( Node* node )
{
    TidyTagId tid = TagId( node  );
    switch ( tid )
    {
    case TidyElem_H1:
        return 1;
    case TidyElem_H2:
        return 2;
    case TidyElem_H3:
        return 3;
    case TidyElem_H4:
        return 4;
    case TidyElem_H5:
        return 5;
    case TidyElem_H6:
        return 6;
    }
    return 0;
}
