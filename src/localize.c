/*
  localize.c

  (c) 1998-2002 (W3C) MIT, INRIA, Keio University
  See tidy.c for the copyright notice.

  You should only need to edit this file and tidy.c
  to localize HTML tidy.
  
  CVS Info :

    $Author: terry_teague $ 
    $Date: 2002/10/06 19:07:11 $ 
    $Revision: 1.64 $ 

*/

#include "platform.h"
#include "html.h"

/* used to point to Web Accessibility Guidelines */
#define ACCESS_URL  "http://www.w3.org/WAI/GL"
/* TRT */
/* points to the Adaptive Technology Resource Centre at the University of Toronto */
#define ATRC_ACCESS_URL  "http://www.aprompt.ca/Tidy/accessibilitychecks.html"

char *release_date = "1st October 2002";

static char *currentFile; /* sasdjb 01May00 for GNU Emacs error parsing */

extern uint optionerrors; /* not used for anything yet */

/*
 This routine is the single point via which
 all output is written and as such is a good
 way to interface Tidy to other code when
 embedding Tidy in a GUI application.
*/
void tidy_out(FILE *fp, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(fp, msg, args);
    va_end(args);
}

void ShowVersion(FILE *fp)
{
    /*
    tidy_out(fp, "HTML Tidy release date: %s\n"
            "See http://www.w3.org/People/Raggett for details\n", release_date);
    */
#ifdef PLATFORM_NAME
    tidy_out(fp, "\nHTML Tidy for %s (release date: %s; built on %s, at %s)\n"
            "See http://www.w3.org/People/Raggett for details\n",
                 PLATFORM_NAME, release_date, __DATE__, __TIME__);
#else
    tidy_out(fp, "\nHTML Tidy (release date: %s; built on %s, at %s)\n"
            "See http://www.w3.org/People/Raggett for details\n",
                 release_date, __DATE__, __TIME__);
#endif
}

void FileError(FILE *fp, const char *file)
{
    tidy_out(fp, "Can't open \"%s\"\n", file);
}

static void ReportTag(Lexer *lexer, Node *tag)
{
    if (tag)
    {
        if (tag->type == StartTag || tag->type == StartEndTag)
            tidy_out(lexer->errout, "<%s>", tag->element);
        else if (tag->type == EndTag)
            tidy_out(lexer->errout, "</%s>", tag->element);
        else if (tag->type == DocTypeTag)
            tidy_out(lexer->errout, "<!DOCTYPE>");
        else if (tag->type == TextNode)
            tidy_out(lexer->errout, "plain text");
        else
            tidy_out(lexer->errout, "%s", tag->element);
    }
}

/* lexer is not defined when this is called */
void ReportUnknownOption(char *option)
{
    optionerrors++;
    fprintf(stderr, "Warning - unknown option: %s\n", option);
}

/* lexer is not defined when this is called */
void ReportBadArgument(char *option)
{
    optionerrors++;
    fprintf(stderr, "Warning - missing or malformed argument for option: %s\n", option);
}

static void NtoS(int n, char *str)
{
    char buf[40];
    int i;

    for (i = 0;; ++i)
    {
        buf[i] = (n % 10) + '0';

        n = n / 10;

        if (n == 0)
            break;
    }

    n = i;

    while (i >= 0)
    {
        str[n-i] = buf[i];
        --i;
    }

    str[n+1] = '\0';
}

static void ReportPosition(Lexer *lexer)
{
    /* Change formatting to be parsable by GNU Emacs */
    if (Emacs)
    {
        tidy_out(lexer->errout, "%s", currentFile);
        tidy_out(lexer->errout, ":%d:", lexer->lines);
        tidy_out(lexer->errout, "%d: ", lexer->columns);
    }
    else /* traditional format */
    {
        tidy_out(lexer->errout, "line %d", lexer->lines);
        tidy_out(lexer->errout, " column %d - ", lexer->columns);
    }
}

void ReportEncodingError(Lexer *lexer, uint code, uint c)
{
    char buf[256];
                
    lexer->warnings++;

    /* keep quiet after <ShowErrors> errors */
    if (lexer->errors > ShowErrors)
        return;

    if (ShowWarnings)
    {
        ReportPosition(lexer);

        /* An encoding mismatch is currently treated as a non-fatal error */
        if ((code & ~DISCARDED_CHAR) == ENCODING_MISMATCH)
        {
            /* actual encoding passed in "c" */
            lexer->badChars |= ENCODING_MISMATCH;
            tidy_out(lexer->errout, "specified input encoding (%s) does not match actual input encoding (%s)",
                     CharEncodingName(lexer->in->encoding), CharEncodingName(c));
        }
        else if ((code & ~DISCARDED_CHAR) == VENDOR_SPECIFIC_CHARS)
        {
            NtoS(c, buf);
            lexer->badChars |= VENDOR_SPECIFIC_CHARS;
            tidy_out(lexer->errout, "Warning: %s invalid character code %s",
                     code & DISCARDED_CHAR?"discarding":"replacing", buf);
        }
        else if ((code & ~DISCARDED_CHAR) == INVALID_SGML_CHARS)
        {
            NtoS(c, buf);
            lexer->badChars |= INVALID_SGML_CHARS;
            tidy_out(lexer->errout, "Warning: %s invalid character code %s",
                     code & DISCARDED_CHAR?"discarding":"replacing", buf);
        }
        else if ((code & ~DISCARDED_CHAR) == INVALID_UTF8)
        {
            sprintf(buf, "U+%04lX", c);
            lexer->badChars |= INVALID_UTF8;
            tidy_out(lexer->errout, "Warning: %s invalid UTF-8 bytes (char. code %s)",
                     code & DISCARDED_CHAR?"discarding":"replacing", buf);
        }

#if SUPPORT_UTF16_ENCODINGS

        else if ((code & ~DISCARDED_CHAR) == INVALID_UTF16)
        {
            sprintf(buf, "U+%04lX", c);
            lexer->badChars |= INVALID_UTF16;
            tidy_out(lexer->errout, "Warning: %s invalid UTF-16 surrogate pair (char. code %s)",
                     code & DISCARDED_CHAR?"discarding":"replacing", buf);
        }

#endif

        else if ((code & ~DISCARDED_CHAR) == INVALID_NCR)
        {
            NtoS(c, buf);
            lexer->badChars |= INVALID_NCR;
            tidy_out(lexer->errout, "Warning: %s invalid numeric character reference %s",
                     code & DISCARDED_CHAR?"discarding":"replacing", buf);
        }

        tidy_out(lexer->errout, "\n");
    }
}

