#ifndef __ATTRS_H__
#define __ATTRS_H__

/* attrs.h -- recognize HTML attributes

  (c) 1998-2003 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: hoehrmann $ 
    $Date: 2003/03/30 23:57:25 $ 
    $Revision: 1.5 $ 

*/

#include "forward.h"

/* declaration for methods that check attribute values */
typedef void (AttrCheck)(TidyDocImpl* doc, Node *node, AttVal *attval);

struct _Attribute
{
    TidyAttrId  id;
    tmbstr      name;
    unsigned    versions;
    AttrCheck*  attrchk;
    Bool        nowrap;
    Bool        literal;

    struct _Attribute* next;
};


/*
 Anchor/Node linked list
*/

struct _Anchor
{
    struct _Anchor *next;
    Node *node;
    char *name;
};

typedef struct _Anchor Anchor;



#define ATTRIB_HASHSIZE 357

struct _TidyAttribImpl
{
    /* anchor/node lookup */
    Anchor*    anchor_list;

    /* Declared literal attributes */
    Attribute* declared_attr_list;
};

typedef struct _TidyAttribImpl TidyAttribImpl;

#define XHTML_NAMESPACE "http://www.w3.org/1999/xhtml"


/*
 Bind attribute types to procedures to check values.
 You can add new procedures for better validation
 and each procedure has access to the node in which
 the attribute occurred as well as the attribute name
 and its value.

 By default, attributes are checked without regard
 to the element they are found on. You have the choice
 of making the procedure test which element is involved
 or in writing methods for each element which controls
 exactly how the attributes of that element are checked.
 This latter approach is best for detecting the absence
 of required attributes.
*/

AttrCheck CheckUrl;
AttrCheck CheckScript;
AttrCheck CheckName;
AttrCheck CheckId;
AttrCheck CheckAlign;
AttrCheck CheckValign;
AttrCheck CheckBool;
AttrCheck CheckLength;
AttrCheck CheckTarget;
AttrCheck CheckFsubmit;
AttrCheck CheckClear;
AttrCheck CheckShape;
AttrCheck CheckNumber;
AttrCheck CheckScope;
AttrCheck CheckColor;
AttrCheck CheckVType;
AttrCheck CheckScroll;
AttrCheck CheckTextDir;
AttrCheck CheckLang;

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


/* public method for finding attribute definition by name */
const Attribute* CheckAttribute( TidyDocImpl* doc, Node *node, AttVal *attval );

const Attribute* FindAttribute( TidyDocImpl* doc, AttVal *attval );

AttVal* GetAttrByName( Node *node, ctmbstr name );

AttVal* AddAttribute( TidyDocImpl* doc,
                      Node *node, ctmbstr name, ctmbstr value );

Bool IsUrl( TidyDocImpl* doc, ctmbstr attrname );

Bool IsBool( TidyDocImpl* doc, ctmbstr attrname );

Bool IsScript( TidyDocImpl* doc, ctmbstr attrname );

Bool IsLiteralAttribute( TidyDocImpl* doc, ctmbstr attrname );

/* may id or name serve as anchor? */
Bool IsAnchorElement( TidyDocImpl* doc, Node* node );

/*
  In CSS1, selectors can contain only the characters A-Z, 0-9, and
  Unicode characters 161-255, plus dash (-); they cannot start with
  a dash or a digit; they can also contain escaped characters and any
  Unicode character as a numeric code (see next item).

  The backslash followed by at most four hexadecimal digits (0..9A..F)
  stands for the Unicode character with that number.

  Any character except a hexadecimal digit can be escaped to remove its
  special meaning, by putting a backslash in front.

  #508936 - CSS class naming for -clean option
*/
Bool IsCSS1Selector( ctmbstr buf );


/* removes anchor for specific node */
void RemoveAnchorByNode( TidyDocImpl* doc, Node *node );

/* add new anchor to namespace */
Anchor* AddAnchor( TidyDocImpl* doc, ctmbstr name, Node *node );

/* return node associated with anchor */
Node* GetNodeByAnchor( TidyDocImpl* doc, ctmbstr name );

