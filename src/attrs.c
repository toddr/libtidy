/* attrs.c -- recognize HTML attributes

  (c) 1998-2003 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: hoehrmann $ 
    $Date: 2003/03/30 23:57:25 $ 
    $Revision: 1.57 $ 

*/

#include "tidy-int.h"
#include "attrs.h"
#include "message.h"
#include "tmbstr.h"

#if 0
#define TEXT        NULL
#define CHARSET     NULL
#define TYPE        NULL
#define CHARACTER   NULL
#define URLS        NULL
#define URL         CheckUrl
#define SCRIPT      CheckScript
#define ALIGN       CheckAlign
#define VALIGN      CheckValign
#define COLOR       CheckColor
#define CLEAR       CheckClear
#define BORDER      CheckBool     /* kludge */
#define LANG        CheckLang
#define BOOL        CheckBool
#define COLS        NULL
#define NUMBER      CheckNumber
#define LENGTH      CheckLength
#define COORDS      NULL
#define DATE        NULL
#define TEXTDIR     CheckTextDir
#define IDREFS      NULL
#define IDREF       NULL
#define IDDEF       CheckId
#define NAME        CheckName
#define TFRAME      NULL
#define FBORDER     NULL
#define MEDIA       NULL
#define FSUBMIT     CheckFsubmit
#define LINKTYPES   NULL
#define TRULES      NULL
#define SCOPE       CheckScope
#define SHAPE       CheckShape
#define SCROLL      CheckScroll
#define TARGET      CheckTarget
#define VTYPE       CheckVType
#endif
/*
*/

