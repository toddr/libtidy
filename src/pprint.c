/*
  pprint.c -- pretty print parse tree  
  
  (c) 1998-2001 (W3C) MIT, INRIA, Keio University
  See tidy.c for the copyright notice.
  
  CVS Info :

    $Author: terry_teague $ 
    $Date: 2001/09/07 07:12:37 $ 
    $Revision: 1.33 $ 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "html.h"

/*
  Block-level and unknown elements are printed on
  new lines and their contents indented 2 spaces

  Inline elements are printed inline.

  Inline content is wrapped on spaces (except in
  attribute values or preformatted text, after
  start tags and before end tags
*/

static void PPrintAsp(Out *fout, uint indent,
                   Lexer *lexer, Node *node);
static void PPrintJste(Out *fout, uint indent,
                   Lexer *lexer, Node *node);
static void PPrintPhp(Out *fout, uint indent,
                   Lexer *lexer, Node *node);


#define NORMAL        0
#define PREFORMATTED  1
#define COMMENT       2
#define ATTRIBVALUE   4
#define NOWRAP        8
#define CDATA         16

extern int CharEncoding;
extern int inCharEncoding;
extern int outCharEncoding;

static uint *linebuf;
static uint lbufsize;
static uint linelen;
static uint wraphere;
static Bool InAttVal;
static Bool InString;

static int slide, count;
static Node *slidecontent;

#define AddAsciiString( s, llen )\
do {\
  char* cp;\
  for (cp=s; *cp; ++cp)\
    AddC( (uint) *cp, llen++ );\
} while (0)

#if SUPPORT_ASIAN_ENCODINGS

/* #431953 - start RJ Wraplen adjusted for smooth international ride */
uint CWrapLen(uint ind)
{

    if ( !wstrcasecmp(Language, "zh") )
        /* Chinese characters take two positions on a fixed-width screen */ 
        /* It would be more accurate to keep a parallel linelen and wraphere
           incremented by 2 for Chinese characters and 1 otherwise, but this
           is way simpler.
        */
        return (ind + (( wraplen - ind ) / 2)) ; 
    
    if ( !wstrcasecmp(Language, "ja") )
        /* average Japanese text is 30% kanji */
        return (ind + ((( wraplen - ind ) * 7) / 10)) ; 
    
    return (wraplen);
}
/* #431953 - end RJ */

#endif

/* return one less than the number of bytes used by the UTF-8 byte sequence */
/* str points to the UTF-8 byte sequence */
/* the Unicode char is returned in *ch */
uint GetUTF8(unsigned char *str, uint *ch)
{
    uint n;
    int bytes;

#if 0
    uint i, c = str[0];

    if ((c & 0xE0) == 0xC0)  /* 110X XXXX  two bytes */
    {
        n = c & 31;
        bytes = 2;
    }
    else if ((c & 0xF0) == 0xE0)  /* 1110 XXXX  three bytes */
    {
        n = c & 15;
        bytes = 3;
    }
    else if ((c & 0xF8) == 0xF0)  /* 1111 0XXX  four bytes */
    {
        n = c & 7;
        bytes = 4;
    }
    else if ((c & 0xFC) == 0xF8)  /* 1111 10XX  five bytes */
    {
        n = c & 3;
        bytes = 5;
    }
    else if ((c & 0xFE) == 0xFC)       /* 1111 110X  six bytes */

    {
        n = c & 1;
        bytes = 6;
    }
    else  /* 0XXX XXXX one byte */
    {
        *ch = c;
        return 0;
    }

    /* successor bytes should have the form 10XX XXXX */
    for (i = 1; i < bytes; ++i)
    {
        c = str[i];
        n = (n << 6) | (c & 0x3F);
    }
#else
    int err;
    
    bytes = 0;
    
    /* first byte "str[0]" is passed in separately from the */
    /* rest of the UTF-8 byte sequence starting at "str[1]" */
    err = DecodeUTF8BytesToChar(&n, str[0], (unsigned char *)&str[1], null, null, &bytes);
    if (err)
    {
#if 0
        extern FILE* errout; /* debug */

        tidy_out(errout, "pprint UTF-8 decoding error for U+%x : ", n); /* debug */
#endif
        n = 0xFFFD; /* replacement char */
    }
#endif

    *ch = n;
    return bytes - 1;
}

/* store char c as UTF-8 encoded byte stream */
char *PutUTF8(char *buf, uint c)
{
#if 0
    if (c < 128)
        *buf++ = c;
    else if (c <= 0x7FF)
    {
        *buf++ =  (0xC0 | (c >> 6));
        *buf++ = (0x80 | (c & 0x3F));
    }
    else if (c <= 0xFFFF)
    {
        *buf++ =  (0xE0 | (c >> 12));
        *buf++ =  (0x80 | ((c >> 6) & 0x3F));
        *buf++ =  (0x80 | (c & 0x3F));
    }
    else if (c <= 0x1FFFFF)
    {
        *buf++ =  (0xF0 | (c >> 18));
        *buf++ =  (0x80 | ((c >> 12) & 0x3F));
        *buf++ =  (0x80 | ((c >> 6) & 0x3F));
        *buf++ =  (0x80 | (c & 0x3F));
    }
    else
    {
        *buf++ =  (0xF8 | (c >> 24));
        *buf++ =  (0x80 | ((c >> 18) & 0x3F));
        *buf++ =  (0x80 | ((c >> 12) & 0x3F));
        *buf++ =  (0x80 | ((c >> 6) & 0x3F));
        *buf++ =  (0x80 | (c & 0x3F));
    }
#else
    int err, count = 0;
        
    err = EncodeCharToUTF8Bytes(c, (unsigned char *)buf, null, null, &count);
    if (err)
    {
#if 0
        extern FILE* errout; /* debug */

        tidy_out(errout, "pprint UTF-8 encoding error for U+%x : ", c); /* debug */
#endif
        /* replacement char 0xFFFD encoded as UTF-8 */
        buf[0] = (char) 0xEF;
        buf[1] = (char) 0xBF;
        buf[2] = (char) 0xBD;
        count = 3;
    }
    
    buf += count;
#endif

    return buf;
}


void FreePrintBuf(void)
{
    if (linebuf)
        MemFree(linebuf);

    linebuf = null;
    lbufsize = 0;
}

static void AddC(uint c, uint index)
{
    if (index + 1 >= lbufsize)
    {
        while (index + 1 >= lbufsize)
        {
            if (lbufsize == 0)
                lbufsize = 256;
            else
                lbufsize = lbufsize * 2;
        }

       linebuf = (uint *)MemRealloc(linebuf, lbufsize*sizeof(uint));
    }

    linebuf[index] = (uint)c;
}

static void WrapLine(Out *fout, uint indent)
{
    uint i, *p, *q;

    if (wraphere == 0)
        return;

    for (i = 0; i < indent; ++i)
        outc(' ', fout);

    for (i = 0; i < wraphere; ++i)
        outc(linebuf[i], fout);

    if (InString)
    {
        outc(' ', fout);
        outc('\\', fout);
    }

    outc('\n', fout);

    if (linelen > wraphere)
    {
        p = linebuf;

        if (linebuf[wraphere] == ' ')
            ++wraphere;

        q = linebuf + wraphere;
        AddC('\0', linelen);

        p = linebuf; q = linebuf + wraphere; /* 433856 - fix by Terry Teague 23 Jun 00 */
        
        while ((*p++ = *q++));

        linelen -= wraphere;
    }
    else
        linelen = 0;

    wraphere = 0;
}

