/* tags.c -- recognize HTML tags

  (c) 1998-2001 (W3C) MIT, INRIA, Keio University
  See tidy.c for the copyright notice.


  The HTML tags are stored as 8 bit ASCII strings.
  Use lookupw() to find a tag given a wide char string.
  
  CVS Info :

    $Author: terry_teague $ 
    $Date: 2001/09/01 04:15:40 $ 
    $Revision: 1.19 $ 

*/


#include "platform.h"   /* platform independent stuff */
#include "html.h"       /* to pull in definition of nodes */

#define HASHSIZE 357

extern Bool XmlTags;

Dict *tag_html;
Dict *tag_head;
Dict *tag_title;
Dict *tag_base;
Dict *tag_meta;
Dict *tag_body;
Dict *tag_frameset;
Dict *tag_frame;
Dict *tag_iframe; /* #433359 - fix by Randy Waki 12 Mar 01 */
Dict *tag_noframes;
Dict *tag_hr;
Dict *tag_h1;
Dict *tag_h2;
Dict *tag_pre;
Dict *tag_listing;
Dict *tag_p;
Dict *tag_ul;
Dict *tag_ol;
Dict *tag_dl;
Dict *tag_dir;
Dict *tag_li;
Dict *tag_dt;
Dict *tag_dd;
Dict *tag_td;
Dict *tag_th;
Dict *tag_tr;
Dict *tag_col;
Dict *tag_br;
Dict *tag_a;
Dict *tag_link;
Dict *tag_b;
Dict *tag_i;
Dict *tag_strong;
Dict *tag_em;
Dict *tag_big;
Dict *tag_small;
Dict *tag_param;
Dict *tag_option;
Dict *tag_optgroup;
Dict *tag_img;
Dict *tag_map;
Dict *tag_area;
Dict *tag_nobr;
Dict *tag_wbr;
Dict *tag_font;
Dict *tag_layer;
Dict *tag_spacer;
Dict *tag_center;
Dict *tag_style;
Dict *tag_script;
Dict *tag_noscript;
Dict *tag_table;
Dict *tag_caption;
Dict *tag_form;
Dict *tag_textarea;
Dict *tag_blockquote;
Dict *tag_applet;
Dict *tag_object;
Dict *tag_div;
Dict *tag_span;
Dict *tag_input;
Dict *tag_q;

Dict *xml_tags;  /* dummy for xml tags */

static Dict *hashtab[HASHSIZE];

/* used by FindFirstDefinedTag and FindNextDefinedTag */
static Dict *tag_blink; /* a proprietary tag added by Tidy, along with tag_nobr, tag_wbr */
static Dict *curDictEntry;
static int curHashIndex;