static const Attribute attribute_defs [] =
{
  {TidyAttr_unknown,  "unknown!",     VERS_PROPRIETARY,  NULL},
  {TidyAttr_abbr,     "abbr",         VERS_HTML40,       TEXT},
  {TidyAttr_accept,   "accept",       VERS_ALL,          TYPE},
  {TidyAttr_accept_charset, "accept-charset", VERS_HTML40, CHARSET},
  {TidyAttr_accesskey, "accesskey",   VERS_HTML40,       CHARACTER},
  {TidyAttr_action,   "action",       VERS_ALL,          URL},
  {TidyAttr_add_date, "add_date",     VERS_NETSCAPE,     TEXT},     /* A */
  {TidyAttr_align,    "align",        VERS_ALL,          ALIGN},    /* varies by element */
  {TidyAttr_alink,    "alink",        VERS_LOOSE,        COLOR},
  {TidyAttr_alt,      "alt",          VERS_ALL,          TEXT,  yes }, /* nowrap */
  {TidyAttr_archive,  "archive",      VERS_HTML40,       URLS},     /* space or comma separated list */
  {TidyAttr_axis,     "axis",         VERS_HTML40,       TEXT},
  {TidyAttr_background, "background", VERS_LOOSE,        URL},
  {TidyAttr_bgcolor,  "bgcolor",      VERS_LOOSE,        COLOR},
  {TidyAttr_bgproperties, "bgproperties", VERS_PROPRIETARY, TEXT},  /* BODY "fixed" fixes background */
  {TidyAttr_border,   "border",       VERS_ALL,          BORDER},   /* like LENGTH + "border" */
  {TidyAttr_bordercolor, "bordercolor",  VERS_MICROSOFT, COLOR},    /* used on TABLE */
  {TidyAttr_bottommargin, "bottommargin",VERS_MICROSOFT, NUMBER},   /* used on BODY */
  {TidyAttr_cellpadding, "cellpadding", VERS_FROM32,     LENGTH},   /* % or pixel values */
  {TidyAttr_cellspacing, "cellspacing", VERS_FROM32,     LENGTH},
  {TidyAttr_char,     "char",         VERS_HTML40,       CHARACTER},
  {TidyAttr_charoff,  "charoff",      VERS_HTML40,       LENGTH},
  {TidyAttr_charset,  "charset",      VERS_HTML40,       CHARSET},
  {TidyAttr_checked,  "checked",      VERS_ALL,          BOOL},     /* i.e. "checked" or absent */
  {TidyAttr_cite,     "cite",         VERS_HTML40,       URL},
  {TidyAttr_class,    "class",        VERS_HTML40,       TEXT},
  {TidyAttr_classid,  "classid",      VERS_HTML40,       URL},
  {TidyAttr_clear,    "clear",        VERS_LOOSE,        CLEAR},    /* BR: left, right, all */
  {TidyAttr_code,     "code",         VERS_LOOSE,        TEXT},     /* APPLET */
  {TidyAttr_codebase, "codebase",     VERS_HTML40,       URL},      /* OBJECT */
  {TidyAttr_codetype, "codetype",     VERS_HTML40,       TYPE},     /* OBJECT */
  {TidyAttr_color,    "color",        VERS_LOOSE,        COLOR},    /* BASEFONT, FONT */
  {TidyAttr_cols,     "cols",         VERS_IFRAME,       COLS},     /* TABLE & FRAMESET */
  {TidyAttr_colspan,  "colspan",      VERS_FROM32,       NUMBER},
  {TidyAttr_compact,  "compact",      VERS_ALL,          BOOL},     /* lists */
  {TidyAttr_content,  "content",      VERS_ALL,          TEXT, yes},/* META, nowrap */
  {TidyAttr_coords,   "coords",       VERS_FROM32,       COORDS},   /* AREA, A */    
  {TidyAttr_data,     "data",         VERS_HTML40,       URL},      /* OBJECT */
  {TidyAttr_datafld,  "datafld",      VERS_MICROSOFT,    TEXT},     /* used on DIV, IMG */
  {TidyAttr_dataformatas, "dataformatas", VERS_MICROSOFT,TEXT},     /* used on DIV, IMG */
  {TidyAttr_datapagesize, "datapagesize", VERS_MICROSOFT,NUMBER},   /* used on DIV, IMG */
  {TidyAttr_datasrc,  "datasrc",      VERS_MICROSOFT,    URL},      /* used on TABLE */
  {TidyAttr_datetime, "datetime",     VERS_HTML40,       DATE},     /* INS, DEL */
  {TidyAttr_declare,  "declare",      VERS_HTML40,       BOOL},     /* OBJECT */
  {TidyAttr_defer,    "defer",        VERS_HTML40,       BOOL},     /* SCRIPT */
  {TidyAttr_dir,      "dir",          VERS_HTML40,       TEXTDIR},  /* ltr or rtl */
  {TidyAttr_disabled, "disabled",     VERS_HTML40,       BOOL},     /* form fields */
  {TidyAttr_encoding, "encoding",     VERS_XML,          TEXT},     /* <?xml?> */
  {TidyAttr_enctype,  "enctype",      VERS_ALL,          TYPE},     /* FORM */
  {TidyAttr_face,     "face",         VERS_LOOSE,        TEXT},     /* BASEFONT, FONT */
  {TidyAttr_for,      "for",          VERS_HTML40,       IDREF},    /* LABEL */
  {TidyAttr_frame,    "frame",        VERS_HTML40,       TFRAME},   /* TABLE */
  {TidyAttr_frameborder, "frameborder", VERS_FRAMESET,   FBORDER},  /* 0 or 1 */
  {TidyAttr_framespacing, "framespacing", VERS_PROPRIETARY, NUMBER},/* pixel value */
  {TidyAttr_gridx,    "gridx",        VERS_PROPRIETARY,  NUMBER},   /* TABLE Adobe golive*/
  {TidyAttr_gridy,    "gridy",        VERS_PROPRIETARY,  NUMBER},   /* TABLE Adobe golive */
  {TidyAttr_headers,  "headers",      VERS_HTML40,       IDREFS},   /* table cells */
  {TidyAttr_height,   "height",       VERS_ALL,          LENGTH},   /* pixels only for TH/TD */
  {TidyAttr_href,     "href",         VERS_ALL,          URL},      /* A, AREA, LINK and BASE */
  {TidyAttr_hreflang, "hreflang",     VERS_HTML40,       LANG},     /* A, LINK */
  {TidyAttr_hspace,   "hspace",       VERS_ALL,          NUMBER},   /* APPLET, IMG, OBJECT */
  {TidyAttr_http_equiv, "http-equiv", VERS_ALL,          TEXT},     /* META */
  {TidyAttr_id,       "id",           VERS_HTML40,       IDDEF},
  {TidyAttr_ismap,    "ismap",        VERS_ALL,          BOOL},     /* IMG */
  {TidyAttr_label,    "label",        VERS_HTML40,       TEXT},     /* OPT, OPTGROUP */
  {TidyAttr_lang,     "lang",         VERS_HTML40,       LANG},
  {TidyAttr_language, "language",     VERS_LOOSE,        TEXT},     /* SCRIPT */
  {TidyAttr_last_modified, "last_modified", VERS_NETSCAPE, TEXT},   /* A */
  {TidyAttr_last_visit, "last_visit", VERS_NETSCAPE,     TEXT},     /* A */
  {TidyAttr_leftmargin, "leftmargin", VERS_MICROSOFT,    NUMBER},   /* used on BODY */
  {TidyAttr_link,     "link",         VERS_LOOSE,        COLOR},    /* BODY */
  {TidyAttr_longdesc, "longdesc",     VERS_HTML40,       URL},      /* IMG */
  {TidyAttr_lowsrc,   "lowsrc",       VERS_PROPRIETARY,  URL},      /* IMG */
  {TidyAttr_marginheight, "marginheight", VERS_IFRAME,   NUMBER},   /* FRAME, IFRAME, BODY */
  {TidyAttr_marginwidth, "marginwidth", VERS_IFRAME,     NUMBER},   /* ditto */
  {TidyAttr_maxlength, "maxlength",   VERS_ALL,          NUMBER},   /* INPUT */
  {TidyAttr_media,    "media",        VERS_HTML40,       MEDIA},    /* STYLE, LINK */
  {TidyAttr_method,   "method",       VERS_ALL,          FSUBMIT},  /* FORM: get or post */
  {TidyAttr_multiple, "multiple",     VERS_ALL,          BOOL},     /* SELECT */
  {TidyAttr_name,     "name",         VERS_ALL,          NAME},
  {TidyAttr_nohref,   "nohref",       VERS_FROM32,       BOOL},     /* AREA */
  {TidyAttr_noresize, "noresize",     VERS_FRAMESET,     BOOL},     /* FRAME */
  {TidyAttr_noshade,  "noshade",      VERS_LOOSE,        BOOL},     /* HR */
  {TidyAttr_nowrap,   "nowrap",       VERS_LOOSE,        BOOL},     /* table cells */
  {TidyAttr_object,   "object",       VERS_HTML40_LOOSE, TEXT},     /* APPLET */
  {TidyAttr_onafterupdate, "onafterupdate", VERS_MICROSOFT, SCRIPT},/* form fields */
  {TidyAttr_onbeforeunload, "onbeforeunload", VERS_MICROSOFT, SCRIPT},/* form fields */
  {TidyAttr_onbeforeupdate, "onbeforeupdate", VERS_MICROSOFT, SCRIPT},/* form fields */
  {TidyAttr_onblur,   "onblur",       VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onchange, "onchange",     VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onclick,  "onclick",      VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_ondataavailable, "ondataavailable", VERS_MICROSOFT, SCRIPT}, /* object, applet */
  {TidyAttr_ondatasetchanged, "ondatasetchanged", VERS_MICROSOFT, SCRIPT}, /* object, applet */
  {TidyAttr_ondatasetcomplete, "ondatasetcomplete", VERS_MICROSOFT, SCRIPT},/* object, applet */
  {TidyAttr_ondblclick, "ondblclick", VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onerrorupdate, "onerrorupdate", VERS_MICROSOFT, SCRIPT},   /* form fields */
  {TidyAttr_onfocus,  "onfocus",      VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onkeydown,"onkeydown",    VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onkeypress, "onkeypress", VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onkeyup,  "onkeyup",      VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onload,   "onload",       VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onmousedown, "onmousedown", VERS_EVENTS,     SCRIPT},   /* event */
  {TidyAttr_onmousemove, "onmousemove", VERS_EVENTS,     SCRIPT},   /* event */
  {TidyAttr_onmouseout, "onmouseout", VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onmouseover, "onmouseover", VERS_EVENTS,     SCRIPT},   /* event */
  {TidyAttr_onmouseup, "onmouseup",   VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onreset,  "onreset",      VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onrowenter, "onrowenter", VERS_MICROSOFT,    SCRIPT},   /* form fields */
  {TidyAttr_onrowexit, "onrowexit",   VERS_MICROSOFT,    SCRIPT},   /* form fields */
  {TidyAttr_onselect, "onselect",     VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onsubmit, "onsubmit",     VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_onunload, "onunload",     VERS_EVENTS,       SCRIPT},   /* event */
  {TidyAttr_profile,  "profile",      VERS_HTML40,       URL},      /* HEAD */
  {TidyAttr_prompt,   "prompt",       VERS_LOOSE,        TEXT},     /* ISINDEX */
  {TidyAttr_rbspan,   "rbspan",       VERS_XHTML11,      NUMBER},   /* ruby markup */
  {TidyAttr_readonly, "readonly",     VERS_HTML40,       BOOL},     /* form fields */
  {TidyAttr_rel,      "rel",          VERS_ALL,          LINKTYPES},/* A, LINK */
  {TidyAttr_rev,      "rev",          VERS_ALL,          LINKTYPES},/* A, LINK */
  {TidyAttr_rightmargin, "rightmargin", VERS_MICROSOFT,  NUMBER},   /* used on BODY */
  {TidyAttr_rows,     "rows",         VERS_ALL,          NUMBER},   /* TEXTAREA */
  {TidyAttr_rowspan,  "rowspan",      VERS_ALL,          NUMBER},   /* table cells */
  {TidyAttr_rules,    "rules",        VERS_HTML40,       TRULES},   /* TABLE */
  {TidyAttr_scheme,   "scheme",       VERS_HTML40,       TEXT},     /* META */
  {TidyAttr_scope,    "scope",        VERS_HTML40,       SCOPE},    /* table cells */
  {TidyAttr_scrolling, "scrolling",   VERS_IFRAME,       SCROLL},   /* yes, no or auto */
  {TidyAttr_selected, "selected",     VERS_ALL,          BOOL},     /* OPTION */
  {TidyAttr_shape,    "shape",        VERS_FROM32,       SHAPE},    /* AREA, A */
  {TidyAttr_showgrid, "showgrid",     VERS_PROPRIETARY,  BOOL},     /* TABLE Adobe golive */
  {TidyAttr_showgridx,"showgridx",    VERS_PROPRIETARY,  BOOL},     /* TABLE Adobe golive*/
  {TidyAttr_showgridy,"showgridy",    VERS_PROPRIETARY,  BOOL},     /* TABLE Adobe golive*/
  {TidyAttr_size,     "size",         VERS_LOOSE,        NUMBER},   /* HR, FONT, BASEFONT, SELECT */
  {TidyAttr_span,     "span",         VERS_HTML40,       NUMBER},   /* COL, COLGROUP */
  {TidyAttr_src,      "src",          VERS_ALL,          URL},      /* IMG, FRAME, IFRAME */
  {TidyAttr_standby,  "standby",      VERS_HTML40,       TEXT},     /* OBJECT */
  {TidyAttr_start,    "start",        VERS_ALL,          NUMBER},   /* OL */
  {TidyAttr_style,    "style",        VERS_HTML40,       TEXT},
  {TidyAttr_summary,  "summary",      VERS_HTML40,       TEXT},     /* TABLE */
  {TidyAttr_tabindex, "tabindex",     VERS_HTML40,       NUMBER},   /* fields, OBJECT  and A */
  {TidyAttr_target,   "target",       VERS_HTML40,       TARGET},   /* names a frame/window */
  {TidyAttr_text,     "text",         VERS_LOOSE,        COLOR},    /* BODY */
  {TidyAttr_title,    "title",        VERS_HTML40,       TEXT},     /* text tool tip */
  {TidyAttr_topmargin,"topmargin",    VERS_MICROSOFT,    NUMBER},   /* used on BODY */
  {TidyAttr_type,     "type",         VERS_FROM32,       TYPE},     /* also used by SPACER */
  {TidyAttr_usemap,   "usemap",       VERS_ALL,          BOOL},     /* things with images */
  {TidyAttr_valign,   "valign",       VERS_FROM32,       VALIGN},
  {TidyAttr_value,    "value",        VERS_ALL,          TEXT, yes},/* nowrap, OPTION, PARAM */
  {TidyAttr_valuetype,"valuetype",    VERS_HTML40,       VTYPE},    /* PARAM: data, ref, object */
  {TidyAttr_version,  "version",      VERS_ALL|VERS_XML, TEXT},     /* HTML <?xml?> */
  {TidyAttr_vlink,    "vlink",        VERS_LOOSE,        COLOR},    /* BODY */
  {TidyAttr_vspace,   "vspace",       VERS_LOOSE,        NUMBER},   /* IMG, OBJECT, APPLET */
  {TidyAttr_width,    "width",        VERS_ALL,          LENGTH},   /* pixels only for TD/TH */
  {TidyAttr_wrap,     "wrap",         VERS_NETSCAPE,     TEXT},     /* textarea */
  {TidyAttr_xml_lang, "xml:lang",     VERS_XML,          TEXT},     /* XML language */
  {TidyAttr_xml_space,"xml:space",    VERS_XML,          TEXT},     /* XML language */
  {TidyAttr_xmlns,    "xmlns",        VERS_ALL,          TEXT},     /* name space */
   
  /* this must be the final entry */
  {N_TIDY_ATTRIBS,    NULL}
};

