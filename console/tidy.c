/*
  tidy.c - HTML TidyLib command line driver

  Copyright (c) 1998-2005 World Wide Web Consortium
  (Massachusetts Institute of Technology, European Research 
  Consortium for Informatics and Mathematics, Keio University).
  All Rights Reserved.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2005/03/30 13:27:01 $ 
    $Revision: 1.25 $ 
*/

#include "tidy.h"

static FILE* errout = NULL;  /* set to stderr */
/* static FILE* txtout = NULL; */  /* set to stdout */

static Bool samefile( ctmbstr filename1, ctmbstr filename2 )
{
#if FILENAMES_CASE_SENSITIVE
    return ( strcmp( filename1, filename2 ) == 0 );
#else
    return ( strcasecmp( filename1, filename2 ) == 0 );
#endif
}

static void help( ctmbstr prog )
{
    printf( "%s [option...] [file...] [option...] [file...]\n", prog );
    printf( "Utility to clean up and pretty print HTML/XHTML/XML\n");
    printf( "see http://tidy.sourceforge.net/\n");
    printf( "\n");

#ifdef PLATFORM_NAME
    printf( "Options for HTML Tidy for %s released on %s:\n",
             PLATFORM_NAME, tidyReleaseDate() );
#else
    printf( "Options for HTML Tidy released on %s:\n", tidyReleaseDate() );
#endif
    printf( "\n");

    printf( "File manipulation\n");
    printf( "-----------------\n");
    printf( "  -out or -o <file> specify the output markup file\n");
    printf( "  -config <file>    set configuration options from the specified <file>\n");
    printf( "  -f      <file>    write errors to the specified <file>\n");
    printf( "  -modify or -m     modify the original input files\n");
    printf( "\n");

    printf( "Processing directives\n");
    printf( "---------------------\n");
    printf( "  -indent  or -i    indent element content\n");
    printf( "  -wrap <column>    wrap text at the specified <column> (default is 68)\n");
    printf( "  -upper   or -u    force tags to upper case (default is lower case)\n");
    printf( "  -clean   or -c    replace FONT, NOBR and CENTER tags by CSS\n");
    printf( "  -bare    or -b    strip out smart quotes and em dashes, etc.\n");
    printf( "  -numeric or -n    output numeric rather than named entities\n");
    printf( "  -errors  or -e    only show errors\n");
    printf( "  -quiet   or -q    suppress nonessential output\n");
    printf( "  -omit             omit optional end tags\n");
    printf( "  -xml              specify the input is well formed XML\n");
    printf( "  -asxml            convert HTML to well formed XHTML\n");
    printf( "  -asxhtml          convert HTML to well formed XHTML\n");
    printf( "  -ashtml           force XHTML to well formed HTML\n");

/* TRT */
#if SUPPORT_ACCESSIBILITY_CHECKS
    printf( "  -access <level>   do additional accessibility checks (<level> = 1, 2, 3)\n");
#endif

    printf( "\n");

    printf( "Character encodings\n");
    printf( "-------------------\n");
    printf( "  -raw              output values above 127 without conversion to entities\n");
    printf( "  -ascii            use US-ASCII for output, ISO-8859-1 for input\n");
    printf( "  -latin0           use US-ASCII for output, ISO-8859-15 for input\n");
    printf( "  -latin1           use ISO-8859-1 for both input and output\n");
#ifndef NO_NATIVE_ISO2022_SUPPORT
    printf( "  -iso2022          use ISO-2022 for both input and output\n");
#endif
    printf( "  -utf8             use UTF-8 for both input and output\n");
    printf( "  -mac              use MacRoman for input, US-ASCII for output\n");
    printf( "  -win1252          use Windows-1252 for input, US-ASCII for output\n");
    printf( "  -ibm858           use IBM-858 (CP850+Euro) for input, US-ASCII for output\n");

#if SUPPORT_UTF16_ENCODINGS
    printf( "  -utf16le          use UTF-16LE for both input and output\n");
    printf( "  -utf16be          use UTF-16BE for both input and output\n");
    printf( "  -utf16            use UTF-16 for both input and output\n");
#endif

#if SUPPORT_ASIAN_ENCODINGS /* #431953 - RJ */
    printf( "  -big5             use Big5 for both input and output\n"); 
    printf( "  -shiftjis         use Shift_JIS for both input and output\n");
    printf( "  -language <lang>  set the two-letter language code <lang> (for future use)\n");
#endif
    printf( "\n");

    printf( "Miscellaneous\n");
    printf( "-------------\n");
    printf( "  -version  or -v   show the version of Tidy\n");
    printf( "  -help, -h or -?   list the command line options\n");
    printf( "  -help-config      list all configuration options\n");
    printf( "  -xml-config       list all configuration options in XML format\n");
    printf( "  -show-config      list the current configuration settings\n");
    printf( "\n");
    printf( "Use --blah blarg for any configuration option \"blah\" with argument \"blarg\"\n");
    printf( "\n");

    printf( "Input/Output default to stdin/stdout respectively\n");
    printf( "Single letter options apart from -f may be combined\n");
    printf( "as in:  tidy -f errs.txt -imu foo.html\n");
    printf( "For further info on HTML see http://www.w3.org/MarkUp\n");
    printf( "\n");
}

