#ifndef __TIDYENUM_H__
#define __TIDYENUM_H__

/* tidyenum.h -- Split public enums into separate header

  Simplifies enum re-use in various wrappers.  E.g. SWIG
  generated wrappers and COM IDL files.

  Copyright (c) 1998-2003 World Wide Web Consortium
  (Massachusetts Institute of Technology, European Research 
  Consortium for Informatics and Mathematics, Keio University).
  All Rights Reserved.

  CVS Info :

    $Author: hoehrmann $ 
    $Date: 2003/03/31 01:37:31 $ 
    $Revision: 1.6 $ 

  Contributing Author(s):

     Dave Raggett <dsr@w3.org>

  The contributing author(s) would like to thank all those who
  helped with testing, bug fixes and suggestions for improvements. 
  This wouldn't have been possible without your help.

  COPYRIGHT NOTICE:
 
  This software and documentation is provided "as is," and
  the copyright holders and contributing author(s) make no
  representations or warranties, express or implied, including
  but not limited to, warranties of merchantability or fitness
  for any particular purpose or that the use of the software or
  documentation will not infringe any third party patents,
  copyrights, trademarks or other rights. 

  The copyright holders and contributing author(s) will not be held
  liable for any direct, indirect, special or consequential damages
  arising out of any use of the software or documentation, even if
  advised of the possibility of such damage.

  Permission is hereby granted to use, copy, modify, and distribute
  this source code, or portions hereof, documentation and executables,
  for any purpose, without fee, subject to the following restrictions:

  1. The origin of this source code must not be misrepresented.
  2. Altered versions must be plainly marked as such and must
     not be misrepresented as being the original source.
  3. This Copyright notice may not be removed or altered from any
     source or altered source distribution.
 
  The copyright holders and contributing author(s) specifically
  permit, without fee, and encourage the use of this source code
  as a component for supporting the Hypertext Markup Language in
  commercial products. If you use this source code in a product,
  acknowledgment is not required but would be appreciated.


  Created 2001-05-20 by Charles Reitzel
  Updated 2002-07-01 by Charles Reitzel - 1st Implementation
*/

/** @file tidyenum.h - Enumerations defined for use with TidyLib.
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Enumerate configuration options
*/

/** Categories of Tidy configuration options
*/
typedef enum
{
  TidyMarkup,          /**< Markup options: (X)HTML version, etc */
  TidyDiagnostics,     /**< Diagnostics */
  TidyPrettyPrint,     /**< Output layout */
  TidyEncoding,        /**< Character encodings */
  TidyMiscellaneous    /**< File handling, message format, etc. */
} TidyConfigCategory;