/* used by CheckColor() */
struct _colors
{
    ctmbstr name;
    ctmbstr hex;
};

static const struct _colors colors[] =
{
    {"black",   "#000000"},
    {"green",   "#008000"},
    {"silver",  "#C0C0C0"},
    {"lime",    "#00FF00"},
    {"gray",    "#808080"},
    {"olive",   "#808000"},
    {"white",   "#FFFFFF"},
    {"yellow",  "#FFFF00"},
    {"maroon",  "#800000"},
    {"navy",    "#000080"},
    {"red",     "#FF0000"},
    {"blue",    "#0000FF"},
    {"purple",  "#800080"},
    {"teal",    "#008080"},
    {"fuchsia", "#FF00FF"},
    {"aqua",    "#00FFFF"},
    {NULL,      NULL}
};

static const struct _colors fancy_colors[] =
{
    { "darkgreen",            "#006400" },
    { "antiquewhite",         "#FAEBD7" },
    { "aqua",                 "#00FFFF" },
    { "aquamarine",           "#7FFFD4" },
    { "azure",                "#F0FFFF" },
    { "beige",                "#F5F5DC" },
    { "bisque",               "#FFE4C4" },
    { "black",                "#000000" },
    { "blanchedalmond",       "#FFEBCD" },
    { "blue",                 "#0000FF" },
    { "blueviolet",           "#8A2BE2" },
    { "brown",                "#A52A2A" },
    { "burlywood",            "#DEB887" },
    { "cadetblue",            "#5F9EA0" },
    { "chartreuse",           "#7FFF00" },
    { "chocolate",            "#D2691E" },
    { "coral",                "#FF7F50" },
    { "cornflowerblue",       "#6495ED" },
    { "cornsilk",             "#FFF8DC" },
    { "crimson",              "#DC143C" },
    { "cyan",                 "#00FFFF" },
    { "darkblue",             "#00008B" },
    { "darkcyan",             "#008B8B" },
    { "darkgoldenrod",        "#B8860B" },
    { "darkgray",             "#A9A9A9" },
    { "darkgreen",            "#006400" },
    { "darkkhaki",            "#BDB76B" },
    { "darkmagenta",          "#8B008B" },
    { "darkolivegreen",       "#556B2F" },
    { "darkorange",           "#FF8C00" },
    { "darkorchid",           "#9932CC" },
    { "darkred",              "#8B0000" },
    { "darksalmon",           "#E9967A" },
    { "darkseagreen",         "#8FBC8F" },
    { "darkslateblue",        "#483D8B" },
    { "darkslategray",        "#2F4F4F" },
    { "darkturquoise",        "#00CED1" },
    { "darkviolet",           "#9400D3" },
    { "deeppink",             "#FF1493" },
    { "deepskyblue",          "#00BFFF" },
    { "dimgray",              "#696969" },
    { "dodgerblue",           "#1E90FF" },
    { "firebrick",            "#B22222" },
    { "floralwhite",          "#FFFAF0" },
    { "forestgreen",          "#228B22" },
    { "fuchsia",              "#FF00FF" },
    { "gainsboro",            "#DCDCDC" },
    { "ghostwhite",           "#F8F8FF" },
    { "gold",                 "#FFD700" },
    { "goldenrod",            "#DAA520" },
    { "gray",                 "#808080" },
    { "green",                "#008000" },
    { "greenyellow",          "#ADFF2F" },
    { "honeydew",             "#F0FFF0" },
    { "hotpink",              "#FF69B4" },
    { "indianred",            "#CD5C5C" },
    { "indigo",               "#4B0082" },
    { "ivory",                "#FFFFF0" },
    { "khaki",                "#F0E68C" },
    { "lavender",             "#E6E6FA" },
    { "lavenderblush",        "#FFF0F5" },
    { "lawngreen",            "#7CFC00" },
    { "lemonchiffon",         "#FFFACD" },
    { "lightblue",            "#ADD8E6" },
    { "lightcoral",           "#F08080" },
    { "lightcyan",            "#E0FFFF" },
    { "lightgoldenrodyellow", "#FAFAD2" },
    { "lightgreen",           "#90EE90" },
    { "lightgrey",            "#D3D3D3" },
    { "lightpink",            "#FFB6C1" },
    { "lightsalmon",          "#FFA07A" },
    { "lightseagreen",        "#20B2AA" },
    { "lightskyblue",         "#87CEFA" },
    { "lightslategray",       "#778899" },
    { "lightsteelblue",       "#B0C4DE" },
    { "lightyellow",          "#FFFFE0" },
    { "lime",                 "#00FF00" },
    { "limegreen",            "#32CD32" },
    { "linen",                "#FAF0E6" },
    { "magenta",              "#FF00FF" },
    { "maroon",               "#800000" },
    { "mediumaquamarine",     "#66CDAA" },
    { "mediumblue",           "#0000CD" },
    { "mediumorchid",         "#BA55D3" },
    { "mediumpurple",         "#9370DB" },
    { "mediumseagreen",       "#3CB371" },
    { "mediumslateblue",      "#7B68EE" },
    { "mediumspringgreen",    "#00FA9A" },
    { "mediumturquoise",      "#48D1CC" },
    { "mediumvioletred",      "#C71585" },
    { "midnightblue",         "#191970" },
    { "mintcream",            "#F5FFFA" },
    { "mistyrose",            "#FFE4E1" },
    { "moccasin",             "#FFE4B5" },
    { "navajowhite",          "#FFDEAD" },
    { "navy",                 "#000080" },
    { "oldlace",              "#FDF5E6" },
    { "olive",                "#808000" },
    { "olivedrab",            "#6B8E23" },
    { "orange",               "#FFA500" },
    { "orangered",            "#FF4500" },
    { "orchid",               "#DA70D6" },
    { "palegoldenrod",        "#EEE8AA" },
    { "palegreen",            "#98FB98" },
    { "paleturquoise",        "#AFEEEE" },
    { "palevioletred",        "#DB7093" },
    { "papayawhip",           "#FFEFD5" },
    { "peachpuff",            "#FFDAB9" },
    { "peru",                 "#CD853F" },
    { "pink",                 "#FFC0CB" },
    { "plum",                 "#DDA0DD" },
    { "powderblue",           "#B0E0E6" },
    { "purple",               "#800080" },
    { "red",                  "#FF0000" },
    { "rosybrown",            "#BC8F8F" },
    { "royalblue",            "#4169E1" },
    { "saddlebrown",          "#8B4513" },
    { "salmon",               "#FA8072" },
    { "sandybrown",           "#F4A460" },
    { "seagreen",             "#2E8B57" },
    { "seashell",             "#FFF5EE" },
    { "sienna",               "#A0522D" },
    { "silver",               "#C0C0C0" },
    { "skyblue",              "#87CEEB" },
    { "slateblue",            "#6A5ACD" },
    { "slategray",            "#708090" },
    { "snow",                 "#FFFAFA" },
    { "springgreen",          "#00FF7F" },
    { "steelblue",            "#4682B4" },
    { "tan",                  "#D2B48C" },
    { "teal",                 "#008080" },
    { "thistle",              "#D8BFD8" },
    { "tomato",               "#FF6347" },
    { "turquoise",            "#40E0D0" },
    { "violet",               "#EE82EE" },
    { "wheat",                "#F5DEB3" },
    { "white",                "#FFFFFF" },
    { "whitesmoke",           "#F5F5F5" },
    { "yellow",               "#FFFF00" },
    { "yellowgreen",          "#9ACD32" },
    { NULL,      NULL }
};


