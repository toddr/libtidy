/* access.c -- carry out accessibility checks

  Copyright University of Toronto
  Portions (c) 1998-2002 (W3C) MIT, INRIA, Keio University
  See tidy.c for the copyright notice.
  
  CVS Info :

    $Author: terry_teague $ 
    $Date: 2002/05/31 21:54:53 $ 
    $Revision: 1.1 $ 

*/

/*********************************************************************
**********
* AccessibilityChecks
*
* Carries out processes for all accessibility checks.  Traverses
* through all the content within the tree and evaluates the tags for
* accessibility.
*
* To perform the following checks, 'AccessibilityChecks' must be
* called AFTER the tree structure has been formed.
*
* If, in the command prompt, there is no specification of which
* accessibility priorities to check, no accessibility checks will be 
* performed.  (ie. '1' for priority 1, '2' for priorities 1 and 2, 
*                  and '3') for priorities 1, 2 and 3.)
*
* Copyright University of Toronto
* Programmed by: Mike Lam and Chris Ridpath
* Modifications by : Terry Teague (TRT)
*
***********************************************************************
**********/

#include "platform.h"
#include "html.h"

/*
#include <string.h>
#include <stdlib.h>
*/

#if SUPPORT_ACCESSIBILITY_CHECKS

/* 
	The accessibility checks to perform depending on user's desire.

	1. priority 1
	2. priority 1 & 2
	3. priority 1, 2, & 3
*/

static int PRIORITYCHK = 0; /* gets set from Tidy variable AccessibilityCheckLevel - TRT */

/* List of possible image types */
static char arrayImageExtensions[13][6] = /* TRT */
{".jpg", ".gif", ".tif", ".pct", ".pic", ".iff", ".dib",
 ".tga", ".pcx", ".png", ".jpeg", ".tiff", ".bmp"};

/* List of possible sound file types */
static char arraySoundExtensions[7][6] = /* TRT */
{".wav", ".au", ".aiff", ".snd", ".ra", ".rm"};

/* List of possible media extensions */
static char arrayMediaExtensions[18][7] = 
{".mpg", ".mov", ".asx", ".avi", ".ivf", ".m1v", ".mmm", ".mp2v",
 ".mpa", ".mpe", ".mpeg", ".ram", ".smi", ".smil", ".swf",
 ".wm", ".wma", ".wmv"};

/* List of possible frame sources */
static char arrayFrameExtensions[10][7] = /* TRT */
{".htm", ".html", ".shtm", ".shtml", ".cfm", ".cfml",
".asp", ".cgi", ".pl", ".smil"};

/* List of possible colour values */
static int arrayColorValues[16][3] = /* TRT */
{0,0,0,  192,192,192,  128,128,128,  255,255,255,  192,0,0,  255,0,0,
 128,0,128,  255,0,255, 0,128,0,  0,255,0,  128,128,0,  255,255,0,  
 0,0,128,  0,0,255,  0,128,128, 0,255,255 };

/* These arrays are used to convert color names to their RGB values */
static char arrayColorNames[16][8] = /* TRT */
{ "black", "silver", "grey", "white", "maroon", "red",
  "purple", "fuchsia", "green", "lime", "olive", "yellow", 
  "navy", "blue", "teal", "aqua"};

/* Number of characters that are found within the concatenated text */
static int counter = 0; /* TRT */

/* The list of characters in the text nodes found within a container element */
static char textNode[101]; /* TRT */

/* The list of characters found within one text node */
static char text[101]; /* TRT */

/* The number of words found within the document (text only) */
static int wordsCounted = 0; /* TRT */

/* Number of frame elements found within a frameset */
static int numFrames = 0; /* TRT */

/* Number of 'longdesc' attributes found within a frameset */
static int HasCheckedLongDesc = 0; /* TRT */

/* Check for the DocType only once for each document */
static Bool HasCheckedDocType = no; /* TRT */

/* For 'USEMAP' identifier */
static Bool HasUseMap = no; /* TRT */
static Bool HasName = no; /* TRT */
static Bool HasMap = no;

/* Reference to the beginning of the document */
static int reference = 0; /* TRT */
static Node* beginning; /* TRT */

/* Reference to the <map> node */
static Node* mapNode;
static Bool gotMapNode = no;

/* For tracking nodes that are deleted from the original parse tree - TRT */
/* Node *access_tree = NULL; */

/* Ensure that certain warning messages are only displayed once */
/* TRT */
static Bool checkedHTML = no;
static Bool shown = no;
static Bool DisplayedLayout = no;
static Bool HasTH = no;
static Bool checkedForStyleSheets = no;
static Bool StyleSheetsPresent = no;

static Bool HasValidFor = no;
static Bool HasValidId = no;
static Bool HasValidRowHeaders = no;
static Bool HasValidColumnHeaders = no;
static Bool HasInvalidRowHeader = no;
static Bool HasInvalidColumnHeader = no;
static Bool checkedForListElements = no;
static int listElements = 0;
static int otherListElements = 0;
static int checkedHeaders = 0;
static int ForID = 0;

/* Determines if the client-side text link is found within the document */
typedef struct AreaLinks
{
	struct AreaLinks* next;
	char* link;
	Bool HasBeenFound;
}AreaLinks;

/* Pointer to attribute and values within the attributes of an element */
static AttVal* attval; /* TRT */
static Attribute* attribute; /* TRT */

/* The list of attributes to check for validity */
extern Attribute* attr_href;
extern Attribute* attr_src;
extern Attribute* attr_id;
extern Attribute* attr_name;
extern Attribute* attr_summary;
extern Attribute* attr_alt;
extern Attribute* attr_longdesc;
extern Attribute* attr_usemap;
extern Attribute* attr_ismap;
extern Attribute* attr_language;
extern Attribute* attr_type;
extern Attribute* attr_value;
extern Attribute* attr_content;
extern Attribute* attr_title;
extern Attribute* attr_xmlns;
extern Attribute* attr_datafld;
extern Attribute* attr_width;
extern Attribute* attr_height;
extern Attribute* attr_for;
extern Attribute* attr_selected;
extern Attribute* attr_checked;
extern Attribute* attr_lang;
extern Attribute* attr_target;
extern Attribute* attr_httpEquiv;
extern Attribute* attr_rel;
extern Attribute* attr_onMouseMove;
extern Attribute* attr_onMouseDown;
extern Attribute* attr_onMouseUp;
extern Attribute* attr_onClick;
extern Attribute* attr_onMouseOver;
extern Attribute* attr_onMouseOut;
extern Attribute* attr_onKeyDown;
extern Attribute* attr_onKeyUp;
extern Attribute* attr_onKeyPress;
extern Attribute* attr_onFocus;
extern Attribute* attr_onBlur;
extern Attribute* attr_bgcolor;
extern Attribute* attr_text;
extern Attribute* attr_link;
extern Attribute* attr_alink;
extern Attribute* attr_vlink;
extern Attribute* attr_style;
extern Attribute* attr_abbr;
extern Attribute* attr_colspan;
extern Attribute* attr_font;
extern Attribute* attr_basefont;
extern Attribute* attr_rowspan;


/* The list of tags which are not exported directly by Tidy - TRT */
static Dict *tag_label;
static Dict *tag_h3;
static Dict *tag_h4;
static Dict *tag_h5;
static Dict *tag_h6;
static Dict *tag_address;
static Dict *tag_xmp;
static Dict *tag_select;
static Dict *tag_blink;
static Dict *tag_marquee;
static Dict *tag_embed;
static Dict *tag_basefont;
static Dict *tag_isindex;
static Dict *tag_s;
static Dict *tag_strike;
static Dict *tag_u;
static Dict *tag_menu;

/* The type of error:  0 = warning; 1 = error */
/* TRT */
enum accessErrorTypes
{
	ACCESS_WARNING_ERROR,
	ACCESS_FATAL_ERROR
};

/* 
	Determines which error/warning message should be displayed, depending on the
    error code that was called.
*/
enum accessErrorCodes
{
	/* [1.1.1.1] */		IMG_MISSING_ALT,
	/* [1.1.1.2] */		IMG_ALT_SUSPICIOUS_FILENAME,
	/* [1.1.1.3] */		IMG_ALT_SUSPICIOUS_FILE_SIZE,
	/* [1.1.1.4] */		IMG_ALT_SUSPICIOUS_PLACEHOLDER,
	/* [1.1.1.10] */	IMG_ALT_SUSPICIOUS_TOO_LONG,
	/* [1.1.1.11] */	IMG_MISSING_ALT_BULLET,
	/* [1.1.1.12] */	IMG_MISSING_ALT_H_RULE,
	/* [1.1.2.1] */		IMG_MISSING_LONGDESC_DLINK,
	/* [1.1.2.2] */		IMG_MISSING_DLINK,
	/* [1.1.2.3] */		IMG_MISSING_LONGDESC,
	/* [1.1.2.5] */		LONGDESC_NOT_REQUIRED,
	/* [1.1.3.1] */		IMG_BUTTON_MISSING_ALT, 
	/* [1.1.4.1] */		APPLET_MISSING_ALT,
	/* [1.1.5.1] */		OBJECT_MISSING_ALT,
	/* [1.1.6.1] */		AUDIO_MISSING_TEXT_WAV,
	/* [1.1.6.2] */		AUDIO_MISSING_TEXT_AU,
	/* [1.1.6.3] */		AUDIO_MISSING_TEXT_AIFF,
	/* [1.1.6.4] */		AUDIO_MISSING_TEXT_SND,
	/* [1.1.6.5] */		AUDIO_MISSING_TEXT_RA,
	/* [1.1.6.6] */		AUDIO_MISSING_TEXT_RM,
	/* [1.1.8.1] */		FRAME_MISSING_LONGDESC,
	/* [1.1.9.1] */		AREA_MISSING_ALT,
	/* [1.1.10.1] */	SCRIPT_MISSING_NOSCRIPT,
	/* [1.1.12.1] */	ASCII_REQUIRES_DESCRIPTION,
	/* [1.2.1.1] */		IMG_MAP_SERVER_REQUIRES_TEXT_LINKS,
	/* [1.4.1.1] */		MULTIMEDIA_REQUIRES_TEXT,
	/* [1.5.1.1] */		IMG_MAP_CLIENT_MISSING_TEXT_LINKS,
	/* [2.1.1.1] */		INFORMATION_NOT_CONVEYED_IMAGE,
	/* [2.1.1.2] */		INFORMATION_NOT_CONVEYED_APPLET,
	/* [2.1.1.3] */		INFORMATION_NOT_CONVEYED_OBJECT,
	/* [2.1.1.4] */		INFORMATION_NOT_CONVEYED_SCRIPT,
	/* [2.1.1.5] */		INFORMATION_NOT_CONVEYED_INPUT,
	/* [2.2.1.1] */		COLOR_CONTRAST_TEXT,
	/* [2.2.1.2] */		COLOR_CONTRAST_LINK,
	/* [2.2.1.3] */		COLOR_CONTRAST_ACTIVE_LINK,
	/* [2.2.1.4] */		COLOR_CONTRAST_VISITED_LINK,
	/* [3.2.1.1] */		DOCTYPE_MISSING,
	/* [3.3.1.1] */		STYLE_SHEET_CONTROL_PRESENTATION,
	/* [3.5.1.1] */		HEADERS_IMPROPERLY_NESTED,
	/* [3.5.2.1] */		POTENTIAL_HEADER_BOLD,
	/* [3.5.2.2] */		POTENTIAL_HEADER_ITALICS,
	/* [3.5.2.3] */		POTENTIAL_HEADER_UNDERLINE,
	/* [3.5.3.1] */		HEADER_USED_FORMAT_TEXT,
	/* [3.6.1.1] */		LIST_USAGE_INVALID_UL,
	/* [3.6.1.2] */		LIST_USAGE_INVALID_OL,
	/* [3.6.1.4] */		LIST_USAGE_INVALID_LI,
	/* [4.1.1.1] */		INDICATE_CHANGES_IN_LANGUAGE,
	/* [4.3.1.1] */		LANGUAGE_NOT_IDENTIFIED,
	/* [4.3.1.1] */		LANGUAGE_INVALID,
	/* [5.1.2.1] */		DATA_TABLE_MISSING_HEADERS,
	/* [5.1.2.2] */		DATA_TABLE_MISSING_HEADERS_COLUMN,
	/* [5.1.2.3] */		DATA_TABLE_MISSING_HEADERS_ROW,
	/* [5.2.1.1] */		DATA_TABLE_REQUIRE_MARKUP_COLUMN_HEADERS,
	/* [5.2.1.2] */		DATA_TABLE_REQUIRE_MARKUP_ROW_HEADERS,
	/* [5.3.1.1] */		LAYOUT_TABLES_LINEARIZE_PROPERLY,
	/* [5.4.1.1] */		LAYOUT_TABLE_INVALID_MARKUP,
	/* [5.5.1.1] */		TABLE_MISSING_SUMMARY,
	/* [5.5.1.2] */		TABLE_SUMMARY_INVALID_NULL,
	/* [5.5.1.3] */		TABLE_SUMMARY_INVALID_SPACES,
	/* [5.5.1.6] */		TABLE_SUMMARY_INVALID_PLACEHOLDER,
	/* [5.5.2.1] */		TABLE_MISSING_CAPTION,
	/* [5.6.1.1] */		TABLE_MAY_REQUIRE_HEADER_ABBR,
	/* [5.6.1.2] */		TABLE_MAY_REQUIRE_HEADER_ABBR_NULL,
	/* [5.6.1.3] */		TABLE_MAY_REQUIRE_HEADER_ABBR_SPACES,
	/* [6.1.1.1] */		STYLESHEETS_REQUIRE_TESTING_LINK,
	/* [6.1.1.2] */		STYLESHEETS_REQUIRE_TESTING_STYLE_ELEMENT,
	/* [6.1.1.3] */		STYLESHEETS_REQUIRE_TESTING_STYLE_ATTR,
	/* [6.2.1.1] */		FRAME_SRC_INVALID,
	/* [6.2.2.1] */		TEXT_EQUIVALENTS_REQUIRE_UPDATING_APPLET,
	/* [6.2.2.2] */		TEXT_EQUIVALENTS_REQUIRE_UPDATING_SCRIPT,
	/* [6.2.2.3] */		TEXT_EQUIVALENTS_REQUIRE_UPDATING_OBJECT,
	/* [6.3.1.1] */		PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_SCRIPT,
	/* [6.3.1.2] */		PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_OBJECT,
	/* [6.3.1.3] */		PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_EMBED,
	/* [6.3.1.4] */		PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_APPLET,
	/* [6.5.1.1] */		FRAME_MISSING_NOFRAMES,
	/* [6.5.1.2] */		NOFRAMES_INVALID_NO_VALUE,
	/* [6.5.1.3] */		NOFRAMES_INVALID_CONTENT,
	/* [6.5.1.4] */		NOFRAMES_INVALID_LINK,
	/* [7.1.1.1] */		REMOVE_FLICKER_SCRIPT,
	/* [7.1.1.2] */		REMOVE_FLICKER_OBJECT,
	/* [7.1.1.3] */		REMOVE_FLICKER_EMBED,
	/* [7.1.1.4] */		REMOVE_FLICKER_APPLET,
	/* [7.1.1.5] */		REMOVE_FLICKER_ANIMATED_GIF,
	/* [7.2.1.1] */		REMOVE_BLINK_MARQUEE,
	/* [7.4.1.1] */		REMOVE_AUTO_REFRESH,
	/* [7.5.1.1] */		REMOVE_AUTO_REDIRECT,
	/* [8.1.1.1] */		ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_SCRIPT,
	/* [8.1.1.2] */		ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_OBJECT,
	/* [8.1.1.3] */		ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_APPLET,
	/* [8.1.1.4] */		ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_EMBED,
	/* [9.1.1.1] */		IMAGE_MAP_SERVER_SIDE_REQUIRES_CONVERSION,
	/* [9.3.1.1] */		SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_DOWN,
	/* [9.3.1.2] */		SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_UP,
	/* [9.3.1.3] */		SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_CLICK,
	/* [9.3.1.4] */		SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_OVER,
	/* [9.3.1.5] */		SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_OUT,
	/* [9.3.1.6] */		SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_MOVE,
	/* [10.1.1.1] */	NEW_WINDOWS_REQUIRE_WARNING_NEW,
	/* [10.1.1.2] */	NEW_WINDOWS_REQUIRE_WARNING_BLANK,
	/* [10.2.1.1] */	LABEL_NEEDS_REPOSITIONING_BEFORE_INPUT,
	/* [10.2.1.2] */	LABEL_NEEDS_REPOSITIONING_AFTER_INPUT,
	/* [10.4.1.1] */	FORM_CONTROL_REQUIRES_DEFAULT_TEXT,
	/* [10.4.1.2] */	FORM_CONTROL_DEFAULT_TEXT_INVALID_NULL,
	/* [10.4.1.3] */	FORM_CONTROL_DEFAULT_TEXT_INVALID_SPACES,
	/* [11.2.1.1] */	REPLACE_DEPRECATED_HTML_APPLET,
	/* [11.2.1.2] */	REPLACE_DEPRECATED_HTML_BASEFONT,
	/* [11.2.1.3] */	REPLACE_DEPRECATED_HTML_CENTER,
	/* [11.2.1.4] */	REPLACE_DEPRECATED_HTML_DIR,
	/* [11.2.1.5] */	REPLACE_DEPRECATED_HTML_FONT,
	/* [11.2.1.6] */	REPLACE_DEPRECATED_HTML_ISINDEX,
	/* [11.2.1.7] */	REPLACE_DEPRECATED_HTML_MENU,
	/* [11.2.1.8] */	REPLACE_DEPRECATED_HTML_S,
	/* [11.2.1.9] */	REPLACE_DEPRECATED_HTML_STRIKE,
	/* [11.2.1.10] */	REPLACE_DEPRECATED_HTML_U,
	/* [12.1.1.1] */	FRAME_MISSING_TITLE,
	/* [12.1.1.2] */	FRAME_TITLE_INVALID_NULL,
	/* [12.1.1.3] */	FRAME_TITLE_INVALID_SPACES,
	/* [12.4.1.1] */	ASSOCIATE_LABELS_EXPLICITLY,
	/* [12.4.1.2] */	ASSOCIATE_LABELS_EXPLICITLY_FOR,
	/* [12.4.1.3] */	ASSOCIATE_LABELS_EXPLICITLY_ID,
	/* [13.1.1.1] */	LINK_TEXT_NOT_MEANINGFUL,
	/* [13.1.1.2] */	LINK_TEXT_MISSING,
	/* [13.1.1.3] */	LINK_TEXT_TOO_LONG,
	/* [13.1.1.4] */	LINK_TEXT_NOT_MEANINGFUL_CLICK_HERE,
	/* [13.1.1.5] */	LINK_TEXT_NOT_MEANINGFUL_MORE,
	/* [13.1.1.6] */	LINK_TEXT_NOT_MEANINGFUL_FOLLOW_THIS,
	/* [13.2.1.1] */	METADATA_MISSING,
	/* [13.2.1.2] */	METADATA_MISSING_LINK,
	/* [13.2.1.3] */	METADATA_MISSING_REDIRECT_AUTOREFRESH,
	/* [13.10.1.1] */	SKIPOVER_ASCII_ART,
	
