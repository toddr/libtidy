/*
  config.c - read config file and manage config properties
  
  (c) 1998-2001 (W3C) MIT, INRIA, Keio University
  See tidy.c for the copyright notice.

  CVS Info :

    $Author: terry_teague $ 
    $Date: 2001/12/08 07:51:32 $ 
    $Revision: 1.34 $ 

*/

/*
  config files associate a property name with a value.

  // comments can start at the beginning of a line
  # comments can start at the beginning of a line
  name: short values fit onto one line
  name: a really long value that
   continues on the next line

  property names are case insensitive and should be less than
  60 characters in length and must start at the begining of
  the line, as whitespace at the start of a line signifies a
  line continuation.
*/

#include "platform.h"
#include "html.h"

typedef union
{
    int *number;
    Bool *logical;
    char **string;
} Location;

typedef void (ParseProperty)(Location location, char *option);

ParseProperty ParseInt;     /* parser for integer values */
ParseProperty ParseBool;    /* parser for 't'/'f', 'true'/'false', 'y'/'n', 'yes'/'no' or '1'/'0' */
ParseProperty ParseInvBool; /* parser for 't'/'f', 'true'/'false', 'y'/'n', 'yes'/'no' or '1'/'0' */
ParseProperty ParseName;    /* a string excluding whitespace */
ParseProperty ParseString;  /* a string including whitespace */
ParseProperty ParseTagNames; /* a space or comma separated list of tag names */
/* RAW, ASCII, LATIN1, UTF8, ISO2022, MACROMAN, UTF16LE, UTF16BE, UTF16, WIN1252, BIG5, SHIFTJIS */
ParseProperty ParseCharEncoding;
ParseProperty ParseIndent;  /* specific to the indent option - Bool and 'auto' */
ParseProperty ParseDocType; /* omit | auto | strict | loose | <fpi> */
ParseProperty ParseRepeatedAttribute; /* keep-first or keep-last? */
ParseProperty ParseBOM;     /* specific to the output-bom option - Bool and 'auto' */

uint spaces =  2;           /* default indentation */
uint wraplen = 68;          /* default wrap margin */
int tabsize = 4;

int CharEncoding = ASCII;
int inCharEncoding = LATIN1;
int outCharEncoding = ASCII;

DocTypeMode doctype_mode = doctype_auto; /* see doctype property */
DupAttrMode DuplicateAttrs = keep_last; /* Keep first or last duplicate attribute */

char *alt_text = null;      /* default text for alt attribute */
char *slide_style = null;   /* style sheet for slides: not used for anything yet */
char *Language = null;      /* #431953 - RJ language property: not used for anything yet */
char *doctype_str = null;   /* user specified doctype */
char *errfile = null;       /* file name to write errors to */
Bool writeback = no;        /* if true then output tidied markup */

Bool OnlyErrors = no;       /* if true normal output is suppressed */
Bool ShowWarnings = yes;    /* however errors are always shown */
Bool Quiet = no;            /* no 'Parsing X', guessed DTD or summary */
Bool IndentContent = no;    /* indent content of appropriate tags */
Bool SmartIndent = no;      /* does text/block level content effect indentation */
Bool HideEndTags = no;      /* suppress optional end tags */
Bool XmlTags = no;          /* treat input as XML */
Bool XmlOut = no;           /* create output as XML */
Bool xHTML = no;            /* output extensible HTML */
Bool HtmlOut = no;          /* output plain-old HTML, even for XHTML input. Yes means set explicitly. */
Bool XmlPi = no;            /* add <?xml?> for XML docs */
Bool RawOut = no;           /* avoid mapping values > 127 to entities: not used for anything yet */
Bool UpperCaseTags = no;    /* output tags in upper not lower case */
Bool UpperCaseAttrs = no;   /* output attributes in upper not lower case */
Bool MakeBare = no;         /* Make bare HTML: remove Microsoft cruft */
Bool MakeClean = no;        /* replace presentational clutter by style rules */
Bool LogicalEmphasis = no;  /* replace i by em and b by strong */
Bool DropPropAttrs = no;    /* discard proprietary attributes */
Bool DropFontTags = no;     /* discard presentation tags */
Bool DropEmptyParas = yes;  /* discard empty p elements */
Bool FixComments = yes;     /* fix comments with adjacent hyphens */
Bool BreakBeforeBR = no;    /* o/p newline before <br> or not? */
Bool BurstSlides = no;      /* create slides on each h2 element */
Bool NumEntities = no;      /* use numeric entities */
Bool QuoteMarks = no;       /* output " marks as &quot; */
Bool QuoteNbsp = yes;       /* output non-breaking space as entity */
Bool QuoteAmpersand = yes;  /* output naked ampersand as &amp; */
Bool WrapAttVals = no;      /* wrap within attribute values */
Bool WrapScriptlets = no;   /* wrap within JavaScript string literals */
Bool WrapSection = yes;     /* wrap within <![ ... ]> section tags */
Bool WrapAsp = yes;         /* wrap within ASP pseudo elements */
Bool WrapJste = yes;        /* wrap within JSTE pseudo elements */
Bool WrapPhp = yes;         /* wrap within PHP pseudo elements */
Bool FixBackslash = yes;    /* fix URLs by replacing \ with / */
Bool IndentAttributes = no; /* newline+indent before each attribute */
Bool XmlPIs = no;           /* if set to yes PIs must end with ?> */
Bool XmlSpace = no;         /* if set to yes adds xml:space attr as needed */
Bool EncloseBodyText = no;  /* if yes text at body is wrapped in <p>'s */
Bool EncloseBlockText = no; /* if yes text in blocks is wrapped in <p>'s */
Bool KeepFileTimes = yes;   /* if yes last modied time is preserved */
Bool Word2000 = no;         /* draconian cleaning for Word2000 */
Bool TidyMark = yes;        /* add meta element indicating tidied doc */
Bool Emacs = no;            /* if true format error output for GNU Emacs */
Bool LiteralAttribs = no;   /* if true attributes may use newlines */
Bool BodyOnly = no;         /* #434940 - output BODY content only */
Bool FixUri = yes;          /* applies URI encoding if necessary */
Bool LowerLiterals = yes;   /* folds known attribute values to lower case */
Bool HideComments = no;     /* hides all (real) comments in output */
Bool IndentCdata = no;      /* indent <!CDATA[ ... ]]> section */
Bool ForceOutput = no;      /* output document even if errors were found */
uint ShowErrors = 6;        /* number of errors to put out */
Bool AsciiChars = yes;      /* convert quotes and dashes to nearest ASCII char */
Bool JoinClasses = no;      /* join multiple class attributes */
Bool JoinStyles = yes;      /* join multiple style attributes */
Bool EscapeCdata = no;      /* replace <![CDATA[]]> sections with escaped text */
Bool NCR = yes;             /* #431953 - RJ allow numeric character references */
Bool OutputBOM = no;        /* output a Byte Order Mark (BOM) when using UTF-8/UTF-16 encodings */
Bool SmartBOM = yes;        /* if input stream has BOM, do we automatically output a BOM? */