static const Attribute* lookup( ctmbstr atnam )
{
    if ( atnam )
    {
        const Attribute *np = attribute_defs;
        for ( /**/; np && np->name; ++np )
            if ( tmbstrcmp(atnam, np->name) == 0 )
                return np;
    }
    return NULL;
}


/* Locate attributes by type
*/

AttVal* AttrGetById( Node* node, TidyAttrId id )
{
   AttVal* av;
   for ( av = node->attributes; av; av = av->next )
   {
     if ( AttrIsId(av, id) )
         return av;
   }
   return NULL;
}

/* public method for finding attribute definition by name */
const Attribute* FindAttribute( TidyDocImpl* doc, AttVal *attval )
{
    if ( attval )
       return lookup( attval->attribute );
    return NULL;
}

AttVal* GetAttrByName( Node *node, ctmbstr name )
{
    AttVal *attr;
    for (attr = node->attributes; attr != NULL; attr = attr->next)
    {
        if (attr->attribute && tmbstrcmp(attr->attribute, name) == 0)
            break;
    }
    return attr;
}

AttVal* AddAttribute( TidyDocImpl* doc,
                      Node *node, ctmbstr name, ctmbstr value )
{
    AttVal *av = NewAttribute();
    av->delim = '"';
    av->attribute = tmbstrdup( name );
    av->value = tmbstrdup( value );
    av->dict = lookup( name );

    if ( node->attributes == NULL )
        node->attributes = av;
    else /* append to end of attributes */
    {
        AttVal *here = node->attributes;
        while (here->next)
            here = here->next;
        here->next = av;
    }
    return av;
}