	LAST_ACCESS_ERR	/* must be last */
};


/* 
	List of error/warning messages to be displayed.  The error code corresponds
	to the check that is listed in the AERT (HTML specifications).
*/
static char *errorMsgs[] =  
{
	"[1.1.1.1]: <img> missing 'alt' text.",
	"[1.1.1.2]: suspicious 'alt' text (filename).",
	"[1.1.1.3]: suspicious 'alt' text (file size).",
	"[1.1.1.4]: suspicious 'alt' text (placeholder).",
	"[1.1.1.10]: suspicious 'alt' text (too long).",
	"[1.1.1.11]: <img> missing 'alt' text (bullet).",
	"[1.1.1.12]: <img> missing 'alt' text (horizontal rule).",
	"[1.1.2.1]: <img> missing 'longdesc' and d-link.",
	"[1.1.2.2]: <img> missing d-link.",
	"[1.1.2.3]: <img> missing 'longdesc'.",
	"[1.1.2.5]: 'longdesc' not required.",
	"[1.1.3.1]: <img> (button) missing 'alt' text.", 
	"[1.1.4.1]: <applet> missing alternate content.",
	"[1.1.5.1]: <object> missing alternate content.",
	"[1.1.6.1]: audio missing text transcript (wav).",
	"[1.1.6.2]: audio missing text transcript (au).",
	"[1.1.6.3]: audio missing text transcript (aiff).",
	"[1.1.6.4]: audio missing text transcript (snd).",
	"[1.1.6.5]: audio missing text transcript (ra).",
	"[1.1.6.6]: audio missing text transcript (rm).",
	"[1.1.8.1]: <frame> may require 'longdesc'.",
	"[1.1.9.1]: <area> missing 'alt' text.",
	"[1.1.10.1]: <script> missing <noscript> section.",
	"[1.1.12.1]: ascii art requires description.",
	"[1.2.1.1]: image map (server-side) requires text links.",
	"[1.4.1.1]: multimedia requires synchronized text equivalents.", 
	"[1.5.1.1]: image map (client-side) missing text links.",
	"[2.1.1.1]: ensure information not conveyed through color alone (image).",
	"[2.1.1.2]: ensure information not conveyed through color alone (applet).",
	"[2.1.1.3]: ensure information not conveyed through color alone (object).",
	"[2.1.1.4]: ensure information not conveyed through color alone (script).",
	"[2.1.1.5]: ensure information not conveyed through color alone (input).",
	"[2.2.1.1]: poor color contrast (text).",
	"[2.2.1.2]: poor color contrast (link).",
	"[2.2.1.3]: poor color contrast (active link).",
	"[2.2.1.4]: poor color contrast (visited link).",
	"[3.2.1.1]: <doctype> missing.",
	"[3.3.1.1]: use style sheets to control presentation.",
	"[3.5.1.1]: headers improperly nested.",
	"[3.5.2.1]: potential header (bold).",
	"[3.5.2.2]: potential header (italics).",
	"[3.5.2.3]: potential header (underline).",
	"[3.5.3.1]: header used to format text.",
	"[3.6.1.1]: list usage invalid <ul>.",
	"[3.6.1.2]: list usage invalid <ol>.",
	"[3.6.1.4]: list usage invalid <li>.",
	"[4.1.1.1]: indicate changes in language.",
	"[4.3.1.1]: language not identified.",
	"[4.3.1.2]: language attribute invalid.",
	"[5.1.2.1]: data <table> missing row/column headers (all).",
	"[5.1.2.2]: data <table> missing row/column headers (1 col).",
	"[5.1.2.3]: data <table> missing row/column headers (1 row).",
	"[5.2.1.1]: data <table> may require markup (column headers).",
	"[5.2.1.2]: data <table> may require markup (row headers).",
	"[5.3.1.1]: verify layout tables linearize properly.",
	"[5.4.1.1]: invalid markup used in layout <table>.",
	"[5.5.1.1]: <table> missing summary.",
	"[5.5.1.2]: <table> summary invalid (null).",
	"[5.5.1.3]: <table> summary invalid (spaces).",
	"[5.5.1.6]: <table> summary invalid (placeholder text).",
	"[5.5.2.1]: <table> missing <caption>.",
	"[5.6.1.1]: <table> may require header abbreviations.",
	"[5.6.1.2]: <table> header abbreviations invalid (null).",
	"[5.6.1.3]: <table> header abbreviations invalid (spaces).",
	"[6.1.1.1]: style sheets require testing (link).",
	"[6.1.1.2]: style sheets require testing (style element).",
	"[6.1.1.3]: style sheets require testing (style attribute).",
	"[6.2.1.1]: <frame> source invalid.",
	"[6.2.2.1]: text equivalents require updating (applet).",
	"[6.2.2.2]: text equivalents require updating (script).",
	"[6.2.2.3]: text equivalents require updating (object).",
	"[6.3.1.1]: programmatic objects require testing (script).",
	"[6.3.1.2]: programmatic objects require testing (object).",
	"[6.3.1.3]: programmatic objects require testing (embed).",
	"[6.3.1.4]: programmatic objects require testing (applet).",
	"[6.5.1.1]: <frameset> missing <noframes> section.", 
	"[6.5.1.2]: <noframes> section invalid (no value).",
	"[6.5.1.3]: <noframes> section invalid (content).",
	"[6.5.1.4]: <noframes> section invalid (link).",
	"[7.1.1.1]: remove flicker (script).",
	"[7.1.1.2]: remove flicker (object).",
	"[7.1.1.3]: remove flicker (embed).",
	"[7.1.1.4]: remove flicker (applet).",
	"[7.1.1.5]: remove flicker (animated gif).",
	"[7.2.1.1]: remove blink/marquee.",
	"[7.4.1.1]: remove auto-refresh.",
	"[7.5.1.1]: remove auto-redirect.",
	"[8.1.1.1]: ensure programmatic objects are accessible (script).",
	"[8.1.1.2]: ensure programmatic objects are accessible (object).",
	"[8.1.1.3]: ensure programmatic objects are accessible (applet).",
	"[8.1.1.4]: ensure programmatic objects are accessible (embed).",
	"[9.1.1.1]: image map (server-side) requires conversion.",
	"[9.3.1.1]: <script> not keyboard accessible (onMouseDown).",
	"[9.3.1.2]: <script> not keyboard accessible (onMouseUp).",
	"[9.3.1.3]: <script> not keyboard accessible (onClick).",
	"[9.3.1.4]: <script> not keyboard accessible (onMouseOver).",
	"[9.3.1.5]: <script> not keyboard accessible (onMouseOut).",
	"[9.3.1.6]: <script> not keyboard accessible (onMouseMove).",
	"[10.1.1.1]: new windows require warning (_new).",
	"[10.1.1.2]: new windows require warning (_blank).",
	"[10.2.1.1]: <label> needs repositioning (<label> before <input>).",
	"[10.2.1.2]: <label> needs repositioning (<label> after <input>).",
	"[10.4.1.1]: form control requires default text.",
	"[10.4.1.2]: form control default text invalid (null).",
	"[10.4.1.3]: form control default text invalid (spaces).",
	"[11.2.1.1]: replace deprecated html <applet>.",
	"[11.2.1.2]: replace deprecated html <basefont>.",
	"[11.2.1.3]: replace deprecated html <center>.",
	"[11.2.1.4]: replace deprecated html <dir>.",
	"[11.2.1.5]: replace deprecated html <font>.",
	"[11.2.1.6]: replace deprecated html <isindex>.",
	"[11.2.1.7]: replace deprecated html <menu>.",
	"[11.2.1.8]: replace deprecated html <s>.",
	"[11.2.1.9]: replace deprecated html <strike>.",
	"[11.2.1.10]: replace deprecated html <u>.",
	"[12.1.1.1]: <frame> missing title.",
	"[12.1.1.2]: <frame> title invalid (null).",
	"[12.1.1.3]: <frame> title invalid (spaces).",
	"[12.4.1.1]: associate labels explicitly with form controls.",
	"[12.4.1.2]: associate labels explicitly with form controls (for).",
	"[12.4.1.3]: associate labels explicitly with form controls (id).",
	"[13.1.1.1]: link text not meaningful.",
	"[13.1.1.2]: link text missing.",
	"[13.1.1.3]: link text too long.",
	"[13.1.1.4]: link text not meaningful (click here).",
	"[13.1.1.5]: link text not meaningful (more).",
	"[13.1.1.6]: link text not meaningful (follow this).",
	"[13.2.1.1]: Metadata missing.",
	"[13.2.1.2]: Metadata missing (link element).",
	"[13.2.1.3]: Metadata missing (redirect/auto-refresh).",
 	"[13.10.1.1]: skip over ascii art."
};


/* function prototypes - TRT */
static void GetRgbForeGround (Lexer* lexer, Node* node, char* color);
static void GetRgbBackGround (Lexer* lexer, Node* node, char* color);
static int CharToNumber (char character);
static void DisplayHTMLTableAlgorithm (FILE *errout); /* TRT */
static void CheckMapAccess (Lexer* lexer, Node* node, Node* front); /* TRT */
static void GetMapLinks (Lexer* lexer, Node* node, Node* front);
static void CompareAnchorLinks (Lexer* lexer, Node* front, int counter);
static void FindMissingLinks (Lexer* lexer, Node* node, int counter);
static Bool CompareColors (Lexer* lexer, Node* node, int red1, int green1, 
					               int blue1, int red2, int green2, int blue2);
static void CheckFormControls (Lexer* lexer, Node* node);
static void MetaDataPresent (Lexer* lexer, Node* node);
static void CheckEmbed (Lexer* lexer, Node* node);
static void CheckListUsage (Lexer* lexer, Node* node);

/*
	GetFileExtension takes a path and returns the extension portion of the path (if any).
 */

static void GetFileExtension (const char *path, char *ext) /* TRT */
{
	int		len = strlen(path);
	int		i = len - 1;
	
	ext[0] = '\0';
	
	do {
		if ((path[i] == '/') || (path[i] == '\\'))
			break; else
		if (path[i] == '.')
		{
			int		j;
			
			for (j = 0; j < len - i; ++i, j++)
				ext[j] = path[i];
			break;
		} else
			i--;
	} while (i > 0);
}

/*************************************************************************
*********
* ReportWarningsAndErrors
*
* Reports and displays errors/warnings.
**************************************************************************
*********/

static void ReportWarningsAndErrors (Lexer* lexer, Node* node, int ErrorType, int errorCode) /* TRT */
{
	char *error;	/* TRT */
	
	error = errorMsgs[errorCode];

	lexer->badAccess = yes;	/* TRT */
    
    /* keep quiet after <ShowErrors> errors */
	if (lexer->errors > ShowErrors)
	{
	    return;
	}
	
	/* TRT */
	tidy_out(lexer->errout, "line %d", node->line);
	tidy_out(lexer->errout, " column %d", node->column);
	
	/* TRT */
	if (ErrorType == ACCESS_WARNING_ERROR)
	{
		lexer->warnings++;
		tidy_out(lexer->errout, " - Warning %s\n", error);
	} else

	if (ErrorType == ACCESS_FATAL_ERROR)
	{
		lexer->errors++;
		tidy_out(lexer->errout, " - Error %s\n", error);
	}
}


/************************************************************************
******
* IsImage
*
* Checks if the given filename is an image file.
* Returns 'yes' if it is, 'no' if it's not.
*************************************************************************
*****/

static Bool IsImage (char* iType) /* TRT */
{
	int i; /* TRT */

	/* Get the file extension */
	char fileExtension[20];
	GetFileExtension (iType, fileExtension); /* TRT */

	/* Compare it to the array of known image file extensions */
	for (i = 0; i < 13; i++) /* TRT */
	{
		if (wstrcasecmp(fileExtension, arrayImageExtensions[i]) == 0) /* TRT */
		{
			return yes;
		}
	}
	
	return no;
}


/***********************************************************************
********
* IsSoundFile
*
* Checks if the given filename is a sound file.
* Returns 'yes' if it is, 'no' if it's not.
************************************************************************
*********/

static Bool IsSoundFile (char* sType) /* TRT */
{
	int i;
	
	/* Gets the file extension */
	char fileExtension[100];
	GetFileExtension (sType, fileExtension); /* TRT */

	/* 
	   Compares the given file extension with the list of 
	   possible sound file extensions 
	*/
	for (i = 0; i < 13; i++)
	{
		if (wstrcasecmp(fileExtension, arraySoundExtensions[i]) == 0) /* TRT */
		{
			return yes;
		}
	}
	
	return no;
}


/***********************************************************************
*******
* IsValidSrcExtension
*
* Checks if the 'SRC' value within the FRAME element is valid
* The 'SRC' extension must end in ".htm", ".html", ".shtm", ".shtml", 
* ".cfm", ".cfml", ".asp", ".cgi", ".pl", or ".smil"
*
* Returns yes if it is, returns no otherwise.
************************************************************************
*******/