static uint c;      /* current char in input stream */
static FILE *fin;   /* file pointer for input stream */

/* not used to store anything */
static char *inline_tags;
static char *block_tags;
static char *empty_tags;
static char *pre_tags;

/* track what types of tags user has defined to eliminate unnecessary searches */
static int defined_tags = 0;

/* used by ParseBool, ParseInvBool, ParseTriState, ParseIndent, ParseBOM */
typedef enum
{
   boolState,    /* also maps to 'no' */
   invBoolState, /* also maps to 'yes' */
   autoState
} triState;

typedef struct _plist PList;

struct _plist
{
    char *name;                     /* property name */
    Location location;              /* place to store value */
    ParseProperty *parser;          /* parsing method */
    PList *next;                    /* linear hash chaining */
};

#define HASHSIZE 101

static PList *hashtable[HASHSIZE];   /* private hash table */
static Bool initialized = no;

/* used for parsing the command line */
static char *config_text;

static struct Flag
{
    char *name;                     /* property name */
    Location location;              /* place to store value */
    ParseProperty *parser;          /* parsing method */
} flags[] =
{
    {"indent-spaces",   {(int *)&spaces},           ParseInt},
    {"wrap",            {(int *)&wraplen},          ParseInt},
    {"wrap-attributes", {(int *)&WrapAttVals},      ParseBool},
    {"wrap-script-literals", {(int *)&WrapScriptlets}, ParseBool},
    {"wrap-sections",   {(int *)&WrapSection},      ParseBool},
    {"wrap-asp",        {(int *)&WrapAsp},          ParseBool},
    {"wrap-jste",       {(int *)&WrapJste},         ParseBool},
    {"wrap-php",        {(int *)&WrapPhp},          ParseBool},
    {"literal-attributes", {(int *)&LiteralAttribs}, ParseBool},
    {"tab-size",        {(int *)&tabsize},          ParseInt},
    {"markup",          {(int *)&OnlyErrors},       ParseInvBool},
    {"quiet",           {(int *)&Quiet},            ParseBool},
    {"tidy-mark",       {(int *)&TidyMark},         ParseBool},
    {"indent",          {(int *)&IndentContent},    ParseIndent},
    {"indent-attributes", {(int *)&IndentAttributes}, ParseBool},
    {"hide-endtags",    {(int *)&HideEndTags},      ParseBool},
    {"input-xml",       {(int *)&XmlTags},          ParseBool},
    {"output-xml",      {(int *)&XmlOut},           ParseBool},
    {"output-xhtml",    {(int *)&xHTML},            ParseBool},
    {"output-html",     {(int *)&HtmlOut},          ParseBool},
    {"add-xml-pi",      {(int *)&XmlPi},            ParseBool},
    {"add-xml-decl",    {(int *)&XmlPi},            ParseBool},
    {"assume-xml-procins",  {(int *)&XmlPIs},       ParseBool},
    {"raw",             {(int *)&RawOut},           ParseBool},
    {"uppercase-tags",  {(int *)&UpperCaseTags},    ParseBool},
    {"uppercase-attributes", {(int *)&UpperCaseAttrs}, ParseBool},
    {"bare",            {(int *)&MakeBare},         ParseBool},
    {"clean",           {(int *)&MakeClean},        ParseBool},
    {"logical-emphasis", {(int *)&LogicalEmphasis}, ParseBool},
    {"word-2000",       {(int *)&Word2000},         ParseBool},
    {"drop-empty-paras", {(int *)&DropEmptyParas},  ParseBool},
    {"drop-font-tags",  {(int *)&DropFontTags},     ParseBool},
    {"drop-proprietary-attributes", {(int *)&DropPropAttrs}, ParseBool},
    {"enclose-text",    {(int *)&EncloseBodyText},  ParseBool},
    {"enclose-block-text", {(int *)&EncloseBlockText}, ParseBool},
    {"alt-text",        {(int *)&alt_text},         ParseString},
    {"add-xml-space",   {(int *)&XmlSpace},         ParseBool},
    {"fix-bad-comments", {(int *)&FixComments},     ParseBool},
    {"split",           {(int *)&BurstSlides},      ParseBool},
    {"break-before-br", {(int *)&BreakBeforeBR},    ParseBool},
    {"numeric-entities", {(int *)&NumEntities},     ParseBool},
    {"quote-marks",     {(int *)&QuoteMarks},       ParseBool},
    {"quote-nbsp",      {(int *)&QuoteNbsp},        ParseBool},
    {"quote-ampersand", {(int *)&QuoteAmpersand},   ParseBool},
    {"write-back",      {(int *)&writeback},        ParseBool},
    {"keep-time",       {(int *)&KeepFileTimes},    ParseBool},
    {"show-warnings",   {(int *)&ShowWarnings},     ParseBool},
    {"error-file",      {(int *)&errfile},          ParseString},
    {"show-body-only",   {(int *)&BodyOnly},        ParseBool}, /* #434940 */
    {"slide-style",     {(int *)&slide_style},      ParseName},
    {"new-inline-tags",     {(int *)&inline_tags},  ParseTagNames},
    {"new-blocklevel-tags", {(int *)&block_tags},   ParseTagNames},
    {"new-empty-tags",  {(int *)&empty_tags},       ParseTagNames},
    {"new-pre-tags",    {(int *)&pre_tags},         ParseTagNames},
    {"char-encoding",   {(int *)&CharEncoding},     ParseCharEncoding},
/* turn off the following if you don't want the user to */
/* control the output encoding separately from the input encoding */
#if 1
    {"input-encoding",  {(int *)&inCharEncoding},   ParseCharEncoding},
    {"output-encoding", {(int *)&outCharEncoding},  ParseCharEncoding},
#endif

#if SUPPORT_ASIAN_ENCODINGS

    {"language",        {(void *)&Language},        ParseName},  /* #431953 - RJ */
    {"ncr",             {(void *)&NCR},             ParseBool},  /* #431953 - RJ */

#endif

    {"doctype",         {(int *)&doctype_str},      ParseDocType},
    {"fix-backslash",   {(int *)&FixBackslash},     ParseBool},
    {"gnu-emacs",       {(int *)&Emacs},            ParseBool},
    {"fix-uri",         {(int *)&FixUri},           ParseBool},
    {"lower-literals",  {(int *)&LowerLiterals},    ParseBool},
    {"hide-comments",   {(int *)&HideComments},     ParseBool},
    {"indent-cdata",    {(int *)&IndentCdata},      ParseBool},
    {"force-output",    {(int *)&ForceOutput},      ParseBool},
    {"show-errors",     {(int *)&ShowErrors},       ParseInt},
    {"ascii-chars",     {(int *)&AsciiChars},       ParseBool},
    {"join-classes",    {(int *)&JoinClasses},      ParseBool},
    {"join-styles",     {(int *)&JoinStyles},       ParseBool},
    {"escape-cdata",    {(int *)&EscapeCdata},      ParseBool},
    {"repeated-attributes", {(int *)&DuplicateAttrs}, ParseRepeatedAttribute},
    {"output-bom",      {(int *)&OutputBOM},        ParseBOM},

  /* this must be the final entry */
    {0,          0,             0}
};