void ReportEntityError(Lexer *lexer, uint code, char *entity, int c)
{
    /* replacement for #427818 - fix by Tony Goodwin 11 Oct 00 */
    char *entityname = "NULL";

    if (entity)
        entityname = entity;

    lexer->warnings++;

    /* keep quiet after <ShowErrors> errors */
    if (lexer->errors > ShowErrors)
        return;

    if (ShowWarnings)
    {
        ReportPosition(lexer);

        if (code == MISSING_SEMICOLON)
        {
            tidy_out(lexer->errout, "Warning: entity \"%s\" doesn't end in ';'", entityname);
        }
        if (code == MISSING_SEMICOLON_NCR)
        {
            tidy_out(lexer->errout, "Warning: numeric character reference \"%s\" doesn't end in ';'", entityname);
        }
        else if (code == UNKNOWN_ENTITY)
        {
            tidy_out(lexer->errout, "Warning: unescaped & or unknown entity \"%s\"", entityname);
        }
        else if (code == UNESCAPED_AMPERSAND)
        {
            tidy_out(lexer->errout, "Warning: unescaped & which should be written as &amp;");
        }
        else if (code == APOS_UNDEFINED)
        {
            tidy_out(lexer->errout, "Warning: named entity &apos; only defined in XML/XHTML");
        }

        tidy_out(lexer->errout, "\n");
    }
}

void ReportAttrError(Lexer *lexer, Node *node, AttVal *av, uint code)
{
    /* replacement for #427676 - fix by Tony Goodwin 11 Oct 00 */
    char *name = "NULL", *value = "NULL";

    if (av)
    {
        if (av->attribute)
            name = av->attribute;
        if (av->value)
            value = av->value;
    }

    if (code == UNEXPECTED_GT)
        lexer->errors++;
    else
        lexer->warnings++;

    /* keep quiet after <ShowErrors> errors */
    if (lexer->errors > ShowErrors)
        return;

    if (ShowWarnings)
    {
        /* on end of file adjust reported position to end of input */
        if (code == UNEXPECTED_END_OF_FILE)
        {
            lexer->lines = lexer->in->curline;
            lexer->columns = lexer->in->curcol;
        }

        ReportPosition(lexer);

        if (code == UNKNOWN_ATTRIBUTE)
            tidy_out(lexer->errout, "Warning: unknown attribute \"%s\"", name);
        else if (code == MISSING_ATTRIBUTE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " lacks \"%s\" attribute", name);
        }
        else if (code == MISSING_ATTR_VALUE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " attribute \"%s\" lacks value", name);
        }
        else if (code == MISSING_IMAGEMAP) /* this is not used anywhere */
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " should use client-side image map");
            lexer->badAccess |= MISSING_IMAGE_MAP;
        }
        else if (code == BAD_ATTRIBUTE_VALUE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " attribute \"%s\" has invalid value \"%s\"", name, value);
        }
        else if (code == XML_ID_SYNTAX)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " ID \"%s\" uses XML ID syntax", value, name);
        }
        else if (code == XML_ATTRIBUTE_VALUE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " has XML attribute \"%s\"", name);
        }
        else if (code == UNEXPECTED_QUOTEMARK)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " unexpected or duplicate quote mark");
        }
        else if (code == MISSING_QUOTEMARK)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " attribute with missing trailing quote mark");
        }
        else if (code == REPEATED_ATTRIBUTE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " dropping value \"%s\" for repeated attribute \"%s\"", value, name);
        }
        else if (code == PROPRIETARY_ATTR_VALUE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " proprietary attribute value \"%s\"", value);
        }
        else if (code == PROPRIETARY_ATTRIBUTE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " proprietary attribute \"%s\"", name);
        }
        else if (code == UNEXPECTED_END_OF_FILE)
        {
            tidy_out(lexer->errout, "Warning: end of file while parsing attributes");
        }
        else if (code == ID_NAME_MISMATCH)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " id and name attribute value mismatch");
        }
        else if (code == BACKSLASH_IN_URI)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " URI reference contains backslash. Typo?");
        }
        else if (code == FIXED_BACKSLASH)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " converting backslash in URI to slash");
        }
        else if (code == ILLEGAL_URI_REFERENCE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " improperly escaped URI reference");
        }
        else if (code == ESCAPED_ILLEGAL_URI)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " escaping malformed URI reference");
        }
        else if (code == NEWLINE_IN_URI)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " discarding newline in URI reference");
        }
        else if (code == ANCHOR_NOT_UNIQUE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " Anchor \"%s\" already defined", value);
        }
        else if (code == ENTITY_IN_ID)
        {
            tidy_out(lexer->errout, "Warning: No entities allowed in id attribute, discarding \"&\"");
        }
        else if (code == JOINING_ATTRIBUTE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " joining values of repeated attribute \"%s\"", name);
        }
        else if (code == UNEXPECTED_EQUALSIGN)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " unexpected '=', expected attribute name");
        }
        else if (code == ATTR_VALUE_NOT_LCASE)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " attribute value \"%s\" must be lower case for XHTML", value);
        }

        if ((code != UNEXPECTED_GT))
            tidy_out(lexer->errout, "\n");
    }
    
    if (code == UNEXPECTED_GT)
    {
        if (!ShowWarnings)
            ReportPosition(lexer);

        tidy_out(lexer->errout, "Error: ");
        ReportTag(lexer, node);
        tidy_out(lexer->errout, " missing '>' for end of tag\n");
    }
}