static Bool IsValidSrcExtension(char* sType) /* TRT */
{
	int i;

	char fileExtension[20];
	GetFileExtension (sType, fileExtension); /* TRT */

	/*
	   Compares the given file extension with the list of
	   possible src file extensions
	*/
	
	for (i = 0; i < 10; i++)
	{
		if (wstrcasecmp(fileExtension, arrayFrameExtensions[i]) == 0) /* TRT */
		{
			return yes;
		}
	}

	return no;
}


/*********************************************************************
********
* IsValidMediaExtension
*
* Checks to warn the user that syncronized text equivalents are 
* required if multimedia is used.
**********************************************************************
********/

static Bool IsValidMediaExtension(char* sType) /* TRT */
{
	int i;

	char fileExtension[20];
	GetFileExtension (sType, fileExtension); /* TRT */

	/*
	   Compares the given file extension with the list of
	   possible src file extensions
	*/
	
	/* printf ("%s\n", fileExtension); */
	for (i = 0; i < 10; i++)
	{
		if (wstrcasecmp(fileExtension, arrayMediaExtensions[i]) == 0) /* TRT */
		{
			return yes;
		}
	}

	return no;
}


/************************************************************************
******
* IsWhitespace
*
* Checks if the given string is all whitespace.
* Returns 'yes' if it is, 'no' if it's not.
*************************************************************************
*****/

static Bool IsWhitespace (char* pString) /* TRT */
{
	int x;

	for (x = 0; x < (int)strlen (pString); x++)
	{
		/* Checks for whitespace within attribute value */
		if ((pString[x] != ' ') &&
			(pString[x] != '\t'))
		{
			return no;
		}
	}
	return yes;
}


/***********************************************************************
********
* IsPlaceholderAlt
*  
* Checks to see if there is an image and photo place holder contained
* in the ALT text.
*
* Returns 'yes' if there is, 'no' if not.
************************************************************************
********/

static Bool IsPlaceholderAlt(char* str) /* TRT */
{
	/* Checks to see if 'image' or 'photo' is contained within the alt text. */
	if ((strstr(str, "image") != NULL) || 
		(strstr(str, "photo") != NULL))
	{
		return yes;
	}
	
	return no;
}


/***********************************************************************
********
* IsPlaceholderTitle
*  
* Checks to see if there is an TITLE place holder contained
* in the 'ALT' text.
*
* Returns 'yes' if there is, 'no' if not.
************************************************************************
********/

static Bool IsPlaceHolderTitle(char* str) /* TRT */
{
	/* Checks to see if 'title' is contained within the alt text. */
	if (strstr(str, "title") != NULL)
	{
		return yes;
	}
	
	return no;
}


/***********************************************************************
********
* IsPlaceHolderObject
*  
* Checks to see if there is an OBJECT place holder contained
* in the 'ALT' text.
*
* Returns 'yes' if there is, 'no' if not.
************************************************************************
********/

static Bool IsPlaceHolderObject(char* str) /* TRT */
{
	/* Checks to see if 'object' is contained within the alt text. */
	if (strstr(str, "object") != NULL)
	{
		return yes;
	}
	
	return no;
}


/**********************************************************
******
* EndsWithBytes
*
* Checks to see if the ALT text ends with 'bytes'
* Returns 'yes', if true, 'no' otherwise.
***********************************************************
******/

static Bool EndsWithBytes(char* str) /* TRT */
{
	/* Checks that the ALT text end with 'bytes' */
	if (strcmp (&str[strlen(str) - 5], "bytes") == 0)
	{
		return yes;
	}

	return no;
}


/*******************************************************
*********
* textFromOneNode
*
* Returns a list of characters contained within one
* text node.
********************************************************
*********/

static char* textFromOneNode (Lexer* lexer, Node* node) /* TRT */
{
	uint i;

	int x = 0;
	
	/* Clears the list of characters */
	text[0] = 0;

	/* TRT */
	if (node == NULL)
	{
		return text;
	}

	/* Retrieves the list of characters found within a text node */
	for (i = node->start; i < node->end; i++)
	{
		text[x] = (char)lexer->lexbuf[i];
		
		x++;

		/* Text node must not contain more that 100 characters */
		if (x > 99)
		{
		   	break;
		}
	}

	text[x] = 0;

	return text;
}


/*********************************************************
*********
* getTextNode
*
* Locates text nodes within a container element.
* Retrieves text that are found contained within 
* text nodes, and concatenates the text.
**********************************************************
********/
	
static void getTextNode (Lexer* lexer, Node* node) /* TRT */
{
	Node* TNode;
		
	uint i;
	
	/* 
	   Continues to traverse through container element until it no
	   longer contains any more contents 
	*/
	
	if (node == NULL)
	{
		return;
	}

	/* If the tag of the node is NULL, then grab the text within the node */
	if (node->tag == NULL)
	{
		/* Retrieves each character found within the text node */
		for (i = node->start; i < node->end; i++)
		{
			textNode[counter] = (char)lexer->lexbuf[i];
							
			counter++;
			
			/* The text must not exceed 100 characters */
			if (counter > 99)
			{
		    	break;
			}
		}
	}   

	/* Traverses through the contents within a container element */
	for (TNode = node->content; TNode != NULL; TNode = TNode->next)
	{
		getTextNode (lexer, TNode);
	}
	
	textNode[counter] = 0;
}


/**********************************************************
*********
* getTextNodeClear
*
* Clears the current 'textNode' and reloads it with new
* text.  The textNode must be cleared before use.
***********************************************************
********/

static char* getTextNodeClear (Lexer* lexer, Node* node) /* TRT */
{
	/* Clears list */
	textNode[0] = 0;

	getTextNode (lexer, node);

	return textNode;
}
	

/********************************************************
********
* CheckColorAvailable
*
* Verify that information conveyed with color is 
* available without color.
*********************************************************
********/

static void CheckColorAvailable (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (strcmp (node->element, "img") == 0)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, INFORMATION_NOT_CONVEYED_IMAGE);
		}

		if (strcmp (node->element, "applet") == 0)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, INFORMATION_NOT_CONVEYED_APPLET);
		}

		if (strcmp (node->element, "object") == 0)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, INFORMATION_NOT_CONVEYED_OBJECT);
		}

		if (strcmp (node->element, "script") == 0)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, INFORMATION_NOT_CONVEYED_SCRIPT);
		}

		if (strcmp (node->element, "input") == 0)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, INFORMATION_NOT_CONVEYED_INPUT);
		}
	}
}


/***************************************************
***********
* MaxColor
* 
* Returns the largest of two colour values 
****************************************************
***********/

static int maxColor (int colour1, int colour2) /* TRT */
{
	if (colour1 > colour2)
	{
		return colour1;
	}

	return colour2;
}


/***************************************************
***********
* MinColor
* 
* Returns the smallest of two colour values 
****************************************************
***********/

static int minColor (int colour1, int colour2) /* TRT */
{
	if (colour1 > colour2)
	{
		return colour2;
	}

	return colour1;
}


/*********************************************************************
*******
* CheckColorContrast
*
* Checks elements for color contrast.  Must have valid contrast for
* valid visibility.
**********************************************************************
*******/

static int red;
static int green;
static int blue;
static int red2;
static int green2;
static int blue2;
static int colorAttrs = 0;

static void CheckColorContrast (Lexer* lexer, Node* node) /* TRT */
{
	if (PRIORITYCHK == 3)
	{
		/* Check for 'BGCOLOR' first to compare with other color attributes */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{			
			if (attval->dict == attr_bgcolor)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					GetRgbBackGround (lexer, node, attval->value);
				}
			}
		}
		
		/* 
		   Search for COLOR attributes to compare with background color
		   Must have valid colour contrast
		*/
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			if (attval->dict == attr_text)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					GetRgbForeGround (lexer, node, attval->value);
	
					if (colorAttrs == 1)
					{
						if (CompareColors (lexer, node, red, green, blue, red2, green2, blue2) == no)
						{
							ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, COLOR_CONTRAST_TEXT);
						}
											
						colorAttrs = 0;
					}
				}
			}

			if (attval->dict == attr_alink)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					GetRgbForeGround (lexer, node, attval->value);

					if (colorAttrs == 1)
					{
						if (CompareColors (lexer, node, red, green, blue, red2, green2, blue2) == no)
						{
							ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, COLOR_CONTRAST_ACTIVE_LINK);
						}
											
						colorAttrs = 0;
					}
				}
			}

			if (attval->dict == attr_vlink)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					GetRgbForeGround (lexer, node, attval->value);

					if (colorAttrs == 1)
					{
						if (CompareColors (lexer, node, red, green, blue, red2, green2, blue2) == no)
						{
							ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, COLOR_CONTRAST_VISITED_LINK);
						}
											
						colorAttrs = 0;
					}
				}
			}

			if (attval->dict == attr_link)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					GetRgbForeGround (lexer, node, attval->value);

					if (colorAttrs == 1)
					{
						if (CompareColors (lexer, node, red, green, blue, red2, green2, blue2) == no)
						{
							ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, COLOR_CONTRAST_LINK);
						}
											
						colorAttrs = 0;
					}
				}
			}
		}
	}
}


/***********************************************************************
***********
* CompareColors
*
* Compares two RGB colors to see if there is good contrast between them.
************************************************************************
***********/

static Bool CompareColors (Lexer* lexer, Node* node, int red1, int green1, 
					               int blue1, int red2, int green2, int blue2) /* TRT */
{
	int bright1 = ((red1 * 299) + (green1 * 587) + (blue1 * 114)) / 1000;
	int bright2 = ((red2 * 299) + (green2 * 587) + (blue2 * 114)) / 1000;

	int differenceBright = maxColor (bright1, bright2) - minColor (bright1, bright2); /* TRT */

	/* TRT */
	int differenceColor = (maxColor (red1, red2) - minColor (red1, red2)) +
	   (maxColor (green1, green2) - minColor (green1, green2)) +
	   (maxColor (blue1, blue2) - minColor (blue1, blue2));

	if ((differenceBright > 180) &&
		(differenceColor > 500))
	{
		return yes;
	}

	return no;
}


/*********************************************************************
***********
* GetRgbBackGround
*
* Gets the red, green and blue values for this attribute for the 
* background.
*
* Example: If attribute is BGCOLOR="#121005" then red = 18, green = 16,
* blue = 5.
**********************************************************************
***********/

static void GetRgbBackGround (Lexer* lexer, Node* node, char* color) /* TRT */
{
	int x; /* TRT */

	/* Check if we have a color name */
	for (x = 0; x < 16; x++) /* TRT */
	{
		if (strstr(arrayColorNames[x], color) != NULL)
		{
			red  = arrayColorValues[x][0];
			green = arrayColorValues[x][1];
			blue = arrayColorValues[x][2];
		}
	}

	/*
	   No color name so must be hex values 
	   Is this a number in hexadecimal format?
	*/
	
	if (color[0] == '#')
	{
		/* Must be 7 characters in the RGB value (including '#') */
		if (strlen(color) == 7)
		{
			red  = (CharToNumber (color[1]) * 16) + CharToNumber
																(color[2]);
			green = (CharToNumber (color[3]) * 16) + CharToNumber
														(color[4]);
			blue = (CharToNumber (color[5]) * 16) + CharToNumber
															(color[6]);
		}
	}
} 


/*********************************************************************
***********
* GetRgbBackGround
*
* Gets the red, green and blue values for this attribute for the 
* background.
*
* Example: If attribute is BGCOLOR="#121005" then red = 18, green = 16,
* blue = 5.
**********************************************************************
***********/

static void GetRgbForeGround (Lexer* lexer, Node* node, char* color) /* TRT */
{
	int x; /* TRT */

	/* Check if we have a color name */
	for (x = 0; x < 16; x++) /* TRT */
	{
		if (strstr(arrayColorNames[x], color) != NULL)
		{
			red2  = arrayColorValues[x][0];
			green2 = arrayColorValues[x][1];
			blue2 = arrayColorValues[x][2];

			colorAttrs = 1;
		}
	}

	/*
	   No color name so must be hex values 
	   Is this a number in hexadecimal format?
	*/
	
	if (color[0] == '#')
	{
		/* Must be 7 characters in the RGB value (including '#') */
		if (strlen(color) == 7)
		{
			red2  = (CharToNumber (color[1]) * 16) + CharToNumber
																(color[2]);
			green2 = (CharToNumber (color[3]) * 16) + CharToNumber
														(color[4]);
			blue2 = (CharToNumber (color[5]) * 16) + CharToNumber
															(color[6]);
			colorAttrs = 1;
		}
	}
} 


/*******************************************************************
**************
* CharToNumber
*
* Converts a character to a number.
* Example: if given character is 'A' then returns 10.
*
* Returns the number that the character represents. Returns -1 if not a
* valid number.
********************************************************************
**************/

static int CharToNumber (char character) /* TRT */
{
	if ((character >= '0') &&
		(character <= '9'))
	{
		 return character - '0';
	}

	if ((character >= 'a') &&
		(character <= 'f'))
	{
		return (character - 'a') + 10;
	}

	if ((character >= 'A') &&
		(character <= 'F'))
	{
		return (character - 'A') + 10;
	}

	return -1;
}


/***********************************************************
********
* CheckImage
*
* Checks all image attributes for specific elements to
* check for validity of the values contained within
* the attributes.  An appropriate warning message is displayed
* to indicate the error.  
************************************************************
********/