static unsigned hash(char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = toupper(*s) + 31*hashval;

    return hashval % HASHSIZE;
}

static PList *lookup(char *s)
{
    PList *np;

    for (np = hashtable[hash(s)]; np != null; np = np->next)
        if (wstrcasecmp(s, np->name) == 0)
            return np;
    return null;
}

static PList *install(char *name, Location location, ParseProperty *parser)
{
    PList *np;
    unsigned hashval;

    if ((np = lookup(name)) == null)
    {
        np = (PList *)MemAlloc(sizeof(*np));

        if (np == null || (np->name = wstrdup(name)) == null)
            return null;

        hashval = hash(name);
        np->next = hashtable[hashval];
        hashtable[hashval] = np;
    }

    np->location = location;
    np->parser = parser;
    return np;
}

void InitConfig(void)
{
    struct Flag *p;

    if (!initialized)
    {
        initialized = yes;
    
        for(p = flags; p->name != null; ++p)
            install(p->name, p->location, p->parser);
    }

    c = 0;  /* init single char buffer */
}

void FreeConfig(void)
{
    PList *prev, *next;
    int i;

    for (i = 0; i < HASHSIZE; ++i)
    {
        prev = null;
        next = hashtable[i];

        while(next)
        {
            prev = next->next;
            MemFree(next->name);
            MemFree(next);
            next = prev;
        }

        hashtable[i] = null;
    }

    if (slide_style)
        MemFree(slide_style);

    if (doctype_str)
        MemFree(doctype_str);

    if (errfile)
        MemFree(errfile);
}