void ReportMissingAttr(Lexer* lexer, Node* node, char* name)
{
   AttVal *dummy = NewAttributeEx(name, null);
   ReportAttrError(lexer, node, dummy, MISSING_ATTRIBUTE);
   FreeAttribute(dummy);
}

void ReportWarning(Lexer *lexer, Node *element, Node *node, uint code)
{
    if ((code == DISCARDING_UNEXPECTED) && lexer->badForm)
        /* lexer->errors++ */; /* already done in BadForm() */
    else
        lexer->warnings++;

    /* keep quiet after <ShowErrors> errors */
    if (lexer->errors > ShowErrors)
        return;

    if (ShowWarnings)
    {
        /* on end of file adjust reported position to end of input */
        if (code == UNEXPECTED_END_OF_FILE)
        {
            lexer->lines = lexer->in->curline;
            lexer->columns = lexer->in->curcol;
        }

        ReportPosition(lexer);

        if (code == MISSING_ENDTAG_FOR)
            tidy_out(lexer->errout, "Warning: missing </%s>", element->element);
        else if (code == MISSING_ENDTAG_BEFORE)
        {
            tidy_out(lexer->errout, "Warning: missing </%s> before ", element->element);
            ReportTag(lexer, node);
        }
        else if ((code == DISCARDING_UNEXPECTED) && (lexer->badForm == no))
        {
            /* the case for when this is an error not a warning, is handled later */
            tidy_out(lexer->errout, "Warning: discarding unexpected ");
            ReportTag(lexer, node);
        }
        else if (code == NESTED_EMPHASIS)
        {
            tidy_out(lexer->errout, "Warning: nested emphasis ");
            ReportTag(lexer, node);
        }
        else if (code == COERCE_TO_ENDTAG)
        {
            tidy_out(lexer->errout, "Warning: <%s> is probably intended as </%s>",
                node->element, node->element);
        }
        else if (code == NON_MATCHING_ENDTAG)
        {
            tidy_out(lexer->errout, "Warning: replacing unexpected ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " by </%s>", element->element);
        }
        else if (code == TAG_NOT_ALLOWED_IN)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " isn't allowed in <%s> elements", element->element);
        }
        else if (code == DOCTYPE_AFTER_TAGS)
        {
            tidy_out(lexer->errout, "Warning: <!DOCTYPE> isn't allowed after elements");
        }
        else if (code == MISSING_STARTTAG)
            tidy_out(lexer->errout, "Warning: missing <%s>", node->element);
        else if (code == UNEXPECTED_ENDTAG)
        {
            tidy_out(lexer->errout, "Warning: unexpected </%s>", node->element);

            if (element)
                tidy_out(lexer->errout, " in <%s>", element->element);
        }
        else if (code == TOO_MANY_ELEMENTS)
        {
            tidy_out(lexer->errout, "Warning: too many %s elements", node->element);

            if (element)
                tidy_out(lexer->errout, " in <%s>", element->element);
        }
        else if (code == USING_BR_INPLACE_OF)
        {
            tidy_out(lexer->errout, "Warning: using <br> in place of ");
            ReportTag(lexer, node);
        }
        else if (code == INSERTING_TAG)
            tidy_out(lexer->errout, "Warning: inserting implicit <%s>", node->element);
        else if (code == CANT_BE_NESTED)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " can't be nested");
        }
        else if (code == PROPRIETARY_ELEMENT)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " is not approved by W3C");

            if (node->tag == tag_layer)
                lexer->badLayout |= USING_LAYER;
            else if (node->tag == tag_spacer)
                lexer->badLayout |= USING_SPACER;
            else if (node->tag == tag_nobr)
                lexer->badLayout |= USING_NOBR;
        }
        else if (code == OBSOLETE_ELEMENT)
        {
            if (element->tag && (element->tag->model & CM_OBSOLETE))
                tidy_out(lexer->errout, "Warning: replacing obsolete element ");
            else
                tidy_out(lexer->errout, "Warning: replacing element ");

            ReportTag(lexer, element);
            tidy_out(lexer->errout, " by ");
            ReportTag(lexer, node);
        }
        else if (code == UNESCAPED_ELEMENT)
        {
            tidy_out(lexer->errout, "Warning: unescaped ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " in pre content");
        }
        else if (code == TRIM_EMPTY_ELEMENT)
        {
            tidy_out(lexer->errout, "Warning: trimming empty ");
            ReportTag(lexer, element);
        }
        else if (code == MISSING_TITLE_ELEMENT)
            tidy_out(lexer->errout, "Warning: inserting missing 'title' element");
        else if (code == ILLEGAL_NESTING)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, element);
            tidy_out(lexer->errout, " shouldn't be nested");
        }
        else if (code == NOFRAMES_CONTENT)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, node);
            tidy_out(lexer->errout, " not inside 'noframes' element");
        }
        else if (code == INCONSISTENT_VERSION)
        {
            tidy_out(lexer->errout, "Warning: HTML DOCTYPE doesn't match content");
        }
        else if (code == MALFORMED_DOCTYPE)
        {
            tidy_out(lexer->errout, "Warning: expected \"html PUBLIC\" or \"html SYSTEM\"");
        }
        else if (code == CONTENT_AFTER_BODY)
        {
            tidy_out(lexer->errout, "Warning: content occurs after end of body");
        }
        else if (code == MALFORMED_COMMENT)
        {
            tidy_out(lexer->errout, "Warning: adjacent hyphens within comment");
        }
        else if (code == BAD_COMMENT_CHARS)
        {
            tidy_out(lexer->errout, "Warning: expecting -- or >");
        }
        else if (code == BAD_XML_COMMENT)
        {
            tidy_out(lexer->errout, "Warning: XML comments can't contain --");
        }
        else if (code == BAD_CDATA_CONTENT)
        {
            tidy_out(lexer->errout, "Warning: '<' + '/' + letter not allowed here");
        }
        else if (code == INCONSISTENT_NAMESPACE)
        {
            tidy_out(lexer->errout, "Warning: HTML namespace doesn't match content");
        }
        else if (code == DTYPE_NOT_UPPER_CASE)
        {
            tidy_out(lexer->errout, "Warning: SYSTEM, PUBLIC, W3C, DTD, EN must be upper case");
        }
        else if (code == UNEXPECTED_END_OF_FILE)
        {
            tidy_out(lexer->errout, "Warning: unexpected end of file");
            ReportTag(lexer, element);
        }
        else if (code == NESTED_QUOTATION)
        {
            tidy_out(lexer->errout, "Warning: nested q elements, possible typo.");
        }
        else if (code == ELEMENT_NOT_EMPTY)
        {
            tidy_out(lexer->errout, "Warning: ");
            ReportTag(lexer, element);
            tidy_out(lexer->errout, " element not empty or not closed");
        }

        if ((code != DISCARDING_UNEXPECTED) || (lexer->badForm == no))
            tidy_out(lexer->errout, "\n");
    }
    
    if ((code == DISCARDING_UNEXPECTED) && lexer->badForm)
    {
        /* the case for when this is a warning not an error, is handled earlier */
        if (!ShowWarnings)
            ReportPosition(lexer);

        tidy_out(lexer->errout, "Error: discarding unexpected ");
        ReportTag(lexer, node);
        tidy_out(lexer->errout, "\n");
    }
}