static const char *cutToWhiteSpace(const char *s, uint offset, char *sbuf)
{
    if (!s)
    {
        sbuf[0] = '\0';
        return NULL;
    }
    else if (strlen(s) <= offset)
    {
        strcpy(sbuf,s);
        sbuf[offset] = '\0';
        return NULL;
    }
    else
    {
        uint j, l, n;
        j = offset;
        while(j && s[j] != ' ')
            --j;
        l = j;
        n = j+1;
        /* no white space */
        if (j==0)
        {
            l = offset;
            n = offset;
        }
        strncpy(sbuf,s,l);
        sbuf[l] = '\0';
        return s+n;
    }
}

static void print3Columns( const char* fmt, uint l1, uint l2, uint l3,
                           const char *c1, const char *c2, const char *c3 )
{
    const char *pc1=c1, *pc2=c2, *pc3=c3;
    char *c1buf = malloc(l1+1);
    char *c2buf = malloc(l2+1);
    char *c3buf = malloc(l3+1);

    do
    {
        pc1 = cutToWhiteSpace(pc1, l1, c1buf);
        pc2 = cutToWhiteSpace(pc2, l2, c2buf);
        pc3 = cutToWhiteSpace(pc3, l3, c3buf);
        printf(fmt,
               c1buf[0]!='\0'?c1buf:"",
               c2buf[0]!='\0'?c2buf:"",
               c3buf[0]!='\0'?c3buf:"");
    } while (pc1 || pc2 || pc3);
    free(c1buf);
    free(c2buf);
    free(c3buf);
}

#define kMaxValFieldWidth 40
static const char* fmt = "%-27.27s %-9.9s  %-40.40s\n";
static const char* valfmt = "%-27.27s %-9.9s %-1.1s%-39.39s\n";
static const char* ul 
        = "=================================================================";

static Bool isAutoBool( TidyOption topt )
{
    TidyIterator pos;
    ctmbstr def;

    if ( tidyOptGetType( topt ) != TidyInteger)
        return no;

    pos = tidyOptGetPickList( topt );
    while ( pos )
    {
        def = tidyOptGetNextPick( topt, &pos );
        if (0==strcmp(def,"yes"))
           return yes;
    }
    return no;
}

static
ctmbstr ConfigCategoryName( TidyConfigCategory id )
{
    switch( id )
    {
    case TidyMarkup:
        return "markup";
    case TidyDiagnostics:
        return "diagnostics";
    case TidyPrettyPrint:
        return "print";
    case TidyEncoding:
        return "encoding";
    case TidyMiscellaneous:
        return "misc";
    }
    assert(0);
    abort();
}

/* Description of an option */
typedef struct {
    ctmbstr name;  /**< Name */
    ctmbstr cat;   /**< Category */
    ctmbstr type;  /**< "String, ... */
    ctmbstr vals;  /**< Potential values. If NULL, use an external function */
    ctmbstr def;   /**< default */
    tmbchar tempdefs[80]; /**< storage for default such as integer */
} OptionDesc;

typedef void (*OptionFunc)( TidyOption, const OptionDesc * );