static void WrapAttrVal(Out *fout, uint indent, Bool inString)
{
    uint i, *p, *q;

    for (i = 0; i < indent; ++i)
        outc(' ', fout);

    for (i = 0; i < wraphere; ++i)
        outc(linebuf[i], fout);

    outc(' ', fout);

    if (inString)
        outc('\\', fout);

    outc('\n', fout);

    if (linelen > wraphere)
    {
        p = linebuf;

        if (linebuf[wraphere] == ' ')
            ++wraphere;

        q = linebuf + wraphere;
        AddC('\0', linelen);

        p = linebuf; q = linebuf + wraphere; /* 433856 - fix by Terry Teague 23 Jun 00 */
        
        while ((*p++ = *q++));

        linelen -= wraphere;
    }
    else
        linelen = 0;

    wraphere = 0;
}

void PFlushLine(Out *fout, uint indent)
{
    uint i;

    if (linelen > 0)
    {
        if (indent + linelen >= wraplen)
            WrapLine(fout, indent);

        if (!InAttVal || IndentAttributes)
        {
            for (i = 0; i < indent; ++i)
                outc(' ', fout);
        }

        for (i = 0; i < linelen; ++i)
            outc(linebuf[i], fout);
    }

    outc('\n', fout);
    linelen = wraphere = 0;
    InAttVal = no;
}

void PCondFlushLine(Out *fout, uint indent)
{
    uint i;

    if (linelen > 0)
    {
        if (indent + linelen >= wraplen)
            WrapLine(fout, indent);

        if (!InAttVal || IndentAttributes)
        {
            for (i = 0; i < indent; ++i)
                outc(' ', fout);
        }

        for (i = 0; i < linelen; ++i)
            outc(linebuf[i], fout);

        outc('\n', fout);
        linelen = wraphere = 0;
        InAttVal = no;
    }
}

static void PPrintChar(uint c, uint mode)
{
    char *p, entity[128];
    Bool breakable=no; /* #431953 - RJ */

    if (c == ' ' && !(mode & (PREFORMATTED | COMMENT | ATTRIBVALUE | CDATA)))
    {
        /* coerce a space character to a non-breaking space */
        if (mode & NOWRAP)
        {
            /* by default XML doesn't define &nbsp; */
            if (NumEntities || XmlTags)
            {
                AddC('&', linelen++);
                AddC('#', linelen++);
                AddC('1', linelen++);
                AddC('6', linelen++);
                AddC('0', linelen++);
                AddC(';', linelen++);
            }
            else /* otherwise use named entity */
            {
                AddC('&', linelen++);
                AddC('n', linelen++);
                AddC('b', linelen++);
                AddC('s', linelen++);
                AddC('p', linelen++);
                AddC(';', linelen++);
            }
            return;
        }
        else
            wraphere = linelen;
    }

    /* comment characters are passed raw */
    if (mode & (COMMENT | CDATA))
    {
        AddC(c, linelen++);
        return;
    }

    /* except in CDATA map < to &lt; etc. */
    if (! (mode & CDATA) )
    {
        if (c == '<')
        {
            AddC('&', linelen++);
            AddC('l', linelen++);
            AddC('t', linelen++);
            AddC(';', linelen++);
            return;
        }
            
        if (c == '>')
        {
            AddC('&', linelen++);
            AddC('g', linelen++);
            AddC('t', linelen++);
            AddC(';', linelen++);
            return;
        }

        /*
          naked '&' chars can be left alone or
          quoted as &amp; The latter is required
          for XML where naked '&' are illegal.
        */
        if (c == '&' && QuoteAmpersand)
        {
            AddC('&', linelen++);
            AddC('a', linelen++);
            AddC('m', linelen++);
            AddC('p', linelen++);
            AddC(';', linelen++);
            return;
        }

        if (c == '"' && QuoteMarks)
        {
            AddC('&', linelen++);
            AddC('q', linelen++);
            AddC('u', linelen++);
            AddC('o', linelen++);
            AddC('t', linelen++);
            AddC(';', linelen++);
            return;
        }

        if (c == '\'' && QuoteMarks)
        {
            AddC('&', linelen++);
            AddC('#', linelen++);
            AddC('3', linelen++);
            AddC('9', linelen++);
            AddC(';', linelen++);
            return;
        }

        if (c == 160 && outCharEncoding != RAW)
        {
            if (MakeBare)
                AddC(' ', linelen++);
            else if (QuoteNbsp)
            {
                AddC('&', linelen++);

                if (NumEntities || XmlOut) /* #433604 - fix by Bjšrn Hšhrmann 15 Jun 01 */
                {
                    AddC('#', linelen++);
                    AddC('1', linelen++);
                    AddC('6', linelen++);
                    AddC('0', linelen++);
                }
                else
                {
                    AddC('n', linelen++);
                    AddC('b', linelen++);
                    AddC('s', linelen++);
                    AddC('p', linelen++);
                }

                AddC(';', linelen++);
            }
            else
                AddC(c, linelen++);

            return;
        }
    }

#if SUPPORT_ASIAN_ENCODINGS

    /* #431953 - start RJ */
    /* Handle encoding-specific issues */
    switch (outCharEncoding)
    {
    case UTF8:
    /* Chinese doesn't have spaces, so it needs other kinds of breaks */
    /* This will also help documents using nice Unicode punctuation */
    /* But we leave the ASCII range punctuation untouched */

    /* Break after any punctuation or spaces characters */
    if ((c >= 0x2000) && !(mode & PREFORMATTED))
    {
        if(((c >= 0x2000) && ( c<= 0x2006 ))
        || ((c >= 0x2008) && ( c<= 0x2010 ))
        || ((c >= 0x2011) && ( c<= 0x2046 ))
        || ((c >= 0x207D) && ( c<= 0x207E )) 
        || ((c >= 0x208D) && ( c<= 0x208E )) 
        || ((c >= 0x2329) && ( c<= 0x232A )) 
        || ((c >= 0x3001) && ( c<= 0x3003 )) 
        || ((c >= 0x3008) && ( c<= 0x3011 )) 
        || ((c >= 0x3014) && ( c<= 0x301F )) 
        || ((c >= 0xFD3E) && ( c<= 0xFD3F )) 
        || ((c >= 0xFE30) && ( c<= 0xFE44 )) 
        || ((c >= 0xFE49) && ( c<= 0xFE52 )) 
        || ((c >= 0xFE54) && ( c<= 0xFE61 )) 
        || ((c >= 0xFE6A) && ( c<= 0xFE6B )) 
        || ((c >= 0xFF01) && ( c<= 0xFF03 )) 
        || ((c >= 0xFF05) && ( c<= 0xFF0A )) 
        || ((c >= 0xFF0C) && ( c<= 0xFF0F )) 
        || ((c >= 0xFF1A) && ( c<= 0xFF1B )) 
        || ((c >= 0xFF1F) && ( c<= 0xFF20 )) 
        || ((c >= 0xFF3B) && ( c<= 0xFF3D )) 
        || ((c >= 0xFF61) && ( c<= 0xFF65 )))
        {
            wraphere = linelen + 2; /* 2, because AddChar is not till later */
            breakable = yes;
        } 
        else switch (c)
        {
            case 0xFE63:
            case 0xFE68:
            case 0x3030:
            case 0x30FB:
            case 0xFF3F:
            case 0xFF5B:
            case 0xFF5D:
                wraphere = linelen + 2;
                breakable = yes;
        }
        /* but break before a left punctuation */
        if (breakable == yes)
        { 
            if (((c >= 0x201A) && (c <= 0x201C)) ||
                ((c >= 0x201E) && (c <= 0x201F)))
            {
                wraphere--;
            }
            else switch (c)
            {
            case 0x2018:
            case 0x2039:
            case 0x2045:
            case 0x207D:
            case 0x208D:
            case 0x2329:
            case 0x3008:
            case 0x300A:
            case 0x300C:
            case 0x300E:
            case 0x3010:
            case 0x3014:
            case 0x3016:
            case 0x3018:
            case 0x301A:
            case 0x301D:
            case 0xFD3E:
            case 0xFE35:
            case 0xFE37:
            case 0xFE39:
            case 0xFE3B:
            case 0xFE3D:
            case 0xFE3F:
            case 0xFE41:
            case 0xFE43:
            case 0xFE59:
            case 0xFE5B:
            case 0xFE5D:
            case 0xFF08:
            case 0xFF3B:
            case 0xFF5B:
            case 0xFF62:
                wraphere--; 
            }
        }
    }
    break;

    case BIG5:
    /* Allow linebreak at Chinese punctuation characters */
    /* There are not many spaces in Chinese */
    AddC(c, linelen++);
    if (((c & 0xFF00) == 0xA100) & !(mode & PREFORMATTED))
    {
        wraphere = linelen;
        /* opening brackets have odd codes: break before them */
        if ((c > 0x5C) && (c < 0xAD) && ((c & 1) == 1)) 
            wraphere--; 
    }
    return;

    case SHIFTJIS:
    case ISO2022: /* ISO 2022 characters are passed raw */
    case RAW:
        AddC(c, linelen++);
        return;
    }
    /* #431953 - end RJ */

#else

    /* otherwise ISO 2022 characters are passed raw */
    if (outCharEncoding == ISO2022 || outCharEncoding == RAW)
    {
        AddC(c, linelen++);
        return;
    }

#endif

    /* if preformatted text, map &nbsp; to space */
    if (c == 160 && (mode & PREFORMATTED))
    {
        AddC(' ', linelen++);
        return;
    }

    /*
     Filters from Word and PowerPoint often use smart
     quotes resulting in character codes between 128
     and 159. Unfortunately, the corresponding HTML 4.0
     entities for these are not widely supported. The
     following converts dashes and quotation marks to
     the nearest ASCII equivalent. My thanks to
     Andrzej Novosiolov for his help with this code.
    */

    if ( (MakeClean && AsciiChars) || MakeBare )
    {
        if (c >= 0x2013 && c <= 0x201E)
        {
            switch (c) {
              case 0x2013: /* en dash */
              case 0x2014: /* em dash */
                c = '-';
                break;
              case 0x2018: /* left single  quotation mark */
              case 0x2019: /* right single quotation mark */
              case 0x201A: /* single low-9 quotation mark */
                c = '\'';
                break;
              case 0x201C: /* left double  quotation mark */
              case 0x201D: /* right double quotation mark */
              case 0x201E: /* double low-9 quotation mark */
                c = '"';
                break;
              }
        }
    }

    /* don't map latin-1 chars to entities */
    if (outCharEncoding == LATIN1)
    {
        if (c > 255)  /* multi byte chars */
        {
            if (!NumEntities && (p = EntityName(c)) != null)
                sprintf(entity, "&%s;", p);
            else
                sprintf(entity, "&#%u;", c);

            for (p = entity; *p; ++p)
                AddC(*p, linelen++);

            return;
        }

        if (c > 126 && c < 160)
        {
            sprintf(entity, "&#%d;", c);

            for (p = entity; *p; ++p)
                AddC(*p, linelen++);

            return;
        }

        AddC(c, linelen++);
        return;
    }

    /* don't map UTF-8 chars to entities */
    if (outCharEncoding == UTF8)
    {
        AddC(c, linelen++);
        return;
    }

#if SUPPORT_UTF16_ENCODINGS

    /* don't map UTF-16 chars to entities */
    if (outCharEncoding == UTF16 || outCharEncoding == UTF16LE || outCharEncoding == UTF16BE)
    {
        AddC(c, linelen++);
        return;
    }

#endif

    /* use numeric entities only  for XML */
    if (XmlTags)
    {
        /* if ASCII use numeric entities for chars > 127 */
        if (c > 127 && outCharEncoding == ASCII)
        {
            sprintf(entity, "&#%u;", c);

            for (p = entity; *p; ++p)
                AddC(*p, linelen++);

            return;
        }

        /* otherwise output char raw */
        AddC(c, linelen++);
        return;
    }

    /* default treatment for ASCII */
    if ((outCharEncoding == ASCII) && (c > 126 || (c < ' ' && c != '\t')))
    {
        if (!NumEntities && (p = EntityName(c)) != null)
            sprintf(entity, "&%s;", p);
        else
            sprintf(entity, "&#%u;", c);

        for (p = entity; *p; ++p)
            AddC(*p, linelen++);

        return;
    }

    AddC(c, linelen++);
}