/** Option IDs Used to get/set option values.
*/
typedef enum
{
  TidyUnknownOption,   /**< Unknown option! */
  TidyIndentSpaces,    /**< Indentation n spaces */
  TidyWrapLen,         /**< Wrap margin */
  TidyTabSize,         /**< Expand tabs to n spaces */

  TidyCharEncoding,    /**< In/out character encoding */
  TidyInCharEncoding,  /**< Input character encoding (if different) */
  TidyOutCharEncoding, /**< Output character encoding (if different) */    
  TidyNewline,         /**< Output line ending (default to platform) */

  TidyDoctypeMode,     /**< See doctype property */
  TidyDoctype,         /**< User specified doctype */

  TidyDuplicateAttrs,  /**< Keep first or last duplicate attribute */
  TidyAltText,         /**< Default text for alt attribute */
  TidySlideStyle,      /**< Style sheet for slides: not used for anything yet */
  TidyErrFile,         /**< File name to write errors to */
  TidyOutFile,         /**< File name to write markup to */
  TidyWriteBack,       /**< If true then output tidied markup */
  TidyShowMarkup,      /**< If false, normal output is suppressed */
  TidyShowWarnings,    /**< However errors are always shown */
  TidyQuiet,           /**< No 'Parsing X', guessed DTD or summary */
  TidyIndentContent,   /**< Indent content of appropriate tags */
                       /**< "auto" does text/block level content indentation */
  TidyHideEndTags,     /**< Suppress optional end tags */
  TidyXmlTags,         /**< Treat input as XML */
  TidyXmlOut,          /**< Create output as XML */
  TidyXhtmlOut,        /**< Output extensible HTML */
  TidyHtmlOut,         /**< Output plain HTML, even for XHTML input.
                           Yes means set explicitly. */
  TidyXmlDecl,         /**< Add <?xml?> for XML docs */
  TidyUpperCaseTags,   /**< Output tags in upper not lower case */
  TidyUpperCaseAttrs,  /**< Output attributes in upper not lower case */
  TidyMakeBare,        /**< Make bare HTML: remove Microsoft cruft */
  TidyMakeClean,       /**< Replace presentational clutter by style rules */
  TidyLogicalEmphasis, /**< Replace i by em and b by strong */
  TidyDropPropAttrs,   /**< Discard proprietary attributes */
  TidyDropFontTags,    /**< Discard presentation tags */
  TidyDropEmptyParas,  /**< Discard empty p elements */
  TidyFixComments,     /**< Fix comments with adjacent hyphens */
  TidyBreakBeforeBR,   /**< Output newline before <br> or not? */
  TidyBurstSlides,     /**< Create slides on each h2 element */
  TidyNumEntities,     /**< Use numeric entities */
  TidyQuoteMarks,      /**< Output " marks as &quot; */
  TidyQuoteNbsp,       /**< Output non-breaking space as entity */
  TidyQuoteAmpersand,  /**< Output naked ampersand as &amp; */
  TidyWrapAttVals,     /**< Wrap within attribute values */
  TidyWrapScriptlets,  /**< Wrap within JavaScript string literals */
  TidyWrapSection,     /**< Wrap within <![ ... ]> section tags */
  TidyWrapAsp,         /**< Wrap within ASP pseudo elements */
  TidyWrapJste,        /**< Wrap within JSTE pseudo elements */
  TidyWrapPhp,         /**< Wrap within PHP pseudo elements */
  TidyFixBackslash,    /**< Fix URLs by replacing \ with / */
  TidyIndentAttributes,/**< Newline+indent before each attribute */
  TidyXmlPIs,          /**< If set to yes PIs must end with ?> */
  TidyXmlSpace,        /**< If set to yes adds xml:space attr as needed */
  TidyEncloseBodyText, /**< If yes text at body is wrapped in P's */
  TidyEncloseBlockText,/**< If yes text in blocks is wrapped in P's */
  TidyKeepFileTimes,   /**< If yes last modied time is preserved */
  TidyWord2000,        /**< Draconian cleaning for Word2000 */
  TidyMark,            /**< Add meta element indicating tidied doc */
  TidyEmacs,           /**< If true format error output for GNU Emacs */
  TidyEmacsFile,       /**< Name of current Emacs file */
  TidyLiteralAttribs,  /**< If true attributes may use newlines */
  TidyBodyOnly,        /**< Output BODY content only */
  TidyFixUri,          /**< Applies URI encoding if necessary */
  TidyLowerLiterals,   /**< Folds known attribute values to lower case */
  TidyHideComments,    /**< Hides all (real) comments in output */
  TidyIndentCdata,     /**< Indent <!CDATA[ ... ]]> section */
  TidyForceOutput,     /**< Output document even if errors were found */
  TidyShowErrors,      /**< Number of errors to put out */
  TidyAsciiChars,      /**< Convert quotes and dashes to nearest ASCII char */
  TidyJoinClasses,     /**< Join multiple class attributes */
  TidyJoinStyles,      /**< Join multiple style attributes */
  TidyEscapeCdata,     /**< Replace <![CDATA[]]> sections with escaped text */

#if SUPPORT_ASIAN_ENCODINGS
  TidyLanguage,        /**< Language property: not used for anything yet */
  TidyNCR,             /**< Allow numeric character references */
#endif
#if SUPPORT_UTF16_ENCODINGS
  TidyOutputBOM,      /**< Output a Byte Order Mark (BOM) for UTF-16 encodings */
                      /**< auto: if input stream has BOM, we output a BOM */
#endif

  TidyReplaceColor,    /**< Replace hex color attribute values with names */
  TidyCSSPrefix,       /**< CSS class naming for -clean option */

  TidyInlineTags,      /**< Declared inline tags */
  TidyBlockTags,       /**< Declared block tags */
  TidyEmptyTags,       /**< Declared empty tags */
  TidyPreTags,         /**< Declared pre tags */

  TidyAccessibilityCheckLevel, /**< Accessibility check level 
                                   0 (old style), or 1, 2, 3 */

  TidyVertSpace,       /**< degree to which markup is spread out vertically */
  N_TIDY_OPTIONS       /**< Must be last */
} TidyOptionId;