/* Create description "d" related to "opt" */
static
void GetOption( TidyDoc tdoc, TidyOption topt, OptionDesc *d )
{
    TidyOptionId optId = tidyOptGetId( topt );
    TidyOptionType optTyp = tidyOptGetType( topt );

    d->name = tidyOptGetName( topt );
    d->cat = ConfigCategoryName( tidyOptGetCategory( topt ) );
    d->vals = NULL;
    d->def = NULL;

    if ( tidyOptIsReadOnly(topt) )
        return;

    /* Handle special cases first.
     */
    switch ( optId )
    {
    case TidyDuplicateAttrs:
    case TidyNewline:
    case TidyAccessibilityCheckLevel:
        d->type = "enum";
        d->vals = NULL;
        d->def =
            optId==TidyNewline ?
            "<em>Platform dependent</em>"
            :tidyOptGetCurrPick( tdoc, optId );
        break;

    case TidyDoctype:
        d->type = "DocType";
        d->vals = NULL;
        {
            ctmbstr sdef = NULL;
            sdef = (tmbstr) tidyOptGetCurrPick( tdoc, TidyDoctypeMode );
            if ( !sdef || *sdef == '*' )
                sdef = tidyOptGetValue( tdoc, TidyDoctype );
            d->def = sdef;
        }
        break;

    case TidyInlineTags:
    case TidyBlockTags:
    case TidyEmptyTags:
    case TidyPreTags:
        d->type = "Tag names";
        d->vals = "tagX, tagY, ...";
        d->def = "-none-";
        break;

    case TidyCharEncoding:
    case TidyInCharEncoding:
    case TidyOutCharEncoding:
        d->type = "Encoding";
        d->def = tidyOptGetEncName( tdoc, optId );
        if (!d->def)
            d->def = "?";
        d->vals = NULL;
        break;

        /* General case will handle remaining */
    default:
        switch ( optTyp )
        {
        case TidyBoolean:
            d->type = "Boolean";
            d->vals = "y/n, yes/no, t/f, true/false, 1/0";
            d->def = tidyOptGetCurrPick( tdoc, optId );
            break;

        case TidyInteger:
            if (isAutoBool(topt))
            {
                d->type = "AutoBool";
                d->vals = "auto, y/n, yes/no, t/f, true/false, 1/0";
                d->def = tidyOptGetCurrPick( tdoc, optId );
            } else
            {
                uint idef;
                d->type = "Integer";
                if ( optId == TidyWrapLen )
                    d->vals = "0 (no wrapping), 1, 2, ...";
                else
                    d->vals = "0, 1, 2, ...";

                idef = tidyOptGetInt( tdoc, optId );
                sprintf(d->tempdefs, "%u", idef);
                d->def = d->tempdefs;
            }
            break;

        case TidyString:
            d->type = "String";
            d->vals = "-";
            d->def = tidyOptGetValue( tdoc, optId );
            if (!d->def)
                d->def = "-";
            break;
        }
    }
}

static void optionhelpImp( TidyDoc tdoc, OptionFunc OptionPrint )
{
    TidyIterator pos = tidyGetOptionList( tdoc );

    while ( pos )
    {
        TidyOption topt = tidyGetNextOption( tdoc, &pos );
        OptionDesc d;

        if ( tidyOptIsReadOnly(topt) )
            continue;
        GetOption( tdoc, topt, &d );
        (*OptionPrint)( topt, &d );
    }
}

static
tmbstr GetAllowedValuesFromPick( TidyOption topt )
{
    TidyIterator pos;
    Bool first;
    ctmbstr def;
    uint len = 0;
    tmbstr val;

    pos = tidyOptGetPickList( topt );
    first = yes;
    while ( pos )
    {
        if (first)
            first = no;
        else
            len += 2;
        def = tidyOptGetNextPick( topt, &pos );
        len += strlen(def);
    }
    val = malloc(len+1);
    val[0] = '\0';
    pos = tidyOptGetPickList( topt );
    first = yes;
    while ( pos )
    {
        if (first)
            first = no;
        else
            strcat(val, ", ");
        def = tidyOptGetNextPick( topt, &pos );
        strcat(val, def);
    }
    return val;
}

static
tmbstr GetAllowedValues( TidyOption topt, const OptionDesc *d )
{
    if (d->vals)
    {
        tmbstr val = malloc(1+strlen(d->vals));
        strcpy(val, d->vals);
        return val;
    }
    else
        return GetAllowedValuesFromPick( topt );
}

static
void printOption( TidyOption topt, const OptionDesc *d )
{
    if ( *d->name || *d->type )
    {
       ctmbstr pval = d->vals;
       tmbstr val = NULL;
       if (pval == NULL)
       {
           val = GetAllowedValues( topt, d);
           pval = val;
       }
       print3Columns( fmt, 27, 9, 40, d->name, d->type, pval );
       if (val)
           free(val);
    }
}