/* 
  The line buffer is uint not char so we can
  hold Unicode values unencoded. The translation
  to UTF-8 is deferred to the outc routine called
  to flush the line buffer.
*/
static void PPrintText(Out *fout, uint mode, uint indent,
                Lexer *lexer, uint start, uint end)
{
    uint i, c;

    for (i = start; i < end; ++i)
    {
        if (indent + linelen >= wraplen)
            WrapLine(fout, indent);

        c = (unsigned char)lexer->lexbuf[i];

        /* look for UTF-8 multibyte character */
        if (c > 0x7F)
             i += GetUTF8((unsigned char *)lexer->lexbuf + i, &c);

        if (c == '\n')
        {
            PFlushLine(fout, indent);
            continue;
        }

        PPrintChar(c, mode);
    }
}

static void PPrintString(Out *fout, uint indent, char *str)
{
    while (*str != '\0')
        AddC(*str++, linelen++);
}

static void PPrintAttrValue(Out *fout, uint indent,
                            char *value, int delim, Bool wrappable)
{
    uint c;
    Bool wasinstring = no;

    int mode = (wrappable ? (NORMAL | ATTRIBVALUE) : (PREFORMATTED | ATTRIBVALUE));

    /* look for ASP, Tango or PHP instructions for computed attribute value */
    if (value && value[0] == '<')
    {
        if (value[1] == '%' || value[1] == '@'|| wstrncmp(value, "<?php", 5) == 0)
            mode |= CDATA;
    }

    if (delim == null)
        delim = '"';

    AddC('=', linelen++);

    /* don't wrap after "=" for xml documents */
    if (!XmlOut)
    {
        if (indent + linelen < wraplen)
            wraphere = linelen;

        if (indent + linelen >= wraplen)
            WrapLine(fout, indent);

        if (indent + linelen < wraplen)
            wraphere = linelen;
        else
            PCondFlushLine(fout, indent);
    }

    AddC(delim, linelen++);

    if (value)
    {
        InString = no;

        while (*value != '\0')
        {
            c = (unsigned char)*value;

            if (wrappable && c == ' ' && indent + linelen < wraplen)
            {
                wraphere = linelen;
                wasinstring = InString;
            }

            if (wrappable && wraphere > 0 && indent + linelen >= wraplen)
                WrapAttrVal(fout, indent, wasinstring);

            if (c == (uint)delim)
            {
                char *entity;

                entity = (c == '"' ? "&quot;" : "&#39;");

                while (*entity != '\0')
                    AddC(*entity++, linelen++);

                ++value;
                continue;
            }
            else if (c == '"')
            {
                if (QuoteMarks)
                {
                    AddC('&', linelen++);
                    AddC('q', linelen++);
                    AddC('u', linelen++);
                    AddC('o', linelen++);
                    AddC('t', linelen++);
                    AddC(';', linelen++);
                }
                else
                    AddC('"', linelen++);

                if (delim == '\'')
                    InString = (Bool)(!InString);

                ++value;
                continue;
            }
            else if (c == '\'')
            {
                if (QuoteMarks)
                {
                    AddC('&', linelen++);
                    AddC('#', linelen++);
                    AddC('3', linelen++);
                    AddC('9', linelen++);
                    AddC(';', linelen++);
                }
                else
                    AddC('\'', linelen++);

                if (delim == '"')
                    InString = (Bool)(!InString);

                ++value;
                continue;
            }

            /* look for UTF-8 multibyte character */
            if (c > 0x7F)
                 value += GetUTF8((unsigned char *)value, &c);

            ++value;

            if (c == '\n')
            {
                PFlushLine(fout, indent);
                continue;
            }

            PPrintChar(c, mode);
        }
    }

    InString = no;
    AddC(delim, linelen++);
}