/* free all anchors */
void FreeAnchors( TidyDocImpl* doc );


/* public methods for inititializing/freeing attribute dictionary */
void InitAttrs( TidyDocImpl* doc );
void FreeAttrTable( TidyDocImpl* doc );

/*
Henry Zrepa reports that some folk are
using embed with script attributes where
newlines are signficant. These need to be
declared and handled specially!
*/
void DeclareLiteralAttrib( TidyDocImpl* doc, ctmbstr name );

/*
 the same attribute name can't be used
 more than once in each element
*/
void RepairDuplicateAttributes( TidyDocImpl* doc, Node* node );

Bool IsBoolAttribute( AttVal* attval );
Bool attrIsEvent( AttVal* attval );

AttVal* AttrGetById( Node* node, TidyAttrId id );

/* 0 == TidyAttr_unknown */
#define AttrId(av) ((av) && (av)->dict ? (av)->dict->id : TidyAttr_unknown)
#define AttrIsId(av, atid) ((av) && (av)->dict && ((av)->dict->id == atid))

#define AttrHasValue(attr)      ((attr) && (attr)->value)
#define AttrMatches(attr, val)  (AttrHasValue(attr) && \
                                 tmbstrcasecmp((attr)->value, val) == 0)
#define AttrContains(attr, val) (AttrHasValue(attr) && \
                                 tmbsubstr((attr)->value, val) != NULL)
#define AttrVersions(attr)      ((attr) && (attr)->dict ? (attr)->dict->versions : VERS_PROPRIETARY)


