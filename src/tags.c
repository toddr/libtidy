/* tags.c -- recognize HTML tags

  (c) 1998-2003 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: hoehrmann $ 
    $Date: 2003/04/07 22:56:58 $ 
    $Revision: 1.31 $ 

  The HTML tags are stored as 8 bit ASCII strings.

*/

#include "tags.h"
#include "tidy-int.h"
#include "message.h"
#include "tmbstr.h"
#include "parser.h"  /* For FixId() */

static const Dict tag_defs[] =
{
  { TidyTag_UNKNOWN,    "unknown!",   0,                         NULL,                       (0),                                           NULL,          NULL           },
  { TidyTag_A,          "a",          VERS_ALL,                  &W3CAttrsFor_A[0],          (CM_INLINE),                                   ParseInline,   CheckAnchor    },
  { TidyTag_ABBR,       "abbr",       VERS_HTML40,               &W3CAttrsFor_ABBR[0],       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_ACRONYM,    "acronym",    VERS_HTML40,               &W3CAttrsFor_ACRONYM[0],    (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_ADDRESS,    "address",    VERS_ALL,                  &W3CAttrsFor_ADDRESS[0],    (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_ALIGN,      "align",      VERS_NETSCAPE,             NULL,                       (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_APPLET,     "applet",     VERS_LOOSE,                &W3CAttrsFor_APPLET[0],     (CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM),         ParseBlock,    NULL           },
  { TidyTag_AREA,       "area",       (VERS_ALL)&~VERS_BASIC,    &W3CAttrsFor_AREA[0],       (CM_BLOCK|CM_EMPTY),                           ParseEmpty,    CheckAREA      },
  { TidyTag_B,          "b",          (VERS_ALL)&~VERS_BASIC,    &W3CAttrsFor_B[0],          (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_BASE,       "base",       VERS_ALL,                  &W3CAttrsFor_BASE[0],       (CM_HEAD|CM_EMPTY),                            ParseEmpty,    NULL           },
  { TidyTag_BASEFONT,   "basefont",   VERS_LOOSE,                &W3CAttrsFor_BASEFONT[0],   (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyTag_BDO,        "bdo",        (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_BDO[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_BGSOUND,    "bgsound",    VERS_MICROSOFT,            NULL,                       (CM_HEAD|CM_EMPTY),                            ParseEmpty,    NULL           },
  { TidyTag_BIG,        "big",        (VERS_FROM32)&~VERS_BASIC, &W3CAttrsFor_BIG[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_BLINK,      "blink",      VERS_PROPRIETARY,          NULL,                       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_BLOCKQUOTE, "blockquote", VERS_ALL,                  &W3CAttrsFor_BLOCKQUOTE[0], (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_BODY,       "body",       VERS_ALL,                  &W3CAttrsFor_BODY[0],       (CM_HTML|CM_OPT|CM_OMITST),                    ParseBody,     NULL           },
  { TidyTag_BR,         "br",         VERS_ALL,                  &W3CAttrsFor_BR[0],         (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyTag_BUTTON,     "button",     (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_BUTTON[0],     (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_CAPTION,    "caption",    VERS_FROM32,               &W3CAttrsFor_CAPTION[0],    (CM_TABLE),                                    ParseInline,   CheckCaption   },
  { TidyTag_CENTER,     "center",     VERS_LOOSE,                &W3CAttrsFor_CENTER[0],     (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_CITE,       "cite",       VERS_ALL,                  &W3CAttrsFor_CITE[0],       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_CODE,       "code",       VERS_ALL,                  &W3CAttrsFor_CODE[0],       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_COL,        "col",        VERS_HTML40,               &W3CAttrsFor_COL[0],        (CM_TABLE|CM_EMPTY),                           ParseEmpty,    NULL           },
  { TidyTag_COLGROUP,   "colgroup",   VERS_HTML40,               &W3CAttrsFor_COLGROUP[0],   (CM_TABLE|CM_OPT),                             ParseColGroup, NULL           },
  { TidyTag_COMMENT,    "comment",    VERS_MICROSOFT,            NULL,                       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_DD,         "dd",         VERS_ALL,                  &W3CAttrsFor_DD[0],         (CM_DEFLIST|CM_OPT|CM_NO_INDENT),              ParseBlock,    NULL           },
  { TidyTag_DEL,        "del",        (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_DEL[0],        (CM_INLINE|CM_BLOCK|CM_MIXED),                 ParseInline,   NULL           },
  { TidyTag_DFN,        "dfn",        VERS_ALL,                  &W3CAttrsFor_DFN[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_DIR,        "dir",        VERS_LOOSE,                &W3CAttrsFor_DIR[0],        (CM_BLOCK|CM_OBSOLETE),                        ParseList,     NULL           },
  { TidyTag_DIV,        "div",        VERS_FROM32,               &W3CAttrsFor_DIV[0],        (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_DL,         "dl",         VERS_ALL,                  &W3CAttrsFor_DL[0],         (CM_BLOCK),                                    ParseDefList,  NULL           },
  { TidyTag_DT,         "dt",         VERS_ALL,                  &W3CAttrsFor_DT[0],         (CM_DEFLIST|CM_OPT|CM_NO_INDENT),              ParseInline,   NULL           },
  { TidyTag_EM,         "em",         VERS_ALL,                  &W3CAttrsFor_EM[0],         (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_EMBED,      "embed",      VERS_NETSCAPE,             NULL,                       (CM_INLINE|CM_IMG|CM_EMPTY),                   ParseEmpty,    NULL           },
  { TidyTag_FIELDSET,   "fieldset",   (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_FIELDSET[0],   (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_FONT,       "font",       VERS_LOOSE,                &W3CAttrsFor_FONT[0],       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_FORM,       "form",       VERS_ALL,                  &W3CAttrsFor_FORM[0],       (CM_BLOCK),                                    ParseBlock,    CheckFORM      },
  { TidyTag_FRAME,      "frame",      VERS_FRAMESET,             &W3CAttrsFor_FRAME[0],      (CM_FRAMES|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyTag_FRAMESET,   "frameset",   VERS_FRAMESET,             &W3CAttrsFor_FRAMESET[0],   (CM_HTML|CM_FRAMES),                           ParseFrameSet, NULL           },
  { TidyTag_H1,         "h1",         VERS_ALL,                  &W3CAttrsFor_H1[0],         (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyTag_H2,         "h2",         VERS_ALL,                  &W3CAttrsFor_H2[0],         (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyTag_H3,         "h3",         VERS_ALL,                  &W3CAttrsFor_H3[0],         (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyTag_H4,         "h4",         VERS_ALL,                  &W3CAttrsFor_H4[0],         (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyTag_H5,         "h5",         VERS_ALL,                  &W3CAttrsFor_H5[0],         (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyTag_H6,         "h6",         VERS_ALL,                  &W3CAttrsFor_H6[0],         (CM_BLOCK|CM_HEADING),                         ParseInline,   NULL           },
  { TidyTag_HEAD,       "head",       VERS_ALL,                  &W3CAttrsFor_HEAD[0],       (CM_HTML|CM_OPT|CM_OMITST),                    ParseHead,     NULL           },
  { TidyTag_HR,         "hr",         (VERS_ALL)&~VERS_BASIC,    &W3CAttrsFor_HR[0],         (CM_BLOCK|CM_EMPTY),                           ParseEmpty,    CheckHR        },
  { TidyTag_HTML,       "html",       VERS_ALL,                  &W3CAttrsFor_HTML[0],       (CM_HTML|CM_OPT|CM_OMITST),                    ParseHTML,     CheckHTML      },
  { TidyTag_I,          "i",          (VERS_ALL)&~VERS_BASIC,    &W3CAttrsFor_I[0],          (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_IFRAME,     "iframe",     VERS_IFRAME,               &W3CAttrsFor_IFRAME[0],     (CM_INLINE),                                   ParseBlock,    NULL           },
  { TidyTag_ILAYER,     "ilayer",     VERS_NETSCAPE,             NULL,                       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_IMG,        "img",        VERS_ALL,                  &W3CAttrsFor_IMG[0],        (CM_INLINE|CM_IMG|CM_EMPTY),                   ParseEmpty,    CheckIMG       },
  { TidyTag_INPUT,      "input",      VERS_ALL,                  &W3CAttrsFor_INPUT[0],      (CM_INLINE|CM_IMG|CM_EMPTY),                   ParseEmpty,    NULL           },
  { TidyTag_INS,        "ins",        (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_INS[0],        (CM_INLINE|CM_BLOCK|CM_MIXED),                 ParseInline,   NULL           },
  { TidyTag_ISINDEX,    "isindex",    VERS_LOOSE,                &W3CAttrsFor_ISINDEX[0],    (CM_BLOCK|CM_EMPTY),                           ParseEmpty,    NULL           },
  { TidyTag_KBD,        "kbd",        VERS_ALL,                  &W3CAttrsFor_KBD[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_KEYGEN,     "keygen",     VERS_NETSCAPE,             NULL,                       (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyTag_LABEL,      "label",      VERS_HTML40,               &W3CAttrsFor_LABEL[0],      (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_LAYER,      "layer",      VERS_NETSCAPE,             NULL,                       (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_LEGEND,     "legend",     (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_LEGEND[0],     (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_LI,         "li",         VERS_ALL,                  &W3CAttrsFor_LI[0],         (CM_LIST|CM_OPT|CM_NO_INDENT),                 ParseBlock,    NULL           },
  { TidyTag_LINK,       "link",       VERS_ALL,                  &W3CAttrsFor_LINK[0],       (CM_HEAD|CM_EMPTY),                            ParseEmpty,    CheckLINK      },
  { TidyTag_LISTING,    "listing",    VERS_ALL,                  &W3CAttrsFor_LISTING[0],    (CM_BLOCK|CM_OBSOLETE),                        ParsePre,      NULL           },
  { TidyTag_MAP,        "map",        (VERS_FROM32)&~VERS_BASIC, &W3CAttrsFor_MAP[0],        (CM_INLINE),                                   ParseBlock,    CheckMap       },
  { TidyTag_MARQUEE,    "marquee",    VERS_MICROSOFT,            NULL,                       (CM_INLINE|CM_OPT),                            ParseInline,   NULL           },
  { TidyTag_MENU,       "menu",       VERS_LOOSE,                &W3CAttrsFor_MENU[0],       (CM_BLOCK|CM_OBSOLETE),                        ParseList,     NULL           },
  { TidyTag_META,       "meta",       VERS_ALL,                  &W3CAttrsFor_META[0],       (CM_HEAD|CM_EMPTY),                            ParseEmpty,    CheckMETA      },
  { TidyTag_MULTICOL,   "multicol",   VERS_NETSCAPE,             NULL,                       (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_NOBR,       "nobr",       VERS_PROPRIETARY,          NULL,                       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_NOEMBED,    "noembed",    VERS_NETSCAPE,             NULL,                       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_NOFRAMES,   "noframes",   VERS_IFRAME,               &W3CAttrsFor_NOFRAMES[0],   (CM_BLOCK|CM_FRAMES),                          ParseNoFrames, NULL           },
  { TidyTag_NOLAYER,    "nolayer",    VERS_NETSCAPE,             NULL,                       (CM_BLOCK|CM_INLINE|CM_MIXED),                 ParseBlock,    NULL           },
  { TidyTag_NOSAVE,     "nosave",     VERS_NETSCAPE,             NULL,                       (CM_BLOCK),                                    ParseBlock,    NULL           },
  { TidyTag_NOSCRIPT,   "noscript",   (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_NOSCRIPT[0],   (CM_BLOCK|CM_INLINE|CM_MIXED),                 ParseBlock,    NULL           },
  { TidyTag_OBJECT,     "object",     VERS_HTML40,               &W3CAttrsFor_OBJECT[0],     (CM_OBJECT|CM_HEAD|CM_IMG|CM_INLINE|CM_PARAM), ParseBlock,    NULL           },
  { TidyTag_OL,         "ol",         VERS_ALL,                  &W3CAttrsFor_OL[0],         (CM_BLOCK),                                    ParseList,     NULL           },
  { TidyTag_OPTGROUP,   "optgroup",   (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_OPTGROUP[0],   (CM_FIELD|CM_OPT),                             ParseOptGroup, NULL           },
  { TidyTag_OPTION,     "option",     VERS_ALL,                  &W3CAttrsFor_OPTION[0],     (CM_FIELD|CM_OPT),                             ParseText,     NULL           },
  { TidyTag_P,          "p",          VERS_ALL,                  &W3CAttrsFor_P[0],          (CM_BLOCK|CM_OPT),                             ParseInline,   NULL           },
  { TidyTag_PARAM,      "param",      VERS_FROM32,               &W3CAttrsFor_PARAM[0],      (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyTag_PLAINTEXT,  "plaintext",  VERS_ALL,                  &W3CAttrsFor_PLAINTEXT[0],  (CM_BLOCK|CM_OBSOLETE),                        ParsePre,      NULL           },
  { TidyTag_PRE,        "pre",        VERS_ALL,                  &W3CAttrsFor_PRE[0],        (CM_BLOCK),                                    ParsePre,      NULL           },
  { TidyTag_Q,          "q",          VERS_HTML40,               &W3CAttrsFor_Q[0],          (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_RB,         "rb",         VERS_XHTML11,              &W3CAttrsFor_RB[0],         (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_RBC,        "rbc",        VERS_XHTML11,              &W3CAttrsFor_RBC[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_RP,         "rp",         VERS_XHTML11,              &W3CAttrsFor_RP[0],         (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_RT,         "rt",         VERS_XHTML11,              &W3CAttrsFor_RT[0],         (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_RTC,        "rtc",        VERS_XHTML11,              &W3CAttrsFor_RTC[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_RUBY,       "ruby",       VERS_XHTML11,              &W3CAttrsFor_RUBY[0],       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_S,          "s",          VERS_LOOSE,                &W3CAttrsFor_S[0],          (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_SAMP,       "samp",       VERS_ALL,                  &W3CAttrsFor_SAMP[0],       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_SCRIPT,     "script",     (VERS_FROM32)&~VERS_BASIC, &W3CAttrsFor_SCRIPT[0],     (CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE),         ParseScript,   CheckSCRIPT    },
  { TidyTag_SELECT,     "select",     VERS_ALL,                  &W3CAttrsFor_SELECT[0],     (CM_INLINE|CM_FIELD),                          ParseSelect,   NULL           },
  { TidyTag_SERVER,     "server",     VERS_NETSCAPE,             NULL,                       (CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE),         ParseScript,   NULL           },
  { TidyTag_SERVLET,    "servlet",    VERS_SUN,                  NULL,                       (CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM),         ParseBlock,    NULL           },
  { TidyTag_SMALL,      "small",      (VERS_FROM32)&~VERS_BASIC, &W3CAttrsFor_SMALL[0],      (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_SPACER,     "spacer",     VERS_NETSCAPE,             NULL,                       (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyTag_SPAN,       "span",       VERS_FROM32,               &W3CAttrsFor_SPAN[0],       (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_STRIKE,     "strike",     VERS_LOOSE,                &W3CAttrsFor_STRIKE[0],     (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_STRONG,     "strong",     VERS_ALL,                  &W3CAttrsFor_STRONG[0],     (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_STYLE,      "style",      (VERS_FROM32)&~VERS_BASIC, &W3CAttrsFor_STYLE[0],      (CM_HEAD),                                     ParseScript,   CheckSTYLE     },
  { TidyTag_SUB,        "sub",        (VERS_FROM32)&~VERS_BASIC, &W3CAttrsFor_SUB[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_SUP,        "sup",        (VERS_FROM32)&~VERS_BASIC, &W3CAttrsFor_SUP[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_TABLE,      "table",      VERS_FROM32,               &W3CAttrsFor_TABLE[0],      (CM_BLOCK),                                    ParseTableTag, CheckTABLE     },
  { TidyTag_TBODY,      "tbody",      (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_TBODY[0],      (CM_TABLE|CM_ROWGRP|CM_OPT),                   ParseRowGroup, NULL           },
  { TidyTag_TD,         "td",         VERS_FROM32,               &W3CAttrsFor_TD[0],         (CM_ROW|CM_OPT|CM_NO_INDENT),                  ParseBlock,    CheckTableCell },
  { TidyTag_TEXTAREA,   "textarea",   VERS_ALL,                  &W3CAttrsFor_TEXTAREA[0],   (CM_INLINE|CM_FIELD),                          ParseText,     NULL           },
  { TidyTag_TFOOT,      "tfoot",      (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_TFOOT[0],      (CM_TABLE|CM_ROWGRP|CM_OPT),                   ParseRowGroup, NULL           },
  { TidyTag_TH,         "th",         VERS_FROM32,               &W3CAttrsFor_TH[0],         (CM_ROW|CM_OPT|CM_NO_INDENT),                  ParseBlock,    CheckTableCell },
  { TidyTag_THEAD,      "thead",      (VERS_HTML40)&~VERS_BASIC, &W3CAttrsFor_THEAD[0],      (CM_TABLE|CM_ROWGRP|CM_OPT),                   ParseRowGroup, NULL           },
  { TidyTag_TITLE,      "title",      VERS_ALL,                  &W3CAttrsFor_TITLE[0],      (CM_HEAD),                                     ParseTitle,    NULL           },
  { TidyTag_TR,         "tr",         VERS_FROM32,               &W3CAttrsFor_TR[0],         (CM_TABLE|CM_OPT),                             ParseRow,      NULL           },
  { TidyTag_TT,         "tt",         (VERS_ALL)&~VERS_BASIC,    &W3CAttrsFor_TT[0],         (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_U,          "u",          VERS_LOOSE,                &W3CAttrsFor_U[0],          (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_UL,         "ul",         VERS_ALL,                  &W3CAttrsFor_UL[0],         (CM_BLOCK),                                    ParseList,     NULL           },
  { TidyTag_VAR,        "var",        VERS_ALL,                  &W3CAttrsFor_VAR[0],        (CM_INLINE),                                   ParseInline,   NULL           },
  { TidyTag_WBR,        "wbr",        VERS_PROPRIETARY,          NULL,                       (CM_INLINE|CM_EMPTY),                          ParseEmpty,    NULL           },
  { TidyTag_XMP,        "xmp",        VERS_ALL,                  &W3CAttrsFor_XMP[0],        (CM_BLOCK|CM_OBSOLETE),                        ParsePre,      NULL           },
  { TidyTag_NEXTID,     "nextid",     VERS_HTML20,               &W3CAttrsFor_NEXTID[0],     (CM_HEAD|CM_EMPTY),                            ParseEmpty,    NULL           },

  /* this must be the final entry */
  { 0,                  NULL,         0,                         NULL,                       (0),                                           NULL,          NULL           }
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
        if ( np->id == TidyTag_UNKNOWN )
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
  if ( tid > TidyTag_UNKNOWN && tid < N_TIDY_TAGS )
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
            if ( id == TidyAttr_ALT )
                HasAlt = yes;
            else if ( id == TidyAttr_SRC )
                HasSrc = yes;
            else if ( id == TidyAttr_USEMAP )
                HasUseMap = yes;
            else if ( id == TidyAttr_ISMAP )
                HasIsMap = yes;
            else if ( id == TidyAttr_DATAFLD )
                HasDataFld = yes;
            else if ( id == TidyAttr_WIDTH || id == TidyAttr_HEIGHT )
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
          if ( dict->id == TidyAttr_ALT )
              HasAlt = yes;
          else if ( dict->id == TidyAttr_HREF )
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
        if ( dict && dict->id == TidyAttr_SUMMARY )
            HasSummary = yes;
    }

    /* a missing summary attribute is bad accessibility, no matter
       what HTML version is involved; a document wihtout is valid */
    if (cfg(doc, TidyAccessibilityCheckLevel) == 0)
    {
        Lexer* lexer = doc->lexer;
        if (!HasSummary)
        {
            doc->badAccess |= MISSING_SUMMARY;
            ReportMissingAttr( doc, node, "summary");
        }
    }

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
             tid == TidyTag_H1 ||
             tid == TidyTag_H2 ||
             tid == TidyTag_H3 ||        
             tid == TidyTag_H4 ||        
             tid == TidyTag_H5 ||
             tid == TidyTag_H6 );
}

uint nodeHeaderLevel( Node* node )
{
    TidyTagId tid = TagId( node  );
    switch ( tid )
    {
    case TidyTag_H1:
        return 1;
    case TidyTag_H2:
        return 2;
    case TidyTag_H3:
        return 3;
    case TidyTag_H4:
        return 4;
    case TidyTag_H5:
        return 5;
    case TidyTag_H6:
        return 6;
    }
    return 0;
}