static void CheckImage (Lexer* lexer, Node* node) /* TRT */
{
	Bool HasAlt = no;
	Bool HasIsMap = no;
	Bool HasDataFld = no;
	Bool HasLongDesc = no;
	Bool HasDLINK = no;
	Bool HasValidHeight = no;
	Bool HasValidWidthBullet = no;
	Bool HasValidWidthHR = no; 
	Bool HasTriggeredMissingAlt = no;
	Bool HasTriggeredMissingLongDesc = no;
	
	Node* current = node;
	char* word;
				
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks all image attributes for invalid values within attributes */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			/* 
			   Checks for valid ALT attribute.
			   The length of the alt text must be less than 150 characters 
			   long.
			*/
			if (attval->dict == attr_alt)
			{
				if (attval->value != NULL) 
				{
					if ((strlen (attval->value) < 150) &&
						(IsPlaceholderAlt (attval->value) == no) &&
						(IsPlaceHolderObject (attval->value) == no) &&
						(EndsWithBytes (attval->value) == no) &&
						(IsImage (attval->value) == no))
					{
						HasAlt = yes;
					}

					else if (strlen (attval->value) > 150)
					{
						HasAlt = yes;
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_ALT_SUSPICIOUS_TOO_LONG);
					}

					else if (IsImage (attval->value) == yes)
					{
						HasAlt = yes;
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_ALT_SUSPICIOUS_FILENAME);
					}
			
					else if (IsPlaceholderAlt (attval->value) == yes)
					{
						HasAlt = yes;
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_ALT_SUSPICIOUS_PLACEHOLDER);
					}

					else if (EndsWithBytes (attval->value) == yes)
					{
						HasAlt = yes;
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_ALT_SUSPICIOUS_FILE_SIZE);
					}
				}
			}

			/* 
			   Checks for width values of 'bullets' and 'horizontal
			   rules' for validity.

			   Valid pixel width for 'bullets' must be < 30, and > 150 for
			   horizontal rules.
			*/
			else if (attval->dict == attr_width)
			{
				/* Longdesc attribute needed if width attribute is not present. */
				if ((attval->value != NULL) && 
					(IsWhitespace (attval->value) == no))
				{
					if (atoi(attval->value) < 30)
					{
						HasValidWidthBullet = yes;
					}

					if (atoi(attval->value) > 150)
					{
						HasValidWidthHR = yes;
					}
				}
			}

			/* 
			   Checks for height values of 'bullets' and horizontal
			   rules for validity.

			   Valid pixel height for 'bullets' and horizontal rules 
			   mustt be < 30.
			*/
			else if (attval->dict == attr_height)
			{
				/* Longdesc attribute needed if height attribute is not present. */
				if ((attval->value != NULL) &&
					(IsWhitespace (attval->value) == no)&&
					(atoi(attval->value) < 30))
				{
					HasValidHeight = yes;
				}
			}

			/* 
			   Checks for longdesc and determines validity.  
			   The length of the 'longdesc' must be > 1
			*/
			else if (attval->dict == attr_longdesc)
			{
				if (attval->value != NULL) 
				{
					if ((strlen(attval->value) > 1) && 
						(IsWhitespace (attval->value) == no))
					{
						HasLongDesc = yes;
					}
				}
  			}

			/* 
			   Checks for 'USEMAP' attribute.  Ensures that
			   text links are provided for client-side image maps
			*/
			else if (attval->dict == attr_usemap)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					HasUseMap = yes;
				}
			}	

			else if (attval->dict == attr_ismap)
			{
				HasIsMap = yes;
			}
		}	
		
		
		/* 
			Check to see if a dLINK is present.  The ANCHOR element must
			be present following the IMG element.  The text found between 
			the ANCHOR tags must be < 6 characters long, and must contain
			the letter 'd'.
		*/
		if ((node->next != NULL && 
			(node->next)->tag == tag_a))
		{
			node = node->next;
			
			/* 
				Node following the anchor must be a text node
				for dLINK to exist 
			*/

			if(node->content != NULL && (node->content)->tag == NULL)
			{
				/* Number of characters found within the text node */
				word = textFromOneNode (lexer, node->content);
					
				if ((strcmp(word,"d") == 0)||
					(strcmp(word,"D") == 0))
				{
					HasDLINK = yes;
				}
			}
		}
					
		/*
			Special case check for dLINK.  This will occur if there is 
			whitespace between the <img> and <a> elements.  Ignores 
			whitespace and continues check for dLINK.
		*/
		
		if ((node->next != NULL)&&
			((node->next)->tag == NULL))
		{
			node = node->next;

			if (node->next != NULL && (node->next)->tag == tag_a)
			{
				node = node->next;
						
				/* 
					Node following the ANCHOR must be a text node
					for dLINK to exist 
				*/
				if(node->content != NULL && node->content->tag == NULL)
				{
					/* Number of characters found within the text node */
					word = textFromOneNode (lexer, node->content);

					if ((strcmp(word, "d") == 0)||
						(strcmp(word, "D") == 0))
					{
						HasDLINK = yes;
					}
				}
			}	
		}

		if ((HasAlt == no)&&
			(HasValidWidthBullet == yes)&&
			(HasValidHeight == yes))
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, IMG_MISSING_ALT_BULLET);
			HasTriggeredMissingAlt = yes;
		}

		if ((HasAlt == no)&&
			(HasValidWidthHR == yes)&&
			(HasValidHeight == yes))
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, IMG_MISSING_ALT_H_RULE);
			HasTriggeredMissingAlt = yes;
		}
		
		if (HasTriggeredMissingAlt == no)
		{
			if (HasAlt == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, IMG_MISSING_ALT);
			}
		}

		if ((HasLongDesc == no)&&
			(HasValidHeight ==yes)&&
			(HasValidWidthHR == yes)||
			(HasValidWidthBullet == yes))
		{
			HasTriggeredMissingLongDesc = yes;
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LONGDESC_NOT_REQUIRED);
		}

		if (HasTriggeredMissingLongDesc == no)
		{
			if ((HasDLINK == yes)&&
				(HasLongDesc == no))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_MISSING_LONGDESC);
			}

			if ((HasLongDesc == yes)&&
				(HasDLINK == no))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_MISSING_DLINK);
			}
			
			if ((HasLongDesc == no)&&
				(HasDLINK == no))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_MISSING_LONGDESC_DLINK);
			}
		}
		
		if (HasIsMap == yes)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, IMAGE_MAP_SERVER_SIDE_REQUIRES_CONVERSION);

			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, IMG_MAP_SERVER_REQUIRES_TEXT_LINKS);
		}
	}
}


/***********************************************************
*********
* CheckApplet
*
* Checks APPLET element to check for validity pertaining 
* the 'ALT' attribute.  An appropriate warning message is 
* displayed  to indicate the error. An appropriate warning 
* message is displayed to indicate the error.  If no 'ALT'
* text is present, then there must be alternate content
* within the APPLET element.
************************************************************
*********/

static void CheckApplet (Lexer* lexer, Node* node) /* TRT */
{
	Bool HasAlt = no;
	Bool HasDescription = no;

	char* word;
		
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks for attributes within the APPLET element */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			/*
			   Checks for valid ALT attribute.
			   The length of the alt text must be > 4 characters in length
			   but must be < 150 characters long.
			*/

			if (attval->dict == attr_alt)
			{
				if (attval->value != NULL)
				{
					HasAlt = yes;
				}
			}
		}

		if (HasAlt == no)
		{
			/* Must have alternate text representation for that element */
			if (node->content != NULL) 
			{
				if (node->content->tag == NULL)
				{
					word = textFromOneNode (lexer, node->content);
				}

				if (node->content->content != NULL)
				{
					if (node->content->content->tag == NULL)
					{
						word = textFromOneNode (lexer, node->content->content);
					}
				}
				
				if ((word != NULL)&&
					(IsWhitespace(word) == no))
				{
					HasDescription = yes;
				}
			}
		}

		if (HasDescription == no)
		{
			if (HasAlt == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, APPLET_MISSING_ALT);
			}
		}
	}
}


/*******************************************************************
* CheckObject
*
* Checks to verify whether the OBJECT element contains
* 'ALT' text, and to see that the sound file selected is 
* of a valid sound file type.  OBJECT must have an alternate text 
* representation.
********************************************************************
**********/

static void CheckObject (Lexer* lexer, Node* node) /* TRT */
{
	char *word;

	Node* TNode;

	Bool HasAlt = no;
	Bool HasDescription = no;

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->content != NULL)
		{
			if ((node->content)->tag != NULL)
			{
				TNode = node->content;

				for (attval = TNode->attributes; attval != NULL; attval = attval->next)
				{
					if (attval->dict == attr_alt)
					{
						if (attval->value != NULL)
						{
							HasAlt = yes;
							break;
						}
					}
				}
			}

			/* Must have alternate text representation for that element */
			if (HasAlt == no)
			{
				if (node->content->tag == NULL)
				{
					word = textFromOneNode (lexer, node->content);
				}

				if (node->content->content != NULL)
				{
					if (node->content->content->tag == NULL)
					{
						word = textFromOneNode (lexer, node->content->content);
					}
				}
					
				if ((word != NULL)&&
					(IsWhitespace(word) == no))
				{
					HasDescription = yes;
				}
			}
		}

		if (HasAlt == no && HasDescription == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, OBJECT_MISSING_ALT);
		}
	}
}


/***************************************************************
********
* CheckMissingStyleSheets
*
* Ensures that stylesheets are used to control the presentation.
****************************************************************
********/

static void CheckMissingStyleSheets (Lexer* lexer, Node* node)
{
	Node* content;

	for (content = node->content; content != NULL; content = content->next)
	{
		if ((content->tag == tag_link)||
			(content->tag == tag_style))
		{
			StyleSheetsPresent = yes;
		}

		for (attval = content->attributes; attval != NULL; attval = attval->next)
		{
			if ((attval->dict == attr_style)||
				(attval->dict == attr_font)||
				(attval->dict == attr_basefont)||
				(attval->dict == attr_text)||
				(attval->dict == attr_vlink)||
				(attval->dict == attr_alink)||
				(attval->dict == attr_link))
			{
				StyleSheetsPresent = yes;
			}

			if (attval->dict == attr_rel)
			{
				if ((attval->value != NULL)&&
					(strcmp (attval->value, "stylesheet") == 0))
				{
					StyleSheetsPresent = yes;
				}
			}
		}

		CheckMissingStyleSheets (lexer, content);
	}
}

/*******************************************************************
*********
* CheckFrame
*
* Checks if the URL is valid and to check if a 'LONGDESC' is needed
* within the FRAME element.  If a 'LONGDESC' is needed, the value must 
* be valid. The URL must end with the file extension, htm, or html. 
* Also, checks to ensure that the 'SRC' and 'TITLE' values are valid. 
********************************************************************
********/

static void CheckFrame (Lexer* lexer, Node* node) /* TRT */
{
	Bool HasTitle = no;

	numFrames++;

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks for attributes within the FRAME element */
		for (attval = node->attributes; attval != null; attval = attval->next)
		{
			/* Checks if 'LONGDESC' value is valid only if present */
			if (attval->dict == attr_longdesc)
			{
				if (((attval->value) != NULL) &&
					(strlen(attval->value) > 1) && 
					(IsWhitespace (attval->value) == no))
				{
					HasCheckedLongDesc++;
				}
			}

			/* Checks for valid 'SRC' value within the frame element */
			else if (attval->dict == attr_src)
			{
				if ((attval->value != NULL) &&
					(IsWhitespace (attval->value) == no))
				{
					if (IsValidSrcExtension (attval->value) == no)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FRAME_SRC_INVALID);
					}
				}
			}

			/* Checks for valid 'TITLE' value within frame element */
			else if (attval->dict == attr_title)
			{
				if ((attval->value != NULL) &&
					(IsWhitespace (attval->value) == no))
				{
					HasTitle = yes;
				}

				if (HasTitle == no)
				{
					if ((attval->value == NULL)||
						(strlen (attval->value) == 0))
					{
						HasTitle = yes;
						ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FRAME_TITLE_INVALID_NULL);
					}

					else
					{
						if ((IsWhitespace (attval->value) == yes)&&
							(strlen (attval->value) > 0))
						{
							HasTitle = yes;
							ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FRAME_TITLE_INVALID_SPACES);
						}
					}
				}
			}
		}

		if (HasTitle == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FRAME_MISSING_TITLE);
		}

		if (numFrames == 3 && HasCheckedLongDesc < 3)
		{
			numFrames = 0;
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, FRAME_MISSING_LONGDESC);
		}
	}
}


/****************************************************************
*********
* CheckIFrame
*
* Checks if 'SRC' value is valid.  Must end in appropriate
* file extension.
*****************************************************************
********/

static void CheckIFrame (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks for attributes within the IFRAME element */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			/* Checks for valid 'SRC' value within the frame element */
			if (attval->dict == attr_src)
			{
				/* The check for validitiy */
				if ((attval->value != NULL) &&
					(IsWhitespace (attval->value) == no))
				{
					if (IsValidSrcExtension (attval->value) == no)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FRAME_SRC_INVALID);
					}
				}
			}
		}
	}
}


/**********************************************************************
********
* CheckAnchor
*
* Checks that the sound file is valid, and to ensure that
* text transcript is present describing the 'HREF' within the 
* ANCHOR element.  Also checks to see ensure that the 'TARGET' attribute
* (if it exists) is not null and does not contain '_new' or '_blank'.
***********************************************************************
********/

static void CheckAnchor (Lexer* lexer, Node* node) /* TRT */
{
	char *word;
	int checked = 0;
	Bool HasDescription = no;
	Bool HasTriggeredLink = no;

	/* Checks for attributes within the ANCHOR element */
	for (attval = node->attributes; attval != NULL; attval = attval->next)
	{
		if ((PRIORITYCHK == 1)||
			(PRIORITYCHK == 2)||
			(PRIORITYCHK == 3))	
		{
			/* Must be of valid sound file type */
			if (attval->dict == attr_href)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					char fileExtension[100];
					GetFileExtension (attval->value, fileExtension); /* TRT */

					/* Checks to see if multimedia is used */
					if (IsValidMediaExtension (attval->value) == yes)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, MULTIMEDIA_REQUIRES_TEXT);
					}
			
					/* 
						Checks for validity of sound file, and checks to see if 
						the file is described within the document, or by a link
						that is present which gives the description.
					*/
									
					if ((strlen (fileExtension) < 6)&&
						(strlen (fileExtension) > 0))
					{
						if (IsSoundFile(attval->value) == yes)
						{
							if (node->next != NULL)
							{
								if (node->next->tag == NULL)
								{
									word = textFromOneNode (lexer, node->next);
								
									/* Must contain at least one letter in the text */
									if (IsWhitespace (word) == no)
									{
										HasDescription = yes;
									}
								}
							}

							/* Must contain text description of sound file */
							if (HasDescription == no)
							{
								if (strcmp (fileExtension, ".wav") == 0)
								{
									ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, AUDIO_MISSING_TEXT_WAV);
								}

								if (strcmp (fileExtension, ".au") == 0)
								{
									ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, AUDIO_MISSING_TEXT_AU);
								}

								if (strcmp (fileExtension, ".aiff") == 0)
								{
									ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, AUDIO_MISSING_TEXT_AIFF);
								}

								if (strcmp (fileExtension, ".snd") == 0)
								{
									ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, AUDIO_MISSING_TEXT_SND);
								}

								if (strcmp (fileExtension, ".ra") == 0)
								{
									ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, AUDIO_MISSING_TEXT_RA);
								}

								if (strcmp (fileExtension, ".rm") == 0)
								{
									ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, AUDIO_MISSING_TEXT_RM);
								}
							}
						}
					}
				}
			}
		}

		if ((PRIORITYCHK == 2)||
			(PRIORITYCHK == 3))
		{
			/* Checks 'TARGET' attribute for validity if it exists */
			if (attval->dict == attr_target)
			{
				checked = 1;

				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					if (strcmp (attval->value, "_new") == 0)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, NEW_WINDOWS_REQUIRE_WARNING_NEW);
					}
					
					if (strcmp (attval->value, "_blank") == 0)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, NEW_WINDOWS_REQUIRE_WARNING_BLANK);
					}
				}
			}
		}
	}
	
	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if ((node->content != NULL)&&
			(node->content->tag == NULL))
		{
			word = textFromOneNode (lexer, node->content);

			if ((word != NULL)&&
				(IsWhitespace (word) == no))
			{
				if (strcmp (word, "more") == 0)
				{
					HasTriggeredLink = yes;
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LINK_TEXT_NOT_MEANINGFUL_MORE);
				}

				if (strcmp (word, "follow this") == 0)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LINK_TEXT_NOT_MEANINGFUL_FOLLOW_THIS);
				}

				if (strcmp (word, "click here") == 0)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LINK_TEXT_NOT_MEANINGFUL_CLICK_HERE);
				}

				if (HasTriggeredLink == no)
				{
					if (strlen (word) < 6)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LINK_TEXT_NOT_MEANINGFUL);
					}
				}

				if (strlen (word) > 60)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LINK_TEXT_TOO_LONG);
				}

			}
		}
		
		if (node->content == NULL)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LINK_TEXT_MISSING);
		}
	}
}


/************************************************************
* CheckArea
*
* Checks attributes within the AREA element to 
* determine if the 'ALT' text and 'HREF' values are valid.
* Also checks to see ensure that the 'TARGET' attribute
* (if it exists) is not null and does not contain '_new' 
* or '_blank'.
*************************************************************
********/

static void CheckArea (Lexer* lexer, Node* node) /* TRT */
{
	Bool HasAlt = no;

	int checked = 0;

	/* Checks all attributes within the AREA element */
	for (attval = node->attributes; attval != null; attval = attval->next)
	{
		if ((PRIORITYCHK == 1)||
			(PRIORITYCHK == 2)||
			(PRIORITYCHK == 3))
		{
			/*
			  Checks for valid ALT attribute.
			  The length of the alt text must be > 4 characters long
			  but must be less than 150 characters long.
			*/
				
			if (attval->dict == attr_alt)
			{
				/* The check for validity */
				if (attval->value != NULL) 
				{
					HasAlt = yes;
				}
			}
		}

		if ((PRIORITYCHK == 2)||
			(PRIORITYCHK == 3))
		{
			if (attval->dict == attr_target)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					if (strcmp (attval->value, "_new") == 0)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, NEW_WINDOWS_REQUIRE_WARNING_NEW);
					}
						
					if (strcmp (attval->value, "_blank") == 0)
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, NEW_WINDOWS_REQUIRE_WARNING_BLANK);
					}
				}
			}
		}
	}

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* AREA must contain alt text */
		if (HasAlt == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, AREA_MISSING_ALT);
		}	
	}
}


/***************************************************
********
* CheckScript
*
* Checks the SCRIPT element to ensure that a
* NOSCRIPT section follows the SCRIPT.  
****************************************************
********/

static void CheckScript (Lexer* lexer, Node* node) /* TRT */
{
    if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* NOSCRIPT element must appear immediately following SCRIPT element */
		if ((node->next == NULL)||
			(node->next->tag != tag_noscript))
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SCRIPT_MISSING_NOSCRIPT);
		}
	}
}