#define attrIsABBR(av)              AttrIsId( av, TidyAttr_abbr )
#define attrIsACCEPT(av)            AttrIsId( av, TidyAttr_accept )
#define attrIsACCEPT_CHARSET(av)    AttrIsId( av, TidyAttr_accept_charset )
#define attrIsACCESSKEY(av)         AttrIsId( av, TidyAttr_accesskey )
#define attrIsACTION(av)            AttrIsId( av, TidyAttr_action )
#define attrIsADD_DATE(av)          AttrIsId( av, TidyAttr_add_date )
#define attrIsALIGN(av)             AttrIsId( av, TidyAttr_align )
#define attrIsALINK(av)             AttrIsId( av, TidyAttr_alink )
#define attrIsALT(av)               AttrIsId( av, TidyAttr_alt )
#define attrIsARCHIVE(av)           AttrIsId( av, TidyAttr_archive )
#define attrIsAXIS(av)              AttrIsId( av, TidyAttr_axis )
#define attrIsBACKGROUND(av)        AttrIsId( av, TidyAttr_background )
#define attrIsBGCOLOR(av)           AttrIsId( av, TidyAttr_bgcolor )
#define attrIsBGPROPERTIES(av)      AttrIsId( av, TidyAttr_bgproperties )
#define attrIsBORDER(av)            AttrIsId( av, TidyAttr_border )
#define attrIsBORDERCOLOR(av)       AttrIsId( av, TidyAttr_bordercolor )
#define attrIsBOTTOMMARGIN(av)      AttrIsId( av, TidyAttr_bottommargin )
#define attrIsCELLPADDING(av)       AttrIsId( av, TidyAttr_cellpadding )
#define attrIsCELLSPACING(av)       AttrIsId( av, TidyAttr_cellspacing )
#define attrIsCHAR(av)              AttrIsId( av, TidyAttr_char )
#define attrIsCHAROFF(av)           AttrIsId( av, TidyAttr_charoff )
#define attrIsCHARSET(av)           AttrIsId( av, TidyAttr_charset )
#define attrIsCHECKED(av)           AttrIsId( av, TidyAttr_checked )
#define attrIsCITE(av)              AttrIsId( av, TidyAttr_cite )
#define attrIsCLASS(av)             AttrIsId( av, TidyAttr_class )
#define attrIsCLASSID(av)           AttrIsId( av, TidyAttr_classid )
#define attrIsCLEAR(av)             AttrIsId( av, TidyAttr_clear )
#define attrIsCODE(av)              AttrIsId( av, TidyAttr_code )
#define attrIsCODEBASE(av)          AttrIsId( av, TidyAttr_codebase )
#define attrIsCODETYPE(av)          AttrIsId( av, TidyAttr_codetype )
#define attrIsCOLOR(av)             AttrIsId( av, TidyAttr_color )
#define attrIsCOLS(av)              AttrIsId( av, TidyAttr_cols )
#define attrIsCOLSPAN(av)           AttrIsId( av, TidyAttr_colspan )
#define attrIsCOMPACT(av)           AttrIsId( av, TidyAttr_compact )
#define attrIsCONTENT(av)           AttrIsId( av, TidyAttr_content )
#define attrIsCOORDS(av)            AttrIsId( av, TidyAttr_coords )
#define attrIsDATA(av)              AttrIsId( av, TidyAttr_data )
#define attrIsDATAFLD(av)           AttrIsId( av, TidyAttr_datafld )
#define attrIsDATAFORMATAS(av)      AttrIsId( av, TidyAttr_dataformatas )
#define attrIsDATAPAGESIZE(av)      AttrIsId( av, TidyAttr_datapagesize )
#define attrIsDATASRC(av)           AttrIsId( av, TidyAttr_datasrc )
#define attrIsDATETIME(av)          AttrIsId( av, TidyAttr_datetime )
#define attrIsDECLARE(av)           AttrIsId( av, TidyAttr_declare )
#define attrIsDEFER(av)             AttrIsId( av, TidyAttr_defer )
#define attrIsDIR(av)               AttrIsId( av, TidyAttr_dir )
#define attrIsDISABLED(av)          AttrIsId( av, TidyAttr_disabled )
#define attrIsENCODING(av)          AttrIsId( av, TidyAttr_encoding )
#define attrIsENCTYPE(av)           AttrIsId( av, TidyAttr_enctype )
#define attrIsFACE(av)              AttrIsId( av, TidyAttr_face )
#define attrIsFOR(av)               AttrIsId( av, TidyAttr_for )
#define attrIsFRAME(av)             AttrIsId( av, TidyAttr_frame )
#define attrIsFRAMEBORDER(av)       AttrIsId( av, TidyAttr_frameborder )
#define attrIsFRAMESPACING(av)      AttrIsId( av, TidyAttr_framespacing )
#define attrIsGRIDX(av)             AttrIsId( av, TidyAttr_gridx )
#define attrIsGRIDY(av)             AttrIsId( av, TidyAttr_gridy )
#define attrIsHEADERS(av)           AttrIsId( av, TidyAttr_headers )
#define attrIsHEIGHT(av)            AttrIsId( av, TidyAttr_height )
#define attrIsHREF(av)              AttrIsId( av, TidyAttr_href )
#define attrIsHREFLANG(av)          AttrIsId( av, TidyAttr_hreflang )
#define attrIsHSPACE(av)            AttrIsId( av, TidyAttr_hspace )
#define attrIsHTTP_EQUIV(av)        AttrIsId( av, TidyAttr_http_equiv )
#define attrIsID(av)                AttrIsId( av, TidyAttr_id )
#define attrIsISMAP(av)             AttrIsId( av, TidyAttr_ismap )
#define attrIsLABEL(av)             AttrIsId( av, TidyAttr_label )
#define attrIsLANG(av)              AttrIsId( av, TidyAttr_lang )
#define attrIsLANGUAGE(av)          AttrIsId( av, TidyAttr_language )
#define attrIsLAST_MODIFIED(av)     AttrIsId( av, TidyAttr_last_modified )
#define attrIsLAST_VISIT(av)        AttrIsId( av, TidyAttr_last_visit )
#define attrIsLEFTMARGIN(av)        AttrIsId( av, TidyAttr_leftmargin )
#define attrIsLINK(av)              AttrIsId( av, TidyAttr_link )
#define attrIsLONGDESC(av)          AttrIsId( av, TidyAttr_longdesc )
#define attrIsLOWSRC(av)            AttrIsId( av, TidyAttr_lowsrc )
#define attrIsMARGINHEIGHT(av)      AttrIsId( av, TidyAttr_marginheight )
#define attrIsMARGINWIDTH(av)       AttrIsId( av, TidyAttr_marginwidth )
#define attrIsMAXLENGTH(av)         AttrIsId( av, TidyAttr_maxlength )
#define attrIsMEDIA(av)             AttrIsId( av, TidyAttr_media )
#define attrIsMETHOD(av)            AttrIsId( av, TidyAttr_method )
#define attrIsMULTIPLE(av)          AttrIsId( av, TidyAttr_multiple )
#define attrIsNAME(av)              AttrIsId( av, TidyAttr_name )
#define attrIsNOHREF(av)            AttrIsId( av, TidyAttr_nohref )
#define attrIsNORESIZE(av)          AttrIsId( av, TidyAttr_noresize )
#define attrIsNOSHADE(av)           AttrIsId( av, TidyAttr_noshade )
#define attrIsNOWRAP(av)            AttrIsId( av, TidyAttr_nowrap )
#define attrIsOBJECT(av)            AttrIsId( av, TidyAttr_object )
#define attrIsOnAFTERUPDATE(av)     AttrIsId( av, TidyAttr_onafterupdate )
#define attrIsOnBEFOREUNLOAD(av)    AttrIsId( av, TidyAttr_onbeforeunload )
#define attrIsOnBEFOREUPDATE(av)    AttrIsId( av, TidyAttr_onbeforeupdate )
#define attrIsOnBLUR(av)            AttrIsId( av, TidyAttr_onblur )
#define attrIsOnCHANGE(av)          AttrIsId( av, TidyAttr_onchange )
#define attrIsOnCLICK(av)           AttrIsId( av, TidyAttr_onclick )
#define attrIsOnDATAAVAILABLE(av)   AttrIsId( av, TidyAttr_ondataavailable )
#define attrIsOnDATASETCHANGED(av)  AttrIsId( av, TidyAttr_ondatasetchanged )
#define attrIsOnDATASETCOMPLETE(av) AttrIsId( av, TidyAttr_ondatasetcomplete )
#define attrIsOnDBLCLICK(av)        AttrIsId( av, TidyAttr_ondblclick )
#define attrIsOnERRORUPDATE(av)     AttrIsId( av, TidyAttr_onerrorupdate )
#define attrIsOnFOCUS(av)           AttrIsId( av, TidyAttr_onfocus )
#define attrIsOnKEYDOWN(av)         AttrIsId( av, TidyAttr_onkeydown )
#define attrIsOnKEYPRESS(av)        AttrIsId( av, TidyAttr_onkeypress )
#define attrIsOnKEYUP(av)           AttrIsId( av, TidyAttr_onkeyup )
#define attrIsOnLOAD(av)            AttrIsId( av, TidyAttr_onload )
#define attrIsOnMOUSEDOWN(av)       AttrIsId( av, TidyAttr_onmousedown )
#define attrIsOnMOUSEMOVE(av)       AttrIsId( av, TidyAttr_onmousemove )
#define attrIsOnMOUSEOUT(av)        AttrIsId( av, TidyAttr_onmouseout )
#define attrIsOnMOUSEOVER(av)       AttrIsId( av, TidyAttr_onmouseover )
#define attrIsOnMOUSEUP(av)         AttrIsId( av, TidyAttr_onmouseup )
#define attrIsOnRESET(av)           AttrIsId( av, TidyAttr_onreset )
#define attrIsOnROWENTER(av)        AttrIsId( av, TidyAttr_onrowenter )
#define attrIsOnROWEXIT(av)         AttrIsId( av, TidyAttr_onrowexit )
#define attrIsOnSELECT(av)          AttrIsId( av, TidyAttr_onselect )
#define attrIsOnSUBMIT(av)          AttrIsId( av, TidyAttr_onsubmit )
#define attrIsOnUNLOAD(av)          AttrIsId( av, TidyAttr_onunload )
#define attrIsPROFILE(av)           AttrIsId( av, TidyAttr_profile )
#define attrIsPROMPT(av)            AttrIsId( av, TidyAttr_prompt )
#define attrIsRBSPAN(av)            AttrIsId( av, TidyAttr_rbspan )
#define attrIsREADONLY(av)          AttrIsId( av, TidyAttr_readonly )
#define attrIsREL(av)               AttrIsId( av, TidyAttr_rel )
#define attrIsREV(av)               AttrIsId( av, TidyAttr_rev )
#define attrIsRIGHTMARGIN(av)       AttrIsId( av, TidyAttr_rightmargin )
#define attrIsROWS(av)              AttrIsId( av, TidyAttr_rows )
#define attrIsROWSPAN(av)           AttrIsId( av, TidyAttr_rowspan )
#define attrIsRULES(av)             AttrIsId( av, TidyAttr_rules )
#define attrIsSCHEME(av)            AttrIsId( av, TidyAttr_scheme )
#define attrIsSCOPE(av)             AttrIsId( av, TidyAttr_scope )
#define attrIsSCROLLING(av)         AttrIsId( av, TidyAttr_scrolling )
#define attrIsSELECTED(av)          AttrIsId( av, TidyAttr_selected )
#define attrIsSHAPE(av)             AttrIsId( av, TidyAttr_shape )
#define attrIsSHOWGRID(av)          AttrIsId( av, TidyAttr_showgrid )
#define attrIsSHOWGRIDX(av)         AttrIsId( av, TidyAttr_showgridx )
#define attrIsSHOWGRIDY(av)         AttrIsId( av, TidyAttr_showgridy )
#define attrIsSIZE(av)              AttrIsId( av, TidyAttr_size )
#define attrIsSPAN(av)              AttrIsId( av, TidyAttr_span )
#define attrIsSRC(av)               AttrIsId( av, TidyAttr_src )
#define attrIsSTANDBY(av)           AttrIsId( av, TidyAttr_standby )
#define attrIsSTART(av)             AttrIsId( av, TidyAttr_start )
#define attrIsSTYLE(av)             AttrIsId( av, TidyAttr_style )
#define attrIsSUMMARY(av)           AttrIsId( av, TidyAttr_summary )
#define attrIsTABINDEX(av)          AttrIsId( av, TidyAttr_tabindex )
#define attrIsTARGET(av)            AttrIsId( av, TidyAttr_target )
#define attrIsTEXT(av)              AttrIsId( av, TidyAttr_text )
#define attrIsTITLE(av)             AttrIsId( av, TidyAttr_title )
#define attrIsTOPMARGIN(av)         AttrIsId( av, TidyAttr_topmargin )
#define attrIsTYPE(av)              AttrIsId( av, TidyAttr_type )
#define attrIsUSEMAP(av)            AttrIsId( av, TidyAttr_usemap )
#define attrIsVALIGN(av)            AttrIsId( av, TidyAttr_valign )
#define attrIsVALUE(av)             AttrIsId( av, TidyAttr_value )
#define attrIsVALUETYPE(av)         AttrIsId( av, TidyAttr_valuetype )
#define attrIsVERSION(av)           AttrIsId( av, TidyAttr_version )
#define attrIsVLINK(av)             AttrIsId( av, TidyAttr_vlink )
#define attrIsVSPACE(av)            AttrIsId( av, TidyAttr_vspace )
#define attrIsWIDTH(av)             AttrIsId( av, TidyAttr_width )
#define attrIsWRAP(av)              AttrIsId( av, TidyAttr_wrap )
#define attrIsXMLNS(av)             AttrIsId( av, TidyAttr_xmlns )
#define attrIsXML_LANG(av)          AttrIsId( av, TidyAttr_xml_lang )
#define attrIsXML_SPACE(av)         AttrIsId( av, TidyAttr_xml_space )