static Bool CheckAttrType( TidyDocImpl* doc,
                           ctmbstr attrname, AttrCheck type )
{
    const Attribute* np = lookup( attrname );
    return (Bool)( np && np->attrchk == type );
}

Bool IsUrl( TidyDocImpl* doc, ctmbstr attrname )
{
    return CheckAttrType( doc, attrname, URL );
}

Bool IsBool( TidyDocImpl* doc, ctmbstr attrname )
{
    return CheckAttrType( doc, attrname, BOOL );
}

Bool IsScript( TidyDocImpl* doc, ctmbstr attrname )
{
    return CheckAttrType( doc, attrname, SCRIPT );
}

Bool IsLiteralAttribute( TidyDocImpl* doc, ctmbstr attrname )
{
    const Attribute* np = lookup( attrname );
    return (Bool)( np && np->literal );
}

/* may id or name serve as anchor? */
Bool IsAnchorElement( TidyDocImpl* doc, Node *node)
{
    TidyTagImpl* tags = &doc->tags;
    TidyTagId tid = TagId( node );
    if ( tid == TidyElem_A      ||
         tid == TidyElem_APPLET ||
         tid == TidyElem_FORM   ||
         tid == TidyElem_FRAME  ||
         tid == TidyElem_IFRAME ||
         tid == TidyElem_IMG    ||
         tid == TidyElem_MAP )
        return yes;

    return no;
}

/*
  In CSS1, selectors can contain only the characters A-Z, 0-9, and Unicode characters 161-255, plus dash (-);
  they cannot start with a dash or a digit; they can also contain escaped characters and any Unicode character
  as a numeric code (see next item).

  The backslash followed by at most four hexadecimal digits (0..9A..F) stands for the Unicode character with that number.

  Any character except a hexadecimal digit can be escaped to remove its special meaning, by putting a backslash in front.

  #508936 - CSS class naming for -clean option
*/
Bool IsCSS1Selector( ctmbstr buf )
{
    Bool valid = yes;
    int esclen = 0;
    byte c;
    int pos;

    for ( pos=0; valid && (c = *buf++); ++pos )
    {
        if ( c == '\\' )
        {
            esclen = 1;  /* ab\555\444 is 4 chars {'a', 'b', \555, \444} */
        }
        else if ( isxdigit( c ) )
        {
            /* Digit not 1st, unless escaped (Max length "\112F") */
            if ( esclen > 0 )
                valid = ( ++esclen < 6 );
            if ( valid )
                valid = ( pos>0 || esclen>0 );
        }
        else
        {
            valid = (
                esclen > 0                       /* Escaped? Anything goes. */
                || ( pos>0 && c == '-' )         /* Dash cannot be 1st char */
                || isalpha(c)                    /* a-z, A-Z anywhere */
                || ( c >= 161 && c <= 255 )      /* Unicode 161-255 anywhere */
            );
            esclen = 0;
        }
    }
    return valid;
}





/* free single anchor */
static void FreeAnchor(Anchor *a)
{
    if ( a )
        MemFree( a->name );
    MemFree( a );
}

/* removes anchor for specific node */
void RemoveAnchorByNode( TidyDocImpl* doc, Node *node )
{
    TidyAttribImpl* attribs = &doc->attribs;
    Anchor *delme = NULL, *curr, *prev = NULL;

    for ( curr=attribs->anchor_list; curr!=NULL; curr=curr->next )
    {
        if ( curr->node == node )
        {
            if ( prev )
                prev->next = curr->next;
            else
                attribs->anchor_list = curr->next;
            delme = curr;
            break;
        }
        prev = curr;
    }
    FreeAnchor( delme );
}

/* initialize new anchor */
static Anchor* NewAnchor( ctmbstr name, Node* node )
{
    Anchor *a = (Anchor*) MemAlloc( sizeof(Anchor) );

    a->name = tmbstrdup( name );
    a->node = node;
    a->next = NULL;

    return a;
}

/* add new anchor to namespace */
Anchor* AddAnchor( TidyDocImpl* doc, ctmbstr name, Node *node )
{
    TidyAttribImpl* attribs = &doc->attribs;
    Anchor *a = NewAnchor( name, node );

    if ( attribs->anchor_list == NULL)
         attribs->anchor_list = a;
    else
    {
        Anchor *here =  attribs->anchor_list;
        while (here->next)
            here = here->next;
        here->next = a;
    }

    return attribs->anchor_list;
}

/* return node associated with anchor */
Node* GetNodeByAnchor( TidyDocImpl* doc, ctmbstr name )
{
    TidyAttribImpl* attribs = &doc->attribs;
    Anchor *found;
    for ( found = attribs->anchor_list; found != NULL; found = found->next )
    {
        if ( tmbstrcasecmp(found->name, name) == 0 )
            break;
    }
    
    if ( found )
        return found->node;
    return NULL;
}

/* free all anchors */
void FreeAnchors( TidyDocImpl* doc )
{
    TidyAttribImpl* attribs = &doc->attribs;
    Anchor* a;
    while ( a = attribs->anchor_list )
    {
        attribs->anchor_list = a->next;
        MemFree( a->name );
        MemFree( a );
    }
}

/* public method for inititializing attribute dictionary */
void InitAttrs( TidyDocImpl* doc )
{
    ClearMemory( &doc->attribs, sizeof(TidyAttribImpl) );

#ifdef _DEBUG
    {
      ctmbstr prev = NULL;
      TidyAttrId id;
      for ( id=1; id < N_TIDY_ATTRIBS; ++id )
      {
        const Attribute* dict = &attribute_defs[ id ];
        assert( dict->id == id );
        if ( prev )
            assert( tmbstrcmp( prev, dict->name ) < 0 );
        prev = dict->name;
      }
    }
#endif
}