static unsigned GetC(FILE *fp)
{
    if (fp)
        return getc(fp);

    if (!config_text)
        return EOF;
 
    if (*config_text)
        return *config_text++;

    return EOF;
}

static int AdvanceChar()
{
    if (c != EOF)
        c = (uint)GetC(fin);
    return c;
}

static int SkipWhite()
{
    while (IsWhite((uint) c))
        c = (uint)GetC(fin);
    return c;
}

/* skip until end of line */
static void SkipToEndofLine()
{
    while (c != EOF)
    {
        c = (uint)GetC(fin);

        if (c == '\n' || c == '\r')
            break;
    }
}

/*
 skip over line continuations
 to start of next property
*/
static int NextProperty()
{
    do
    {
        /* skip to end of line */
        while (c != '\n' && c != '\r' && c != EOF)
            c = (uint)GetC(fin);

        /* treat  \r\n   \r  or  \n as line ends */
        if (c == '\r')
            c = (uint)GetC(fin);

        if (c == '\n')
            c = (uint)GetC(fin);
    }
    while (IsWhite(c));  /* line continuation? */

    return c;
}

#ifdef SUPPORT_GETPWNAM
/*
 Tod Lewis contributed this code for expanding
 ~/foo or ~your/foo according to $HOME and your
 user name. This will only work on Unix systems.
*/
const char *ExpandTilde(const char *filename)
{
    static char *expanded_filename;

    char *home_dir, *p;
    struct passwd *passwd = null;

    if (!filename) return(null);

    if (filename[0] != '~')
        return(filename);

    if (filename[1] == '/')
    {
        home_dir = getenv("HOME");
        if ( ! home_dir )
            return filename;
        filename++;
    }
    else
    {
        const char *s;
        char *t;

        s = filename+1;

        while(*s && *s != '/') s++;

        if (t = MemAlloc(s - filename))
        {
            memcpy(t, filename+1, s-filename-1);
            t[s-filename-1] = 0;

            passwd = getpwnam(t);

            MemFree(t);
        }

        if (!passwd)
            return(filename);

        filename = s;
        home_dir = passwd->pw_dir;
    }

    if (p = realloc(expanded_filename, strlen(filename)+strlen(home_dir)+1))
    {
        strcat(strcpy(expanded_filename = p, home_dir), filename);
        return(expanded_filename);
    }

    return(filename);
}
#endif /* SUPPORT_GETPWNAM */

void ParseConfigFile(char *file)
{
    int i;
    char name[64];
    const char *fname;
    PList *entry;

    /* setup property name -> parser table */

    InitConfig();

#ifdef SUPPORT_GETPWNAM
    /* expand filenames starting with ~ */
    fname = ExpandTilde( file );
#else
    fname = file;
#endif

    /* open the file and parse its contents */

    if ((fin = fopen(fname, "r")) == null)
        FileError(stderr, fname);
    else
    {
        config_text = null;
        AdvanceChar();  /* first char */

        while (c != EOF)
        {
            /* // starts a comment */
            /* also # starts a comment */
            while (c == '/' || c == '#')
                NextProperty();

            i = 0;

            while (c != ':' && c != EOF && i < 60)
            {
                name[i++] = (char)c;
                AdvanceChar();
            }

            name[i] = '\0';
            entry = lookup(name);

            if (c == ':' && entry)
            {
                AdvanceChar();
                entry->parser(entry->location, name);
            }
            else
                NextProperty();
        }

        fclose(fin);
    }
}

/* returns false if unknown option, missing parameter, or option doesn't use parameter */
Bool ParseConfig(char *option, char *parameter)
{
    PList *entry;
    FILE *ffp;

    if (option /* && parameter */)
    {
        ffp = fin;
    
        fin = null;
    
        /*
        c = *parameter;
        parameter++;
        */
    
        entry = lookup(option);
    
        if (!entry)
        {
            fin = ffp;
            ReportUnknownOption(option);
            return no;
        }

        if (parameter)
        {
            c = *parameter;
            parameter++;
    
            config_text = parameter;
            entry->parser(entry->location, option);
    
            fin = ffp;
        }
        else
        {
            ReportBadArgument(option);
            return no;
        }
        
    }
    /*
    else if (!parameter)
    {
        ReportBadArgument(option);
        return no;
    }
    */
    
    return yes;
}