/* Attribute Retrieval macros
*/
#define attrGetHREF( nod )        AttrGetById( nod, TidyAttr_href )
#define attrGetSRC( nod )         AttrGetById( nod, TidyAttr_src )
#define attrGetID( nod )          AttrGetById( nod, TidyAttr_id )
#define attrGetNAME( nod )        AttrGetById( nod, TidyAttr_name )
#define attrGetSUMMARY( nod )     AttrGetById( nod, TidyAttr_summary )
#define attrGetALT( nod )         AttrGetById( nod, TidyAttr_alt )
#define attrGetLONGDESC( nod )    AttrGetById( nod, TidyAttr_longdesc )
#define attrGetUSEMAP( nod )      AttrGetById( nod, TidyAttr_usemap )
#define attrGetISMAP( nod )       AttrGetById( nod, TidyAttr_ismap )
#define attrGetLANGUAGE( nod )    AttrGetById( nod, TidyAttr_language )
#define attrGetTYPE( nod )        AttrGetById( nod, TidyAttr_type )
#define attrGetVALUE( nod )       AttrGetById( nod, TidyAttr_value )
#define attrGetCONTENT( nod )     AttrGetById( nod, TidyAttr_content )
#define attrGetTITLE( nod )       AttrGetById( nod, TidyAttr_title )
#define attrGetXMLNS( nod )       AttrGetById( nod, TidyAttr_xmlns )
#define attrGetDATAFLD( nod )     AttrGetById( nod, TidyAttr_datafld )
#define attrGetWIDTH( nod )       AttrGetById( nod, TidyAttr_width )
#define attrGetHEIGHT( nod )      AttrGetById( nod, TidyAttr_height )
#define attrGetFOR( nod )         AttrGetById( nod, TidyAttr_for )
#define attrGetSELECTED( nod )    AttrGetById( nod, TidyAttr_selected )
#define attrGetCHECKED( nod )     AttrGetById( nod, TidyAttr_checked )
#define attrGetLANG( nod )        AttrGetById( nod, TidyAttr_lang )
#define attrGetTARGET( nod )      AttrGetById( nod, TidyAttr_target )
#define attrGetHTTP_EQUIV( nod )  AttrGetById( nod, TidyAttr_http_equiv )
#define attrGetREL( nod )         AttrGetById( nod, TidyAttr_rel )