void ReportError(Lexer *lexer, Node *element, Node *node, uint code)
{
    /* lexer->warnings++; */
    lexer->errors++;

    /* keep quiet after <ShowErrors> errors */
    if (lexer->errors > ShowErrors)
        return;

    ReportPosition(lexer);

    if (code == SUSPECTED_MISSING_QUOTE)
    {
        tidy_out(lexer->errout, "Error: missing quote mark for attribute value");
    }
    else if (code == DUPLICATE_FRAMESET)
    {
        tidy_out(lexer->errout, "Error: repeated FRAMESET element");
    }
    else if (code == UNKNOWN_ELEMENT)
    {
        tidy_out(lexer->errout, "Error: ");
        ReportTag(lexer, node);
        tidy_out(lexer->errout, " is not recognized!");
    }
    else if (code == UNEXPECTED_ENDTAG)  /* generated by XML docs */
    {
        tidy_out(lexer->errout, "Error: unexpected </%s>", node->element); /* #434100 - fix by various */

        if (element)
            tidy_out(lexer->errout, " in <%s>", element->element);
    }

    tidy_out(lexer->errout, "\n");
}

void ErrorSummary(Lexer *lexer)
{
    /* adjust badAccess to that its null if frames are ok */
    if (lexer->badAccess & (USING_FRAMES | USING_NOFRAMES))
    {
        if (!((lexer->badAccess & USING_FRAMES) && !(lexer->badAccess & USING_NOFRAMES)))
            lexer->badAccess &= ~(USING_FRAMES | USING_NOFRAMES);
    }

    if (lexer->badChars)
    {
#if 0
        if (lexer->badChars & WINDOWS_CHARS)
        {
            tidy_out(lexer->errout, "Characters codes for the Microsoft Windows fonts in the range\n");
            tidy_out(lexer->errout, "128 - 159 may not be recognized on other platforms. You are\n");
            tidy_out(lexer->errout, "instead recommended to use named entities, e.g. &trade; rather\n");
            tidy_out(lexer->errout, "than Windows character code 153 (0x2122 in Unicode). Note that\n");
            tidy_out(lexer->errout, "as of February 1998 few browsers support the new entities.\n\n");
        }
#endif
        if (lexer->badChars & VENDOR_SPECIFIC_CHARS)
        {
            tidy_out(lexer->errout, "It is unlikely that vendor-specific, system-dependent encodings\n");
            tidy_out(lexer->errout, "work widely enough on the World Wide Web; you should avoid using the \n");
            tidy_out(lexer->errout, "%s character encoding, instead you are recommended to\n",
                     (lexer->in->encoding == WIN1252)?"Windows-1252":
                     (lexer->in->encoding == MACROMAN)?"MacRoman":"specified");
            tidy_out(lexer->errout, "use named entities, e.g. &trade;.\n\n");
        }
        if ((lexer->badChars & INVALID_SGML_CHARS) || (lexer->badChars & INVALID_NCR))
        {
            tidy_out(lexer->errout, "Character codes 128 to 159 (U+0080 to U+009F) are not allowed in HTML;\n");
            tidy_out(lexer->errout, "even if they were, they would likely be unprintable control characters.\n");
            tidy_out(lexer->errout, "Tidy assumed you wanted to refer to a character with the same byte value in the \n");
            tidy_out(lexer->errout, "%s encoding and replaced that reference with the Unicode equivalent.\n\n",
                     (ReplacementCharEncoding == WIN1252)?"Windows-1252":
                     (ReplacementCharEncoding == MACROMAN)?"MacRoman":"default");
        }
        if (lexer->badChars & INVALID_UTF8)
        {
            tidy_out(lexer->errout, "Character codes for UTF-8 must be in the range: U+0000 to U+10FFFF.\n");
            tidy_out(lexer->errout, "The definition of UTF-8 in Annex D of ISO/IEC 10646-1:2000 also\n");
            tidy_out(lexer->errout, "allows for the use of five- and six-byte sequences to encode\n");
            tidy_out(lexer->errout, "characters that are outside the range of the Unicode character set;\n");
            tidy_out(lexer->errout, "those five- and six-byte sequences are illegal for the use of\n");
            tidy_out(lexer->errout, "UTF-8 as a transformation of Unicode characters. ISO/IEC 10646\n");
            tidy_out(lexer->errout, "does not allow mapping of unpaired surrogates, nor U+FFFE and U+FFFF\n");
            tidy_out(lexer->errout, "(but it does allow other noncharacters). For more information please refer to\n");
            tidy_out(lexer->errout, "http://www.unicode.org/unicode and http://www.cl.cam.ac.uk/~mgk25/unicode.html\n\n");
        }

#if SUPPORT_UTF16_ENCODINGS

        if (lexer->badChars & INVALID_UTF16)
        {
            tidy_out(lexer->errout, "Character codes for UTF-16 must be in the range: U+0000 to U+10FFFF.\n");
            tidy_out(lexer->errout, "The definition of UTF-16 in Annex C of ISO/IEC 10646-1:2000 does not allow the\n");
            tidy_out(lexer->errout, "mapping of unpaired surrogates. For more information please refer to\n");
            tidy_out(lexer->errout, "http://www.unicode.org/unicode and http://www.cl.cam.ac.uk/~mgk25/unicode.html\n\n");
        }

#endif

        if (lexer->badChars & INVALID_URI)
        {
            tidy_out(lexer->errout, "URIs must be properly escaped, they must not contain unescaped\n");
            tidy_out(lexer->errout, "characters below U+0021 including the space character and not\n");
            tidy_out(lexer->errout, "above U+007E. Tidy escapes the URI for you as recommended by\n");
            tidy_out(lexer->errout, "HTML 4.01 section B.2.1 and XML 1.0 section 4.2.2. Some user agents\n");
            tidy_out(lexer->errout, "use another algorithm to escape such URIs and some server-sided\n");
            tidy_out(lexer->errout, "scripts depend on that. If you want to depend on that, you must\n");
            tidy_out(lexer->errout, "escape the URI by your own. For more information please refer to\n");
            tidy_out(lexer->errout, "http://www.w3.org/International/O-URL-and-ident.html\n\n");
        }
    }

    if (lexer->badForm)
    {
        tidy_out(lexer->errout, "You may need to move one or both of the <form> and </form>\n");
        tidy_out(lexer->errout, "tags. HTML elements should be properly nested and form elements\n");
        tidy_out(lexer->errout, "are no exception. For instance you should not place the <form>\n");
        tidy_out(lexer->errout, "in one table cell and the </form> in another. If the <form> is\n");
        tidy_out(lexer->errout, "placed before a table, the </form> cannot be placed inside the\n");
        tidy_out(lexer->errout, "table! Note that one form can't be nested inside another!\n\n");
    }
    
    if (lexer->badAccess)
    {
/* TRT */
#if !USE_ORIGINAL_ACCESSIBILITY_CHECKS
        if (AccessibilityCheckLevel != 0)
        {
            tidy_out(lexer->errout, "For further advice on how to make your pages accessible, see\n");
            tidy_out(lexer->errout, "\"%s\" and \n\"%s\".\n", ACCESS_URL, ATRC_ACCESS_URL);
            tidy_out(lexer->errout, "You may also want to try \"http://www.cast.org/bobby/\" which is a free Web-based\n");
            tidy_out(lexer->errout, "service for checking URLs for accessibility.\n\n");
        } else
#endif
        {
        if (lexer->badAccess & MISSING_SUMMARY)
        {
            tidy_out(lexer->errout, "The table summary attribute should be used to describe\n");
            tidy_out(lexer->errout, "the table structure. It is very helpful for people using\n");
            tidy_out(lexer->errout, "non-visual browsers. The scope and headers attributes for\n");
            tidy_out(lexer->errout, "table cells are useful for specifying which headers apply\n");
            tidy_out(lexer->errout, "to each table cell, enabling non-visual browsers to provide\n");
            tidy_out(lexer->errout, "a meaningful context for each cell.\n\n");
        }

        if (lexer->badAccess & MISSING_IMAGE_ALT)
        {
            tidy_out(lexer->errout, "The alt attribute should be used to give a short description\n");
            tidy_out(lexer->errout, "of an image; longer descriptions should be given with the\n");
            tidy_out(lexer->errout, "longdesc attribute which takes a URL linked to the description.\n");
            tidy_out(lexer->errout, "These measures are needed for people using non-graphical browsers.\n\n");
        }

        if (lexer->badAccess & MISSING_IMAGE_MAP)
        {
            tidy_out(lexer->errout, "Use client-side image maps in preference to server-side image\n");
            tidy_out(lexer->errout, "maps as the latter are inaccessible to people using non-\n");
            tidy_out(lexer->errout, "graphical browsers. In addition, client-side maps are easier\n");
            tidy_out(lexer->errout, "to set up and provide immediate feedback to users.\n\n");
        }

        if (lexer->badAccess & MISSING_LINK_ALT)
        {
            tidy_out(lexer->errout, "For hypertext links defined using a client-side image map, you\n");
            tidy_out(lexer->errout, "need to use the alt attribute to provide a textual description\n");
            tidy_out(lexer->errout, "of the link for people using non-graphical browsers.\n\n");
        }

        if ((lexer->badAccess & USING_FRAMES) && !(lexer->badAccess & USING_NOFRAMES))
        {
            tidy_out(lexer->errout, "Pages designed using frames presents problems for\n");
            tidy_out(lexer->errout, "people who are either blind or using a browser that\n");
            tidy_out(lexer->errout, "doesn't support frames. A frames-based page should always\n");
            tidy_out(lexer->errout, "include an alternative layout inside a NOFRAMES element.\n\n");
        }

        tidy_out(lexer->errout, "For further advice on how to make your pages accessible\n");
        tidy_out(lexer->errout, "see \"%s\". You may also want to try\n", ACCESS_URL);
        tidy_out(lexer->errout, "\"http://www.cast.org/bobby/\" which is a free Web-based\n");
        tidy_out(lexer->errout, "service for checking URLs for accessibility.\n\n");
        }
    }

    if (lexer->badLayout)
    {
        if (lexer->badLayout & USING_LAYER)
        {
            tidy_out(lexer->errout, "The Cascading Style Sheets (CSS) Positioning mechanism\n");
            tidy_out(lexer->errout, "is recommended in preference to the proprietary <LAYER>\n");
            tidy_out(lexer->errout, "element due to limited vendor support for LAYER.\n\n");
        }

        if (lexer->badLayout & USING_SPACER)
        {
            tidy_out(lexer->errout, "You are recommended to use CSS for controlling white\n");
            tidy_out(lexer->errout, "space (e.g. for indentation, margins and line spacing).\n");
            tidy_out(lexer->errout, "The proprietary <SPACER> element has limited vendor support.\n\n");
        }

        if (lexer->badLayout & USING_FONT)
        {
            tidy_out(lexer->errout, "You are recommended to use CSS to specify the font and\n");
            tidy_out(lexer->errout, "properties such as its size and color. This will reduce\n");
            tidy_out(lexer->errout, "the size of HTML files and make them easier to maintain\n"); /* #427674 - fix by Andrew Billington 22 Aug 00 */
            tidy_out(lexer->errout, "compared with using <FONT> elements.\n\n");
        }

        if (lexer->badLayout & USING_NOBR)
        {
            tidy_out(lexer->errout, "You are recommended to use CSS to control line wrapping.\n");
            tidy_out(lexer->errout, "Use \"white-space: nowrap\" to inhibit wrapping in place\n");
            tidy_out(lexer->errout, "of inserting <NOBR>...</NOBR> into the markup.\n\n");
        }

        if (lexer->badLayout & USING_BODY)
        {
            tidy_out(lexer->errout, "You are recommended to use CSS to specify page and link colors\n");
        }
    }
}