/* ensure that char encodings are self consistent */
void AdjustCharEncoding(int encoding)
{
    CharEncoding = encoding;
    
    if (CharEncoding == RAW)
    {
        inCharEncoding = RAW;
        outCharEncoding = RAW;
    }
    else if (CharEncoding == ASCII)
    {
        inCharEncoding = LATIN1;
        outCharEncoding = ASCII;
    }
    else if (CharEncoding == LATIN1)
    {
        inCharEncoding = LATIN1;
        outCharEncoding = LATIN1;
    }
    else if (CharEncoding == UTF8)
    {
        inCharEncoding = UTF8;
        outCharEncoding = UTF8;
    }
    else if (CharEncoding == ISO2022)
    {
        inCharEncoding = ISO2022;
        outCharEncoding = ISO2022;
    }
    else if (CharEncoding == MACROMAN)
    {
        inCharEncoding = MACROMAN;
        outCharEncoding = ASCII;
    }

#if SUPPORT_UTF16_ENCODINGS

    else if (CharEncoding == UTF16LE)
    {
        inCharEncoding = UTF16LE;
        outCharEncoding = UTF16LE;
    }
    else if (CharEncoding == UTF16BE)
    {
        inCharEncoding = UTF16BE;
        outCharEncoding = UTF16BE;
    }
    else if (CharEncoding == UTF16)
    {
        inCharEncoding = UTF16;
        outCharEncoding = UTF16;
    }

#endif

    else if (CharEncoding == WIN1252)
    {
        inCharEncoding = WIN1252;
        outCharEncoding = ASCII;
    }

#if SUPPORT_ASIAN_ENCODINGS

    else if (CharEncoding == SHIFTJIS) /* #431953 - RJ */
    {
        inCharEncoding = SHIFTJIS;
        outCharEncoding = SHIFTJIS;
    }
    else if (CharEncoding == BIG5) /* #431953 - RJ */
    {
        inCharEncoding = BIG5;
        outCharEncoding = BIG5;
    }

#endif

}

/* ensure that config is self consistent */
void AdjustConfig(void)
{
    if (EncloseBlockText)
        EncloseBodyText = yes;

 /* avoid the need to set IndentContent when SmartIndent is set */

    if (SmartIndent)
        IndentContent = yes;

 /* disable wrapping */
    if (wraplen == 0)
        wraplen = 0x7FFFFFFF;

 /* Word 2000 needs o:p to be declared as inline */
    if (Word2000)
    {
        defined_tags |= tagtype_inline;
        DefineTag(tagtype_inline, "o:p");
    }

 /* XHTML is written in lower case */
    if (xHTML)
    {
        XmlOut = yes;
        UpperCaseTags = no;
        UpperCaseAttrs = no;
    }

 /* if XML in, then XML out */
    if (XmlTags)
    {
        XmlOut = yes;
        XmlPIs = yes;
    }

 /*
   #427837 - fix by Dave Raggett 02 Jun 01
   generate <?xml version="1.0" encoding="iso-8859-1"?>
   if the output character encoding is Latin-1 etc.
 */
    if (outCharEncoding == LATIN1 || outCharEncoding == ISO2022)
    {
        if (XmlOut)
            XmlPi = yes;
    }

 /* XML requires end tags */
    if (XmlOut)
    {
        QuoteAmpersand = yes;
        HideEndTags = no;
    }

#if SUPPORT_UTF16_ENCODINGS

 /* XML requires a BOM on output if using UTF-16 encoding */
    if (XmlOut &&
       (outCharEncoding == UTF16LE ||
        outCharEncoding == UTF16BE ||
        outCharEncoding == UTF16))
        OutputBOM = yes;

#endif
}

/* unsigned integers */
void ParseInt(Location location, char *option)
{
    int number = 0;
    Bool digits = no;

    SkipWhite();

    while(IsDigit(c))
    {
        number = c - '0' + (10 * number);
        digits = yes;
        AdvanceChar();
    }

    if (!digits)
        ReportBadArgument(option);
    
    *location.number = number;
    NextProperty();
}

/* true/false or yes/no or 0/1 or "auto" only looks at 1st char */
int ParseTriState(triState theState, Location location, char *option)
{
    int flag = no;
    SkipWhite();

    if (c == 't' || c == 'T' || c == 'y' || c == 'Y' || c == '1')
        flag = yes;
    else if (c == 'f' || c == 'F' || c == 'n' || c == 'N' || c == '0')
        flag = no;
    else if (theState == autoState && (c == 'a' || c =='A'))
        flag = autoState;
    else
        ReportBadArgument(option);

    if (theState == boolState)
        *location.logical = (Bool)flag;
    else if (theState == invBoolState)
        *location.logical = (Bool)(!flag);
    /* else if (theState == autoState) */ /* handled by the caller */
    
    NextProperty();
    
    return flag;
}

void ParseBool(Location location, char *option)
{
   ParseTriState(boolState, location, option);
}

void ParseInvBool(Location location, char *option)
{
   ParseTriState(invBoolState, location, option);
}