static struct tag
{
    char *name;
    unsigned versions;
    unsigned model;
    Parser *parser;
    CheckAttribs *chkattrs;
} tags[] =
{
    {"html",       VERS_ALL,     (CM_HTML|CM_OPT|CM_OMITST),  ParseHTML, CheckHTML},

    {"head",       VERS_ALL,     (CM_HTML|CM_OPT|CM_OMITST), ParseHead, null},

    {"title",      VERS_ALL,     CM_HEAD, ParseTitle, null},
    {"base",       VERS_ALL,     (CM_HEAD|CM_EMPTY), ParseEmpty, null},
    {"link",       VERS_ALL,     (CM_HEAD|CM_EMPTY), ParseEmpty, CheckLINK},
    {"meta",       VERS_ALL,     (CM_HEAD|CM_EMPTY), ParseEmpty, CheckMETA},
    {"style",      (VERS_FROM32)&~VERS_BASIC,  CM_HEAD, ParseScript, CheckSTYLE},
    {"script",     (VERS_FROM32)&~VERS_BASIC,  (CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE), ParseScript, CheckSCRIPT},
    {"server",     VERS_NETSCAPE,  (CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE), ParseScript, null},

    {"body",       VERS_ALL,     (CM_HTML|CM_OPT|CM_OMITST), ParseBody, null},
    {"frameset",   VERS_FRAMESET, (CM_HTML|CM_FRAMES), ParseFrameSet, null},

    {"p",          VERS_ALL,     (CM_BLOCK|CM_OPT), ParseInline, null},
    {"h1",         VERS_ALL,     (CM_BLOCK|CM_HEADING), ParseInline, null},
    {"h2",         VERS_ALL,     (CM_BLOCK|CM_HEADING), ParseInline, null},
    {"h3",         VERS_ALL,     (CM_BLOCK|CM_HEADING), ParseInline, null},
    {"h4",         VERS_ALL,     (CM_BLOCK|CM_HEADING), ParseInline, null},
    {"h5",         VERS_ALL,     (CM_BLOCK|CM_HEADING), ParseInline, null},
    {"h6",         VERS_ALL,     (CM_BLOCK|CM_HEADING), ParseInline, null},
    {"ul",         VERS_ALL,     CM_BLOCK, ParseList, null},
    {"ol",         VERS_ALL,     CM_BLOCK, ParseList, null},
    {"dl",         VERS_ALL,     CM_BLOCK, ParseDefList, null},
    {"dir",        VERS_LOOSE,   (CM_BLOCK|CM_OBSOLETE), ParseList, null},
    {"menu",       VERS_LOOSE,   (CM_BLOCK|CM_OBSOLETE), ParseList, null},
    {"pre",        VERS_ALL,     CM_BLOCK, ParsePre, null},
    {"listing",    VERS_ALL,     (CM_BLOCK|CM_OBSOLETE), ParsePre, null},
    {"xmp",        VERS_ALL,     (CM_BLOCK|CM_OBSOLETE), ParsePre, null},
    {"plaintext",  VERS_ALL,     (CM_BLOCK|CM_OBSOLETE), ParsePre, null},
    {"address",    VERS_ALL,     CM_BLOCK, ParseBlock, null},
    {"blockquote", VERS_ALL,     CM_BLOCK, ParseBlock, null},
    {"form",       VERS_ALL,     CM_BLOCK, ParseBlock, CheckFORM},
    {"isindex",    VERS_LOOSE,   (CM_BLOCK|CM_EMPTY), ParseEmpty, null},
    {"fieldset",   (VERS_HTML40)&~VERS_BASIC,  CM_BLOCK, ParseBlock, null},
    {"table",      VERS_FROM32,  CM_BLOCK, ParseTableTag, CheckTABLE},
    {"hr",         (VERS_ALL)&~VERS_BASIC,     (CM_BLOCK|CM_EMPTY), ParseEmpty, CheckHR},
    {"div",        VERS_FROM32,  CM_BLOCK, ParseBlock, null},
    {"multicol",   VERS_NETSCAPE,  CM_BLOCK, ParseBlock, null},
    {"nosave",     VERS_NETSCAPE, CM_BLOCK, ParseBlock, null},
    {"layer",      VERS_NETSCAPE, CM_BLOCK, ParseBlock, null},
    {"ilayer",     VERS_NETSCAPE, CM_INLINE, ParseInline, null},
    {"nolayer",    VERS_NETSCAPE, (CM_BLOCK|CM_INLINE|CM_MIXED), ParseBlock, null},
    {"align",      VERS_NETSCAPE, CM_BLOCK, ParseBlock, null},
    {"center",     VERS_LOOSE,   CM_BLOCK, ParseBlock, null},
    {"ins",        (VERS_HTML40)&~VERS_BASIC,  (CM_INLINE|CM_BLOCK|CM_MIXED), ParseInline, null},
    {"del",        (VERS_HTML40)&~VERS_BASIC,  (CM_INLINE|CM_BLOCK|CM_MIXED), ParseInline, null},

    {"li",         VERS_ALL,     (CM_LIST|CM_OPT|CM_NO_INDENT), ParseBlock, null},
    {"dt",         VERS_ALL,     (CM_DEFLIST|CM_OPT|CM_NO_INDENT), ParseInline, null},
    {"dd",         VERS_ALL,     (CM_DEFLIST|CM_OPT|CM_NO_INDENT), ParseBlock, null},

    {"caption",    VERS_FROM32,  CM_TABLE, ParseInline, CheckCaption},
    {"colgroup",   VERS_HTML40,  (CM_TABLE|CM_OPT), ParseColGroup, null},
    {"col",        VERS_HTML40,  (CM_TABLE|CM_EMPTY), ParseEmpty, null},
    {"thead",      (VERS_HTML40)&~VERS_BASIC,  (CM_TABLE|CM_ROWGRP|CM_OPT), ParseRowGroup, null},
    {"tfoot",      (VERS_HTML40)&~VERS_BASIC,  (CM_TABLE|CM_ROWGRP|CM_OPT), ParseRowGroup, null},
    {"tbody",      (VERS_HTML40)&~VERS_BASIC,  (CM_TABLE|CM_ROWGRP|CM_OPT), ParseRowGroup, null},
    {"tr",         VERS_FROM32,  (CM_TABLE|CM_OPT), ParseRow, null},
    {"td",         VERS_FROM32,  (CM_ROW|CM_OPT|CM_NO_INDENT), ParseBlock, CheckTableCell},
    {"th",         VERS_FROM32,  (CM_ROW|CM_OPT|CM_NO_INDENT), ParseBlock, CheckTableCell},

    {"q",          VERS_HTML40,  CM_INLINE, ParseInline, null},
    {"a",          VERS_ALL,     CM_INLINE, ParseInline, CheckAnchor},
    {"br",         VERS_ALL,     (CM_INLINE|CM_EMPTY), ParseEmpty, null},
    {"img",        VERS_ALL,     (CM_INLINE|CM_IMG|CM_EMPTY), ParseEmpty, CheckIMG},
    {"object",     VERS_HTML40,  (CM_OBJECT|CM_HEAD|CM_IMG|CM_INLINE|CM_PARAM), ParseBlock, null},
    {"applet",     VERS_LOOSE,   (CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM), ParseBlock, null},
    {"servlet",    VERS_SUN,     (CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM), ParseBlock, null},
    {"param",      VERS_FROM32,  (CM_INLINE|CM_EMPTY), ParseEmpty, null},
    {"embed",      VERS_NETSCAPE, (CM_INLINE|CM_IMG|CM_EMPTY), ParseEmpty, null},
    {"noembed",    VERS_NETSCAPE, CM_INLINE, ParseInline, null},
    {"iframe",     VERS_IFRAME,  CM_INLINE, ParseBlock, null},
    {"frame",      VERS_FRAMESET, (CM_FRAMES|CM_EMPTY), ParseEmpty, null},
    {"noframes",   VERS_IFRAME,  (CM_BLOCK|CM_FRAMES), ParseNoFrames,  null},
    {"noscript",   (VERS_HTML40)&~VERS_BASIC,  (CM_BLOCK|CM_INLINE|CM_MIXED), ParseBlock, null},
    {"b",          (VERS_ALL)&~VERS_BASIC,     CM_INLINE, ParseInline, null},
    {"i",          (VERS_ALL)&~VERS_BASIC,     CM_INLINE, ParseInline, null},
    {"u",          VERS_LOOSE,   CM_INLINE, ParseInline, null},
    {"tt",         (VERS_ALL)&~VERS_BASIC,     CM_INLINE, ParseInline, null},
    {"s",          VERS_LOOSE,   CM_INLINE, ParseInline, null},
    {"strike",     VERS_LOOSE,   CM_INLINE, ParseInline, null},
    {"big",        (VERS_FROM32)&~VERS_BASIC,  CM_INLINE, ParseInline, null},
    {"small",      (VERS_FROM32)&~VERS_BASIC,  CM_INLINE, ParseInline, null},
    {"sub",        (VERS_FROM32)&~VERS_BASIC,  CM_INLINE, ParseInline, null},
    {"sup",        (VERS_FROM32)&~VERS_BASIC,  CM_INLINE, ParseInline, null},
    {"em",         VERS_ALL,     CM_INLINE, ParseInline, null},
    {"strong",     VERS_ALL,     CM_INLINE, ParseInline, null},
    {"dfn",        VERS_ALL,     CM_INLINE, ParseInline, null},
    {"code",       VERS_ALL,     CM_INLINE, ParseInline, null},
    {"samp",       VERS_ALL,     CM_INLINE, ParseInline, null},
    {"kbd",        VERS_ALL,     CM_INLINE, ParseInline, null},
    {"var",        VERS_ALL,     CM_INLINE, ParseInline, null},
    {"cite",       VERS_ALL,     CM_INLINE, ParseInline, null},
    {"abbr",       VERS_HTML40,  CM_INLINE, ParseInline, null},
    {"acronym",    VERS_HTML40,  CM_INLINE, ParseInline, null},
    {"span",       VERS_FROM32,  CM_INLINE, ParseInline, null},
    {"blink",      VERS_PROPRIETARY, CM_INLINE, ParseInline, null},
    {"nobr",       VERS_PROPRIETARY, CM_INLINE, ParseInline, null},
    {"wbr",        VERS_PROPRIETARY, (CM_INLINE|CM_EMPTY), ParseEmpty, null},
    {"marquee",    VERS_MICROSOFT, (CM_INLINE|CM_OPT), ParseInline, null},
    {"bgsound",    VERS_MICROSOFT, (CM_HEAD|CM_EMPTY), ParseEmpty, null},
    {"comment",    VERS_MICROSOFT, CM_INLINE, ParseInline, null},
    {"spacer",     VERS_NETSCAPE, (CM_INLINE|CM_EMPTY), ParseEmpty, null},
    {"keygen",     VERS_NETSCAPE, (CM_INLINE|CM_EMPTY), ParseEmpty, null},
    /* next 2 are already defined above - does no harm though */
    {"nolayer",    VERS_NETSCAPE, (CM_BLOCK|CM_INLINE|CM_MIXED), ParseBlock, null},
    {"ilayer",     VERS_NETSCAPE, CM_INLINE, ParseInline, null},
    {"map",        (VERS_FROM32)&~VERS_BASIC,  CM_INLINE, ParseBlock, CheckMap},
    {"area",       (VERS_ALL)&~VERS_BASIC,     (CM_BLOCK|CM_EMPTY), ParseEmpty, CheckAREA},
    {"input",      VERS_ALL,     (CM_INLINE|CM_IMG|CM_EMPTY), ParseEmpty, null},
    {"select",     VERS_ALL,     (CM_INLINE|CM_FIELD), ParseSelect, null},
    {"option",     VERS_ALL,     (CM_FIELD|CM_OPT), ParseText, null},
    {"optgroup",   (VERS_HTML40)&~VERS_BASIC,  (CM_FIELD|CM_OPT), ParseOptGroup, null},
    {"textarea",   VERS_ALL,     (CM_INLINE|CM_FIELD), ParseText, null},
    {"label",      VERS_HTML40,  CM_INLINE, ParseInline, null},
    {"legend",     (VERS_HTML40)&~VERS_BASIC,  CM_INLINE, ParseInline, null},
    {"button",     (VERS_HTML40)&~VERS_BASIC,  CM_INLINE, ParseInline, null},
    {"basefont",   VERS_LOOSE,   (CM_INLINE|CM_EMPTY), ParseEmpty, null},
    {"font",       VERS_LOOSE,   CM_INLINE, ParseInline, null},
    {"bdo",        (VERS_HTML40)&~VERS_BASIC,  CM_INLINE, ParseInline, null},

  /* elements for XHTML 1.1 */

    {"ruby",       VERS_XHTML11, CM_INLINE, ParseInline, null},
    {"rbc",        VERS_XHTML11, CM_INLINE, ParseInline, null}, 
    {"rtc",        VERS_XHTML11, CM_INLINE, ParseInline, null},
    {"rb",         VERS_XHTML11, CM_INLINE, ParseInline, null},
    {"rt",         VERS_XHTML11, CM_INLINE, ParseInline, null},
    {"rp",         VERS_XHTML11, CM_INLINE, ParseInline, null},


  /* this must be the final entry */
    {null,         0,            0,          0,       0}
};