/**********************************************************
********
* CheckRows
*
* Check to see that each table has a row of headers if
* a column of columns doesn't exist. 
***********************************************************
*********/

static void CheckRows (Lexer* lexer, Node* node) /* TRT */
{
	int numTR = 0;
	int numValidTH = 0;
	char *word;
	
	checkedHeaders++;

	while (1)
	{
		if (node == NULL)
		{
			break;
		}

		else 
		{
			numTR++;

			if ((node->content != NULL)&&
				(node->content->tag == tag_th))
			{
				HasTH = yes;
			
				if ((node->content->content != NULL)&&
					(node->content->content->tag == NULL))
				{
					word = textFromOneNode (lexer, node->content->content);

					if ((word != NULL)&&
						(IsWhitespace (word) == no))
					{
						numValidTH++;
					}
				}
			}
		}
		
		node = node->next;
	}

	if (numTR == numValidTH)
	{
		HasValidRowHeaders = yes;
	}

	if ((numTR >= 2)&&
		(numTR > numValidTH)&&
		(numValidTH >= 2)&&
		(HasTH == yes))
	{
		HasInvalidRowHeader = yes;
	}
}


/**********************************************************
********
* CheckColumns
*
* Check to see that each table has a column of headers if
* a row of columns doesn't exist.  
***********************************************************
*********/

static void CheckColumns (Lexer* lexer, Node* node) /* TRT */
{
	Node* TNode;
	char* word;
	int numTH = 0;
	Bool isMissingHeader = no;

	checkedHeaders++;

	/* Table must have row of headers if headers for columns don't exist */
	if (node->content != NULL && node->content->tag == tag_th)
	{
		HasTH = yes;

		TNode = node->content;
		
		while (TNode != NULL)
		{
			if (TNode->tag == tag_th)
			{
				if ((TNode->content != NULL)&&
					(TNode->content->tag == NULL))
				{
					word = textFromOneNode (lexer, TNode->content);

					if ((word != NULL)&&
						(IsWhitespace (word) == no))
					{
						numTH++;
					}
				}
			}

			if (TNode->tag != tag_th)
			{
				isMissingHeader = yes;
			}

			TNode = TNode->next;
		}
	}

	if ((isMissingHeader == no)&&
		(numTH > 0))
	{
		HasValidColumnHeaders = yes;
	}

	if ((isMissingHeader == yes)&&
		(numTH >= 2))
	{
		HasInvalidColumnHeader = yes;
	}
}


/*****************************************************
********
* CheckTH
*
* Checks to see if the header provided for a table
* requires an abbreviation. (only required if the 
* length of the header is greater than 15 characters)
******************************************************
********/

static void CheckTH (Lexer* lexer, Node* node) /* TRT */
{
	Bool HasAbbr = no;

	char* word;

	if (PRIORITYCHK == 3)
	{
		/* Checks TH element for 'ABBR' attribute */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			if (attval->dict == attr_abbr)	/* TRT */
			{
				/* Value must not be null and must be less than 15 characters */
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					HasAbbr = yes;
				}

				if ((attval->value == NULL)||
					(strlen (attval->value) == 0))
				{
					HasAbbr = yes;
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, TABLE_MAY_REQUIRE_HEADER_ABBR_NULL);
				}
				
				if ((IsWhitespace (attval->value) == yes)&&
					(strlen (attval->value) > 0))
				{
					HasAbbr = yes;
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, TABLE_MAY_REQUIRE_HEADER_ABBR_SPACES);
				}
			}
		}

		/* If the header is greater than 15 characters, an abbreviation is needed */
		word = textFromOneNode (lexer, node->content);

		if ((word != NULL)&&
			(IsWhitespace (word) == no))
		{
			/* Must have 'ABBR' attribute if header is > 15 characters */
			if ((strlen (word) > 15)&&
				(HasAbbr == no))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, TABLE_MAY_REQUIRE_HEADER_ABBR);
			}
		}
	}
}


/****************************************************
********
* CheckMultiHeaders
*
* Layout tables should make sense when linearized.
* TABLE must contain at least one TH element.
* This technique applies only to tables used for layout purposes, 
* not to data tables. Checks for column of multiple headers.
*****************************************************
********/

static void CheckMultiHeaders (Lexer* lexer, Node* node) /* TRT */
{
	Node* TNode;
	Node* temp;
	
	Bool validColSpanRows = yes;
	Bool validColSpanColumns = yes;

	int flag = 0;

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->content != NULL)
		{
			TNode = node->content;

			/* 
			   Checks for column of multiple headers found 
			   within a data table. 
			*/
			while (TNode != NULL)
			{
				if (TNode->tag == tag_tr)
				{
					if (TNode->content != NULL)
					{
						temp = TNode->content;

						if (temp->tag == tag_th)
						{
							for (attval = temp->attributes; attval != null; attval = attval->next)
							{
								if (attval->dict == attr_rowspan)
								{
									if (atoi(attval->value) > 1)
									{
										validColSpanRows = no;
									}
								}
							}
						}

						/* The number of TH elements found within TR element */
						if (flag == 0)
						{
							while (temp != NULL)
							{
								/* 
								   Must contain at least one TH element 
								   within in the TR element 
								*/
								if (temp->tag == tag_th)
								{
									for (attval = temp->attributes; attval != null; attval = attval->next)
									{
										if (attval->dict == attr_colspan)
										{
											if (atoi(attval->value) > 1)
											{
												validColSpanColumns = no;
											}
										}
									}
								}

								temp = temp->next;
							}	

							flag = 1;
						}
					}
				}
			
				TNode = TNode->next;
			}

			/* Displays HTML 4 Table Algorithm when multiple column of headers used */
			if (validColSpanRows == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, DATA_TABLE_REQUIRE_MARKUP_ROW_HEADERS);
				DisplayHTMLTableAlgorithm(lexer->errout); /* TRT */			
			}

			if (validColSpanColumns == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, DATA_TABLE_REQUIRE_MARKUP_COLUMN_HEADERS);
				DisplayHTMLTableAlgorithm(lexer->errout); /* TRT */			
			}
		}
	}
}


/****************************************************
********
* CheckTable
*
* Checks the TABLE element to ensure that the
* table is not missing any headers.  Must have either
* a row or column of headers.  
*****************************************************
********/

static void CheckTable (Lexer* lexer, Node* node) /* TRT */
{
	Node* TNode;
	Node* temp;

	char* word;
	char *value = null;

	int numTR = 0;

	Bool HasSummary = no;
	Bool HasCaption = no;

	if (PRIORITYCHK == 3)
	{
		/* Table must have a 'SUMMARY' describing the purpose of the table */
		for (attval = node->attributes; attval != null; attval = attval->next)
		{
			if (attval->dict == attr_summary)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					if ((strstr (attval->value, "summary") == NULL)&&
						(strstr (attval->value, "table") == NULL))
					{
						HasSummary = yes;
					}

					if ((strstr (attval->value, "summary") != NULL)||
						(strstr (attval->value, "table") != NULL))
					{
						HasSummary = yes;
						ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, TABLE_SUMMARY_INVALID_PLACEHOLDER);
					}
				}

				if ((attval->value == NULL)||
					(strlen (attval->value) == 0))
				{
					HasSummary = yes;
					ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, TABLE_SUMMARY_INVALID_NULL);
				}

				else if ((IsWhitespace (attval->value) == yes)&&
						 (strlen (attval->value) > 0))
				{
					HasSummary = yes;
					ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, TABLE_SUMMARY_INVALID_SPACES);
				}
			}
		}

		/* TABLE must have content. */
		if (node->content == NULL)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, DATA_TABLE_MISSING_HEADERS);
		
			return;
		}
	}

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks for multiple headers */
		CheckMultiHeaders (lexer, node);
	}
	
	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Table must have a CAPTION describing the purpose of the table */
		if (node->content != NULL && node->content->tag == tag_caption)
		{
			TNode = node->content;

			if (TNode->content->tag == NULL)
			{
				word = getTextNodeClear (lexer, TNode);
			}

			if ((word != NULL) &&
				(IsWhitespace (word) == no))
			{
				HasCaption = yes;
			}
		}

		if (HasCaption == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, TABLE_MISSING_CAPTION);
		}
	}

	
	if (node->content != NULL)
	{
		if (node->content->tag == tag_caption && node->content->next->tag == tag_tr)
		{
			CheckColumns (lexer, node->content->next);
		}

		else 
		{
			if (node->content != NULL && node->content->tag == tag_tr)
			{
				CheckColumns (lexer, node->content);
			}
		}
	}
	
	if (HasValidColumnHeaders == no)
	{
		if (node->content != NULL)
		{
			if (node->content->tag == tag_caption && node->content->next->tag == tag_tr)
			{
				CheckRows (lexer, node->content->next);
			}
		}

		else 
		{
			if (node->content != NULL && node->content->tag == tag_tr)
			{
				CheckRows (lexer, node->content);
			}
		}
	}
	
	
	if (PRIORITYCHK == 3)
	{
		/* Suppress warning for missing 'SUMMARY for HTML 2.0 and HTML 3.2 */
		if (HasSummary == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, TABLE_MISSING_SUMMARY);
		}
	}

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->content != NULL)
		{
			temp = node->content;

			while (temp != NULL)
			{
				if (temp->tag == tag_tr)
				{
					numTR++;
				}

				temp = temp->next;
			}

			if (numTR == 1)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LAYOUT_TABLES_LINEARIZE_PROPERLY);
			}
		}
	
		if (HasTH == yes)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LAYOUT_TABLE_INVALID_MARKUP);
		}
	}

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (checkedHeaders == 2)
		{
			if ((HasValidRowHeaders == no)&&
				(HasValidColumnHeaders == no)&&
				(HasInvalidRowHeader == no)&&
				(HasInvalidColumnHeader == no))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, DATA_TABLE_MISSING_HEADERS);
			}

			if ((HasValidRowHeaders == no)&&
				(HasInvalidRowHeader == yes))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, DATA_TABLE_MISSING_HEADERS_ROW);
			}

			if ((HasValidColumnHeaders == no)&&
				(HasInvalidColumnHeader == yes))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, DATA_TABLE_MISSING_HEADERS_COLUMN);
			}
		}
	}
}


/*********************************************************
********
* DisplayHTMLTableAlgorithm()
*
* If the table does contain 2 or more logical levels of 
* row or column headers, the HTML 4 table algorithm 
* to show the author how the headers are currently associated 
* with the cells.
**********************************************************
********/
 
static void DisplayHTMLTableAlgorithm (FILE *errout) /* TRT */
{
	/* TRT */
	tidy_out(errout, " \n");
	tidy_out(errout, "      - First, search left from the cell's position to find row header cells.\n");
	tidy_out(errout, "      - Then search upwards to find column header cells.\n");
	tidy_out(errout, "      - The search in a given direction stops when the edge of the table is\n");
	tidy_out(errout, "        reached or when a data cell is found after a header cell.\n"); 
    tidy_out(errout, "      - Row headers are inserted into the list in the order they appear in\n");
	tidy_out(errout, "        the table. \n");
	tidy_out(errout, "      - For left-to-right tables, headers are inserted from left to right.\n");
	tidy_out(errout, "      - Column headers are inserted after row headers, in \n");
	tidy_out(errout, "        the order they appear in the table, from top to bottom. \n");
	tidy_out(errout, "      - If a header cell has the headers attribute set, then the headers \n");
	tidy_out(errout, "        referenced by this attribute are inserted into the list and the \n");
	tidy_out(errout, "        search stops for the current direction.\n");
	tidy_out(errout, "        TD cells that set the axis attribute are also treated as header cells.\n");
	tidy_out(errout, " \n");
}


/***************************************************
**********
* CheckASCII
* 
* Checks for valid text equivalents for XMP and PRE
* elements for ASCII art.  Ensures that there is
* a skip over link to skip multi-lined ASCII art.
****************************************************
**********/

static void CheckASCII (Lexer* lexer, Node* node) /* TRT */
{
	Node* temp1;
	Node* temp2;

	char* skipOver;
	
	Bool IsAscii = no;

	int HasSkipOverLink = 0;
		
	uint i, x;
	int newLines = -1;
	char compareLetter;
	int matchingCount;
	
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* 
		   Checks the text within the PRE and XMP tags to see if ascii 
		   art is present 
		*/
		for (i = node->content->start + 1; i < node->content->end; i++)
		{
			matchingCount = 0;

			/* Counts the number of lines of text */
			if (lexer->lexbuf[i] == '\n')
			{
				newLines++;
			}
			
			compareLetter = lexer->lexbuf[i];

			/* Counts consecutive character matches */
			for (x = i; x < i + 5; x++)
			{
				if (lexer->lexbuf[x] == compareLetter)
				{
					matchingCount++;
				}

				else
				{
					break;
				}
			}

			/* Must have at least 5 consecutive character matches */
			if (matchingCount >= 5)
			{
				break;
			}
		}

		/* 
		   Must have more than 6 lines of text OR 5 or more consecutive 
		   letters that are the same for there to be ascii art 
		*/
		if (newLines >= 6 || matchingCount >= 5)
		{
			IsAscii = yes;
		}

		/* Checks for skip over link if ASCII art is present */
		if (IsAscii == yes)
		{
			if (node->prev != NULL && node->prev->prev != NULL)
			{
				temp1 = node->prev->prev;

				/* Checks for 'HREF' attribute */
				for (attval = temp1->attributes; attval != null; attval = attval->next)
				{
					if (attval->dict == attr_href)
					{
						if ((attval->value != NULL)&&
							(IsWhitespace (attval->value) == no))
						{
							skipOver = attval->value;
							HasSkipOverLink++;
						}
					}
				}
			}
		}
	}

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* 
		   Checks for A element following PRE to ensure proper skipover link
		   only if there is an A element preceding PRE.
		*/
		if (HasSkipOverLink == 1)
		{
			if (node->next != NULL && node->next->tag == tag_a)
			{
				temp2 = node->next;
				
				/* Checks for 'NAME' attribute */
				for (attval = temp2->attributes; attval != null; attval = attval->next)
				{
					if (attval->dict == attr_name)
					{
						if ((attval->value != NULL)&&
							(IsWhitespace (attval->value) == no))
						{
							/* 
							   Value within the 'HREF' attribute must be the same
							   as the value within the 'NAME' attribute for valid
							   skipover.
							*/
							if (strstr(skipOver, attval->value) != NULL)
							{
								HasSkipOverLink++;
							}
						}
					}
				}
			}
		}

		if (IsAscii == yes)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, ASCII_REQUIRES_DESCRIPTION);
		}

		if (HasSkipOverLink < 2)
		{
			if (IsAscii == yes)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SKIPOVER_ASCII_ART);
			}
		}
	}
}


/***********************************************************
********
* CheckFormControls
*
* <form> must have valid 'FOR' attribute, and <label> must
* have valid 'ID' attribute for valid form control.
************************************************************
********/

static void CheckFormControls (Lexer* lexer, Node* node)
{
	if ((HasValidFor == no)&&
		(HasValidId == yes))
	{
		ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, ASSOCIATE_LABELS_EXPLICITLY_FOR);
	}	

	if ((HasValidId == no)&&
		(HasValidFor == yes))
	{
		ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, ASSOCIATE_LABELS_EXPLICITLY_ID);
	}

	if ((HasValidId == no)&&
		(HasValidFor == no))
	{
		ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, ASSOCIATE_LABELS_EXPLICITLY);
	}
}


/************************************************************
************
* CheckLabel
*
* Check for valid 'FOR' attribute within the LABEL element
*************************************************************
************/

static void CheckLabel (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{	
		ForID++;

		/* Checks for valid 'FOR' attribute */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			if (attval->dict == attr_for)
			{
				if ((attval->value != NULL) &&
					(IsWhitespace (attval->value) == no))
				{
					HasValidFor = yes;
				}
			}
		}

		if (ForID == 2)
		{
			ForID = 0;
			CheckFormControls (lexer, node);
		}
	}
}


/************************************************************
*********
* CheckInputLabel
* 
* Checks for valid 'ID' attribute within the INPUT element.
* Checks to see if there is a LABEL directly before
* or after the INPUT element determined by the 'TYPE'.  
* Each INPUT element must have a LABEL describing the form.
*************************************************************
*********/