/* a string excluding whitespace */
void ParseName(Location location, char *option)
{
    char buf[256];
    int i = 0;

    SkipWhite();

    while (i < 254 && c != EOF && !IsWhite(c))
    {
        buf[i++] = c;
        AdvanceChar();
    }

    buf[i] = '\0';

    if (i == 0)
        ReportBadArgument(option);

    *location.string = wstrdup(buf);
    NextProperty();
}

/* a space or comma separated list of tag names */
void ParseTagNames(Location location, char *option)
{
    char buf[1024];
    int i = 0;

    do
    {
        if (c == ' ' || c == '\t' || c == ',')
        {
            AdvanceChar();
            continue;
        }

        if (c == '\r')
        {
            AdvanceChar();

            if (c == '\n')
                AdvanceChar();

            if (!(IsWhite((uint) c)))
                break;
        }

        if (c == '\n')
        {
            AdvanceChar();

            if (!(IsWhite((uint) c)))
                break;
        }

        while (i < 1022 && c != EOF && !IsWhite(c) && c != ',')
        {
            buf[i++] = ToLower(c);
            AdvanceChar();
        }

        buf[i] = '\0';
        if (i == 0)
        /* we shouldn't be here, but there is a bug when there is a trailing space on the line */
            continue;
            
        /* add tag to dictionary */

        if (location.string == &inline_tags)
        {
            defined_tags |= tagtype_inline;
            DefineTag(tagtype_inline, buf);
        }
        else if (location.string == &block_tags)
        {
            defined_tags |= tagtype_block;
            DefineTag(tagtype_block, buf);
        }
        else if (location.string == &empty_tags)
        {
            defined_tags |= tagtype_empty;
            DefineTag(tagtype_empty, buf);
        }
        else if (location.string == &pre_tags)
        {
            defined_tags |= tagtype_pre;
            DefineTag(tagtype_pre, buf);
        }

        i = 0;
    }
    while (c != EOF);
}

/* a string including whitespace */
/* munges whitespace sequences */
void ParseString(Location location, char *option)
{
    char buf[8192];
    int i = 0;
    unsigned delim = 0;
    Bool waswhite = yes;

    SkipWhite();

    if (c == '"' || c == '\'')
    {
        delim = c;
        AdvanceChar(); /* #431889 - fix by Dave Bryan 04 Jan 2001 */
    }

    while (i < 8190 && c != EOF && c != '\r' && c != '\n') /* #431889 - fix by Dave Bryan 04 Jan 2001 */
    {
/* #431889 - fix by Dave Bryan 04 Jan 2001 */
#if 0
        /* treat  \r\n   \r  or  \n as line ends */
        if (c == '\r')
        {
            AdvanceChar();

            if (c != '\n' && !IsWhite(c))
                break;
        }

        if (c == '\n')
        {
            AdvanceChar();

            if (!IsWhite(c))
                break;
        }
#endif

        if (c == delim && delim != '\0')
            break;

        if (IsWhite(c))
        {
            if (waswhite)
            {
                AdvanceChar();
                continue;
            }

            c = ' ';
        }
        else
            waswhite = no;

        buf[i++] = c;
        AdvanceChar();
    }

    buf[i] = '\0';

    if (*location.string)
        MemFree(*location.string);
#if 0
    if (i == 0)
        ReportBadArgument(option);
#endif
    *location.string = wstrdup(buf);
    NextProperty(); /* #431889 - fix by Dave Bryan 04 Jan 2001 */
}

void ParseCharEncoding(Location location, char *option)
{
    char buf[64];
    int i = 0;
    Bool validEncoding = yes;
    
    SkipWhite();

    while (i < 62 && c != EOF && !IsWhite(c))
    {
        buf[i++] = c;
        AdvanceChar();
    }

    buf[i] = '\0';

    if (wstrcasecmp(buf, "ascii") == 0)
        *location.number = ASCII;
    else if (wstrcasecmp(buf, "latin1") == 0)
        *location.number = LATIN1;
    else if (wstrcasecmp(buf, "raw") == 0)
        *location.number = RAW;
    else if (wstrcasecmp(buf, "utf8") == 0)
        *location.number = UTF8;
    else if (wstrcasecmp(buf, "iso2022") == 0)
        *location.number = ISO2022;
    else if (wstrcasecmp(buf, "mac") == 0)
        *location.number = MACROMAN;

#if SUPPORT_UTF16_ENCODINGS

    else if (wstrcasecmp(buf, "utf16le") == 0)
        *location.number = UTF16LE;
    else if (wstrcasecmp(buf, "utf16be") == 0)
        *location.number = UTF16BE;
    else if (wstrcasecmp(buf, "utf16") == 0)
        *location.number = UTF16;

#endif

    else if (wstrcasecmp(buf, "win1252") == 0)
        *location.number = WIN1252;

#if SUPPORT_ASIAN_ENCODINGS

    else if (wstrcasecmp(buf, "big5") == 0) /* #431953 - RJ */
        *location.number = BIG5; /* #431953 - RJ */
    else if (wstrcasecmp(buf, "shiftjis") == 0) /* #431953 - RJ */
        *location.number = SHIFTJIS; /* #431953 - RJ */

#endif

    else
    {
        validEncoding = no;
        ReportBadArgument(option);
    }

    if (validEncoding && (location.number == &CharEncoding))
        AdjustCharEncoding(*location.number);
    
    NextProperty();
}