/** Option data types
*/
typedef enum
{
  TidyString,          /**< String */
  TidyInteger,         /**< Integer or enumeration */
  TidyBoolean          /**< Boolean flag */
} TidyOptionType;


/** AutoBool values used by ParseBool, ParseTriState, ParseIndent, ParseBOM
*/
typedef enum
{
   TidyNoState,     /**< maps to 'no' */
   TidyYesState,    /**< maps to 'yes' */
   TidyAutoState    /**< Automatic */
} TidyTriState;

/** TidyNewline option values to control output line endings.
*/
typedef enum
{
    TidyLF,         /**< Use Unix style: LF */
    TidyCRLF,       /**< Use DOS/Windows style: CR+LF */
    TidyCR          /**< Use Macintosh style: CR */
} TidyLineEnding;


/** Mode controlling treatment of doctype
*/
typedef enum
{
    TidyDoctypeOmit,    /**< Omit DOCTYPE altogether */
    TidyDoctypeAuto,    /**< Keep DOCTYPE in input.  Set version to content */
    TidyDoctypeStrict,  /**< Convert document to HTML 4 strict content model */
    TidyDoctypeLoose,   /**< Convert document to HTML 4 transitional
                             content model */
    TidyDoctypeUser     /**< Set DOCTYPE FPI explicitly */
} TidyDoctypeModes;

/** Mode controlling treatment of duplicate Attributes
*/
typedef enum
{
    TidyKeepFirst,
    TidyKeepLast
} TidyDupAttrModes;


/* I/O and Message handling interface
**
** By default, Tidy will define, create and use 
** instances of input and output handlers for 
** standard C buffered I/O (i.e. FILE* stdin,
** FILE* stdout and FILE* stderr for content
** input, content output and diagnostic output,
** respectively.  A FILE* cfgFile input handler
** will be used for config files.  Command line
** options will just be set directly.
*/

/** Message severity level
*/
typedef enum 
{
  TidyInfo,             /**< Information about markup usage */
  TidyWarning,          /**< Warning message */
  TidyConfig,           /**< Configuration error */
  TidyAccess,           /**< Accessibility message */
  TidyError,            /**< Error message - output suppressed */
  TidyBadDocument,      /**< I/O or file system error */
  TidyFatal             /**< Crash! */
} TidyReportLevel;


/* Document tree traversal functions
*/

/** Node types
*/
typedef enum 
{
  TidyNode_Root,        /**< Root */
  TidyNode_DocType,     /**< DOCTYPE */
  TidyNode_Comment,     /**< Comment */
  TidyNode_ProcIns,     /**< Processing Instruction */
  TidyNode_Text,        /**< Text */
  TidyNode_Start,       /**< Start Tag */
  TidyNode_End,         /**< End Tag */
  TidyNode_StartEnd,    /**< Start/End (empty) Tag */
  TidyNode_CDATA,       /**< Unparsed Text */
  TidyNode_Section,     /**< XML Section */
  TidyNode_Asp,         /**< ASP Source */
  TidyNode_Jste,        /**< JSTE Source */
  TidyNode_Php,         /**< PHP Source */
  TidyNode_XmlDecl      /**< XML Declaration */
} TidyNodeType;