static void CheckInputLabel (Lexer* lexer, Node* node) /* TRT */
{
	int flag = 0;

	char* word;
	char* text;

	Node* temp;

	Bool HasLabelBefore = no;
	Bool HasLabelAfter = no;
	Bool HasValidLabel = no;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		ForID++;

		/* Checks attributes within the INPUT element */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			/* Must have valid 'ID' value */
			if (attval->dict == attr_id)
			{
				if ((attval->value != NULL) &&
					(IsWhitespace (attval->value) == no))
				{
					HasValidId = yes;
				}
			}
	
			/* 
			   Determines where the LABEL should be located determined by 
			   the 'TYPE' of form the INPUT is.
			*/
			if (attval->dict == attr_type)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					if ((strstr (attval->value, "checkbox") != NULL)||
						(strstr (attval->value, "radio") != NULL)||
					 	(strstr (attval->value, "text") != NULL)||
						(strstr (attval->value, "password") != NULL)||
						(strstr (attval->value, "file") != NULL))
					{
						if ((node->prev != NULL)&&
							(node->prev->prev != NULL))
						{
							if(node->prev->prev->tag == tag_label)
							{
								flag = 1;
								
								temp = node->prev->prev;
								
								if (temp->content->tag == NULL)
								{
									word = textFromOneNode (lexer, temp->content);

									if ((word != NULL)&&
										(IsWhitespace(word) == no))
									{
										HasLabelBefore = yes;
									}
								}
							}
							
							if (HasLabelBefore == yes)
							{
								if (node->prev->tag == NULL)
								{
									text = textFromOneNode (lexer, node->prev);

									if ((text == NULL)||
										(IsWhitespace (text) == yes))
									{
										HasValidLabel = yes;
									}
								}
							}
						}

						if (flag == 0)
						{
							if ((node->next != NULL)&&
								(node->next->next != NULL))
							{
								if (node->next->next->tag == tag_label)
								{
									temp = node->next->next;

									if (temp->content->tag == NULL)
									{
										word = textFromOneNode (lexer, temp->content);

										if ((word != NULL)&&
											(IsWhitespace(word) == no))
										{
											HasLabelAfter = yes;
										}
									} 
								}

								if (HasLabelAfter == yes)
								{
									if (node->next->tag == NULL)
									{
										text = textFromOneNode (lexer, node->next);
		
										if ((text == NULL)||
											(IsWhitespace (text) == yes))
										{
											HasValidLabel = yes;
										}
									}
								}
							}
						}
					}

					/* The following 'TYPES' do not require a LABEL */
					if ((strcmp (attval->value, "image") == 0)||
						(strcmp (attval->value, "hidden") == 0)||
						(strcmp (attval->value, "submit") == 0)||
						(strcmp (attval->value, "reset") == 0)||
						(strcmp (attval->value, "button") == 0))
					{
						HasValidLabel = yes;
					}
				}
			}
		}

		if ((HasLabelBefore == yes)&&
			(HasValidLabel == no))
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LABEL_NEEDS_REPOSITIONING_BEFORE_INPUT);
		}
		
		if ((HasLabelAfter == yes)&&
			(HasValidLabel == no))
		{	
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LABEL_NEEDS_REPOSITIONING_AFTER_INPUT);
		}
		
		if (ForID == 2)
		{
			ForID = 0;
			CheckFormControls (lexer, node);
		}
	}
}


/***************************************************************
********
* CheckInputAttributes 
*
* INPUT element must have a valid 'ALT' attribute if the
* 'VALUE' attribute is present.
****************************************************************
********/

static void CheckInputAttributes (Lexer* lexer, Node* node) /* TRT */
{
	Bool HasValue = no;
	Bool HasAlt = no;
	Bool MustHaveAlt = no;
	Bool MustHaveValue = no;

	/* Checks attributes within the INPUT element */
	for (attval = node->attributes; attval != NULL; attval = attval->next)
	{
		/* 'VALUE' must be found if the 'TYPE' is 'text' or 'checkbox' */
		if (attval->dict == attr_type)
		{
			if ((attval->value != NULL)&&
				(IsWhitespace (attval->value) == no))
			{
				if ((PRIORITYCHK == 1)||
					(PRIORITYCHK == 2)||
					(PRIORITYCHK == 3))
				{
					if (strcmp (attval->value, "image") == 0)
					{
						MustHaveAlt = yes;
					}
				}

				if (PRIORITYCHK == 3)
				{
					if ((strcmp (attval->value, "text") == 0)||
						(strcmp (attval->value, "checkbox") == 0))
					{	
						MustHaveValue = yes;
					}
				}
			}
		}
		
		if (attval->dict == attr_alt)
		{
			if ((attval->value != NULL)&&
				(IsWhitespace (attval->value) == no))
			{
				HasAlt = yes;
			}
		}

		if (attval->dict == attr_value)
		{
			if ((attval->value != NULL)&&
				(IsWhitespace (attval->value) == no))
			{
				HasValue = yes;
			}

			else if ((attval->value == NULL)||
					 (strlen (attval->value) == 0))
			{
				HasValue = yes;
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FORM_CONTROL_DEFAULT_TEXT_INVALID_NULL);
			}

			else if ((IsWhitespace (attval->value) == yes)&&
					 (strlen (attval->value) > 0))
			{
				HasValue = yes;
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FORM_CONTROL_DEFAULT_TEXT_INVALID_SPACES);
			}
		}
	}

	if (MustHaveAlt == yes)
	{
		if (HasAlt == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, IMG_BUTTON_MISSING_ALT);
		}
	}

	if (MustHaveValue == yes)
	{
		if (HasValue == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FORM_CONTROL_REQUIRES_DEFAULT_TEXT);
		}
	}
}


/***************************************************************
********
* CheckFrameSet
*
* Frameset must have valid NOFRAME section.  Must contain some 
* text but must not contain information telling user to update 
* browsers, 
****************************************************************
********/

static void CheckFrameSet (Lexer* lexer, Node* node) /* TRT */
{
	Node* temp;
	char* word;
	
	Bool HasNoFrames = no;

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->content != NULL)
		{
			temp = node->content;

			while (temp != NULL)
			{
				if (temp->tag == tag_a)
				{
					ReportWarningsAndErrors (lexer, temp, ACCESS_FATAL_ERROR, NOFRAMES_INVALID_LINK);
				}

				if (temp->tag == tag_noframes)
				{
					HasNoFrames = yes;

					if ((temp->content != NULL)&&
						(temp->content->content != NULL)&&
						(temp->content->content->tag == tag_p))
					{
						if ((temp->content->content->content != NULL)&&
							(temp->content->content->content->tag == NULL))
						{
							word = textFromOneNode (lexer, temp->content->content->content);

							if (strstr (word, "browser") != NULL)
							{
								ReportWarningsAndErrors (lexer, temp->content->content, ACCESS_FATAL_ERROR, NOFRAMES_INVALID_CONTENT);
							}
						}
					}
					
					else if (temp->content == NULL)
					{
						ReportWarningsAndErrors (lexer, temp, ACCESS_FATAL_ERROR, NOFRAMES_INVALID_NO_VALUE);
					}

					else if ((temp->content != NULL)&&
						     (IsWhitespace (textFromOneNode (lexer, temp->content)) == yes))
					{
						ReportWarningsAndErrors (lexer, temp, ACCESS_FATAL_ERROR, NOFRAMES_INVALID_NO_VALUE);
					}
				}

				temp = temp->next;
			}
		}

		if (HasNoFrames == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, FRAME_MISSING_NOFRAMES);
		}
	}
}


/***********************************************************
***********
* CheckHeaderNesting
*
* Checks for heading increases and decreases.  Headings must
* not increase by more than one header level, but may
* decrease at from any level to any level.  Text within 
* headers must not be more than 20 words in length.  
************************************************************
***********/

static void CheckHeaderNesting (Lexer* lexer, Node* node) /* TRT */
{
	Node* temp;
	char* word;
	uint i;
	int numWords = 1;

	Bool IsValidIncrease = no;
	Bool NeedsDescription = no;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* 
		   Text within header element cannot contain more than 20 words without
		   a separate description
		*/
		if (node->content->tag == NULL)
		{
			word = textFromOneNode (lexer, node->content);

			for(i = 0; i < strlen (word); i++)
			{
				if (word[i] == ' ')
				{
					numWords++;
				}
			}

			if (numWords > 20)
			{
				NeedsDescription = yes;
			}
		}

		/* Header following H1 must be H2 for valid heading increase size */
		if (node->tag == tag_h1)
		{
			IsValidIncrease = yes;

			for (temp = node->next; temp != NULL; temp = temp->next)
			{
				if (temp->tag == tag_h1)
				{
					IsValidIncrease = yes;
					break;
				}

				else if (temp->tag == tag_h2)
				{
					IsValidIncrease = yes;
					break;
				}
				
				else if ((temp->tag == tag_h3)||		
						 (temp->tag == tag_h4)||		
						 (temp->tag == tag_h5)||		
						 (temp->tag == tag_h6))	
				{
					IsValidIncrease = no;
					break;
				}
			}
		}

		/* Header following H2 must be H3 for valid heading increase size */
		if (node->tag == tag_h2)
		{
			IsValidIncrease = yes;

			for (temp = node->next; temp != NULL; temp = temp->next)
			{
				if (temp->tag == tag_h2)
				{
					IsValidIncrease = yes;
					break;
				}
				
				else if (temp->tag == tag_h3)
				{
					IsValidIncrease = yes;
					break;
				}

				else if ((temp->tag == tag_h4)||		
						 (temp->tag == tag_h5)||		
						 (temp->tag == tag_h6))		
				{
					IsValidIncrease = no;
					break;
				}
			}
		}

		/* Header following H3 must be H4 for valid heading increase size */
		if (node->tag == tag_h3)
		{
			IsValidIncrease = yes;

			for (temp = node->next; temp != NULL; temp = temp->next)
			{
				if (temp->tag == tag_h3)
				{
					IsValidIncrease = yes;
					break;
				}
				
				else if (temp->tag == tag_h4)
				{
					IsValidIncrease = yes;
					break;
				}

				else if ((temp->tag == tag_h5)||		
						 (temp->tag == tag_h6))		
				{
					IsValidIncrease = no;
					break;
				}
			}
		}

		/* Header following H4 must be H5 for valid heading increase size */
		if (node->tag == tag_h4)
		{
			IsValidIncrease = yes;

			for (temp = node->next; temp != NULL; temp = temp->next)
			{
				if (temp->tag == tag_h4)
				{
					IsValidIncrease = yes;
					break;
				}
				
				else if (temp->tag == tag_h5)
				{
					IsValidIncrease = yes;
					break;
				}
				
				else if (temp->tag == tag_h6)		
				{
					IsValidIncrease = no;
					break;
				}
			}
		}

		/* Header following H5 must be H6 for valid heading increase size */
		if (node->tag == tag_h5)
		{
			IsValidIncrease = yes;

			for (temp = node->next; temp != NULL; temp = temp->next)
			{
				if (temp->tag == tag_h6)
				{
					IsValidIncrease = yes;
					break;
				}
			}
		}

		if (node->tag == tag_h6)
		{
			IsValidIncrease = yes;
		}

		if (IsValidIncrease == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, HEADERS_IMPROPERLY_NESTED);
		}
	
		if (NeedsDescription == yes)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, HEADER_USED_FORMAT_TEXT);	
		}
	}
}


/*************************************************************
*********
* CheckParagraphHeader
*
* Checks to ensure that P elements are not headings.  Must be
* greater than 10 words in length, and they must not be in bold,
* or italics, or underlined, etc.
**************************************************************
*********/

static void CheckParagraphHeader (Lexer* lexer, Node* node) /* TRT */
{
	Bool IsHeading = yes;
	Bool IsNotHeader = no;
	Node* temp;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Cannot contain text formatting elements */
		if (node->content != NULL)   
		{                     
			if (node->content->tag != NULL)
			{
				temp = node->content;

				while (temp != NULL)
				{
					if (temp->tag == NULL)
					{
						IsNotHeader = yes;
						break;
					}
						
					temp = temp->next;
				}
			}

			if (IsNotHeader == no)
			{
				if (node->content->tag == tag_strong)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, POTENTIAL_HEADER_BOLD);
				}

				if (node->content->tag == tag_u)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, POTENTIAL_HEADER_UNDERLINE);
				}

				if (node->content->tag == tag_em)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, POTENTIAL_HEADER_ITALICS);
				}
			}
		}
	}
}


/*********************************************************
********
* CheckSelect
*
* Checks to see if a LABEL follows the SELECT element.
**********************************************************
********/

static void CheckSelect (Lexer* lexer, Node* node) /* TRT */
{
	Node* temp;
	char* label;
	int flag = 0;

	Bool HasLabelBefore = no;
 	Bool HasLabelAfter = no;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Check to see if there is a LABEL preceding SELECT */
		if (node->prev != NULL && node->prev->prev != NULL)
		{
			if (node->prev->prev->tag == tag_label)	/* TRT */
			{
				temp = node->prev->prev;
				
				if ((temp->content != NULL)&&
					(temp->content->tag == NULL))
				{
					label = textFromOneNode (lexer, temp->content);
		
					if ((label != NULL)&&
						(IsWhitespace (label) == no))
					{
						flag = 1;
						HasLabelBefore = yes;
					}
				}
			}

			/* Check to see if there is a LABEL following SELECT */
			if (flag == 0)
			{
				if (node->next->next->tag == tag_label)	/* TRT */
				{
					temp = node->next->next;
					
					if ((temp->content != NULL)&&
						(temp->content->tag == NULL))
					{
						label = textFromOneNode (lexer, temp->content);
			
						if ((label != NULL)&&
							(IsWhitespace (label) == no))
						{
							flag = 1;
							HasLabelAfter = yes;
						}
					}
				}
			}

			if (HasLabelAfter == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LABEL_NEEDS_REPOSITIONING_AFTER_INPUT);
			}

			if (HasLabelBefore == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LABEL_NEEDS_REPOSITIONING_BEFORE_INPUT);
			}
		}
	}
}


/************************************************************
*********
* CheckTextArea
*
* TEXTAREA must contain a label description either before 
* or after the TEXTAREA element. Text must exist within 
* TEXTAREA.
*************************************************************
*********/

static void CheckTextArea (Lexer* lexer, Node* node) /* TRT */
{
	int flag = 0;
	
	Bool HasLabelAfter = no;
	Bool HasLabelBefore = no;

	char* label;
	Node* temp;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Check to see if there is a LABEL before or after TEXTAREA */
		if (node->prev != NULL && node->prev->prev != NULL)
		{
			if (node->prev->prev->tag == tag_label)	/* TRT */
			{
				temp = node->prev->prev;
				
				if ((temp->content != NULL)&&
					(temp->content->tag == NULL))
				{
					label = textFromOneNode (lexer, temp->content);
		
					if ((label != NULL)&&
						(IsWhitespace (label) == no))
					{
						flag = 1;
						HasLabelBefore = yes;
					}
				}
			}

			if (flag == 0)
			{
				if (node->next->next->tag == tag_label)	/* TRT */
				{
					temp = node->next->next;
					
					if ((temp->content != NULL)&&
						(temp->content->tag == NULL))
					{
						label = textFromOneNode (lexer, temp->content);
			
						if ((label != NULL)&&
							(IsWhitespace (label) == no))
						{
							flag = 1;
							HasLabelAfter = yes;
						}
					}
				}
			}
		
			if (HasLabelAfter == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LABEL_NEEDS_REPOSITIONING_AFTER_INPUT);
			}

			if (HasLabelBefore == no)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LABEL_NEEDS_REPOSITIONING_BEFORE_INPUT);
			}
		}
	}
}


/****************************************************************
********
* CheckEmbed
*
* Checks to see if 'SRC' is a multimedia type.  Must have 
* syncronized captions if used.
*****************************************************************
********/

static void CheckEmbed (Lexer* lexer, Node* node)
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		for (attval = node->attributes; attval != null; attval = attval->next)
		{
			if (attval->dict == attr_src)
			{
				if (IsValidMediaExtension (attval->value) == yes)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, MULTIMEDIA_REQUIRES_TEXT);
				}
			}
		}
	}
}