char *CharEncodingName(int encoding)
{
    char *encodingName;
    
    switch(encoding)
    {
        case ASCII    : encodingName = "ascii"; break;
        case LATIN1   : encodingName = "latin1"; break;
        case RAW      : encodingName = "raw"; break;
        case UTF8     : encodingName = "utf8"; break;
        case ISO2022  : encodingName = "iso2022"; break;
        case MACROMAN : encodingName = "mac"; break;

#if SUPPORT_UTF16_ENCODINGS

        case UTF16LE  : encodingName = "utf16le"; break;
        case UTF16BE  : encodingName = "utf16be"; break;
        case UTF16    : encodingName = "utf16"; break;

#endif

        case WIN1252  : encodingName = "win1252"; break;

#if SUPPORT_ASIAN_ENCODINGS

        case BIG5     : encodingName = "big5"; break;
        case SHIFTJIS : encodingName = "shiftjis"; break;

#endif
        default       : encodingName = "unknown"; break;
    }
    
    return encodingName;
}

void ParseIndent(Location location, char *option)
{
    int flag = no;
    
    flag = ParseTriState(autoState, location, option);
    
    if (flag == autoState)
    {
        IndentContent = yes;
        SmartIndent = yes;
    }
    else
    {
        IndentContent = (Bool)(flag);
        SmartIndent = no;
    }
}

/*
   doctype: omit | auto | strict | loose | <fpi>

   where the fpi is a string similar to

      "-//ACME//DTD HTML 3.14159//EN"
*/
void ParseDocType(Location location, char *option)
{
    char buf[64];
    int i = 0;

    SkipWhite();

    /* "-//ACME//DTD HTML 3.14159//EN" or similar */

    if (c == '"' || c == '\'') /* #431889 - fix by Terry Teague 01 Jul 01 */
    {
        ParseString(location, option);
        doctype_mode = doctype_user;
        return;
    }

    /* read first word */
    while (i < 62 && c != EOF && !IsWhite(c))
    {
        buf[i++] = c;
        AdvanceChar();
    }

    buf[i] = '\0';

    /* #443663 - fix by Terry Teague 23 Jul 01 */
    if (wstrcasecmp(buf, "auto") == 0)
        doctype_mode = doctype_auto;
    else if (wstrcasecmp(buf, "omit") == 0)
        doctype_mode = doctype_omit;
    else if (wstrcasecmp(buf, "strict") == 0)
        doctype_mode = doctype_strict;
    else if (wstrcasecmp(buf, "loose") == 0 ||
             wstrcasecmp(buf, "transitional") == 0)
        doctype_mode = doctype_loose;
    else /* if (i == 0) */
        ReportBadArgument(option);

    NextProperty();
}

void ParseRepeatedAttribute(Location location, char *option)
{
    char buf[64];
    int i = 0;

    SkipWhite();

    while (i < 62 && c != EOF && !IsWhite(c))
    {
        buf[i++] = c;
        AdvanceChar();
    }

    buf[i] = '\0';

    if (wstrcasecmp(buf, "keep-first") == 0)
        DuplicateAttrs = keep_first;
    else if (wstrcasecmp(buf, "keep-last") == 0)
        DuplicateAttrs = keep_last;
    else
        ReportBadArgument(option);

    NextProperty();
}

void ParseBOM(Location location, char *option)
{
    int flag = no;
    
    flag = ParseTriState(autoState, location, option);
    
    if (flag == no)
    {
        OutputBOM = no;
        SmartBOM = no;
    }
    else if (flag == yes)
    {
        OutputBOM = yes;
        SmartBOM = yes;
    }
    else
    {
        OutputBOM = no;
        SmartBOM = yes;
    }
}