/*
Henry Zrepa reports that some folk are
using embed with script attributes where
newlines are signficant. These need to be
declared and handled specially!
*/
static void DeclareAttribute( TidyDocImpl* doc, ctmbstr name,
                              uint versions, Bool nowrap, Bool isliteral )
{
    const Attribute *exist = lookup( name );
    if ( exist == NULL )
    {
        TidyAttribImpl* attribs = &doc->attribs;
        Attribute* dict = (Attribute*) MemAlloc( sizeof(Attribute) );
        ClearMemory( dict, sizeof(Attribute) );

        dict->name     = tmbstrdup( name );
        dict->versions = versions;
        dict->nowrap   = nowrap;
        dict->literal  = isliteral;

        dict->next = attribs->declared_attr_list;
        attribs->declared_attr_list = dict;
    }
}


/* free all declared attributes */
static void FreeDeclaredAttributes( TidyDocImpl* doc )
{
    TidyAttribImpl* attribs = &doc->attribs;
    Attribute* dict;
    while ( dict = attribs->declared_attr_list )
    {
        attribs->declared_attr_list = dict->next;
        MemFree( dict->name );
        MemFree( dict );
    }
}

void DeclareLiteralAttrib( TidyDocImpl* doc, ctmbstr name )
{
    DeclareAttribute( doc, name, VERS_PROPRIETARY, no, yes );
}

void FreeAttrTable( TidyDocImpl* doc )
{
    FreeAnchors( doc );
    FreeDeclaredAttributes( doc );
}

/*
 the same attribute name can't be used
 more than once in each element
*/

void RepairDuplicateAttributes( TidyDocImpl* doc, Node *node)
{
    AttVal *attval;

    for (attval = node->attributes; attval != NULL;)
    {
        if (attval->asp == NULL && attval->php == NULL)
        {
            AttVal *current;
            
            for (current = attval->next; current != NULL;)
            {
                if (current->asp == NULL && current->php == NULL &&
                    tmbstrcasecmp(attval->attribute, current->attribute) == 0)
                {
                    AttVal *temp;

                    if ( tmbstrcasecmp(current->attribute, "class") == 0 
                         && cfgBool(doc, TidyJoinClasses) )
                    {
                        /* concatenate classes */

                        current->value = (tmbstr) MemRealloc(current->value, tmbstrlen(current->value) +
                                                                            tmbstrlen(attval->value)  + 2);
                        tmbstrcat(current->value, " ");
                        tmbstrcat(current->value, attval->value);

                        temp = attval->next;

                        if (temp->next == NULL)
                            current = NULL;
                        else
                            current = current->next;

                        ReportAttrError( doc, node, attval, JOINING_ATTRIBUTE);

                        RemoveAttribute(node, attval);
                        attval = temp;
                    }
                    else if ( tmbstrcasecmp(current->attribute, "style") == 0 
                              && cfgBool(doc, TidyJoinStyles) )
                    {
                        /* concatenate styles */

                        /*
                          this doesn't handle CSS comments and
                          leading/trailing white-space very well
                          see http://www.w3.org/TR/css-style-attr
                        */

                        size_t end = strlen(current->value);

                        if (current->value[end] == ';')
                        {
                            /* attribute ends with declaration seperator */

                            current->value = (tmbstr) MemRealloc(current->value,
                                end + tmbstrlen(attval->value) + 2);

                            tmbstrcat(current->value, " ");
                            tmbstrcat(current->value, attval->value);
                        }
                        else if (current->value[end] == '}')
                        {
                            /* attribute ends with rule set */

                            current->value = (tmbstr) MemRealloc(current->value,
                                end + tmbstrlen(attval->value) + 6);

                            tmbstrcat(current->value, " { ");
                            tmbstrcat(current->value, attval->value);
                            tmbstrcat(current->value, " }");
                        }
                        else
                        {
                            /* attribute ends with property value */

                            current->value = (tmbstr) MemRealloc(current->value,
                                end + tmbstrlen(attval->value) + 3);

                            tmbstrcat(current->value, "; ");
                            tmbstrcat(current->value, attval->value);
                        }

                        temp = attval->next;

                        if (temp->next == NULL)
                            current = NULL;
                        else
                            current = current->next;

                        ReportAttrError( doc, node, attval, JOINING_ATTRIBUTE);

                        RemoveAttribute(node, attval);
                        attval = temp;

                    }
                    else if ( cfg(doc, TidyDuplicateAttrs) == TidyKeepLast )
                    {
                        temp = current->next;
                        ReportAttrError( doc, node, current, REPEATED_ATTRIBUTE);
                        RemoveAttribute(node, current);
                        current = temp;
                    }
                    else
                    {
                        temp = attval->next;
                        if (attval->next == NULL)
                            current = NULL;
                        else
                            current = current->next;

                        ReportAttrError( doc, node, attval, REPEATED_ATTRIBUTE);
                        RemoveAttribute(node, attval);
                        attval = temp;
                    }
                }
                else
                    current = current->next;
            }
            attval = attval->next;
        }
        else
            attval = attval->next;
    }
}


/* ignore unknown attributes for proprietary elements */
const Attribute* CheckAttribute( TidyDocImpl* doc, Node *node, AttVal *attval )
{
    const Attribute* attribute = attval->dict;

    if ( attribute != NULL )
    {
        /* if attribute looks like <foo/> check XML is ok */
        if ( attribute->versions & VERS_XML )
        {
            if ( !(cfgBool(doc, TidyXmlTags) || cfgBool(doc, TidyXmlOut)) )
                ReportAttrError( doc, node, attval, XML_ATTRIBUTE_VALUE);
        } /* title first appeared in HTML 4.0 except for a/link */
        else if ( attribute->id != TidyAttr_title ||
                  !(nodeIsA(node) || nodeIsLINK(node)) )
        {
            ConstrainVersion( doc, attribute->versions );
        }
        
        if (attribute->attrchk)
            attribute->attrchk( doc, node, attval );
        else if ( attval->dict->versions & VERS_PROPRIETARY )
            ReportAttrError( doc, node, attval, PROPRIETARY_ATTRIBUTE);
    }
    else if ( !cfgBool(doc, TidyXmlTags)
              && attval->asp == NULL 
              && node->tag != NULL 
              && !(node->tag->versions & VERS_PROPRIETARY) )
    {
        ReportAttrError( doc, node, attval, UNKNOWN_ATTRIBUTE );
    }

    return attribute;
}