/*********************************************************************
************
* CheckHTML
*
* Checks HTML element for valid 'LANG' attribute.  Must be a valid
* language.  ie. 'fr' or 'en'
*********************************************************************
***********/

static void CheckHTML (Lexer* lexer, Node* node) /* TRT */
{
    Bool ValidLang = no;

	if (PRIORITYCHK == 3)
	{
		/* Checks for 'LANG' attribute */
		for (attval = node->attributes; attval != null; attval = attval->next)
		{
			/* Must have valid 'LANG' attribute */
			if (attval->dict == attr_lang)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					if ((strstr (attval->value, "fr") != NULL)||
						(strstr (attval->value, "us") != NULL)||
						(strstr (attval->value, "en") != NULL))
					{
						ValidLang = yes;
					}

					else
					{
						ValidLang = yes;
						ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LANGUAGE_INVALID);
					}
				}
			}
		}

		if (ValidLang == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, LANGUAGE_NOT_IDENTIFIED);
		}
	}
}


/********************************************************
********
* CheckBlink
*
* Document must not contain the BLINK element.  
* It is invalid HTML/XHTML.
*********************************************************
*******/

static void CheckBlink (Lexer* lexer, Node* node) /* TRT */
{
	char* word;
	
	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks to see if text is found within the BLINK element. */
		if ((node->content != NULL)&&
			(node->content->tag == NULL))
		{
			word = textFromOneNode (lexer, node->content);

			if ((word != NULL)&&
				(IsWhitespace (word) == no))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REMOVE_BLINK_MARQUEE);
			}
		}
	}
}


/********************************************************
******
* CheckMarquee
*
* Document must not contain the MARQUEE element.
* It is invalid HTML/XHTML.
*********************************************************
******/

static void CheckMarquee (Lexer* lexer, Node* node) /* TRT */
{
	char* word;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks to see if there is text in between the MARQUEE element */
		if ((node->content != NULL)&&
			(node->content->tag == NULL))
		{
			word = textFromOneNode (lexer, node->content);

			if ((word != NULL)&&
				(IsWhitespace (word) == no))
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REMOVE_BLINK_MARQUEE);
			}
		}
	}
}


/**********************************************************
********
* CheckLink
*
* 'REL' attribute within the LINK element must not contain
* 'stylesheet'.  HTML/XHTML document is unreadable when
* style sheets are applied.
***********************************************************
********/

static void CheckLink (Lexer* lexer, Node* node) /* TRT */
{
	Bool HasRel = no;
	Bool HasType = no;

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Check for valid 'REL' and 'TYPE' attribute */
		for (attval = node->attributes; attval != null; attval = attval->next)
		{
			if (attval->dict == attr_rel)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					if (strstr (attval->value, "stylesheet") != NULL)
					{
						HasRel = yes;
					}
				}
			}

			if (attval->dict == attr_type)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					HasType = yes;
				}
			}
		}

		ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, STYLESHEETS_REQUIRE_TESTING_LINK);
	}
}


/*******************************************************
********
* CheckStyle
*
* Document must not contain STYLE element.  HTML/XHTML 
* document is unreadable when style sheets are applied.
********************************************************
********/

static void CheckStyle (Lexer* lexer, Node* node) /* TRT */
{
  	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, STYLESHEETS_REQUIRE_TESTING_STYLE_ELEMENT);
	}
}


/*************************************************************
*********
* DynamicContent
*
* Verify that equivalents of dynamic content are updated and 
* available as often as the dynamic content.
**************************************************************
********/

static void DynamicContent (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->tag == tag_applet)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, TEXT_EQUIVALENTS_REQUIRE_UPDATING_APPLET);
		}
		
		else if (node->tag == tag_script)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, TEXT_EQUIVALENTS_REQUIRE_UPDATING_SCRIPT);
		}

		else if (node->tag == tag_object)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, TEXT_EQUIVALENTS_REQUIRE_UPDATING_OBJECT);
		}
	}
}


/*************************************************************
*******
* ProgrammaticObjects
*
* Verify that the page is usable when programmatic objects 
* are disabled.
**************************************************************
*******/

static void ProgrammaticObjects (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->tag == tag_script)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_SCRIPT);
		}

		if (node->tag == tag_object)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_OBJECT);
		}

		if (node->tag == tag_embed)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_EMBED);
		}

		if (node->tag == tag_applet)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_APPLET);
		}
	}
}


/*************************************************************
********
* AccessibleCompatible
*
* Verify that programmatic objects are directly accessible.
**************************************************************
*******/

static void AccessibleCompatible (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->tag == tag_script)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_SCRIPT);
		}

		else if (node->tag == tag_object)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_OBJECT);
		}

		else if (node->tag == tag_applet)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_APPLET);
		}

		else if (node->tag == tag_embed)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_EMBED);
		}
	}
}


/********************************************************
**********
* WordCount
*
* Counts the number of words in the document.  Must have
* more than 3 words to verify changes in natural
* language of document.
*********************************************************
**********/

static int HasBeenDisplayed = 0; /* TRT */

static void WordCount (Lexer* lexer, Node* node) /* TRT */
{
	Bool startWordCount = no;

	uint i;
	char* word;

	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		word = textFromOneNode (lexer, node);

		if ((word != NULL)&&
			(IsWhitespace(word) == no))
		{
			wordsCounted++;
					
			/* Counts the number of words found within a text node */
			for (i = 0; i < strlen (word); i++)
			{
				if (word[i] == ' ')
				{
					wordsCounted++;
				}

				if (wordsCounted > 3)
				{
					break;
				}
			}
		}
		
		if (HasBeenDisplayed == 0)
		{
			/* Must contain more than 3 words of text in the document */
			if (wordsCounted > 3)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, INDICATE_CHANGES_IN_LANGUAGE);
				HasBeenDisplayed = 1;
			}
		}
	}
}


/**************************************************
********
* CheckFlicker
*
* Verify that the page does not cause flicker.
***************************************************
********/

static void CheckFlicker (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->tag == tag_script)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, REMOVE_FLICKER_SCRIPT);
		}

		if (node->tag == tag_object)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, REMOVE_FLICKER_OBJECT);
		}

		if (node->tag == tag_embed)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, REMOVE_FLICKER_EMBED);
		}

		if (node->tag == tag_applet)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, REMOVE_FLICKER_APPLET);
		}

		/* Checks for animated gif within the <img> tag. */
		if (node->tag == tag_img)
		{
			for (attval = node->attributes; attval != NULL; attval = attval->next)
			{
				if (attval->dict == attr_src)
				{
					char fileExtension[20];
					GetFileExtension (attval->value, fileExtension); /* TRT */

					if (wstrcasecmp(fileExtension, ".gif") == 0) /* TRT */
					{
						ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, REMOVE_FLICKER_ANIMATED_GIF);
					}

					break;
				}
			}
		}			
	}
}


/**********************************************************
*******
* CheckDeprecated
*
* APPLET, BASEFONT, CENTER, FONT, ISINDEX, 
* S, STRIKE, and U should not be used.  Becomes deprecated
* HTML if any of the above are used.
***********************************************************
*******/

static void CheckDeprecated (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->tag == tag_applet)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_APPLET);
		}

		if (node->tag == tag_basefont)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_BASEFONT);
		}

		if (node->tag == tag_center)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_CENTER);
		}

		if (node->tag == tag_dir)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_DIR);
		}

		if (node->tag == tag_font)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_FONT);
		}

		if (node->tag == tag_isindex)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_ISINDEX);
		}

		if (node->tag == tag_menu)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_MENU);
		}

		if (node->tag == tag_s)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_S);
		}

		if (node->tag == tag_strike)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_STRIKE);
		}

		if (node->tag == tag_u)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REPLACE_DEPRECATED_HTML_U);
		}
	}
}


/************************************************************
********
* CheckScriptKeyboardAccessible
*
* Elements must have a device independent event handler if 
* they have any of the following device dependent event 
* handlers. 
*************************************************************
********/

static void CheckScriptKeyboardAccessible (Lexer* lexer, Node* node) /* TRT */
{
	int HasOnMouseDown = 0;
	int HasOnMouseUp = 0;
	int HasOnClick = 0;
	int HasOnMouseOut = 0;
	int HasOnMouseOver = 0;
	int HasOnMouseMove = 0;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
			/* Checks all elements for their attributes */
		for (attval = node->attributes; attval != null; attval = attval->next)
		{
			/* Must also have 'ONKEYDOWN' attribute with 'ONMOUSEDOWN' */
			if (attval->dict == attr_onMouseDown)
			{
				HasOnMouseDown++;
			}

			/* Must also have 'ONKEYUP' attribute with 'ONMOUSEUP' */
			if (attval->dict == attr_onMouseUp)
			{
				HasOnMouseUp++;
			}

			/* Must also have 'ONKEYPRESS' attribute with 'ONCLICK' */
			if (attval->dict == attr_onClick)
			{
				HasOnClick++;
			}

			/* Must also have 'ONBLUR' attribute with 'ONMOUSEOUT' */
			if (attval->dict == attr_onMouseOut)
			{
				HasOnMouseOut++;
			}

			if (attval->dict == attr_onMouseOver)
			{
				HasOnMouseOver++;
			}

			if (attval->dict == attr_onMouseMove)
			{
				HasOnMouseMove++;
			}

			if (attval->dict == attr_onKeyDown)
			{
				HasOnMouseDown++;
			}

			if (attval->dict == attr_onKeyUp)
			{
				HasOnMouseUp++;
			}

			if (attval->dict == attr_onKeyPress)
			{
				HasOnClick++;
			}

			if (attval->dict == attr_onBlur)
			{
				HasOnMouseOut++;
			}
		}

		if (HasOnMouseDown == 1)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_DOWN);
		}

		if (HasOnMouseUp == 1)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_UP);
		}
		
		if (HasOnClick == 1)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_CLICK);
		}
			
		if (HasOnMouseOut == 1)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_OUT);
		}
		
		if (HasOnMouseOver == 1)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_OVER);
		}

		if (HasOnMouseMove == 1)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_MOVE);
		}
	}
}


/**********************************************************
********
* CheckMetaData
*
* Must have at least one of these elements in the document.
* META, LINK, TITLE or ADDRESS.  <meta> must contain 
* a "content" attribute that doesn't contain a URL, and
* an "http-Equiv" attribute that doesn't contain 'refresh'.
***********************************************************
********/

static Bool HasMetaData = no;

static void CheckMetaData (Lexer* lexer, Node* node) /* TRT */
{
	Node* content;
	char* word;
	Bool HasHttpEquiv = no;
	Bool HasContent = no;
	Bool HasRel = no;
	Bool ContainsAttr = no;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->tag == tag_meta)
		{
			for (attval = node->attributes; attval != null; attval = attval->next)
			{
				if (attval->dict == attr_httpEquiv)
				{
					if ((attval->value != NULL)&&
						(IsWhitespace (attval->value) == no))
					{
						ContainsAttr = yes;

						/* Must not have an auto-refresh */
						if (strcmp(attval->value, "refresh") == 0)
						{
							HasHttpEquiv = yes;
							ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REMOVE_AUTO_REFRESH);
						}
					}
				}

				if (attval->dict == attr_content)
				{
					if ((attval->value != NULL)&&
						(IsWhitespace (attval->value) == no))
					{
						ContainsAttr = yes;

						/* If the value is not an integer, then it must not be a URL */
						if (strstr(attval->value, "http") != NULL)
						{
							HasContent = yes;
							ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, REMOVE_AUTO_REDIRECT);
						}
					}
				}
			}
		
			if ((HasContent == yes)||
				(HasHttpEquiv == yes))
			{
				HasMetaData = yes;

				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, METADATA_MISSING_REDIRECT_AUTOREFRESH);
			}

			else
			{
				if ((ContainsAttr == yes)&&
					(HasContent == no)&&
					(HasHttpEquiv == no))
				{
					HasMetaData = yes;					
				}
			}
		}

		if (HasMetaData == no)
		{
			if (node->tag == tag_address)
			{
				if ((node->content != NULL)&&
					(node->content->tag == tag_a))
				{
				
					HasMetaData = yes;
				}
			}
		}
			
		if (HasMetaData == no)
		{
			if (node->tag == tag_title)
			{
				if ((node->content != NULL)&&
					(node->content->tag == NULL))
				{
					word = textFromOneNode (lexer, node->content);

					if ((word != NULL)&&
						(IsWhitespace (word) == no))
					{
						HasMetaData = yes;
					}
				}
			}
		}

		if (HasMetaData == no)
		{
			if (node->tag == tag_link)
			{
				HasMetaData = yes;

				for (attval = node->attributes; attval != NULL; attval = attval->next)
				{
					if (attval->dict == attr_rel)
					{
						if ((attval->value != NULL)&&
							(IsWhitespace (attval->value) == no))
						{
							if (strstr(attval->value, "stylesheet") != NULL)
							{
								HasRel = yes;
							}
						}
					}
				}

				if (HasRel == yes)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, METADATA_MISSING_LINK);
				}
			}
		}
			
		/* Check for MetaData */
		for (content = node->content; content != null; content = content->next)
		{
			CheckMetaData (lexer, content);
		}
	}
}


/*******************************************************
********
* MetaDataPresent
*
* Determines if MetaData is present in document
********************************************************
********/

static void MetaDataPresent (Lexer* lexer, Node* node)
{
	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (HasMetaData == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, METADATA_MISSING);
		}
	}
}


/*****************************************************
********
* CheckDocType
*
* Checks that every HTML/XHTML document contains a 
* '!DOCTYPE' before the root node. ie.  <HTML>
******************************************************
********/

static void CheckDocType (Lexer* lexer, Node* node) /* TRT */
{
	char* word;
	Bool HasDocType = no;

	if ((PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		if (node->tag == NULL)
		{
			word = textFromOneNode (lexer, node->content);
				
			if ((strstr (word, "HTML PUBLIC") == NULL) &&
				(strstr (word, "html PUBLIC") == NULL))	/* TRT */
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, DOCTYPE_MISSING);
			}
		}
	}
}


/****************************************************
*******
* CheckMapAccess
*
* Verify that text links are provided for client-side 
* image maps.  Document must contain text links for 
* each active area of the image map.
*****************************************************
*******/

static void CheckMapAccess (Lexer* lexer, Node* node, Node* front) /* TRT */
{
	if (PRIORITYCHK == 3)
	{
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			if (attval->dict == attr_name)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					HasName = yes;
				}
			}
		}
		
		GetMapLinks (lexer, node, front);
	}
}


/********************************************************
********
* GetMapLinks
*
* Checks to see if an HREF for A element matches HREF
* for AREA element.  There must be an HREF attribute 
* of an A element for every HREF of an AREA element. 
*********************************************************
********/

/* List containing map-links */
static AreaLinks* links;
static AreaLinks* start;
static AreaLinks* current;

static void GetMapLinks (Lexer* lexer, Node* node, Node* front) /* TRT */
{
	Node* temp;

	int counter = 0;
	int flag = 0;

	if (node->content != NULL)
	{
		temp = node->content;
		
		/* Stores the 'HREF' link of an AREA element within a MAP element */
		while (temp != NULL)
		{
			if (temp->tag == tag_area)
			{
				/* Checks for 'HREF' attribute */				
				for (attval = temp->attributes; attval != NULL; attval = attval->next)
				{
					if (attval->dict == attr_href)
					{
						if ((attval->value != NULL)&&
							(IsWhitespace (attval->value) == no))
						{
							/* Reference to the beginning of the list */
							if (flag == 0)
							{
								start = (AreaLinks*)MemAlloc (sizeof(AreaLinks)); /* TRT */
								/* TRT*/
								if (!start)
								{
									/* FatalError("Out of memory!"); */	/* reported in MemAlloc() */
									return;
								}
								start->link = attval->value;
								start->HasBeenFound = no;
								start->next = NULL;	/* TRT */
								current = start;
								flag = 1;
								counter++;
							}
							
							/* Adds link to the list */
							else
							{
								links = (AreaLinks*)MemAlloc (sizeof(AreaLinks)); /* TRT */
								/* TRT*/
								if (!links)
								{
									/* FatalError("Out of memory!"); */	/* reported in MemAlloc() */
									return;
								}

								links->link = attval->value;
								links->HasBeenFound = no;
								links->next = NULL;	/* TRT */
								current->next = links;
								current = links;
								counter++;
							}
						}
					}
				}
			}
			
			temp = temp->next;
		}
	}

	CompareAnchorLinks (lexer, front, counter);
	FindMissingLinks (lexer, node, counter);
}