static void optionhelp( TidyDoc tdoc )
{
    printf( "\nHTML Tidy Configuration Settings\n\n" );
    printf( "Within a file, use the form:\n\n" );
    printf( "wrap: 72\n" );
    printf( "indent: no\n\n" );
    printf( "When specified on the command line, use the form:\n\n" );
    printf( "--wrap 72 --indent no\n\n");

    printf( fmt, "Name", "Type", "Allowable values" );
    printf( fmt, ul, ul, ul );

    optionhelpImp( tdoc, printOption );
}

static void optionvalues( TidyDoc tdoc )
{
    TidyIterator pos = tidyGetOptionList( tdoc );

    printf( "\nConfiguration File Settings:\n\n" );
    printf( fmt, "Name", "Type", "Current Value" );
    printf( fmt, ul, ul, ul );

    while ( pos )
    {
        TidyOption topt = tidyGetNextOption( tdoc, &pos );
        TidyOptionId optId = tidyOptGetId( topt );
        TidyOptionType optTyp = tidyOptGetType( topt );
        Bool isReadOnly = tidyOptIsReadOnly( topt );

        ctmbstr sval = NULL;
        uint ival = 0;

        ctmbstr name = (tmbstr) tidyOptGetName( topt );
        ctmbstr type = "String";
        tmbchar tempvals[80] = {0};
        ctmbstr vals = &tempvals[0];
        ctmbstr ro   = ( isReadOnly ? "*" : "" );

        /* Handle special cases first.
        */
        switch ( optId )
        {
        case TidyDuplicateAttrs:
            type = "enum";
            vals = (tmbstr) tidyOptGetCurrPick( tdoc, optId );
            break;

        case TidyDoctype:
            sval = (tmbstr) tidyOptGetCurrPick( tdoc, TidyDoctypeMode );
            type = "DocType";
            if ( !sval || *sval == '*' )
                sval = (tmbstr) tidyOptGetValue( tdoc, TidyDoctype );
            vals = (tmbstr) sval;
            break;

        case TidyCSSPrefix:
            type = "Name";
            vals = (tmbstr) tidyOptGetValue( tdoc, TidyCSSPrefix );
            break;

        case TidyInlineTags:
        case TidyBlockTags:
        case TidyEmptyTags:
        case TidyPreTags:
            {
                TidyIterator pos = tidyOptGetDeclTagList( tdoc );
                type = "Tag names";
                while ( pos )
                {
                    vals = (tmbstr) tidyOptGetNextDeclTag(tdoc, optId, &pos);
                    if ( pos )
                    {
                        if ( *name )
                            printf( valfmt, name, type, ro, vals );
                        else
                            printf( fmt, name, type, vals );
                        name = "";
                        type = "";
                    }
                }
            }
            break;

        case TidyCharEncoding:
        case TidyInCharEncoding:
        case TidyOutCharEncoding:
            type = "Encoding";
            sval = tidyOptGetEncName( tdoc, optId );
            vals = (tmbstr) sval;
            break;

        case TidyNewline:
            type = "enum";
            vals = (tmbstr) tidyOptGetCurrPick( tdoc, optId );
            break;

        /* General case will handle remaining */
        default:
            switch ( optTyp )
            {
            case TidyBoolean:
                type = "Boolean";   /* curr pick handles inverse */
                vals = (tmbstr) tidyOptGetCurrPick( tdoc, optId );
                break;

            case TidyInteger:
                if (isAutoBool(topt))
                {
                    type = "AutoBool";
                    vals = tidyOptGetCurrPick( tdoc, optId );
                }
                else
                {
                    type = "Integer";
                    ival = tidyOptGetInt( tdoc, optId );
                    sprintf( tempvals, "%u", ival );
                }
                break;

            case TidyString:
                type = "String";
                vals = (tmbstr) tidyOptGetValue( tdoc, optId );
                break;
            }
        }

        /* fix for http://tidy.sf.net/bug/873921 */
        if ( *name || *type || (vals && *vals) )
        {
            if ( ! vals )
                vals = "";
            if ( *name )
                printf( valfmt, name, type, ro, vals );
            else
                printf( fmt, name, type, vals );
        }
    }

    printf( "\n\nValues marked with an *asterisk are calculated \n"
            "internally by HTML Tidy\n\n" );
}