/** Known HTML element types
*/
typedef enum
{
  TidyElem_UNKNOWN,  /**< Unknown tag! */
  TidyElem_A,        /**< A */
  TidyElem_ABBR,     /**< ABBR */
  TidyElem_ACRONYM,  /**< ACRONYM */
  TidyElem_ADDRESS,  /**< ADDRESS */
  TidyElem_ALIGN,    /**< ALIGN */
  TidyElem_APPLET,   /**< APPLET */
  TidyElem_AREA,     /**< AREA */
  TidyElem_B,        /**< B */
  TidyElem_BASE,     /**< BASE */
  TidyElem_BASEFONT, /**< BASEFONT */
  TidyElem_BDO,      /**< BDO */
  TidyElem_BGSOUND,  /**< BGSOUND */
  TidyElem_BIG,      /**< BIG */
  TidyElem_BLINK,    /**< BLINK */
  TidyElem_BLOCKQUOTE,   /**< BLOCKQUOTE */
  TidyElem_BODY,     /**< BODY */
  TidyElem_BR,       /**< BR */
  TidyElem_BUTTON,   /**< BUTTON */
  TidyElem_CAPTION,  /**< CAPTION */
  TidyElem_CENTER,   /**< CENTER */
  TidyElem_CITE,     /**< CITE */
  TidyElem_CODE,     /**< CODE */
  TidyElem_COL,      /**< COL */
  TidyElem_COLGROUP, /**< COLGROUP */
  TidyElem_COMMENT,  /**< COMMENT */
  TidyElem_DD,       /**< DD */
  TidyElem_DEL,      /**< DEL */
  TidyElem_DFN,      /**< DFN */
  TidyElem_DIR,      /**< DIR */
  TidyElem_DIV,      /**< DIF */
  TidyElem_DL,       /**< DL */
  TidyElem_DT,       /**< DT */
  TidyElem_EM,       /**< EM */
  TidyElem_EMBED,    /**< EMBED */
  TidyElem_FIELDSET, /**< FIELDSET */
  TidyElem_FONT,     /**< FONT */
  TidyElem_FORM,     /**< FORM */
  TidyElem_FRAME,    /**< FRAME */
  TidyElem_FRAMESET, /**< FRAMESET */
  TidyElem_H1,       /**< H1 */
  TidyElem_H2,       /**< H2 */
  TidyElem_H3,       /**< H3 */
  TidyElem_H4,       /**< H4 */
  TidyElem_H5,       /**< H5 */
  TidyElem_H6,       /**< H6 */
  TidyElem_HEAD,     /**< HEAD */
  TidyElem_HR,       /**< HR */
  TidyElem_HTML,     /**< HTML */
  TidyElem_I,        /**< I */
  TidyElem_IFRAME,   /**< IFRAME */
  TidyElem_ILAYER,   /**< ILAYER */
  TidyElem_IMG,      /**< IMG */
  TidyElem_INPUT,    /**< INPUT */
  TidyElem_INS,      /**< INS */
  TidyElem_ISINDEX,  /**< ISINDEX */
  TidyElem_KBD,      /**< KBD */
  TidyElem_KEYGEN,   /**< KEYGEN */
  TidyElem_LABEL,    /**< LABEL */
  TidyElem_LAYER,    /**< LAYER */
  TidyElem_LEGEND,   /**< LEGEND */
  TidyElem_LI,       /**< LI */
  TidyElem_LINK,     /**< LINK */
  TidyElem_LISTING,  /**< LISTING */
  TidyElem_MAP,      /**< MAP */
  TidyElem_MARQUEE,  /**< MARQUEE */
  TidyElem_MENU,     /**< MENU */
  TidyElem_META,     /**< META */
  TidyElem_MULTICOL, /**< MULTICOL */
  TidyElem_NOBR,     /**< NOBR */
  TidyElem_NOEMBED,  /**< NOEMBED */
  TidyElem_NOFRAMES, /**< NOFRAMES */
  TidyElem_NOLAYER,  /**< NOLAYER */
  TidyElem_NOSAVE,   /**< NOSAVE */
  TidyElem_NOSCRIPT, /**< NOSCRIPT */
  TidyElem_OBJECT,   /**< OBJECT */
  TidyElem_OL,       /**< OL */
  TidyElem_OPTGROUP, /**< OPTGROUP */
  TidyElem_OPTION,   /**< OPTION */
  TidyElem_P,        /**< P */
  TidyElem_PARAM,    /**< PARAM */
  TidyElem_PLAINTEXT,    /**< PLAINTEXT */
  TidyElem_PRE,      /**< PRE */
  TidyElem_Q,        /**< Q */
  TidyElem_RB,       /**< RB */
  TidyElem_RBC,      /**< RBC */
  TidyElem_RP,       /**< RP */
  TidyElem_RT,       /**< RT */
  TidyElem_RTC,      /**< RTC */
  TidyElem_RUBY,     /**< RUBY */
  TidyElem_S,        /**< S */
  TidyElem_SAMP,     /**< SAMP */
  TidyElem_SCRIPT,   /**< SCRIPT */
  TidyElem_SELECT,   /**< SELECT */
  TidyElem_SERVER,   /**< SERVER */
  TidyElem_SERVLET,  /**< SERVLET */
  TidyElem_SMALL,    /**< SMALL */
  TidyElem_SPACER,   /**< SPACER */
  TidyElem_SPAN,     /**< SPAN */
  TidyElem_STRIKE,   /**< STRIKE */
  TidyElem_STRONG,   /**< STRONG */
  TidyElem_STYLE,    /**< STYLE */
  TidyElem_SUB,      /**< SUB */
  TidyElem_SUP,      /**< SUP */
  TidyElem_TABLE,    /**< TABLE */
  TidyElem_TBODY,    /**< TBODY */
  TidyElem_TD,       /**< TD */
  TidyElem_TEXTAREA, /**< TEXTAREA */
  TidyElem_TFOOT,    /**< TFOOT */
  TidyElem_TH,       /**< TH */
  TidyElem_THEAD,    /**< THEAD */
  TidyElem_TITLE,    /**< TITLE */
  TidyElem_TR,       /**< TR */
  TidyElem_TT,       /**< TT */
  TidyElem_U,        /**< U */
  TidyElem_UL,       /**< UL */
  TidyElem_VAR,      /**< VAR */
  TidyElem_WBR,      /**< WBR */
  TidyElem_XMP,      /**< XMP */
  TidyElem_NEXTID,   /**< NEXTID */

  N_TIDY_TAGS       /**< Must be last */
} TidyTagId;