/****************************************************************
*********
* CompareAnchorLinks 
*
* Compares links within the MAP element with all the A elements
* found in the document.  All of the MAP links must be contained
* within the document with matching A links.  
*****************************************************************
*********/

static void CompareAnchorLinks (Lexer* lexer, Node* front, int counter) /* TRT */
{
	Node* content;

	int i;

	int counted = 0;

	current = start;

	/* Checks document for A elements */
	if (front->tag == tag_a)
	{
		/* Checks for 'HREF' attribute */
		for (attval = front->attributes; attval != NULL; attval = attval->next)
		{
			if (attval->dict == attr_href)
			{
				if ((attval->value != NULL)&&
					(IsWhitespace (attval->value) == no))
				{
					/* Compares 'HREF' with links within the list of links */
					for (i = 0; i < counter; i++)
					{
						if (current->HasBeenFound == no)
						{
							if (strstr (current->link, attval->value) != NULL)
							{
								current->HasBeenFound = yes;
							}
						}
					
						current = current->next;
					}
				}
			}
		}
	}

	for (content = front->content; content != NULL; content = content->next)
	{
		CompareAnchorLinks(lexer, content, counter);
	}
}


/*****************************************************************
******
* FindMissingLinks
*
* If a MAP link is not found within the document, the warning is
* displayed.  Every MAP link must have an A link to follow it.
******************************************************************
******/

static void FindMissingLinks (Lexer* lexer, Node* node, int counter) /* TRT */
{
	int i;

	current = start;

	/* Checks for any MAP links that are not found in the document */
	for (i = 0; i < counter; i++)
	{
		if (current->HasBeenFound == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_FATAL_ERROR, IMG_MAP_CLIENT_MISSING_TEXT_LINKS);
		}

		current = current->next;
	}
}


/****************************************************
******
* CheckForStyleAttribute
*
* Checks all elements within the document to check 
* for the use of 'STYLE' attribute.
*****************************************************
******/

static void CheckForStyleAttribute (Lexer* lexer, Node* node) /* TRT */
{
	if ((PRIORITYCHK == 1)||
		(PRIORITYCHK == 2)||
		(PRIORITYCHK == 3))
	{
		/* Checks elements for 'STYLE' attributes */
		for (attval = node->attributes; attval != NULL; attval = attval->next)
		{
			/* Must not contain 'STYLE' attribute */
			if (attval->dict == attr_style)
			{
				ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, STYLESHEETS_REQUIRE_TESTING_STYLE_ATTR);
			}
		}
	}
}


/*****************************************************
**********
* CheckForListElements
*
* Checks document for list elements (<ol>, <ul>, <li>)
******************************************************
**********/

static void CheckForListElements (Lexer* lexer, Node* node)
{
	Node* content;

	if (node->tag == tag_li)
	{
		listElements++;
	}

	if ((node->tag == tag_ol)||
		(node->tag == tag_ul))
	{
		otherListElements++;
	}

	for (content = node->content; content != null; content = content->next)
	{
		CheckForListElements (lexer, content);
	}
}


/******************************************************
*********
* CheckListUsage
*
* Ensures that lists are properly used.  <ol> and <ul>
* must contain <li> within itself, and <li> must not be
* by itself.
*******************************************************
*********/

static void CheckListUsage (Lexer* lexer, Node* node)
{
	if (node->tag == tag_ol)
	{
		if ((node->content == NULL)||
			(node->content->tag != tag_li))
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LIST_USAGE_INVALID_OL);
		}
	}

	else if (node->tag == tag_ul)
	{
		if ((node->content == NULL)||
			(node->content->tag != tag_li))
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LIST_USAGE_INVALID_UL);
		}
	}

	else if (node->tag == tag_li)
	{
		if ((listElements == 1)&&
			(otherListElements == 0))
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LIST_USAGE_INVALID_LI);
		}

		else
		{
			if (listElements != otherListElements)
			{
				if (listElements%otherListElements != 0)
				{
					ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, LIST_USAGE_INVALID_LI);
				}
			}
		}
	}
}


/* TRT */

/************************************************************
*********
* GetTagByName
*
* Does a lookup on the specified tag, and returns a pointer to the
* tag dictionary entry
*************************************************************
*********/

static Dict *GetTagByName(Node *node, char *tagName)
{
	Dict *theTag = null;
	
	node->element = wstrdup(tagName);
	FindTag(node);
	theTag = node->tag;
	MemFree(node->element);
	node->element = null;
	
	return theTag;
}

/************************************************************
*********
* InitAccessibilityChecks
*
* Initializes the AccessibilityChecks variables as necessary
*************************************************************
*********/

void InitAccessibilityChecks (int theAccessibilityCheckLevel)
{
	static Bool initialized = no;

	PRIORITYCHK = theAccessibilityCheckLevel;
	
	/* Initialize the static variables each time  - TRT */
	
	/* Number of characters that are found within the concatenated text */
	counter = 0;

	/* The number of words found within the document (text only) */
	wordsCounted = 0;

	/* Number of frame elements found within a frameset */
	numFrames = 0;

	/* Number of 'longdesc' attributes found within a frameset */
	HasCheckedLongDesc = 0;

	/* Check for the DocType only once for each document */
	HasCheckedDocType = no;

	/* For 'USEMAP' identifier */
	HasUseMap = no;
	HasName = no;
	HasMap = no;

	/* Reference to the beginning of the document */
	reference = 0;
	beginning = NULL;

	/* Reference to the <map> node */
	mapNode = NULL;
	gotMapNode = no;

	/* For tracking nodes that are deleted from the original parse tree - TRT */
	/* access_tree = NULL; */
	
	/* Ensure that certain warning messages are only displayed once */
	checkedHTML = no;
	/* comment following line if you want accessibility check heading only once per Tidy run  - TRT */
	shown = no;
	DisplayedLayout = no;
	HasTH = no;
	checkedForStyleSheets = no;
	StyleSheetsPresent = no;

	HasValidFor = no;
	HasValidId = no;
	HasValidRowHeaders = no;
	HasValidColumnHeaders = no;
	HasInvalidRowHeader = no;
	HasInvalidColumnHeader = no;
	checkedForListElements = no;
	listElements = 0;
	otherListElements = 0;
	checkedHeaders = 0;
	ForID = 0;

	/* Pointer to attribute and values within the attributes of an element */
	attval = NULL;
	attribute = NULL;

	colorAttrs = 0;

	HasBeenDisplayed = 0;
	
	HasMetaData = no;
	
	if (!initialized)
	{
	    Node *node;
	    
/* TRT */
#if SUPPORT_ACCESSIBILITY_CHECKS
	    node = NewNode(null);
#else
		node = NewNode();
#endif
	    node->type = StartTag;
	    node->implicit = yes;
	    
		tag_label = GetTagByName(node, "label");
		tag_h3 = GetTagByName(node, "h3");
		tag_h4 = GetTagByName(node, "h4");
		tag_h5 = GetTagByName(node, "h5");
		tag_h6 = GetTagByName(node, "h6");
		tag_address = GetTagByName(node, "address");
		tag_xmp = GetTagByName(node, "xmp");
		tag_select = GetTagByName(node, "select");
		tag_blink = GetTagByName(node, "blink");
		tag_marquee = GetTagByName(node, "marquee");
		tag_embed = GetTagByName(node, "embed");
		tag_basefont = GetTagByName(node, "basefont");
		tag_isindex = GetTagByName(node, "isindex");
		tag_s = GetTagByName(node, "s");
		tag_strike = GetTagByName(node, "strike");
		tag_u = GetTagByName(node, "u");
		tag_menu = GetTagByName(node, "menu");

		FreeNode(node);
		
		initialized = yes;
	}
}

/* TRT */
/************************************************************
*********
* CleanupAccessibilityChecks
*
* Cleans up the AccessibilityChecks variables as necessary
*************************************************************
*********/

void CleanupAccessibilityChecks (void)
{
	/* free any memory allocated for the lists - TRT */
	current = start;
	while (current)
	{
		void	*templink = (void *)current;
		
		current = current->next;
		MemFree(templink);
	}
	start = NULL;
}

/************************************************************
*********
* AccessibilityChecks
*
* Traverses through the individual nodes of the tree
* and checks attributes and elements for accessibility.
* after the tree structure has been formed.
*************************************************************
*********/

void AccessibilityChecks (Lexer* lexer, Node* node)
{
	Node* content;

	/* printf ("%s\n", node->element); */
	if (shown == no)
	{
		/* TRT */
		tidy_out(lexer->errout, "\n");
		tidy_out(lexer->errout, "Accessibility Checks: Version 0.1\n");
		tidy_out(lexer->errout, "\n");

		shown = yes;
	}

	/* Reference to the beginning of the document */
	if (reference == 0)
	{
		beginning = node;
		reference = 1;
	}

	/* Check to see if any list elements are found within the document */
	if (checkedForListElements == no)
	{
		CheckForListElements (lexer, node);
		checkedForListElements = yes;
	}

	/* Checks for natural language change */
	if (checkedHTML == yes)
	{
		if (node->tag == NULL)
		{
			WordCount (lexer, node);
		}
	}
	
	/* Checks to see if stylesheets are used to control the layout */
	if (checkedForStyleSheets == no)		
	{
		CheckMissingStyleSheets (lexer, node);
		checkedForStyleSheets = yes;

		if (StyleSheetsPresent == no)
		{
			ReportWarningsAndErrors (lexer, node, ACCESS_WARNING_ERROR, STYLE_SHEET_CONTROL_PRESENTATION);
		}
	}

	/* Checks all elements for script accessibility */
	CheckScriptKeyboardAccessible (lexer, node);

	/* Checks entire document for the use of 'STYLE' attribute */
	CheckForStyleAttribute (lexer, node);

	/* Checks for '!DOCTYPE' */
	if (HasCheckedDocType == no)
	{
		CheckDocType (lexer, node);
		HasCheckedDocType = yes;
	}
	
	/* Check BODY for color contrast */
	else if (node->tag == tag_body)
	{
		CheckColorContrast (lexer, node);
	}

	/* Checks document for MetaData */
	else if (node->tag == tag_head)
	{
		CheckMetaData (lexer, node);
		MetaDataPresent (lexer, node);
	}
	
	/* Check the ANCHOR tag */
	else if (node->tag == tag_a)
	{
		CheckAnchor (lexer, node);
	}

	/* Check the IMAGE tag */
	else if (node->tag == tag_img)
	{
		CheckFlicker (lexer, node);
		CheckColorAvailable (lexer, node);
		CheckImage (lexer, node);
	}

		/* Checks MAP for client-side text links */
	else if (node->tag == tag_map)
	{
		CheckMapAccess (lexer, node, beginning); /* TRT */
	}

	/* Check the AREA tag */
	else if (node->tag == tag_area)
	{
		CheckArea (lexer, node);
	}

	/* Check the APPLET tag */
	else if (node->tag == tag_applet)
	{
		CheckDeprecated (lexer, node);
		ProgrammaticObjects (lexer, node); /* TRT */
		DynamicContent (lexer, node);
		AccessibleCompatible (lexer, node);
		CheckFlicker (lexer, node);
		CheckColorAvailable (lexer, node);
		CheckApplet(lexer, node);
	}
	
	/* Check the OBJECT tag */
	else if (node->tag == tag_object)
	{
		ProgrammaticObjects (lexer, node); /* TRT */
		DynamicContent (lexer, node);
		AccessibleCompatible (lexer, node);
		CheckFlicker (lexer, node);
		CheckColorAvailable (lexer, node);
		CheckObject (lexer, node);
	}
	
	/* Check the FRAME tag */
	else if (node->tag == tag_frame)
	{
		CheckFrame (lexer, node);
	}
	
	/* Check the IFRAME tag */
	else if (node->tag == tag_iframe)
	{
		CheckIFrame (lexer, node);
	}
	
	/* Check the SCRIPT tag */
	else if (node->tag == tag_script)
	{
		DynamicContent (lexer, node);
		ProgrammaticObjects (lexer, node); /* TRT */
		AccessibleCompatible (lexer, node);
		CheckFlicker (lexer, node);
		CheckColorAvailable (lexer, node);
		CheckScript (lexer, node);
	}

	/* Check the TABLE tag */
	else if (node->tag == tag_table)
	{
		CheckColorContrast (lexer, node);
		CheckTable (lexer, node);
	}

	/* Check the PRE for ASCII art */
	else if ((node->tag == tag_pre)||
			 (node->tag == tag_xmp))
	{
		CheckASCII (lexer, node);
	}

	/* Check the LABEL tag */
	else if (node->tag == tag_label)
	{
		CheckLabel (lexer, node);
	}

	/* Check INPUT tag for validity */
	else if (node->tag == tag_input)
	{
		CheckColorAvailable (lexer, node);
		CheckInputLabel (lexer, node);
		CheckInputAttributes (lexer, node);
	}

	/* Checks FRAMESET element for NOFRAME section */
	else if (node->tag == tag_frameset)
	{
		CheckFrameSet (lexer, node);
	}
	
	/* Checks for header elements for valid header increase */
	else if ((node->tag == tag_h1)||
			 (node->tag == tag_h2)||
			 (node->tag == tag_h3)||		
			 (node->tag == tag_h4)||		
			 (node->tag == tag_h5)||
			 (node->tag == tag_h6))		
	{
		CheckHeaderNesting (lexer, node);
	}

	/* Checks P element to ensure that it is not a header */
	else if (node->tag == tag_p)
	{
		CheckParagraphHeader (lexer, node);
	}

	/* Checks SELECT element for LABEL */
	else if (node->tag == tag_select)
	{
		CheckSelect (lexer, node);
	}

	/* Checks TEXTAREA element for LABEL */
	else if (node->tag == tag_textarea)
	{
		CheckTextArea (lexer, node);
	}

	/* Checks HTML elemnt for valid 'LANG' */
	else if (node->tag == tag_html)
	{
		checkedHTML = yes;
		CheckHTML (lexer, node);
	}

	/* Checks BLINK for any blinking text */
	else if (node->tag == tag_blink)
	{
		CheckBlink (lexer, node);
	}

	/* Checks MARQUEE for any MARQUEE text */
	else if (node->tag == tag_marquee)
	{
		CheckMarquee (lexer, node);
	}

	/* Checks LINK for 'REL' attribute */
	else if (node->tag == tag_link)
	{
		CheckLink (lexer, node);
	}

	/* Checks to see if STYLE is used */
	else if (node->tag == tag_style)
	{
		CheckColorContrast (lexer, node);
		CheckStyle (lexer, node);
	}

	/* Checks to see if EMBED is used */
	else if (node->tag == tag_embed)
	{
		CheckEmbed (lexer, node);
		ProgrammaticObjects (lexer, node); /* TRT */
		AccessibleCompatible (lexer, node);
		CheckFlicker (lexer, node);
	}

	/* Deprecated HTML if the following tags are found in the document */
	else if ((node->tag == tag_basefont)||
			 (node->tag == tag_center)||
			 (node->tag == tag_isindex)||
			 (node->tag == tag_u)||
			 (node->tag == tag_font)||
			 (node->tag == tag_dir)||
			 (node->tag == tag_s)||
			 (node->tag == tag_strike)||
			 (node->tag == tag_menu))
	{
		CheckDeprecated (lexer, node);
	}

	/* Checks for 'ABBR' attribute if needed */
	else if (node->tag == tag_th)
	{
		CheckTH (lexer, node);
	}

	/* Ensures that lists are properly used */
	else if ((node->tag == tag_li)||
			 (node->tag == tag_ol)||
			 (node->tag == tag_ul))
	{
		CheckListUsage (lexer, node);
	}

	/* 
	   Runs through the entire document, checking and evaluating each element 
	   found in the document 
	*/

	for (content = node->content; content != null; content = content->next)
	{
		AccessibilityChecks(lexer, content);
	}
}

#endif