#define attrGetOnMOUSEMOVE( nod ) AttrGetById( nod, TidyAttr_onmousemove )
#define attrGetOnMOUSEDOWN( nod ) AttrGetById( nod, TidyAttr_onmousedown )
#define attrGetOnMOUSEUP( nod )   AttrGetById( nod, TidyAttr_onmouseup )
#define attrGetOnCLICK( nod )     AttrGetById( nod, TidyAttr_onclick )
#define attrGetOnMOUSEOVER( nod ) AttrGetById( nod, TidyAttr_onmouseover )
#define attrGetOnMOUSEOUT( nod )  AttrGetById( nod, TidyAttr_onmouseout )
#define attrGetOnKEYDOWN( nod )   AttrGetById( nod, TidyAttr_onkeydown )
#define attrGetOnKEYUP( nod )     AttrGetById( nod, TidyAttr_onkeyup )
#define attrGetOnKEYPRESS( nod )  AttrGetById( nod, TidyAttr_onkeypress )
#define attrGetOnFOCUS( nod )     AttrGetById( nod, TidyAttr_onfocus )
#define attrGetOnBLUR( nod )      AttrGetById( nod, TidyAttr_onblur )

#define attrGetBGCOLOR( nod )     AttrGetById( nod, TidyAttr_bgcolor )

#define attrGetLINK( nod )        AttrGetById( nod, TidyAttr_link )
#define attrGetALINK( nod )       AttrGetById( nod, TidyAttr_alink )
#define attrGetVLINK( nod )       AttrGetById( nod, TidyAttr_vlink )

#define attrGetTEXT( nod )        AttrGetById( nod, TidyAttr_text )
#define attrGetSTYLE( nod )       AttrGetById( nod, TidyAttr_style )
#define attrGetABBR( nod )        AttrGetById( nod, TidyAttr_abbr )
#define attrGetCOLSPAN( nod )     AttrGetById( nod, TidyAttr_colspan )
#define attrGetFONT( nod )        AttrGetById( nod, TidyAttr_font )
#define attrGetBASEFONT( nod )    AttrGetById( nod, TidyAttr_basefont )
#define attrGetROWSPAN( nod )     AttrGetById( nod, TidyAttr_rowspan )

#endif /* __ATTRS_H__ */
