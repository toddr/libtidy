<?xml version="1.0"?>
<!--
    For generating the `tidy.1` man page from output of `tidy -xml-help`

    (c) 2005 (W3C) MIT, ERCIM, Keio University
    See tidy.h for the copyright notice.

    Written by Jelks Cabaniss and Arnaud Desitter

  CVS Info :

    $Author: arnaud02 $
    $Date: 2005/04/14 15:23:38 $
    $Revision: 1.1 $

-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:strip-space elements="description" />

<xsl:output method="text" />


<xsl:template match="/">.\" tidy man page for the Tidy Sourceforge project
.TH tidy 1 "$Date: 2005/04/14 15:23:38 $" "HTML Tidy <xsl:value-of select="cmdline/@version" />" "User commands"
.SH NAME
.B tidy
\- validate, correct, and pretty-print HTML files (version: <xsl:value-of select="cmdline/@version" />).
.SH SYNOPSIS
\fBtidy\fR [option ...] [file ...] [option ...] [file ...]
.SH DESCRIPTION
Tidy reads HTML, XHTML and XML files and writes cleaned up
markup.  For HTML variants, it detects and corrects many common
coding errors and strives to produce visually equivalent
markup that is both W3C complaint and works on most browsers.
A common use of Tidy is to convert plain HTML to XHTML.  For
generic XML files, Tidy is limited to correcting basic well-formedness
errors and pretty printing.
.P
If no markup file is specified, Tidy reads the standard
input.  If no output file is specified, Tidy writes markup
to the standard output.  If no error file is specified, Tidy
writes messages to the standard error.
.SH OPTIONS
<xsl:call-template name="show-options" />
.SH USAGE
.P
Use --blah blarg for any configuration option "blah" with argument "blarg"
.P
Input/Output default to stdin/stdout respectively
Single letter options apart from -f and -o may be combined
as in:  tidy -f errs.txt -imu foo.html
For further info on HTML see http://www.w3.org/MarkUp
.P
For more information about HTML Tidy, visit the project home page at
http://tidy.sourceforge.net.  Here, you will find links to documentation,
mailing lists (with searchable archives) and links to report bugs.
.SH ENVIRONMENT
.TP
.B HTML_TIDY
Name of the default configuration file.  This should be an absolute path,
since you will probably invoke
.B tidy
from different directories.  The value of HTML_TIDY will be parsed after the
compiled-in default (defined with -DCONFIG_FILE), but before any of the
files specified using
.BR -config .
.SH "EXIT STATUS"
.IP 0
All input files were processed successfully.
.IP 1
There were warnings.
.IP 2
There were errors.
.SH "SEE ALSO"
.B tidy-config(7)
.P
HTML Tidy Project Page at \fIhttp://tidy.sourceforge.net\fR
.SH AUTHOR
\fBTidy\fR was written by Dave Raggett &lt;\fIdsr@w3.org\fR&gt;, and is now maintained and developed by the Tidy team at \fIhttp://tidy.sourceforge.net/\fR.  It is released under the \fIMIT Licence\fR.
.P
Generated automatically with HTML Tidy released on <xsl:value-of select="cmdline/@version" />.
</xsl:template>


<!-- Named Templates: -->

<xsl:template name="show-options">
.SS File manipulation
  <xsl:call-template name="detail">
     <xsl:with-param name="category">file-manip</xsl:with-param>
  </xsl:call-template>
.SS Processing directives
  <xsl:call-template name="detail">
     <xsl:with-param name="category">process-directives</xsl:with-param>
  </xsl:call-template>
.SS Character encodings
  <xsl:call-template name="detail">
     <xsl:with-param name="category">char-encoding</xsl:with-param>
  </xsl:call-template>
.SS Miscellaneous
  <xsl:call-template name="detail">
     <xsl:with-param name="category">misc</xsl:with-param>
  </xsl:call-template>
</xsl:template>


<xsl:template name="detail">
<!--
For each option in one of the 3 categories/classes, provide its
    1. names
    2. description
    3. equivalent configuration option
-->
  <xsl:param name="category" />
    <xsl:for-each select='/cmdline/option[@class=$category]'>
<xsl:text>
.TP
</xsl:text>
       <xsl:call-template name="process-names" />
       <xsl:text>
</xsl:text>
       <xsl:apply-templates select="description" />
       <xsl:text>
</xsl:text>
       <xsl:call-template name="process-eqconfig" />
    </xsl:for-each>
</xsl:template>


<xsl:template name="process-names">
  <xsl:if test="name">
    <xsl:for-each select="name">
      <xsl:text />\fB<xsl:value-of select="." />\fR<xsl:text />
      <xsl:if test="position() != last()">
        <xsl:text>, </xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:if>
</xsl:template>


<xsl:template name="process-eqconfig">
  <xsl:if test="string-length(eqconfig) &gt; 0">
   <xsl:for-each select="eqconfig">
     <xsl:text>(\fI</xsl:text>
     <xsl:value-of select="." />
     <xsl:text>\fR)</xsl:text>
   </xsl:for-each>
  </xsl:if>
</xsl:template>

<xsl:template name="escape-backslash">
<!--
Since backslashes are "special" to the *roff processors used
to generate man pages, we need to escape backslash characters
appearing in content with another backslash.
-->
  <xsl:choose>
    <xsl:when test="contains(.,'\')">
      <xsl:value-of select=
        "concat( substring-before(.,'\'), '\\', substring-after(.,'\') )" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- Regular Templates: -->

<xsl:template match="description">
   <xsl:apply-templates />
</xsl:template>

<xsl:template match="a">
   <xsl:apply-templates />
   <xsl:text /> at \fI<xsl:value-of select="@href" />\fR<xsl:text />
</xsl:template>

<xsl:template match="code | em">
   <xsl:text />\fI<xsl:call-template name="escape-backslash" />\fR<xsl:text />
</xsl:template>

<xsl:template match="br">
   <xsl:text>
.br
</xsl:text>
</xsl:template>

<xsl:template match="strong">
   <xsl:text />\fB<xsl:call-template name="escape-backslash" />\fR<xsl:text />
</xsl:template>


<!--
The following templates
  a) normalize whitespace, primarily necessary for `description`
  b) do so without stripping possible whitespace surrounding `code`
  d) strip leading and trailing whitespace in 'description` and `code`
(courtesy of Ken Holman on the XSL-list):
-->

<xsl:template match="text()[preceding-sibling::node() and
                             following-sibling::node()]">
   <xsl:variable name="ns" select="normalize-space(concat('x',.,'x'))"/>
   <xsl:value-of select="substring( $ns, 2, string-length($ns) - 2 )" />
</xsl:template>

<xsl:template match="text()[preceding-sibling::node() and
                             not( following-sibling::node() )]">
   <xsl:variable name="ns" select="normalize-space(concat('x',.))"/>
   <xsl:value-of select="substring( $ns, 2, string-length($ns) - 1 )" />
</xsl:template>

<xsl:template match="text()[not( preceding-sibling::node() ) and
                             following-sibling::node()]">
   <xsl:variable name="ns" select="normalize-space(concat(.,'x'))"/>
   <xsl:value-of select="substring( $ns, 1, string-length($ns) - 1 )" />
</xsl:template>

<xsl:template match="text()[not( preceding-sibling::node() ) and
                             not( following-sibling::node() )]">
   <xsl:value-of select="normalize-space(.)"/>
</xsl:template>

</xsl:stylesheet>