void PrintConfigOptions(FILE *errout, Bool showCurrent)
{
#define kMaxValFieldWidth 40
    static const char* fmt = "%-26.26s  %-9.9s  %-40.40s\n";
    static const char* ul 
        = "=================================================================";
    struct Flag* configItem;

    tidy_out( errout, "\nConfiguration File Settings:\n\n" );
    if (showCurrent)
        tidy_out( errout, fmt, "Name", "Type", "Current value" );
    else
        tidy_out( errout, fmt, "Name", "Type", "Allowable values" );
    tidy_out( errout, fmt, ul, ul, ul );

    for ( configItem = flags; configItem && configItem->name; configItem++ )
    {
        char* name = configItem->name;
        char* type = "String";
        char tempvals[80];
        char* vals = &tempvals[0];
        
        tempvals[0] = '\0';
        
        if ( configItem->parser == ParseBool || 
             configItem->parser == ParseInvBool )
        {
            type = "Boolean";
            
            if (showCurrent)
                if (configItem->parser == ParseBool)
                    vals = *(configItem->location.logical)?"yes":"no";
                else
                    vals = *(configItem->location.logical)?"no":"yes";
            else
                vals = "y/n, yes/no, t/f, true/false, 1/0";
        }

        else if ( configItem->parser == ParseInt )
        {
            type = "Integer";
            
            if (showCurrent)
                sprintf(tempvals, "%d", *(configItem->location.number));
            else
            {
                if ((uint *)configItem->location.number == &wraplen)
                    vals = "0 (no wrapping), 1, 2, ...";
                else
                    vals = "0, 1, 2, ...";
            }
        }

        else if ( configItem->parser == ParseIndent )
        {
            type = "AutoBool";
             
            if (showCurrent)
            {
                if (SmartIndent)
                    vals = "auto";
                else
                    vals = *(configItem->location.logical)?"yes":"no";
            }
            else
                vals = "auto, y/n, yes/no, t/f, true/false, 1/0";
        }

        else if ( configItem->parser == ParseDocType )
        {
            type = "DocType";
            
            if (showCurrent)
            {
                switch(doctype_mode)
                {
                    case doctype_auto   : vals = "auto"; break;
                    case doctype_omit   : vals = "omit"; break;
                    case doctype_strict : vals = "strict"; break;
                    case doctype_loose  : vals = "loose (transitional)"; break;
                    case doctype_user   : vals = *(configItem->location.string); break;
                }
            }
            else
            {
                vals = "auto, omit, strict, loose, transitional,";
                tidy_out( errout, fmt, name, type, vals );
                name = "";
                type = "";
                vals = "user specified fpi (string)";
            }
        }

        else if ( configItem->parser == ParseName )
        {
            type = "Name";
            
            /* these are not currently used */
            /*
            if ( wstrcasecmp(configItem->name, "slide-style") ||
                 wstrcasecmp(configItem->name, "language") )
                continue;
            */
            if (showCurrent)
                 vals = *(configItem->location.string);
             else
                 vals = "" /* "whole word only" */;
         }

        else if ( configItem->parser == ParseTagNames )
        {
            type = "Tag names";
            
            if (showCurrent)
            {
                int tagType = 0;
                
                if ((configItem->location.string == &inline_tags) &&
                    (defined_tags & tagtype_inline))
                    tagType = tagtype_inline;
                else if ((configItem->location.string == &block_tags) &&
                    (defined_tags & tagtype_block))
                    tagType = tagtype_block;
                else if ((configItem->location.string == &empty_tags) &&
                    (defined_tags & tagtype_empty))
                    tagType = tagtype_empty;
                else if ((configItem->location.string == &pre_tags) &&
                    (defined_tags & tagtype_pre))
                    tagType = tagtype_pre;
                
                if (tagType != 0)
                {
                    char *tagName = null;
                    int totlen = 0;
                    
                    ResetDefinedTagSearch();
                    
                    do
                    {
                        tagName = FindNextDefinedTag(tagType);
                        if (tagName)
                        {
                            if (totlen + wstrlen(tagName) + 2 > kMaxValFieldWidth)
                            {
                                /* output what we have so far */
                                tidy_out( errout, fmt, name, type, vals );
                                name = "";
                                type = "";
                                totlen = 0;
                                *vals = '\0';
                            }

                            wstrcat(vals, tagName);
                            wstrcat(vals, ", ");
                            totlen += wstrlen(tagName) + 2;
                        }
                    } while (tagName != null);
                    
                    if ((totlen > 1) && (vals[totlen - 2] == ','))
                        vals[totlen - 2] = '\0'; /* strip trailing comma/space */
                }
            }
            else
                vals = "tagX, tagY, ...";
        }

        else if ( configItem->parser == ParseCharEncoding )
        {
            type = "Encoding";
            
            if (showCurrent)
                vals = CharEncodingName(*(configItem->location.number));
             else
            {
                vals = "ascii, latin1, raw, utf8, iso2022, mac,";
                tidy_out( errout, fmt, name, type, vals );
                name = "";
                type = "";

#if SUPPORT_UTF16_ENCODINGS

                vals = "utf16le, utf16be, utf16,";
                tidy_out( errout, fmt, name, type, vals );

#endif

#if SUPPORT_ASIAN_ENCODINGS

                vals = "win1252, big5, shiftjis";
 
 #else
 
                vals = "win1252";
 
 #endif
            }
       }
        
        else if ( configItem->parser == ParseRepeatedAttribute )
        {
            type = "-";
            
            if (showCurrent)
                vals = (DuplicateAttrs == keep_first)?"keep-first":"keep-last";
            else
                vals = "keep-first, keep-last";
        }

        else if ( configItem->parser == ParseBOM )
        {
            type = "AutoBool";
             
            if (showCurrent)
            {
                if ((SmartBOM == yes) && (OutputBOM == no))
                    vals = "auto";
                else
                    vals = *(configItem->location.logical)?"yes":"no";
            }
            else
                vals = "auto, y/n, yes/no, t/f, true/false, 1/0";
        }

        if (name != "" || type != "" || vals != "")
            tidy_out( errout, fmt, name, type, vals );
    }
    
}