void UnknownOption(FILE *errout, char c)
{
    tidy_out(errout, "unrecognized option -%c use -help to list options\n", c);
}

void UnknownFile(FILE *errout, char *program, char *file)
{
    tidy_out(errout, "%s: can't open file \"%s\"\n", program, file);
}

void NeedsAuthorIntervention(FILE *errout)
{
    tidy_out(errout, "This document has errors that must be fixed before\n");
    tidy_out(errout, "using HTML Tidy to generate a tidied up version.\n\n");
}

void MissingBody(FILE *errout)
{
    tidy_out(errout, "Can't create slides - document is missing a body element.\n");
}

void ReportNumberOfSlides(FILE *errout, int count)
{
    tidy_out(errout, "%d Slides found\n", count);
}

void GeneralInfo(FILE *errout)
{
    tidy_out(errout, "To learn more about HTML Tidy see http://tidy.sourceforge.net\n");
    tidy_out(errout, "Please send bug reports to html-tidy@w3.org\n");
    tidy_out(errout, "HTML and CSS specifications are available from http://www.w3.org/\n");
    tidy_out(errout, "Lobby your company to join W3C, see http://www.w3.org/Consortium\n");
}

/* #431895 - fix by Dave Bryan 04 Jan 01 */
void SetFilename (char *filename) 
{
    currentFile = filename;  /* for use with Gnu Emacs */
}