static void PPrintAttribute(Out *fout, uint indent,
                            Node *node, AttVal *attr)
{
    char *name;
    Bool wrappable = no;

    if (IndentAttributes)
    {
        PFlushLine(fout, indent);
        indent += spaces;
    }

    name = attr->attribute;

    if (indent + linelen >= wraplen)
        WrapLine(fout, indent);

    if (!XmlTags && !XmlOut && attr->dict)
    {
        if (IsScript(name))
            wrappable = WrapScriptlets;
        else if (!attr->dict->nowrap && WrapAttVals)
            wrappable = yes;
    }

    if (indent + linelen < wraplen)
    {
        wraphere = linelen;
        AddC(' ', linelen++);
    }
    else
    {
        PCondFlushLine(fout, indent);
        AddC(' ', linelen++);
    }

    while (*name != '\0')
        AddC(FoldCase(*name++, UpperCaseAttrs), linelen++);

    if (indent + linelen >= wraplen)
        WrapLine(fout, indent);
 
    if (attr->value == null)
    {
        if (XmlTags || XmlOut)
            PPrintAttrValue(fout, indent,
              IsBool(attr->attribute) ? attr->attribute : "",
              attr->delim, yes);
        else if (!IsBoolAttribute(attr) && !IsNewNode(node))
            PPrintAttrValue(fout, indent, "", attr->delim, yes);
        else if (indent + linelen < wraplen)
            wraphere = linelen;

    }
    else
        PPrintAttrValue(fout, indent, attr->value, attr->delim, wrappable);
}

static void PPrintAttrs(Out *fout, uint indent,
                        Lexer *lexer, Node *node, AttVal *attr)
{
    Attribute *attribute;

    if (attr)
    {
        if (attr->next)
            PPrintAttrs(fout, indent, lexer, node, attr->next);

        if (attr->attribute != null)
        {
            attribute = attr->dict;

            if (!DropPropAttrs ||
                !(attribute == null ||
                    (attribute->versions & VERS_PROPRIETARY)))
            PPrintAttribute(fout, indent, node, attr);
        }
        else if (attr->asp != null)
        {
            AddC(' ', linelen++);
            PPrintAsp(fout, indent, lexer, attr->asp);
        }
        else if (attr->php != null)
        {
            AddC(' ', linelen++);
            PPrintPhp(fout, indent, lexer, attr->php);
        }
    }

    /* add xml:space attribute to pre and other elements */
    if (XmlOut == yes &&
            XmlSpace &&
            XMLPreserveWhiteSpace (node) &&
            !GetAttrByName(node, "xml:space"))
        PPrintString(fout, indent, " xml:space=\"preserve\"");
}

/*
 Line can be wrapped immediately after inline start tag provided
 if follows a text node ending in a space, or it parent is an
 inline element that that rule applies to. This behaviour was
 reverse engineered from Netscape 3.0
*/
static Bool AfterSpace(Lexer *lexer, Node *node)
{
    Node *prev;
    uint c;

    if (!node || !node->tag || !(node->tag->model & CM_INLINE))
        return yes;

    prev = node->prev;

    if (prev)
    {
        if (prev->type == TextNode && prev->end > prev->start)
        {
            c = (unsigned char)lexer->lexbuf[prev->end - 1];

            if (c == 160 || c == ' ' || c == '\n')
                return yes;
        }

        return no;
    }

    return AfterSpace(lexer, node->parent);
}

static void PPrintTag(Lexer *lexer, Out *fout,
                      uint mode, uint indent, Node *node)
{
    char c, *p;

    AddC('<', linelen++);

    if (node->type == EndTag)
        AddC('/', linelen++);

    for (p = node->element; (c = *p); ++p)
        AddC(FoldCase(c, UpperCaseTags), linelen++);

    PPrintAttrs(fout, indent, lexer, node, node->attributes);

    if ((XmlOut == yes || lexer->isvoyager) &&
            ((node->type == StartEndTag && !xHTML) || node->tag->model & CM_EMPTY))
    {
        AddC(' ', linelen++);   /* compatibility hack */
        AddC('/', linelen++);
    }

    AddC('>', linelen++);

    if ((node->type != StartEndTag || xHTML) && !(mode & PREFORMATTED))
    {
        if (indent + linelen >= wraplen)
            WrapLine(fout, indent);

        if (indent + linelen < wraplen)
        {
            /*
             wrap after start tag if is <br/> or if it's not
             inline or it is an empty tag followed by </a>
            */
            if (AfterSpace(lexer, node))
            {
                if (!(mode & NOWRAP) &&
                    (!(node->tag->model & CM_INLINE) ||
                      (node->tag == tag_br) ||
                      ((node->tag->model & CM_EMPTY) && 
                      node->next == null &&
                      node->parent->tag == tag_a)))
                {
                    wraphere = linelen;
                }
            }
        }
        else
            PCondFlushLine(fout, indent);
    }
}

static void PPrintEndTag(Out *fout, uint mode, uint indent, Node *node)
{
    char c, *p;

   /*
     Netscape ignores SGML standard by not ignoring a
     line break before </A> or </U> etc. To avoid rendering 
     this as an underlined space, I disable line wrapping
     before inline end tags by the #if 0 ... #endif
   */
#if 0
    if (indent + linelen < wraplen && !(mode & NOWRAP))
        wraphere = linelen;
#endif

    AddC('<', linelen++);
    AddC('/', linelen++);

    for (p = node->element; (c = *p); ++p)
        AddC(FoldCase(c, UpperCaseTags), linelen++);

    AddC('>', linelen++);
}

static void PPrintComment(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    if (HideComments)
        return;

    if (indent + linelen < wraplen)
        wraphere = linelen;

    AddC('<', linelen++);
    AddC('!', linelen++);
    AddC('-', linelen++);
    AddC('-', linelen++);
#if 0
    if (linelen < wraplen)
        wraphere = linelen;
#endif
    PPrintText(fout, COMMENT, indent,
                    lexer, node->start, node->end);
#if 0
    if (indent + linelen < wraplen)
        wraphere = linelen;
    AddC('-', linelen++);
    AddC('-', linelen++);
#endif
    AddC('>', linelen++);

    if (node->linebreak)
        PFlushLine(fout, indent);
}