/* choose what version to use for new doctype */
int HTMLVersion(Lexer *lexer)
{
    uint versions;

    versions = lexer->versions;

    if (versions & VERS_HTML20)
        return VERS_HTML20;

    if (!(XmlOut|XmlTags|lexer->isvoyager) &&
        versions & VERS_HTML32)
        return VERS_HTML32;

    if (versions & VERS_XHTML11)
        return VERS_XHTML11;

    if (versions & VERS_HTML40_STRICT)
        return VERS_HTML40_STRICT;

    if (versions & VERS_HTML40_LOOSE)
        return VERS_HTML40_LOOSE;

    if (versions & VERS_FRAMESET)
        return VERS_FRAMESET;

    return VERS_UNKNOWN;
}

static unsigned hash(char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31*hashval;

    return hashval % HASHSIZE;
}

static Dict *lookup(char *s)
{
    Dict *np;

    for (np = hashtab[hash(s)]; np != null; np = np->next)
        if (wstrcmp(s, np->name) == 0)
            return np;
    return null;
}

static Dict *install(char *name, uint versions, uint model, 
                     Parser *parser, CheckAttribs *chkattrs)
{
    Dict *np;
    unsigned hashval;

    if ((np = lookup(name)) == null)
    {
        np = (Dict *)MemAlloc(sizeof(*np));

        if (np == null || (np->name = wstrdup(name)) == null)
            return null;

        hashval = hash(name);
        np->next = hashtab[hashval];
        np->model = 0;
        hashtab[hashval] = np;
    }

    np->versions = versions;
    np->model |= model;
    np->parser = parser;
    np->chkattrs = chkattrs;
    return np;
}