void HelloMessage(FILE *errout, char *date, char *filename)
{
    /* #431895 - fix by Dave Bryan 04 Jan 01 */
    /* currentFile = filename; */  /* for use with Gnu Emacs */

    /*
    if (wstrcmp(filename, "stdin") == 0)
        tidy_out(errout, "\nTidy (vers %s) Parsing console input (stdin)\n", date);
    else
        tidy_out(errout, "\nTidy (vers %s) Parsing \"%s\"\n", date, filename);
    */
    
#ifdef PLATFORM_NAME
    if (wstrcmp(filename, "stdin") == 0)
        tidy_out(errout, "\nHTML Tidy for %s (vers %s; built on %s, at %s)\nParsing console input (stdin)\n",
                 PLATFORM_NAME, date, __DATE__, __TIME__);
    else
        tidy_out(errout, "\nHTML Tidy for %s (vers %s; built on %s, at %s)\nParsing \"%s\"\n",
                 PLATFORM_NAME, date, __DATE__, __TIME__, filename);
#else
    if (wstrcmp(filename, "stdin") == 0)
        tidy_out(errout, "\nHTML Tidy (vers %s; built on %s, at %s)\nParsing console input (stdin)\n",
                 date, __DATE__, __TIME__);
    else
        tidy_out(errout, "\nHTML Tidy (vers %s; built on %s, at %s)\nParsing \"%s\"\n",
                 date, __DATE__, __TIME__, filename);
#endif
}