static void PPrintDocType(Out *fout, uint indent,
                          Lexer *lexer, Node *node)
{
    uint i, c, mode = 0;
    Bool q = QuoteMarks;

    QuoteMarks = no;

    if (indent + linelen < wraplen)
        wraphere = linelen;

    PCondFlushLine(fout, indent);

    AddC('<', linelen++);
    AddC('!', linelen++);
    AddC('D', linelen++);
    AddC('O', linelen++);
    AddC('C', linelen++);
    AddC('T', linelen++);
    AddC('Y', linelen++);
    AddC('P', linelen++);
    AddC('E', linelen++);
    AddC(' ', linelen++);

    if (indent + linelen < wraplen)
        wraphere = linelen;

    for (i = node->start; i < node->end; ++i)
    {
        if (indent + linelen >= wraplen)
            WrapLine(fout, indent);

        c = (unsigned char)lexer->lexbuf[i];

        /* inDTDSubset? */
        if ( mode & CDATA ) {
            if (c == ']')
                mode &= ~CDATA;
        }
        else if (c == '[')
            mode |= CDATA;

        /* look for UTF-8 multibyte character */
        if (c > 0x7F)
             i += GetUTF8((unsigned char *)lexer->lexbuf + i, &c);

        if (c == '\n')
        {
            PFlushLine(fout, indent);
            continue;
        }

        PPrintChar(c, mode);
    }

    if (linelen < wraplen)
        wraphere = linelen;

    AddC('>', linelen++);
    QuoteMarks = q;
    PCondFlushLine(fout, indent);
}

static void PPrintPI(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    if (indent + linelen < wraplen)
        wraphere = linelen;

    AddC('<', linelen++);
    AddC('?', linelen++);

    /* set CDATA to pass < and > unescaped */
    PPrintText(fout, CDATA, indent,
                    lexer, node->start, node->end);

    if (lexer->lexbuf[node->end - 1] != '?')
        AddC('?', linelen++);

    AddC('>', linelen++);

    PCondFlushLine(fout, indent);
}

static void PPrintXmlDecl(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    if (indent + linelen < wraplen)
        wraphere = linelen;

    AddC('<', linelen++);
    AddC('?', linelen++);
    AddC('x', linelen++);
    AddC('m', linelen++);
    AddC('l', linelen++);

    PPrintAttrs(fout, indent, lexer, node, node->attributes);

    if (lexer->lexbuf[node->end - 1] != '?')
        AddC('?', linelen++);

    AddC('>', linelen++);

    PCondFlushLine(fout, indent);
}

/* note ASP and JSTE share <% ... %> syntax */
static void PPrintAsp(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    int savewraplen = wraplen;

    /* disable wrapping if so requested */

    if (!WrapAsp || !WrapJste)
        wraplen = 0xFFFFFF;  /* a very large number */

#if 0
    if (indent + linelen < wraplen)
        wraphere = linelen;
#endif
    AddC('<', linelen++);
    AddC('%', linelen++);

    PPrintText(fout, (WrapAsp ? CDATA : COMMENT), indent,
                    lexer, node->start, node->end);

    AddC('%', linelen++);
    AddC('>', linelen++);
    /* PCondFlushLine(fout, indent); */
    wraplen = savewraplen;
}

/* JSTE also supports <# ... #> syntax */
static void PPrintJste(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    int savewraplen = wraplen;

    /* disable wrapping if so requested */

    if (!WrapAsp)
        wraplen = 0xFFFFFF;  /* a very large number */

    AddC('<', linelen++);
    AddC('#', linelen++);

    PPrintText(fout, (WrapJste ? CDATA : COMMENT), indent,
                    lexer, node->start, node->end);

    AddC('#', linelen++);
    AddC('>', linelen++);
    /* PCondFlushLine(fout, indent); */
    wraplen = savewraplen;
}

/* PHP is based on XML processing instructions */
static void PPrintPhp(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    int savewraplen = wraplen;

    /* disable wrapping if so requested */

    if (!WrapPhp)
        wraplen = 0xFFFFFF;  /* a very large number */

#if 0
    if (indent + linelen < wraplen)
        wraphere = linelen;
#endif
    AddC('<', linelen++);
    AddC('?', linelen++);

    PPrintText(fout, (WrapPhp ? CDATA : COMMENT), indent,
                    lexer, node->start, node->end);

    AddC('?', linelen++);
    AddC('>', linelen++);
    /* PCondFlushLine(fout, indent); */
    wraplen = savewraplen;
}

static void PPrintCDATA(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    int savewraplen = wraplen;

    if (!IndentCdata)
        indent = 0;

    PCondFlushLine(fout, indent);

    /* disable wrapping */

    wraplen = 0xFFFFFF;  /* a very large number */

    AddC('<', linelen++);
    AddC('!', linelen++);
    AddC('[', linelen++);
    AddC('C', linelen++);
    AddC('D', linelen++);
    AddC('A', linelen++);
    AddC('T', linelen++);
    AddC('A', linelen++);
    AddC('[', linelen++);

    PPrintText(fout, COMMENT, indent,
                    lexer, node->start, node->end);

    AddC(']', linelen++);
    AddC(']', linelen++);
    AddC('>', linelen++);
    PCondFlushLine(fout, indent);
    wraplen = savewraplen;
}

static void PPrintSection(Out *fout, uint indent,
                   Lexer *lexer, Node *node)
{
    int savewraplen = wraplen;

    /* disable wrapping if so requested */

    if (!WrapSection)
        wraplen = 0xFFFFFF;  /* a very large number */

#if 0
    if (indent + linelen < wraplen)
        wraphere = linelen;
#endif
    AddC('<', linelen++);
    AddC('!', linelen++);
    AddC('[', linelen++);

    PPrintText(fout, (WrapSection ? CDATA : COMMENT), indent,
                    lexer, node->start, node->end);

    AddC(']', linelen++);
    AddC('>', linelen++);
    /* PCondFlushLine(fout, indent); */
    wraplen = savewraplen;
}


#if 0
/*
** Print script and style elements. For XHTML, wrap the content as follows:
**     JavaScript:
**         //<![CDATA[
**             content
**         //]]>
**     VBScript:
**         '<![CDATA[
**             content
**         ']]>
**     CSS:
**         /*<![CDATA[*/
**             content
**         /*]]>*/
**     other:
**         <![CDATA[
**             content
**         ]]>
*/
#endif

static char* CDATA_START           = "<![CDATA[";
static char* CDATA_END             = "]]>";
static char* JS_COMMENT_START      = "//";
static char* JS_COMMENT_END        = "";
static char* VB_COMMENT_START      = "\'";
static char* VB_COMMENT_END        = "";
static char* CSS_COMMENT_START     = "/*";
static char* CSS_COMMENT_END       = "*/";
static char* DEFAULT_COMMENT_START = "";
static char* DEFAULT_COMMENT_END   = "";


static Bool HasCDATA( Lexer* lexer, Node* node )
{
    /* Scan forward through the textarray. Since the characters we're
    ** looking for are < 0x7f, we don't have to do any UTF-8 decoding.
    */
    char* start = lexer->lexbuf + node->start;
    int len = node->end - node->start + 1;

    if ( node->type != TextNode )
        return no;

    return wsubstrn( start, len, CDATA_START );
}