static void version( void )
{
#ifdef PLATFORM_NAME
    printf( "HTML Tidy for %s released on %s\n",
             PLATFORM_NAME, tidyReleaseDate() );
#else
    printf( "HTML Tidy released on %s\n", tidyReleaseDate() );
#endif
}

static void unknownOption( uint c )
{
    fprintf( errout, "HTML Tidy: unknown option: %c\n", (char)c );
}

int main( int argc, char** argv )
{
    ctmbstr prog = argv[0];
    ctmbstr cfgfil = NULL, errfil = NULL, htmlfil = NULL;
    TidyDoc tdoc = tidyCreate();
    int status = 0;

    uint contentErrors = 0;
    uint contentWarnings = 0;
    uint accessWarnings = 0;

    errout = stderr;  /* initialize to stderr */
    status = 0;
    
#ifdef CONFIG_FILE
    if ( tidyFileExists(CONFIG_FILE) )
    {
        status = tidyLoadConfig( tdoc, CONFIG_FILE );
        if ( status != 0 )
            fprintf(errout, "Loading config file \"%s\" failed, err = %d\n", CONFIG_FILE, status);
    }
#endif /* CONFIG_FILE */

    /* look for env var "HTML_TIDY" */
    /* then for ~/.tidyrc (on platforms defining $HOME) */

    if ( cfgfil = getenv("HTML_TIDY") )
    {
        status = tidyLoadConfig( tdoc, cfgfil );
        if ( status != 0 )
            fprintf(errout, "Loading config file \"%s\" failed, err = %d\n", cfgfil, status);
    }
#ifdef USER_CONFIG_FILE
    else if ( tidyFileExists(USER_CONFIG_FILE) )
    {
        status = tidyLoadConfig( tdoc, USER_CONFIG_FILE );
        if ( status != 0 )
            fprintf(errout, "Loading config file \"%s\" failed, err = %d\n", USER_CONFIG_FILE, status);
    }
#endif /* USER_CONFIG_FILE */

    /* read command line */
    while ( argc > 0 )
    {
        if (argc > 1 && argv[1][0] == '-')
        {
            /* support -foo and --foo */
            ctmbstr arg = argv[1] + 1;

            if ( strcasecmp(arg, "xml") == 0)
                tidyOptSetBool( tdoc, TidyXmlTags, yes );

            else if ( strcasecmp(arg,   "asxml") == 0 ||
                      strcasecmp(arg, "asxhtml") == 0 )
            {
                tidyOptSetBool( tdoc, TidyXhtmlOut, yes );
            }
            else if ( strcasecmp(arg,   "ashtml") == 0 )
                tidyOptSetBool( tdoc, TidyHtmlOut, yes );

            else if ( strcasecmp(arg, "indent") == 0 )
            {
                tidyOptSetInt( tdoc, TidyIndentContent, TidyAutoState );
                if ( tidyOptGetInt(tdoc, TidyIndentSpaces) == 0 )
                    tidyOptResetToDefault( tdoc, TidyIndentSpaces );
            }
            else if ( strcasecmp(arg, "omit") == 0 )
                tidyOptSetBool( tdoc, TidyHideEndTags, yes );

            else if ( strcasecmp(arg, "upper") == 0 )
                tidyOptSetBool( tdoc, TidyUpperCaseTags, yes );

            else if ( strcasecmp(arg, "clean") == 0 )
                tidyOptSetBool( tdoc, TidyMakeClean, yes );

            else if ( strcasecmp(arg, "bare") == 0 )
                tidyOptSetBool( tdoc, TidyMakeBare, yes );

            else if ( strcasecmp(arg, "raw") == 0      ||
                      strcasecmp(arg, "ascii") == 0    ||
                      strcasecmp(arg, "latin0") == 0   ||
                      strcasecmp(arg, "latin1") == 0   ||
                      strcasecmp(arg, "utf8") == 0     ||
#ifndef NO_NATIVE_ISO2022_SUPPORT
                      strcasecmp(arg, "iso2022") == 0  ||
#endif
#if SUPPORT_UTF16_ENCODINGS
                      strcasecmp(arg, "utf16le") == 0  ||
                      strcasecmp(arg, "utf16be") == 0  ||
                      strcasecmp(arg, "utf16") == 0    ||
#endif
#if SUPPORT_ASIAN_ENCODINGS
                      strcasecmp(arg, "shiftjis") == 0 ||
                      strcasecmp(arg, "big5") == 0     ||
#endif
                      strcasecmp(arg, "mac") == 0      ||
                      strcasecmp(arg, "win1252") == 0  ||
                      strcasecmp(arg, "ibm858") == 0 )
            {
                tidySetCharEncoding( tdoc, arg );
            }
            else if ( strcasecmp(arg, "numeric") == 0 )
                tidyOptSetBool( tdoc, TidyNumEntities, yes );

            else if ( strcasecmp(arg, "modify") == 0 ||
                      strcasecmp(arg, "change") == 0 ||  /* obsolete */
                      strcasecmp(arg, "update") == 0 )   /* obsolete */
            {
                tidyOptSetBool( tdoc, TidyWriteBack, yes );
            }
            else if ( strcasecmp(arg, "errors") == 0 )
                tidyOptSetBool( tdoc, TidyShowMarkup, no );

            else if ( strcasecmp(arg, "quiet") == 0 )
                tidyOptSetBool( tdoc, TidyQuiet, yes );

            else if ( strcasecmp(arg, "help") == 0 ||
                      strcasecmp(arg,    "h") == 0 || *arg == '?' )
            {
                help( prog );
                tidyRelease( tdoc );
                return 0; /* success */
            }
            else if ( strcasecmp(arg, "help-config") == 0 )
            {
                optionhelp( tdoc );
                tidyRelease( tdoc );
                return 0; /* success */
            }
            else if ( strcasecmp(arg, "show-config") == 0 )
            {
                optionvalues( tdoc );
                tidyRelease( tdoc );
                return 0; /* success */
            }
            else if ( strcasecmp(arg, "config") == 0 )
            {
                if ( argc >= 3 )
                {
                    ctmbstr post;

                    tidyLoadConfig( tdoc, argv[2] );

                    /* Set new error output stream if setting changed */
                    post = tidyOptGetValue( tdoc, TidyErrFile );
                    if ( post && (!errfil || !samefile(errfil, post)) )
                    {
                        errfil = post;
                        errout = tidySetErrorFile( tdoc, post );
                    }

                    --argc;
                    ++argv;
                }
            }

#if SUPPORT_ASIAN_ENCODINGS
            else if ( strcasecmp(arg, "language") == 0 ||
                      strcasecmp(arg,     "lang") == 0 )
            {
                if ( argc >= 3 )
                {
                    tidyOptSetValue( tdoc, TidyLanguage, argv[2] );
                    --argc;
                    ++argv;
                }
            }
#endif

            else if ( strcasecmp(arg, "output") == 0 ||
                      strcasecmp(arg, "-output-file") == 0 ||
                      strcasecmp(arg, "o") == 0 )
            {
                if ( argc >= 3 )
                {
                    tidyOptSetValue( tdoc, TidyOutFile, argv[2] );
                    --argc;
                    ++argv;
                }
            }
            else if ( strcasecmp(arg,  "file") == 0 ||
                      strcasecmp(arg, "-file") == 0 ||
                      strcasecmp(arg,     "f") == 0 )
            {
                if ( argc >= 3 )
                {
                    errfil = argv[2];
                    errout = tidySetErrorFile( tdoc, errfil );
                    --argc;
                    ++argv;
                }
            }
            else if ( strcasecmp(arg,  "wrap") == 0 ||
                      strcasecmp(arg, "-wrap") == 0 ||
                      strcasecmp(arg,     "w") == 0 )
            {
                if ( argc >= 3 )
                {
                    uint wraplen = 0;
                    sscanf( argv[2], "%u", &wraplen );
                    tidyOptSetInt( tdoc, TidyWrapLen, wraplen );
                    --argc;
                    ++argv;
                }
            }
            else if ( strcasecmp(arg,  "version") == 0 ||
                      strcasecmp(arg, "-version") == 0 ||
                      strcasecmp(arg,        "v") == 0 )
            {
                version();
                tidyRelease( tdoc );
                return 0;  /* success */

            }
            else if ( strncmp(argv[1], "--", 2 ) == 0)
            {
                if ( tidyOptParseValue(tdoc, argv[1]+2, argv[2]) )
                {
                    /* Set new error output stream if setting changed */
                    ctmbstr post = tidyOptGetValue( tdoc, TidyErrFile );
                    if ( post && (!errfil || !samefile(errfil, post)) )
                    {
                        errfil = post;
                        errout = tidySetErrorFile( tdoc, post );
                    }

                    ++argv;
                    --argc;
                }
            }

#if SUPPORT_ACCESSIBILITY_CHECKS
            else if ( strcasecmp(arg, "access") == 0 )
            {
                if ( argc >= 3 )
                {
                    uint acclvl = 0;
                    sscanf( argv[2], "%u", &acclvl );
                    tidyOptSetInt( tdoc, TidyAccessibilityCheckLevel, acclvl );
                    --argc;
                    ++argv;
                }
            }
#endif

            else
            {
                uint c;
                ctmbstr s = argv[1];

                while ( c = *++s )
                {
                    switch ( c )
                    {
                    case 'i':
                        tidyOptSetInt( tdoc, TidyIndentContent, TidyAutoState );
                        if ( tidyOptGetInt(tdoc, TidyIndentSpaces) == 0 )
                            tidyOptResetToDefault( tdoc, TidyIndentSpaces );
                        break;

                    /* Usurp -o for output file.  Anyone hiding end tags?
                    case 'o':
                        tidyOptSetBool( tdoc, TidyHideEndTags, yes );
                        break;
                    */

                    case 'u':
                        tidyOptSetBool( tdoc, TidyUpperCaseTags, yes );
                        break;

                    case 'c':
                        tidyOptSetBool( tdoc, TidyMakeClean, yes );
                        break;

                    case 'b':
                        tidyOptSetBool( tdoc, TidyMakeBare, yes );
                        break;

                    case 'n':
                        tidyOptSetBool( tdoc, TidyNumEntities, yes );
                        break;

                    case 'm':
                        tidyOptSetBool( tdoc, TidyWriteBack, yes );
                        break;

                    case 'e':
                        tidyOptSetBool( tdoc, TidyShowMarkup, no );
                        break;

                    case 'q':
                        tidyOptSetBool( tdoc, TidyQuiet, yes );
                        break;

                    default:
                        unknownOption( c );
                        break;
                    }
                }
            }

            --argc;
            ++argv;
            continue;
        }

        if ( argc > 1 )
        {
            htmlfil = argv[1];
            if ( tidyOptGetBool(tdoc, TidyEmacs) )
                tidyOptSetValue( tdoc, TidyEmacsFile, htmlfil );
            status = tidyParseFile( tdoc, htmlfil );
        }
        else
        {
            htmlfil = "stdin";
            status = tidyParseStdin( tdoc );
        }

        if ( status >= 0 )
            status = tidyCleanAndRepair( tdoc );

        if ( status >= 0 )
            status = tidyRunDiagnostics( tdoc );

        if ( status > 1 ) /* If errors, do we want to force output? */
            status = ( tidyOptGetBool(tdoc, TidyForceOutput) ? status : -1 );

        if ( status >= 0 && tidyOptGetBool(tdoc, TidyShowMarkup) )
        {
            if ( tidyOptGetBool(tdoc, TidyWriteBack) && argc > 1 )
                status = tidySaveFile( tdoc, htmlfil );
            else
            {
                ctmbstr outfil = tidyOptGetValue( tdoc, TidyOutFile );
                if ( outfil )
                    status = tidySaveFile( tdoc, outfil );
                else
                    status = tidySaveStdout( tdoc );
            }
        }

        contentErrors   += tidyErrorCount( tdoc );
        contentWarnings += tidyWarningCount( tdoc );
        accessWarnings  += tidyAccessWarningCount( tdoc );

        --argc;
        ++argv;

        if ( argc <= 1 )
            break;
    }

    if (!tidyOptGetBool(tdoc, TidyQuiet) &&
        errout == stderr && !contentErrors)
        fprintf(errout, "\n");

    if (contentErrors + contentWarnings > 0 && 
         !tidyOptGetBool(tdoc, TidyQuiet))
        tidyErrorSummary(tdoc);

    if (!tidyOptGetBool(tdoc, TidyQuiet))
        tidyGeneralInfo(tdoc);

    /* called to free hash tables etc. */
    tidyRelease( tdoc );

    /* return status can be used by scripts */
    if ( contentErrors > 0 )
        return 2;

    if ( contentWarnings > 0 )
        return 1;

    /* 0 signifies all is ok */
    return 0;
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