Bool IsBoolAttribute(AttVal *attval)
{
    const Attribute *attribute = ( attval ? attval->dict : NULL );
    if ( attribute && attribute->attrchk == CheckBool )
        return yes;
    return no;
}

Bool attrIsEvent( AttVal* attval )
{
    TidyAttrId atid = AttrId( attval );
    return ( atid == TidyAttr_onmousemove ||
             atid == TidyAttr_onmousedown ||
             atid == TidyAttr_onmouseup   ||
             atid == TidyAttr_onclick     ||
             atid == TidyAttr_onmouseover ||
             atid == TidyAttr_onmouseout  ||
             atid == TidyAttr_onkeydown   ||
             atid == TidyAttr_onkeyup     ||
             atid == TidyAttr_onkeypress  ||
             atid == TidyAttr_onfocus     ||
             atid == TidyAttr_onblur );
}

static void CheckLowerCaseAttrValue( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr p;
    Bool hasUpper = no;
    
    if (attval == NULL || attval->value == NULL)
        return;

    p = attval->value;
    
    while (*p)
    {
        if (IsUpper(*p)) /* #501230 - fix by Terry Teague - 09 Jan 02 */
        {
            hasUpper = yes;
            break;
        }
        p++;
    }

    if (hasUpper)
    {
        Lexer* lexer = doc->lexer;
        if (lexer->isvoyager)
            ReportAttrError( doc, node, attval, ATTR_VALUE_NOT_LCASE);
  
        if ( lexer->isvoyager || cfgBool(doc, TidyLowerLiterals) )
            attval->value = tmbstrtolower(attval->value);
    }
}

/* methods for checking value of a specific attribute */

void CheckUrl( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbchar c; 
    tmbstr dest, p;
    uint escape_count = 0, backslash_count = 0;
    uint i, pos = 0;
    size_t len;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    p = attval->value;
    
    for (i = 0; c = p[i]; ++i)
    {
        if (c == '\\')
        {
            ++backslash_count;
            if ( cfgBool(doc, TidyFixBackslash) )
                p[i] = '/';
        }
        else if ((c > 0x7e) || (c <= 0x20) || (strchr("<>", c)))
            ++escape_count;
    }
    
    if ( cfgBool(doc, TidyFixUri) && escape_count )
    {
        len = tmbstrlen(p) + escape_count * 2 + 1;
        dest = (tmbstr) MemAlloc(len);
        
        for (i = 0; c = p[i]; ++i)
        {
            if ((c > 0x7e) || (c <= 0x20) || (strchr("<>", c)))
                pos += sprintf( dest + pos, "%%%02X", (byte)c );
            else
                dest[pos++] = c;
        }
        dest[pos++] = 0;

        MemFree(attval->value);
        attval->value = dest;
    }
    if ( backslash_count )
    {
        if ( cfgBool(doc, TidyFixBackslash) )
            ReportAttrError( doc, node, attval, FIXED_BACKSLASH );
        else
            ReportAttrError( doc, node, attval, BACKSLASH_IN_URI );
    }
    if ( escape_count )
    {
        if ( cfgBool(doc, TidyFixUri) )
            ReportAttrError( doc, node, attval, ESCAPED_ILLEGAL_URI);
        else
            ReportAttrError( doc, node, attval, ILLEGAL_URI_REFERENCE);

        doc->badChars |= INVALID_URI;
    }
}

void CheckScript( TidyDocImpl* doc, Node *node, AttVal *attval)
{
}

void CheckName( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    Node *old;

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    if ( IsAnchorElement(doc, node) )
    {
        ConstrainVersion( doc, ~VERS_XHTML11 );

        if ((old = GetNodeByAnchor(doc, attval->value)) &&  old != node)
        {
            ReportAttrError( doc, node, attval, ANCHOR_NOT_UNIQUE);
        }
        else
            AddAnchor( doc, attval->value, node );
    }
}

void CheckId( TidyDocImpl* doc, Node *node, AttVal *attval )
{
    Lexer* lexer = doc->lexer;
    tmbstr p;
    Node *old;
    uint s;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    p = attval->value;
    s = *p++;

    
    if (!IsLetter(s))
    {
        if (lexer->isvoyager && (IsXMLLetter(s) || s == '_' || s == ':'))
            ReportAttrError( doc, node, attval, XML_ID_SYNTAX);
        else
            ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
    }
    else
    {
        while(s = *p++)         /* #559774 tidy version rejects all id values */
        {
            if (!IsNamechar(s))
            {
                if (lexer->isvoyager && IsXMLNamechar(s))
                    ReportAttrError( doc, node, attval, XML_ID_SYNTAX);
                else
                    ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
                break;
            }
        }
    }

    if ((old = GetNodeByAnchor(doc, attval->value)) &&  old != node)
    {
        ReportAttrError( doc, node, attval, ANCHOR_NOT_UNIQUE);
    }
    else
        AddAnchor( doc, attval->value, node );
}

void CheckBool( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    if (attval == NULL || attval->value == NULL)
        return;

    CheckLowerCaseAttrValue( doc, node, attval );
}