/* public interface for finding tag by name */
Bool FindTag(Node *node)
{
    Dict *np;

    if (XmlTags)
    {
        node->tag = xml_tags;
        return yes;
    }

    if (node->element && (np = lookup(node->element)))
    {
        node->tag = np;
        return yes;
    }

    return no;
}

Parser *FindParser(Node *node)
{
        Dict *np;

        if (node->element && (np = lookup(node->element)))
            return np->parser;

        return null;
}

void DefineTag(int tagType, char *name)
{
    switch (tagType)
    {
        case tagtype_empty :
            install(name, VERS_PROPRIETARY, (CM_EMPTY|CM_NO_INDENT|CM_NEW), ParseBlock, null);
            break;
        case tagtype_inline :
            install(name, VERS_PROPRIETARY, (CM_INLINE|CM_NO_INDENT|CM_NEW), ParseInline, null);
            break;
        case tagtype_block :
            install(name, VERS_PROPRIETARY, (CM_BLOCK|CM_NO_INDENT|CM_NEW), ParseBlock, null);
            break;
        case tagtype_pre :
            install(name, VERS_PROPRIETARY, (CM_BLOCK|CM_NO_INDENT|CM_NEW), ParsePre, null);
            break;
    }
}

void ResetDefinedTagSearch(void)
{
    curDictEntry = null;
    curHashIndex = 0;
}

