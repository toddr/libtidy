/*
  cmmdline.c - HTML Tidy command line driver

  Copyright (c) 1998-2002 World Wide Web Consortium
  (Massachusetts Institute of Technology, Institut National de
  Recherche en Informatique et en Automatique, Keio University).
  All Rights Reserved.

  CVS Info :

    $Author: creitzel $ 
    $Date: 2002/07/04 00:12:08 $ 
    $Revision: 1.1 $ 
*/

#include "tidy.h"

uint  contentErrors = 0;
uint  contentWarnings = 0;
uint  optionErrors = 0;
uint  accessWarnings = 0;

FILE* errout = stderr;  /* set to stderr or stdout */
FILE* input = stdin;

void help( TidyDoc tdoc, ctmbstr prog )
{
}

void optionhelp( TidyDoc tdoc, ctmbstr prog )
{
}

void optionvalues( TidyDoc tdoc, ctmbstr prog )
{
}

void version( TidyDoc tdoc, ctmbstr prog )
{
}
void unknownOption( TidyDoc tdoc, uint c )
{
}

int main( int argc, char** argv )
{
    ctmbstr prog = argv[0];
    ctmbstr cfgfil = null, errfil = null, htmlfil = null;
    TidyDoc tdoc = tidyCreate();
    int status = 0;
    Bool quiet = no, writeBack = no;

    /* look for env var "HTML_TIDY" */
    /* then for ~/.tidyrc (on Unix) */

    if ( cfgfil = getenv("HTML_TIDY") )
        tidyLoadConfig( tdoc, cfgfil );
#ifdef SUPPORT_GETPWNAM
    else
        tidyLoadConfig( tdoc, "~/.tidyrc" );
#endif /* SUPPORT_GETPWNAM */

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
            else if (strcasecmp(arg,   "ashtml") == 0 )
                tidyOptSetBool( tdoc, TidyHtmlOut, yes );

            else if (strcasecmp(arg, "indent") == 0 )
            {
                tidyOptSetBool( tdoc, TidyIndentContent, yes );
                tidyOptSetBool( tdoc, TidySmartIndent, yes );
            }
            else if (strcasecmp(arg, "omit") == 0 )
                tidyOptSetBool( tdoc, TidyHideEndTags, yes );

            else if (strcasecmp(arg, "upper") == 0 )
                tidyOptSetBool( tdoc, TidyUpperCaseTags, yes );

            else if (strcasecmp(arg, "clean") == 0 )
                tidyOptSetBool( tdoc, TidyMakeClean, yes );

            else if (strcasecmp(arg, "bare") == 0 )
                tidyOptSetBool( tdoc, TidyMakeBare, yes );

            else if ( strcasecmp(arg, "raw") == 0     ||
                      strcasecmp(arg, "ascii") == 0   ||
                      strcasecmp(arg, "latin1") == 0  ||
                      strcasecmp(arg, "utf8") == 0    ||
                      strcasecmp(arg, "iso2022") == 0 ||
#if SUPPORT_UTF16_ENCODINGS
                      strcasecmp(arg, "utf16le") == 0 ||
                      strcasecmp(arg, "utf16be") == 0 ||
                      strcasecmp(arg, "utf16") == 0   ||
#endif
#if SUPPORT_ASIAN_ENCODINGS
                      strcasecmp(arg, "shiftjis") == 0 ||
                      strcasecmp(arg, "big5") == 0     ||
#endif
                      strcasecmp(arg, "mac") == 0     ||
                      strcasecmp(arg, "win1252") == 0 )
            {
                tidySetCharEncoding( tdoc, arg );
            }
            else if (strcasecmp(arg, "numeric") == 0 )
                tidyOptSetBool( tdoc, TidyNumEntities, yes );

            else if ( strcasecmp(arg, "modify") == 0 ||
                      strcasecmp(arg, "change") == 0 ||  /* obsolete */
                      strcasecmp(arg, "update") == 0 )   /* obsolete */
            {
                tidyOptSetBool( tdoc, TidyWriteBack, writeBack = yes );
            }
            else if (strcasecmp(arg, "errors") == 0 )
                tidyOptSetBool( tdoc, TidyOnlyErrors, yes );

            else if (strcasecmp(arg, "quiet") == 0 )
                tidyOptSetBool( tdoc, TidyQuiet, quiet = yes );

            else if ( strcasecmp(arg, "help") == 0 ||
                      strcasecmp(arg,    "h") == 0 || *arg == '?' )
            {
                help( tdoc, prog );
                tidyRelease( tdoc );
                return 0; /* success */
            }
            else if ( strcasecmp(arg, "help-config") == 0 )
            {
                optionhelp( tdoc, prog );
                tidyRelease( tdoc );
                return 0; /* success */
            }
            else if ( strcasecmp(arg, "show-config") == 0 )
            {
                optionvalues( tdoc, prog );
                tidyRelease( tdoc );
                return 0; /* success */
            }
            else if ( strcasecmp(arg, "config") == 0 )
            {
                if ( argc >= 3 )
                {
                    tidyLoadConfig( tdoc, argv[2] );
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

            else if ( strcasecmp(arg,  "file") == 0 ||
                      strcasecmp(arg, "-file") == 0 ||
                      strcasecmp(arg,     "f") == 0 )
            {
                if ( argc >= 3 )
                {
                    errfil = argv[2];
                    tidySetErrorFile( tdoc, errfil );
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
                    sscanf( argv[2], "%d", &wraplen );
                    tidyOptSetInt( tdoc, TidyWrapLen, wraplen );
                    --argc;
                    ++argv;
                }
            }
            else if ( strcasecmp(arg,  "version") == 0 ||
                      strcasecmp(arg, "-version") == 0 ||
                      strcasecmp(arg,        "v") == 0 )
            {
                version( tdoc, prog );
                tidyRelease( tdoc );
                return 0;  /* success */

            }
            else if ( strncmp(argv[1], "--", 2 ) == 0)
            {
                if ( tidyOptParseValue(tdoc, argv[1]+2, argv[2]) )
                {
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
                    sscanf( argv[2], "%d", &acclvl );
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
                        tidyOptSetBool( tdoc, TidyIndentContent, yes );
                        tidyOptSetBool( tdoc, TidySmartIndent, yes );
                        break;

                    case 'o':
                        tidyOptSetBool( tdoc, TidyHideEndTags, yes );
                        break;

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
                        tidyOptSetBool( tdoc, TidyWriteBack, writeBack = yes );
                        break;

                    case 'e':
                        tidyOptSetBool( tdoc, TidyOnlyErrors, yes );
                        break;

                    case 'q':
                        tidyOptSetBool( tdoc, TidyQuiet, quiet = yes );
                        break;

                    default:
                        unknownOption( tdoc, c );
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

        if ( status >= 0 )
        {
            if ( writeBack )
                status = tidySaveFile( tdoc, htmlfil );
            else
                status = tidySaveStdout( tdoc );

        }

        contentErrors   += tidyErrorCount( tdoc );
        contentWarnings += tidyWarningCount( tdoc );
        accessWarnings  += tidyAccessWarningCount( tdoc );

        --argc;
        ++argv;

        if ( argc <= 1 )
            break;
    }

    if ( contentErrors + contentWarnings > 0 && !quiet )
        tidyGeneralInfo( tdoc );

    if ( errout != stderr )
        fclose( errout );

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