void ReportVersion(FILE *errout, Lexer *lexer, char *filename, Node *doctype)
{
    uint i, c;
    int state = 0;
    char *vers = HTMLVersionName(lexer);

    if (doctype)
    {
        tidy_out(errout, "\n%s: Doctype given is \"", filename);

        for (i = doctype->start; i < doctype->end; ++i)
        {
            c = (unsigned char)lexer->lexbuf[i];

            /* look for UTF-8 multibyte character */
            if (c > 0x7F)
                 i += GetUTF8((unsigned char *)lexer->lexbuf + i, &c);

            if (c == '"')
                ++state;
            else if (state == 1)
                putc(c, errout);
        }

        putc('"', errout);
    }

    tidy_out(errout, "\n%s: Document content looks like %s\n",
                filename, (vers ? vers : "HTML proprietary"));
}

void ReportNumWarnings(FILE *errout, Lexer *lexer)
{
    /*
    if (lexer->warnings > 0)
        tidy_out(errout, "%d warnings/errors were found!\n\n", lexer->warnings);
    */
    if ((lexer->warnings > 0) || (lexer->errors > 0))
    {
        tidy_out(errout, "%d %s, %d %s were found!",
                 lexer->warnings, lexer->warnings == 1?"warning":"warnings",
                 lexer->errors, lexer->errors == 1?"error":"errors");
        if ((lexer->errors > ShowErrors) || !ShowWarnings)
            tidy_out(errout, " Not all warnings/errors were shown.\n\n");
        else
            tidy_out(errout, "\n\n");
    }
    else
        tidy_out(errout, "No warnings or errors were found.\n\n");
}