char *FindNextDefinedTag(int tagType)
{
    char *tagName = null;
    
    do
    {
        if (curDictEntry != null)
        {
            switch (tagType)
            {
                /* defined tags can be empty + inline */
                case tagtype_empty :
                    if ((curDictEntry->versions == VERS_PROPRIETARY) &&
                        ((curDictEntry->model & CM_EMPTY) == CM_EMPTY) &&
                        /* (curDictEntry->parser == ParseBlock) && */
                        (curDictEntry != tag_wbr))
                        tagName = curDictEntry->name;
                    break;
                /* defined tags can be empty + inline */
                case tagtype_inline :
                    if ((curDictEntry->versions == VERS_PROPRIETARY) &&
                        ((curDictEntry->model & CM_INLINE) == CM_INLINE) &&
                        /* (curDictEntry->parser == ParseInline) && */
                        (curDictEntry != tag_blink) &&
                        (curDictEntry != tag_nobr) &&
                        (curDictEntry != tag_wbr))
                        tagName = curDictEntry->name;
                    break;
                /* defined tags can be empty + block */
                case tagtype_block :
                    if ((curDictEntry->versions == VERS_PROPRIETARY) &&
                        ((curDictEntry->model & CM_BLOCK) == CM_BLOCK) &&
                        (curDictEntry->parser == ParseBlock))
                        tagName = curDictEntry->name;
                    break;
                case tagtype_pre :
                    if ((curDictEntry->versions == VERS_PROPRIETARY) &&
                        ((curDictEntry->model & CM_BLOCK) == CM_BLOCK) &&
                        (curDictEntry->parser == ParsePre))
                        tagName = curDictEntry->name;
                    break;
            }
            
            curDictEntry = curDictEntry->next;
        }
        
        if (curDictEntry == null)
            do
            {
                curDictEntry = hashtab[curHashIndex++];
            } while ((curDictEntry == null) && (curHashIndex < HASHSIZE));
    
    } while ((tagName == null) && (curDictEntry != null));
    
    return tagName;
}