void CheckAlign( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;

    /* IMG, OBJECT, APPLET and EMBED use align for vertical position */
    if (node->tag && (node->tag->model & CM_IMG))
    {
        CheckValign( doc, node, attval );
        return;
    }

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval);

    value = attval->value;

    if (! (tmbstrcasecmp(value,    "left") == 0 ||
           tmbstrcasecmp(value,  "center") == 0 ||
           tmbstrcasecmp(value,   "right") == 0 ||
           tmbstrcasecmp(value, "justify") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

void CheckValign( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval );

    value = attval->value;

    if (tmbstrcasecmp(value,      "top") == 0 ||
        tmbstrcasecmp(value,   "middle") == 0 ||
        tmbstrcasecmp(value,   "bottom") == 0 ||
        tmbstrcasecmp(value, "baseline") == 0)
    {
            /* all is fine */
    }
    else if (tmbstrcasecmp(value,  "left") == 0 ||
             tmbstrcasecmp(value, "right") == 0)
    {
        if (!(node->tag && (node->tag->model & CM_IMG)))
            ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
    }
    else if (tmbstrcasecmp(value,    "texttop") == 0 ||
             tmbstrcasecmp(value,  "absmiddle") == 0 ||
             tmbstrcasecmp(value,  "absbottom") == 0 ||
             tmbstrcasecmp(value, "textbottom") == 0)
    {
        ConstrainVersion( doc, VERS_PROPRIETARY );
        ReportAttrError( doc, node, attval, PROPRIETARY_ATTR_VALUE);
    }
    else
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

void CheckLength( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr p;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    /* don't check for <col width=...> and <colgroup width=...> */
    if ( tmbstrcmp(attval->attribute, "width") == 0 &&
         (nodeIsCOL(node) || nodeIsCOLGROUP(node)) )
        return;

    p = attval->value;
    
    if (!IsDigit(*p++))
    {
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
    }
    else
    {
        while (*p)
        {
            if (!IsDigit(*p) && *p != '%')
            {
                ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
                break;
            }
            ++p;
        }
    }
}

void CheckTarget( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    /* No target attribute in strict HTML versions */
    ConstrainVersion( doc, ~VERS_HTML40_STRICT);

    /*
      target names must begin with A-Za-z or be one of
      _blank, _self, _parent and _top
    */
    
    value = attval->value;

    if (IsLetter(value[0]))
        return;
    
    if (! (tmbstrcasecmp(value,  "_blank") == 0 ||
           tmbstrcasecmp(value,   "_self") == 0 ||
           tmbstrcasecmp(value, "_parent") == 0 ||
           tmbstrcasecmp(value,    "_top") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

void CheckFsubmit( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    value = attval->value;

    CheckLowerCaseAttrValue( doc, node, attval);

    if (! (tmbstrcasecmp(value,  "get") == 0 ||
           tmbstrcasecmp(value, "post") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

void CheckClear( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        if (attval->value == NULL)
            attval->value = tmbstrdup( "none" );
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval );
        
    value = attval->value;
    
    if (! (tmbstrcasecmp(value,  "none") == 0 ||
           tmbstrcasecmp(value,  "left") == 0 ||
           tmbstrcasecmp(value, "right") == 0 ||
           tmbstrcasecmp(value,   "all") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

void CheckShape( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval );

    value = attval->value;
    
    if (! (tmbstrcasecmp(value,    "rect") == 0 ||
           tmbstrcasecmp(value, "default") == 0 ||
           tmbstrcasecmp(value,  "circle") == 0 ||
           tmbstrcasecmp(value,    "poly") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

void CheckScope( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval);

    value = attval->value;
    
    if (! (tmbstrcasecmp(value,      "row") == 0 ||
           tmbstrcasecmp(value, "rowgroup") == 0 ||
           tmbstrcasecmp(value,      "col") == 0 ||
           tmbstrcasecmp(value, "colgroup") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

void CheckNumber( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr p;
    
    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    /* don't check <frameset cols=... rows=...> */
    if ( nodeIsFRAMESET(node) &&
         ( tmbstrcmp(attval->attribute, "cols") == 0 ||
           tmbstrcmp(attval->attribute, "rows") == 0 ) )
     return;

    p  = attval->value;
    
    /* font size may be preceded by + or - */
    if ( nodeIsFONT(node) && (*p == '+' || *p == '-') )
        ++p;

    while (*p)
    {
        if (!IsDigit(*p))
        {
            ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
            break;
        }
        ++p;
    }
}

/* check color syntax and beautify value by option */
void CheckColor( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    /* Bool ReplaceColor = yes; */ /* #477643 - replace hex color attribute values with names */
    Bool HexUppercase = yes;
    Bool invalid = no;
    Bool found = no;
    tmbstr given;
    const struct _colors *color;
    uint i = 0;

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    given = attval->value;
    
    for (color = colors; color->name; ++color)
    {
        if (given[0] == '#')
        {
            if (tmbstrlen(given) != 7)
            {
                ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
                invalid = yes;
                break;
            }
            else if (tmbstrcasecmp(given, color->hex) == 0)
            {
                if ( cfgBool(doc, TidyReplaceColor) )
                {
                    MemFree(attval->value);
                    attval->value = tmbstrdup(color->name);
                }
                found = yes;
                break;
            }
        }
        else if (IsLetter(given[0]))
        {
            if (tmbstrcasecmp(given, color->name) == 0)
            {
                if ( cfgBool(doc, TidyReplaceColor) )
                {
                    MemFree(attval->value);
                    attval->value = tmbstrdup(color->name);
                }
                found = yes;
                break;
            }
        }
        else
        {
            ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
            invalid = yes;
            break;
        }
    }
    
    if (!found && !invalid)
    {
        if (given[0] == '#')
        {
            /* check if valid hex digits and letters */
            for (i = 1; i < 7; ++i)
            {
                if (!IsDigit(given[i]) &&
                    !strchr("abcdef", ToLower(given[i])))
                {
                    ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
                    invalid = yes;
                    break;
                }
            }
            
            /* convert hex letters to uppercase */
            if (!invalid && HexUppercase)
            {
                for (i = 1; i < 7; ++i)
                {
                    given[i] = (tmbchar) ToUpper( given[i] );
                }
            }
        }
        else
        {
            /* we could search for more colors and mark the file as HTML
               Proprietary, but I don't thinks it's worth the effort,
               so values not in HTML 4.01 are invalid */

            ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
        }
    }
}

/* check valuetype attribute for element param */
void CheckVType( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval );

    value = attval->value;

    if (! (tmbstrcasecmp(value,   "data") == 0 ||
           tmbstrcasecmp(value, "object") == 0 ||
           tmbstrcasecmp(value,    "ref") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

/* checks scrolling attribute */
void CheckScroll( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval );

    value = attval->value;

    if (! (tmbstrcasecmp(value,   "no") == 0 ||
           tmbstrcasecmp(value, "auto") == 0 ||
           tmbstrcasecmp(value,  "yes") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

/* checks dir attribute */
void CheckTextDir( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    tmbstr value;

    if (attval == NULL || attval->value == NULL)
    {
        ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE);
        return;
    }

    CheckLowerCaseAttrValue( doc, node, attval);

    value = attval->value;

    if (! (tmbstrcasecmp(value, "rtl") == 0 ||
           tmbstrcasecmp(value, "ltr") == 0))
        ReportAttrError( doc, node, attval, BAD_ATTRIBUTE_VALUE);
}

/* checks lang and xml:lang attributes */
void CheckLang( TidyDocImpl* doc, Node *node, AttVal *attval)
{
    if ( attval == NULL || attval->value == NULL )
    {
        if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
        {
            ReportAttrError( doc, node, attval, MISSING_ATTR_VALUE );
        }
        return;
    }

    if (tmbstrcasecmp(attval->attribute, "lang") == 0)
        ConstrainVersion( doc, ~VERS_XHTML11 );
}