/* Attribute interrogation
*/

/** Known HTML attributes
*/
typedef enum
{
  TidyAttr_unknown,           /**< UNKNOWN= */
  TidyAttr_abbr,              /**< ABBR= */
  TidyAttr_accept,            /**< ACCEPT= */
  TidyAttr_accept_charset,    /**< ACCEPT_CHARSET= */
  TidyAttr_accesskey,         /**< ACCESSKEY= */
  TidyAttr_action,            /**< ACTION= */
  TidyAttr_add_date,          /**< ADD_DATE= */
  TidyAttr_align,             /**< ALIGN= */
  TidyAttr_alink,             /**< ALINK= */
  TidyAttr_alt,               /**< ALT= */
  TidyAttr_archive,           /**< ARCHIVE= */
  TidyAttr_axis,              /**< AXIS= */
  TidyAttr_background,        /**< BACKGROUND= */
  TidyAttr_bgcolor,           /**< BGCOLOR= */
  TidyAttr_bgproperties,      /**< BGPROPERTIES= */
  TidyAttr_border,            /**< BORDER= */
  TidyAttr_bordercolor,       /**< BORDERCOLOR= */
  TidyAttr_bottommargin,      /**< BOTTOMMARGIN= */
  TidyAttr_cellpadding,       /**< CELLPADDING= */
  TidyAttr_cellspacing,       /**< CELLSPACING= */
  TidyAttr_char,              /**< CHAR= */
  TidyAttr_charoff,           /**< CHAROFF= */
  TidyAttr_charset,           /**< CHARSET= */
  TidyAttr_checked,           /**< CHECKED= */
  TidyAttr_cite,              /**< CITE= */
  TidyAttr_class,             /**< CLASS= */
  TidyAttr_classid,           /**< CLASSID= */
  TidyAttr_clear,             /**< CLEAR= */
  TidyAttr_code,              /**< CODE= */
  TidyAttr_codebase,          /**< CODEBASE= */
  TidyAttr_codetype,          /**< CODETYPE= */
  TidyAttr_color,             /**< COLOR= */
  TidyAttr_cols,              /**< COLS= */
  TidyAttr_colspan,           /**< COLSPAN= */
  TidyAttr_compact,           /**< COMPACT= */
  TidyAttr_content,           /**< CONTENT= */
  TidyAttr_coords,            /**< COORDS= */
  TidyAttr_data,              /**< DATA= */
  TidyAttr_datafld,           /**< DATAFLD= */
  TidyAttr_dataformatas,      /**< DATAFORMATAS= */
  TidyAttr_datapagesize,      /**< DATAPAGESIZE= */
  TidyAttr_datasrc,           /**< DATASRC= */
  TidyAttr_datetime,          /**< DATETIME= */
  TidyAttr_declare,           /**< DECLARE= */
  TidyAttr_defer,             /**< DEFER= */
  TidyAttr_dir,               /**< DIR= */
  TidyAttr_disabled,          /**< DISABLED= */
  TidyAttr_encoding,          /**< ENCODING= */
  TidyAttr_enctype,           /**< ENCTYPE= */
  TidyAttr_face,              /**< FACE= */
  TidyAttr_for,               /**< FOR= */
  TidyAttr_frame,             /**< FRAME= */
  TidyAttr_frameborder,       /**< FRAMEBORDER= */
  TidyAttr_framespacing,      /**< FRAMESPACING= */
  TidyAttr_gridx,             /**< GRIDX= */
  TidyAttr_gridy,             /**< GRIDY= */
  TidyAttr_headers,           /**< HEADERS= */
  TidyAttr_height,            /**< HEIGHT= */
  TidyAttr_href,              /**< HREF= */
  TidyAttr_hreflang,          /**< HREFLANG= */
  TidyAttr_hspace,            /**< HSPACE= */
  TidyAttr_http_equiv,        /**< HTTP_EQUIV= */
  TidyAttr_id,                /**< ID= */
  TidyAttr_ismap,             /**< ISMAP= */
  TidyAttr_label,             /**< LABEL= */
  TidyAttr_lang,              /**< LANG= */
  TidyAttr_language,          /**< LANGUAGE= */
  TidyAttr_last_modified,     /**< LAST_MODIFIED= */
  TidyAttr_last_visit,        /**< LAST_VISIT= */
  TidyAttr_leftmargin,        /**< LEFTMARGIN= */
  TidyAttr_link,              /**< LINK= */
  TidyAttr_longdesc,          /**< LONGDESC= */
  TidyAttr_lowsrc,            /**< LOWSRC= */
  TidyAttr_marginheight,      /**< MARGINHEIGHT= */
  TidyAttr_marginwidth,       /**< MARGINWIDTH= */
  TidyAttr_maxlength,         /**< MAXLENGTH= */
  TidyAttr_media,             /**< MEDIA= */
  TidyAttr_method,            /**< METHOD= */
  TidyAttr_multiple,          /**< MULTIPLE= */
  TidyAttr_name,              /**< NAME= */
  TidyAttr_nohref,            /**< NOHREF= */
  TidyAttr_noresize,          /**< NORESIZE= */
  TidyAttr_noshade,           /**< NOSHADE= */
  TidyAttr_nowrap,            /**< NOWRAP= */
  TidyAttr_object,            /**< OBJECT= */
  TidyAttr_onafterupdate,     /**< OnAFTERUPDATE= */
  TidyAttr_onbeforeunload,    /**< OnBEFOREUNLOAD= */
  TidyAttr_onbeforeupdate,    /**< OnBEFOREUPDATE= */
  TidyAttr_onblur,            /**< OnBLUR= */
  TidyAttr_onchange,          /**< OnCHANGE= */
  TidyAttr_onclick,           /**< OnCLICK= */
  TidyAttr_ondataavailable,   /**< OnDATAAVAILABLE= */
  TidyAttr_ondatasetchanged,  /**< OnDATASETCHANGED= */
  TidyAttr_ondatasetcomplete, /**< OnDATASETCOMPLETE= */
  TidyAttr_ondblclick,        /**< OnDBLCLICK= */
  TidyAttr_onerrorupdate,     /**< OnERRORUPDATE= */
  TidyAttr_onfocus,           /**< OnFOCUS= */
  TidyAttr_onkeydown,         /**< OnKEYDOWN= */
  TidyAttr_onkeypress,        /**< OnKEYPRESS= */
  TidyAttr_onkeyup,           /**< OnKEYUP= */
  TidyAttr_onload,            /**< OnLOAD= */
  TidyAttr_onmousedown,       /**< OnMOUSEDOWN= */
  TidyAttr_onmousemove,       /**< OnMOUSEMOVE= */
  TidyAttr_onmouseout,        /**< OnMOUSEOUT= */
  TidyAttr_onmouseover,       /**< OnMOUSEOVER= */
  TidyAttr_onmouseup,         /**< OnMOUSEUP= */
  TidyAttr_onreset,           /**< OnRESET= */
  TidyAttr_onrowenter,        /**< OnROWENTER= */
  TidyAttr_onrowexit,         /**< OnROWEXIT= */
  TidyAttr_onselect,          /**< OnSELECT= */
  TidyAttr_onsubmit,          /**< OnSUBMIT= */
  TidyAttr_onunload,          /**< OnUNLOAD= */
  TidyAttr_profile,           /**< PROFILE= */
  TidyAttr_prompt,            /**< PROMPT= */
  TidyAttr_rbspan,            /**< RBSPAN= */
  TidyAttr_readonly,          /**< READONLY= */
  TidyAttr_rel,               /**< REL= */
  TidyAttr_rev,               /**< REV= */
  TidyAttr_rightmargin,       /**< RIGHTMARGIN= */
  TidyAttr_rows,              /**< ROWS= */
  TidyAttr_rowspan,           /**< ROWSPAN= */
  TidyAttr_rules,             /**< RULES= */
  TidyAttr_scheme,            /**< SCHEME= */
  TidyAttr_scope,             /**< SCOPE= */
  TidyAttr_scrolling,         /**< SCROLLING= */
  TidyAttr_selected,          /**< SELECTED= */
  TidyAttr_shape,             /**< SHAPE= */
  TidyAttr_showgrid,          /**< SHOWGRID= */
  TidyAttr_showgridx,         /**< SHOWGRIDX= */
  TidyAttr_showgridy,         /**< SHOWGRIDY= */
  TidyAttr_size,              /**< SIZE= */
  TidyAttr_span,              /**< SPAN= */
  TidyAttr_src,               /**< SRC= */
  TidyAttr_standby,           /**< STANDBY= */
  TidyAttr_start,             /**< START= */
  TidyAttr_style,             /**< STYLE= */
  TidyAttr_summary,           /**< SUMMARY= */
  TidyAttr_tabindex,          /**< TABINDEX= */
  TidyAttr_target,            /**< TARGET= */
  TidyAttr_text,              /**< TEXT= */
  TidyAttr_title,             /**< TITLE= */
  TidyAttr_topmargin,         /**< TOPMARGIN= */
  TidyAttr_type,              /**< TYPE= */
  TidyAttr_usemap,            /**< USEMAP= */
  TidyAttr_valign,            /**< VALIGN= */
  TidyAttr_value,             /**< VALUE= */
  TidyAttr_valuetype,         /**< VALUETYPE= */
  TidyAttr_version,           /**< VERSION= */
  TidyAttr_vlink,             /**< VLINK= */
  TidyAttr_vspace,            /**< VSPACE= */
  TidyAttr_width,             /**< WIDTH= */
  TidyAttr_wrap,              /**< WRAP= */
  TidyAttr_xml_lang,          /**< XML_LANG= */
  TidyAttr_xml_space,         /**< XML_SPACE= */
  TidyAttr_xmlns,             /**< XMLNS= */

  TidyAttr_event,             /**< EVENT= */
  TidyAttr_methods,           /**< METHODS= */
  TidyAttr_n,                 /**< N= */
  TidyAttr_sdaform,           /**< SDAFORM= */
  TidyAttr_sdapref,           /**< SDAPREF= */
  TidyAttr_sdasuff,           /**< SDASUFF= */
  TidyAttr_urn,               /**< URN= */

  N_TIDY_ATTRIBS              /**< Must be last */
} TidyAttrId;

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* __TIDYENUM_H__ */