void InitTags(void)
{
    struct tag *tp;
    
    for(tp = tags; tp->name != null; ++tp)
        install(tp->name, tp->versions, tp->model, tp->parser, tp->chkattrs);

    tag_html = lookup("html");
    tag_head = lookup("head");
    tag_body = lookup("body");
    tag_frameset = lookup("frameset");
    tag_frame = lookup("frame");
    tag_iframe = lookup("iframe"); /* #433359 - fix by Randy Waki 12 Mar 01 */
    tag_noframes = lookup("noframes");
    tag_meta = lookup("meta");
    tag_title = lookup("title");
    tag_base = lookup("base");
    tag_hr = lookup("hr");
    tag_pre = lookup("pre");
    tag_listing = lookup("listing");
    tag_h1 = lookup("h1");
    tag_h2 = lookup("h2");
    tag_p  = lookup("p");
    tag_ul = lookup("ul");
    tag_ol = lookup("ol");
    tag_dir = lookup("dir");
    tag_li = lookup("li");
    tag_dl = lookup("dl");
    tag_dt = lookup("dt");
    tag_dd = lookup("dd");
    tag_td = lookup("td");
    tag_th = lookup("th");
    tag_tr = lookup("tr");
    tag_col = lookup("col");
    tag_br = lookup("br");
    tag_a = lookup("a");
    tag_link = lookup("link");
    tag_b = lookup("b");
    tag_i = lookup("i");
    tag_strong = lookup("strong");
    tag_em = lookup("em");
    tag_big = lookup("big");
    tag_small = lookup("small");
    tag_param = lookup("param");
    tag_option = lookup("option");
    tag_optgroup = lookup("optgroup");
    tag_img = lookup("img");
    tag_map = lookup("map");
    tag_area = lookup("area");
    tag_nobr = lookup("nobr");
    tag_wbr = lookup("wbr");
    tag_font = lookup("font");
    tag_spacer = lookup("spacer");
    tag_layer = lookup("layer");
    tag_center = lookup("center");
    tag_style = lookup("style");
    tag_script = lookup("script");
    tag_noscript = lookup("noscript");
    tag_table = lookup("table");
    tag_caption = lookup("caption");
    tag_form = lookup("form");
    tag_textarea = lookup("textarea");
    tag_blockquote = lookup("blockquote");
    tag_applet = lookup("applet");
    tag_object = lookup("object");
    tag_div = lookup("div");
    tag_span = lookup("span");
    tag_input = lookup("input");
    tag_q = lookup("q");

    /* create dummy entry for all xml tags */
    xml_tags = (Dict *)MemAlloc(sizeof(*xml_tags));
    xml_tags->name = null;
    xml_tags->versions = VERS_ALL;
    xml_tags->model = CM_BLOCK;
    xml_tags->parser = null;
    xml_tags->chkattrs = null;
    
    tag_blink = lookup("blink"); /* so we can skip this in the search for user defined tags */
}

void FreeTags(void)
{
    Dict *prev, *next;
    int i;

    MemFree(xml_tags);

    for (i = 0; i < HASHSIZE; ++i)
    {
        prev = null;
        next = hashtab[i];

        while(next)
        {
            prev = next->next;
            MemFree(next->name);
            MemFree(next);
            next = prev;
        }

        hashtab[i] = null;
    }
}