#if 0 
static Bool StartsWithCDATA( Lexer* lexer, Node* node, char* commentStart )
{
    /* Scan forward through the textarray. Since the characters we're
    ** looking for are < 0x7f, we don't have to do any UTF-8 decoding.
    */
    int i = node->start, j, end = node->end;

    if ( node->type != TextNode )
        return no;

    /* Skip whitespace. */
    while ( i < end && lexer->lexbuf[i] <= ' ' )
        ++i;

    /* Check for starting comment delimiter. */
    for ( j = 0; j < wstrlen(commentStart); ++j )
    {
        if ( i >= end || lexer->lexbuf[i] != commentStart[j] )
            return no;
        ++i;
    }

    /* Skip whitespace. */
    while ( i < end && lexer->lexbuf[i] <= ' ' )
        ++i;

    /* Check for "<![CDATA[". */
    for ( j = 0; j < wstrlen(CDATA_START); ++j )
    {
        if (i >= end || lexer->lexbuf[i] != CDATA_START[j])
            return no;
        ++i;
    }

    return yes;
}


static Bool EndsWithCDATA( Lexer* lexer, Node* node, 
                           char* commentStart, char* commentEnd )
{
    /* Scan backward through the buff. Since the characters we're
    ** looking for are < 0x7f, we don't have do any UTF-8 decoding. Note
    ** that this is true even though we are scanning backwards because every
    ** byte of a UTF-8 multibyte character is >= 0x80.
    */

    int i = node->end - 1, j, start = node->start;

    if ( node->type != TextNode )
        return no;

    /* Skip whitespace. */
    while ( i >= start && (lexer->lexbuf[i] & 0xff) <= ' ' )
        --i;

    /* Check for ending comment delimiter. */
    for ( j = wstrlen(commentEnd) - 1; j >= 0; --j )
    {
        if (i < start || lexer->lexbuf[i] != commentEnd[j])
            return no;
        --i;
    }

    /* Skip whitespace. */
    while (i >= start && (lexer->lexbuf[i] & 0xff) <= ' ')
        --i;

    /* Check for "]]>". */
    for (j = wstrlen(CDATA_END) - 1; j >= 0; j--)
    {
        if (i < start || lexer->lexbuf[i] != CDATA_END[j])
            return no;
        --i;
    }

    /* Skip whitespace. */
    while (i >= start && lexer->lexbuf[i] <= ' ')
        --i;

    /* Check for starting comment delimiter. */
    for ( j = wstrlen(commentStart) - 1; j >= 0; --j )
    {
        if ( i < start || lexer->lexbuf[i] != commentStart[j] )
            return no;
        --i;
    }

    return yes;
}
#endif /* 0 */

void PPrintScriptStyle( Out* fout, uint mode, uint indent,
                        Lexer* lexer, Node* node )
{
    Node* content;
    char* commentStart = DEFAULT_COMMENT_START;
    char* commentEnd = DEFAULT_COMMENT_END;
    Bool  hasCData = no;

    PCondFlushLine(fout, indent);

    indent = 0;
    PPrintTag(lexer, fout, mode, indent, node);
    PFlushLine(fout, indent);

    if (xHTML && node->content != null)
    {
        AttVal* type = GetAttrByName(node, "type");
        if (type != null)
        {
            if (wstrcasecmp(type->value, "text/javascript") == 0)
            {
                commentStart = JS_COMMENT_START;
                commentEnd = JS_COMMENT_END;
            }
            else if (wstrcasecmp(type->value, "text/css") == 0)
            {
                commentStart = CSS_COMMENT_START;
                commentEnd = CSS_COMMENT_END;
            }
            else if (wstrcasecmp(type->value, "text/vbscript") == 0)
            {
                commentStart = VB_COMMENT_START;
                commentEnd = VB_COMMENT_END;
            }
        }

        hasCData = HasCDATA( lexer, node->content );
        if ( ! hasCData )
        {
            /* disable wrapping */
            uint savewraplen = wraplen;
            wraplen = 0xFFFFFF;  /* a very large number */

            AddAsciiString( commentStart, linelen );
            AddAsciiString( CDATA_START,  linelen );
            AddAsciiString( commentEnd,   linelen );
            PCondFlushLine( fout, indent );

            /* restore wrapping */
            wraplen = savewraplen;
        }
    }

    for ( content = node->content;
          content != null;
          content = content->next )
    {
        PPrintTree( fout, (mode | PREFORMATTED | NOWRAP |CDATA), 
                    indent, lexer, content );
    }

    PCondFlushLine(fout, indent);

    if (xHTML && node->content != null)
    {
        if ( ! hasCData )
        {
            /* disable wrapping */
            uint savewraplen = wraplen;
            wraplen = 0xFFFFFF;  /* a very large number */

            AddAsciiString( commentStart, linelen );
            AddAsciiString( CDATA_END,    linelen );
            AddAsciiString( commentEnd,   linelen );

            /* restore wrapping */
            wraplen = savewraplen;
            PCondFlushLine(fout, indent);
        }
    }

    PPrintEndTag(fout, mode, indent, node);
    PFlushLine(fout, indent);

    if (IndentContent == no && node->next != null)
        PFlushLine(fout, indent);
}



static Bool ShouldIndent(Node *node)
{
    if (IndentContent == no)
        return no;

    if (SmartIndent)
    {
        if (node->content && (node->tag->model & CM_NO_INDENT))
        {
            for (node = node->content; node; node = node->next)
                if (node->tag && node->tag->model & CM_BLOCK)
                    return yes;

            return no;
        }

        if (node->tag->model & CM_HEADING)
            return no;

        if (node->tag == tag_p)
            return no;

        if (node->tag == tag_title)
            return no;
    }

    if (node->tag->model & (CM_FIELD | CM_OBJECT))
        return yes;

    if (node->tag == tag_map)
        return yes;

    return (Bool)(!(node->tag->model & CM_INLINE));
}

/*
 Feature request #434940 - fix by Dave Raggett/Ignacio Vazquez-Abrams 21 Jun 01
 print just the content of the body element.
 useful when you want to reuse material from
 other documents.

 -- Sebastiano Vigna <vigna@dsi.unimi.it>
*/
void PrintBody(Out *fout, Lexer *lexer, Node *root)
{
    Node *content, *body = FindBody(root);

    if (body != null)
    {
        for (content = body->content;
            content != null;
            content = content->next)
            PPrintTree(fout, null, 0, lexer, content);
    }
}