void HelpText(FILE *out, char *prog)
{
#if 0  /* old style help text */
    tidy_out(out, "%s: file1 file2 ...\n", prog);
    tidy_out(out, "Utility to clean up & pretty print html files\n");
    tidy_out(out, "see http://www.w3.org/People/Raggett/tidy/\n");
    tidy_out(out, "options for tidy released on %s\n", release_date);
    tidy_out(out, "  -config <file>  set options from config file\n");
    tidy_out(out, "  -indent or -i   indent element content\n");
    tidy_out(out, "  -omit   or -o   omit optional endtags\n");
    tidy_out(out, "  -wrap 72        wrap text at column 72 (default is 68)\n");
    tidy_out(out, "  -upper  or -u   force tags to upper case (default is lower)\n");
    tidy_out(out, "  -clean  or -c   replace font, nobr & center tags by CSS\n");
    tidy_out(out, "  -raw            leave chars > 128 unchanged upon output\n");
    tidy_out(out, "  -ascii          use ASCII for output, Latin-1 for input\n");
    tidy_out(out, "  -latin1         use Latin-1 for both input and output\n");
    tidy_out(out, "  -iso2022        use ISO2022 for both input and output\n");
    tidy_out(out, "  -utf8           use UTF-8 for both input and output\n");
    tidy_out(out, "  -mac            use the Apple MacRoman character set\n");
    tidy_out(out, "  -numeric or -n  output numeric rather than named entities\n");
    tidy_out(out, "  -modify or -m   to modify original files\n");
    tidy_out(out, "  -errors or -e   only show errors\n");
    tidy_out(out, "  -quiet or -q    suppress nonessential output\n");
    tidy_out(out, "  -f <file>       write errors to named <file>\n");
    tidy_out(out, "  -xml            use this when input is wellformed xml\n");
    tidy_out(out, "  -asxml          to convert html to wellformed xml\n");
    tidy_out(out, "  -slides         to burst into slides on h2 elements\n");
    tidy_out(out, "  -version or -v  show version\n");
    tidy_out(out, "  -help   or -h   list command line options (this message)\n");
    tidy_out(out, "  -help-config    list all configuration file options\n");
    tidy_out(out, "Input/Output default to stdin/stdout respectively\n");
    tidy_out(out, "Single letter options apart from -f may be combined\n");
    tidy_out(out, "as in:  tidy -f errs.txt -imu foo.html\n");
    tidy_out(out, "You can also use --blah for any config file option blah\n");
    tidy_out(out, "For further info on HTML see http://www.w3.org/MarkUp\n");
#else
    tidy_out(out, "%s [option...] [file...]\n", prog);
    tidy_out(out, "Utility to clean up and pretty print HTML/XHTML/XML\n");
    tidy_out(out, "see http://www.w3.org/People/Raggett/tidy/\n");
    tidy_out(out, "\n");
#ifdef PLATFORM_NAME
    tidy_out(out, "Options for HTML Tidy for %s released on %s:\n", PLATFORM_NAME, release_date);
#else
    tidy_out(out, "Options for HTML Tidy released on %s:\n", release_date);
#endif
    tidy_out(out, "\n");

    tidy_out(out, "Processing directives\n");
    tidy_out(out, "---------------------\n");
    tidy_out(out, "  -indent  or -i    to indent element content\n");
    tidy_out(out, "  -omit    or -o    to omit optional end tags\n");
    tidy_out(out, "  -wrap <column>    to wrap text at the specified <column> (default is 68)\n");
    tidy_out(out, "  -upper   or -u    to force tags to upper case (default is lower case)\n");
    tidy_out(out, "  -clean   or -c    to replace FONT, NOBR and CENTER tags by CSS\n");
    tidy_out(out, "  -bare    or -b    to strip out smart quotes and em dashes, etc.\n");
    tidy_out(out, "  -numeric or -n    to output numeric rather than named entities\n");
    tidy_out(out, "  -errors  or -e    to only show errors\n");
    tidy_out(out, "  -quiet   or -q    to suppress nonessential output\n");
    tidy_out(out, "  -xml              to specify the input is well formed XML\n");
    tidy_out(out, "  -asxml            to convert HTML to well formed XHTML\n");
    tidy_out(out, "  -asxhtml          to convert HTML to well formed XHTML\n");
    tidy_out(out, "  -ashtml           to force XHTML to well formed HTML\n");
    tidy_out(out, "  -slides           to burst into slides on H2 elements\n");

/* TRT */
#if SUPPORT_ACCESSIBILITY_CHECKS
    tidy_out(out, "  -access <level>   to do additional accessibility checks (<level> = 1, 2, 3)\n");
#endif

    tidy_out(out, "\n");

    tidy_out(out, "Character encodings\n");
    tidy_out(out, "-------------------\n");
    tidy_out(out, "  -raw              to output values above 127 without conversion to entities\n");
    tidy_out(out, "  -ascii            to use US-ASCII for output, ISO-8859-1 for input\n");
    tidy_out(out, "  -latin1           to use ISO-8859-1 for both input and output\n");
    tidy_out(out, "  -iso2022          to use ISO-2022 for both input and output\n");
    tidy_out(out, "  -utf8             to use UTF-8 for both input and output\n");
    tidy_out(out, "  -mac              to use MacRoman for input, US-ASCII for output\n");

#if SUPPORT_UTF16_ENCODINGS

    tidy_out(out, "  -utf16le          to use UTF-16LE for both input and output\n");
    tidy_out(out, "  -utf16be          to use UTF-16BE for both input and output\n");
    tidy_out(out, "  -utf16            to use UTF-16 for both input and output\n");

#endif

    tidy_out(out, "  -win1252          to use Windows-1252 for input, US-ASCII for output\n");

#if SUPPORT_ASIAN_ENCODINGS

    tidy_out(out, "  -big5             to use Big5 for both input and output\n"); /* #431953 - RJ */
    tidy_out(out, "  -shiftjis         to use Shift_JIS for both input and output\n"); /* #431953 - RJ */
    tidy_out(out, "  -language <lang>  to set the two-letter language code <lang> (for future use)\n"); /* #431953 - RJ */

#endif

    tidy_out(out, "\n");

    tidy_out(out, "File manipulation\n");
    tidy_out(out, "-----------------\n");
    tidy_out(out, "  -config <file>    to set configuration options from the specified <file>\n");
    tidy_out(out, "  -f      <file>    to write errors to the specified <file>\n");
    tidy_out(out, "  -modify or -m     to modify the original input files\n");
    tidy_out(out, "\n");

    tidy_out(out, "Miscellaneous\n");
    tidy_out(out, "-------------\n");
    tidy_out(out, "  -version  or -v   to show the version of Tidy\n");
    tidy_out(out, "  -help, -h or -?   to list the command line options\n");
    tidy_out(out, "  -help-config      to list all configuration options\n");
    tidy_out(out, "  -show-config      to list the current configuration settings\n");
    tidy_out(out, "\n");
    tidy_out(out, "You can also use --blah for any configuration option blah\n");
    tidy_out(out, "\n");

    tidy_out(out, "Input/Output default to stdin/stdout respectively\n");
    tidy_out(out, "Single letter options apart from -f may be combined\n");
    tidy_out(out, "as in:  tidy -f errs.txt -imu foo.html\n");
    tidy_out(out, "For further info on HTML see http://www.w3.org/MarkUp\n");
    tidy_out(out, "\n");
#endif
}