void PPrintTree(Out *fout, uint mode, uint indent,
                    Lexer *lexer, Node *node)
{
    Node *content, *last;

    if (node == null)
        return;

    if (node->type == TextNode ||
        (node->type == CDATATag && EscapeCdata))
        PPrintText(fout, mode, indent,
                    lexer, node->start, node->end);
    else if (node->type == CommentTag)
    {
        PPrintComment(fout, indent, lexer, node);
    }
    else if (node->type == RootNode)
    {
        for (content = node->content;
                content != null;
                content = content->next)
           PPrintTree(fout, mode, indent, lexer, content);
    }
    else if (node->type == DocTypeTag)
        PPrintDocType(fout, indent, lexer, node);
    else if (node->type == ProcInsTag)
        PPrintPI(fout, indent, lexer, node);
    else if (node->type == XmlDecl)
        PPrintXmlDecl(fout, indent, lexer, node);
    else if (node->type == CDATATag)
        PPrintCDATA(fout, indent, lexer, node);
    else if (node->type == SectionTag)
        PPrintSection(fout, indent, lexer, node);
    else if (node->type == AspTag)
        PPrintAsp(fout, indent, lexer, node);
    else if (node->type == JsteTag)
        PPrintJste(fout, indent, lexer, node);
    else if (node->type == PhpTag)
        PPrintPhp(fout, indent, lexer, node);
    else if (node->tag->model & CM_EMPTY || (node->type == StartEndTag && !xHTML))
    {
        if (!(node->tag->model & CM_INLINE))
            PCondFlushLine(fout, indent);

        if (node->tag == tag_br && node->prev && node->prev->tag != tag_br && BreakBeforeBR)
            PFlushLine(fout, indent);

        if (MakeClean && node->tag == tag_wbr)
            PPrintString(fout, indent, " ");
        else
            PPrintTag(lexer, fout, mode, indent, node);

        if (node->tag == tag_param || node->tag == tag_area)
            PCondFlushLine(fout, indent);
        else if (node->tag == tag_br || node->tag == tag_hr)
            PFlushLine(fout, indent);
    }
    else /* some kind of container element */
    {
        if (node->tag && node->tag->parser == ParsePre)
        {
            PCondFlushLine(fout, indent);

            indent = 0;
            PCondFlushLine(fout, indent);
            PPrintTag(lexer, fout, mode, indent, node);
            PFlushLine(fout, indent);

            for (content = node->content;
                    content != null;
                    content = content->next)
                PPrintTree(fout, (mode | PREFORMATTED | NOWRAP), indent, lexer, content);

            PCondFlushLine(fout, indent);
            PPrintEndTag(fout, mode, indent, node);
            PFlushLine(fout, indent);

            if (IndentContent == no && node->next != null)
                PFlushLine(fout, indent);
        }
        else if (node->tag == tag_style || node->tag == tag_script)
        {
            PPrintScriptStyle( fout, (mode | PREFORMATTED | NOWRAP |CDATA),
                               indent, lexer, node );
        }
        else if (node->tag->model & CM_INLINE)
        {
            if (MakeClean)
            {
                /* discards <font> and </font> tags */
                if (node->tag == tag_font)
                {
                    for (content = node->content;
                            content != null;
                            content = content->next)
                        PPrintTree(fout, mode, indent, lexer, content);
                    return;
                }

                /* replace <nobr>...</nobr> by &nbsp; or &#160; etc. */
                if (node->tag == tag_nobr)
                {
                    for (content = node->content;
                            content != null;
                            content = content->next)
                        PPrintTree(fout, mode|NOWRAP, indent, lexer, content);
                    return;
                }
            }

            /* otherwise a normal inline element */

            PPrintTag(lexer, fout, mode, indent, node);

            /* indent content for SELECT, TEXTAREA, MAP, OBJECT and APPLET */

            if (ShouldIndent(node))
            {
                PCondFlushLine(fout, indent);
                indent += spaces;

                for (content = node->content;
                        content != null;
                        content = content->next)
                    PPrintTree(fout, mode, indent, lexer, content);

                PCondFlushLine(fout, indent);
                indent -= spaces;
                PCondFlushLine(fout, indent);
            }
            else
            {

                for (content = node->content;
                        content != null;
                        content = content->next)
                    PPrintTree(fout, mode, indent, lexer, content);
            }

            PPrintEndTag(fout, mode, indent, node);
        }
        else /* other tags */
        {
            PCondFlushLine(fout, indent);

            if (SmartIndent && node->prev != null)
                PFlushLine(fout, indent);

            /* do not omit elements with attributes */
            if (HideEndTags == no ||
                !(node->tag && (node->tag->model & CM_OMITST)) ||
                node->attributes != null)
            {
                PPrintTag(lexer, fout, mode, indent, node);

                if (ShouldIndent(node))
                    PCondFlushLine(fout, indent);
                else if (node->tag->model & CM_HTML || node->tag == tag_noframes ||
                            (node->tag->model & CM_HEAD && !(node->tag == tag_title)))
                    PFlushLine(fout, indent);
            }

            if (node->tag == tag_body && BurstSlides)
                PPrintSlide(fout, mode, (IndentContent ? indent+spaces : indent), lexer);
            else
            {
                last = null;

                for (content = node->content;
                        content != null; content = content->next)
                {
                    /* kludge for naked text before block level tag */
                    if (last && !IndentContent && last->type == TextNode &&
                        content->tag && content->tag->model & CM_BLOCK)
                    {
                        PFlushLine(fout, indent);
                        PFlushLine(fout, indent);
                    }

                    PPrintTree(fout, mode,
                        (ShouldIndent(node) ? indent+spaces : indent), lexer, content);

                    last = content;
                }
            }

            /* don't flush line for td and th */
            if (ShouldIndent(node) ||
                ((node->tag->model & CM_HTML || node->tag == tag_noframes ||
                    (node->tag->model & CM_HEAD && !(node->tag == tag_title)))
                    && HideEndTags == no))
            {
                PCondFlushLine(fout, (IndentContent ? indent+spaces : indent));

                if (HideEndTags == no || !(node->tag->model & CM_OPT))
                {
                    PPrintEndTag(fout, mode, indent, node);
                    PFlushLine(fout, indent);
                }
            }
            else
            {
                if (HideEndTags == no || !(node->tag->model & CM_OPT))
                    PPrintEndTag(fout, mode, indent, node);

                PFlushLine(fout, indent);
            }

            if (IndentContent == no &&
                node->next != null &&
                HideEndTags == no &&
                (node->tag->model & (CM_BLOCK|CM_LIST|CM_DEFLIST|CM_TABLE)))
            {
                PFlushLine(fout, indent);
            }
        }
    }
}

void PPrintXMLTree(Out *fout, uint mode, uint indent,
                    Lexer *lexer, Node *node)
{
    if (node == null)
        return;

    if (node->type == TextNode  ||
        (node->type == CDATATag && EscapeCdata))
    {
        PPrintText(fout, mode, indent,
                    lexer, node->start, node->end);
    }
    else if (node->type == CommentTag)
    {
        PCondFlushLine(fout, indent);
        PPrintComment(fout, 0, lexer, node);
        PCondFlushLine(fout, 0);
    }
    else if (node->type == RootNode)
    {
        Node *content;

        for (content = node->content;
                content != null;
                content = content->next)
           PPrintXMLTree(fout, mode, indent, lexer, content);
    }
    else if (node->type == DocTypeTag)
        PPrintDocType(fout, indent, lexer, node);
    else if (node->type == ProcInsTag)
        PPrintPI(fout, indent, lexer, node);
    else if (node->type == XmlDecl)
        PPrintXmlDecl(fout, indent, lexer, node);
    else if (node->type == CDATATag)
        PPrintCDATA(fout, indent, lexer, node);
    else if (node->type == SectionTag)
        PPrintSection(fout, indent, lexer, node);
    else if (node->type == AspTag)
        PPrintAsp(fout, indent, lexer, node);
    else if (node->type == JsteTag)
        PPrintJste(fout, indent, lexer, node);
    else if (node->type == PhpTag)
        PPrintPhp(fout, indent, lexer, node);
    else if (node->tag->model & CM_EMPTY || (node->type == StartEndTag && !xHTML))
    {
        PCondFlushLine(fout, indent);
        PPrintTag(lexer, fout, mode, indent, node);
        PFlushLine(fout, indent);

        if (node->next)
            PFlushLine(fout, indent);
    }
    else /* some kind of container element */
    {
        Node *content;
        Bool mixed = no;
        int cindent;

        for (content = node->content; content; content = content->next)
        {
            if (content->type == TextNode)
            {
                mixed = yes;
                break;
            }
        }

        PCondFlushLine(fout, indent);

        if (XMLPreserveWhiteSpace(node))
        {
            indent = 0;
            cindent = 0;
            mixed = no;
        }
        else if (mixed)
            cindent = indent;
        else
            cindent = indent + spaces;

        PPrintTag(lexer, fout, mode, indent, node);

        if (!mixed)
            PFlushLine(fout, indent);
 
        for (content = node->content;
                content != null;
                content = content->next)
            PPrintXMLTree(fout, mode, cindent, lexer, content);

        if (!mixed)
            PCondFlushLine(fout, cindent);

        PPrintEndTag(fout, mode, indent, node);
        PCondFlushLine(fout, indent);

        if (node->next)
            PFlushLine(fout, indent);
    }
}

/* split parse tree by h2 elements and output to separate files */

/* counts number of h2 children (if any) belonging to node */
int CountSlides(Node *node)
{
    int n = 1; /* assume minimum of 1 slide */

    /* #431716 - fix by Terry Teague 01 Jul 01 */
    if (node && node->content && node->content->tag == tag_h2)
        n--; /* "first" slide is empty, so ignore it */

    for (node = node->content; node; node = node->next)
        if (node->tag == tag_h2)
            ++n;

    return n;
}

/*
   inserts a space gif called "dot.gif" to ensure
   that the  slide is at least n pixels high
 */
static void PrintVertSpacer(Out *fout, uint indent)
{
    PCondFlushLine(fout, indent);
    PPrintString(fout, indent , 
    "<img width=\"0\" height=\"0\" hspace=\"1\" src=\"dot.gif\" vspace=\"%d\" align=\"left\">");
    PCondFlushLine(fout, indent);
}

static void PrintNavBar(Out *fout, uint indent)
{
    char buf[128];

    PCondFlushLine(fout, indent);
    PPrintString(fout, indent , "<center><small>");

    if (slide > 1)
    {
        sprintf(buf, "<a href=\"slide%03d.html\">previous</a> | ", slide-1); /* #427666 - fix by Eric Rossen 02 Aug 00 */
        PPrintString(fout, indent , buf);
        PCondFlushLine(fout, indent);

        if (slide < count)
            PPrintString(fout, indent , "<a href=\"slide001.html\">start</a> | "); /* #427666 - fix by Eric Rossen 02 Aug 00 */
        else
            PPrintString(fout, indent , "<a href=\"slide001.html\">start</a>"); /* #427666 - fix by Eric Rossen 02 Aug 00 */

        PCondFlushLine(fout, indent);
    }

    if (slide < count)
    {
        sprintf(buf, "<a href=\"slide%03d.html\">next</a>", slide+1); /* #427666 - fix by Eric Rossen 02 Aug 00 */
        PPrintString(fout, indent , buf);
    }

    PPrintString(fout, indent , "</small></center>");
    PCondFlushLine(fout, indent);
}

/*
  Called from PPrintTree to print the content of a slide from
  the node slidecontent. On return slidecontent points to the
  node starting the next slide or null. The variables slide
  and count are used to customise the navigation bar.
*/
void PPrintSlide(Out *fout, uint mode, uint indent, Lexer *lexer)
{
    Node *content, *last;
    char buf[256];

    /* insert div for onclick handler */
    sprintf(buf, "<div onclick=\"document.location='slide%03d.html'\">", /* #427666 - fix by Eric Rossen 02 Aug 00 */
                    (slide < count ? slide + 1 : 1));
    PPrintString(fout, indent, buf);
    PCondFlushLine(fout, indent);

    /* first print the h2 element and navbar */
    if (slidecontent && slidecontent->tag == tag_h2) /* #431716 - fix by Terry Teague 01 Jul 01 */
    {
        PrintNavBar(fout, indent);

        /* now print an hr after h2 */

        AddC('<', linelen++);


        AddC(FoldCase('h', UpperCaseTags), linelen++);
        AddC(FoldCase('r', UpperCaseTags), linelen++);

        if (XmlOut == yes)
            PPrintString(fout, indent , " />");
        else
            AddC('>', linelen++);


        if (IndentContent == yes)
            PCondFlushLine(fout, indent);

        /* PrintVertSpacer(fout, indent); */

        /*PCondFlushLine(fout, indent); */

        /* print the h2 element */
        PPrintTree(fout, mode,
            (IndentContent ? indent+spaces : indent), lexer, slidecontent);

        slidecontent = slidecontent->next;
    }
    
    /* now continue until we reach the next h2 (or end) */

    last = null;
    content = slidecontent;

    for (; content != null; content = content->next)
    {
        if (content->tag == tag_h2)
            break;

        /* kludge for naked text before block level tag */
        if (last && !IndentContent && last->type == TextNode &&
            content->tag && content->tag->model & CM_BLOCK)
        {
            PFlushLine(fout, indent);
            PFlushLine(fout, indent);
        }

        PPrintTree(fout, mode,
            (IndentContent ? indent+spaces : indent), lexer, content);

        last = content;
    }

    slidecontent = content;

    /* now print epilog */

    PCondFlushLine(fout, indent);

    PPrintString(fout, indent , "<br clear=\"all\">");
    PCondFlushLine(fout, indent);

    AddC('<', linelen++);


    AddC(FoldCase('h', UpperCaseTags), linelen++);
    AddC(FoldCase('r', UpperCaseTags), linelen++);

    if (XmlOut == yes)
        PPrintString(fout, indent , " />");
    else
        AddC('>', linelen++);


    if (IndentContent == yes)
        PCondFlushLine(fout, indent);

    PrintNavBar(fout, indent);

    /* end tag for div */
    PPrintString(fout, indent, "</div>");
    PCondFlushLine(fout, indent);
}

/*
Add meta element for page transition effect, this works on IE but not NS
*/

void AddTransitionEffect(Lexer *lexer, Node *root, int effect, float duration)
{
    Node *head = FindHEAD(root);
    char transition[128];

    if (0 <= effect && effect <= 23)
        sprintf(transition, "revealTrans(Duration=%g,Transition=%d)", duration, effect);
    else
        sprintf(transition, "blendTrans(Duration=%g)", duration);

    if (head)
    {
        Node *meta = InferredTag(lexer, "meta");
        AddAttribute(meta, "http-equiv", "Page-Enter");
        AddAttribute(meta, "content", transition);
        InsertNodeAtStart(head, meta);
    }
}

void CreateSlides(Lexer *lexer, Node *root)
{
    Node *body;
    char buf[128];
    Out out;
    FILE *fp;

    body = FindBody(root);
    count = CountSlides(body);
    slidecontent = body->content;
    AddTransitionEffect(lexer, root, EFFECT_BLEND, 3.0);

    for (slide = 1; slide <= count; ++slide)
    {
        sprintf(buf, "slide%03d.html", slide); /* #427666 - fix by Eric Rossen 02 Aug 00 */
        out.state = FSM_ASCII;
        out.encoding = outCharEncoding;

        if ((fp = fopen(buf, "w")))
        {
            out.fp = fp;
            PPrintTree(&out, null, 0, lexer, root);
            PFlushLine(&out, 0);
            fclose(fp);
        }
    }

    /*
     delete superfluous slides by deleting slideN.html
     for N = count+1, count+2, etc. until no such file
     is found.     
    */

    for (;;)
    {
        sprintf(buf, "slide%03d.html", slide); /* #427666 - fix by Eric Rossen 02 Aug 00 */

        if (unlink(buf) != 0)
            break;

        ++slide;
    }
}
