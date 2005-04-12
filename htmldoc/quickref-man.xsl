<?xml version="1.0"?>
<!--
    For generating the `tidy-config` man page from output of `tidy -xml-config`

    (c) 2005 (W3C) MIT, ERCIM, Keio University
    See tidy.h for the copyright notice.

    Written by Jelks Cabaniss

  CVS Info :

    $Author: arnaud02 $
    $Date: 2005/04/12 16:20:19 $
    $Revision: 1.1 $

-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:strip-space elements="description" />

<xsl:output method="text" />


<xsl:template match="/">.\" tidy-config man page for the Tidy Sourceforge project
.TH tidy-config 7 "Tidy Configuration Options" "HTML Tidy" "Tidy Configuration Options"
.SH NAME
.B tidy-config
- configuration options for the HTML Tidy program (version: <xsl:value-of select="config/@version" />).
.SH SYNOPSIS
\fBtidy --\fR\fIoption1 \fRvalue1 \fB--\fIoption2 \fRvalue2 [standard options ...]
.br
\fBtidy -config \fIconfig-file \fR[standard options ...]
.SH WARNING
The options detailed here do not include the standard command-line options
which are documented in the \fBtidy\fR man page.
.SH DESCRIPTION
A list of options for configuring the behavior of Tidy, which can be passed
either on the command line, or specified in a configuration file.
.P
A Tidy configuration file is simply a text file, where each option
is listed on a separate line in the form
.LP
.in 1i
\fBoption1\fR: \fIvalue1\fR
.br
\fBoption2\fR: \fIvalue2\fR
.br
etc.
.P
The permissible values for a given option depend on the option's \fBType\fR.  There are five types: \fIBoolean\fR, \fIAutoBool\fR, \fIDocType\fR, \fIEnum\fR, and \fIString\fR. Boolean types allow any of \fIyes/no, y/n, true/false, t/f, 1/0\fR.  AutoBools allow \fIauto\fR in addition to the values allowed by Booleans.  Integer types take non-negative integers.  String types generally have no defaults, and you should provide them in non-quoted form (unless you wish the output to contain the literal quotes).
.P
Enum, Encoding, and DocType "types" have a fixed repertoire of items; consult the \fIExample\fR[s] provided below for the option[s] in question.
.P
You only need to provide options and values for those whose defaults you wish to override, although you may wish to include some already-defaulted options and values for the sake of documentation and explicitness.
.P
Here is a sample config file, with at least one example of each of the five Types:
.P
\fI
    // sample Tidy configuration options
    output-xhtml: yes
    add-xml-decl: no
    doctype: strict
    char-encoding: ascii
    indent: auto
    wrap: 76
    repeated-attributes: keep-last
    error-file: errs.txt
\fR
.P
Below is a summary and brief description of each of the options. They are listed alphabetically within each category.  There are five categories: \fIHTML, XHTML, XML\fR options, \fIDiagnostics\fR options, \fIPretty Print\fR options, \fICharacter Encoding\fR options, and \fIMiscellaneous\fR options.
.P
.SH OPTIONS
<xsl:call-template name="show-options" />
.SH "SEE ALSO"
.B tidy(1)
.SH AUTHOR
\fBTidy\fR was written by Dave Raggett &lt;\fIdsr@w3.org\fR&gt;, and is now maintained and developed by the Tidy team at \fIhttp://tidy.sourceforge.net/\fR.  It is released under the \fIMIT Licence\fR.
.P
Generated automatically with HTML Tidy released on <xsl:value-of select="config/@version" />.
</xsl:template>


<!-- Named Templates: -->

<xsl:template name="show-options">
.SS HTML, XHTML, XML options:
  <xsl:call-template name="detail">
     <xsl:with-param name="category">markup</xsl:with-param>
  </xsl:call-template>
.SS Diagnostics options:
  <xsl:call-template name="detail">
     <xsl:with-param name="category">diagnostics</xsl:with-param>
  </xsl:call-template>
.SS Pretty Print options:
  <xsl:call-template name="detail">
     <xsl:with-param name="category">print</xsl:with-param>
  </xsl:call-template>
.SS Character Encoding options:
  <xsl:call-template name="detail">
     <xsl:with-param name="category">encoding</xsl:with-param>
  </xsl:call-template>
.SS Miscellaneous options:
  <xsl:call-template name="detail">
     <xsl:with-param name="category">misc</xsl:with-param>
  </xsl:call-template>
</xsl:template>


<xsl:template name="detail">
<!--
For each option in one of the 5 categories/classes, provide its
    1. name
    2. type
    3. default (if any)
    4. example (if any)
    5. seealso (if any)
    6. description
-->
  <xsl:param name="category" />
    <xsl:for-each select='/config/option[@class=$category]'>
       <xsl:sort select="name" order="ascending" />
.P
\fB<xsl:apply-templates select="name" />\fR
.LP
.in 1i
Type:    \fI<xsl:apply-templates select="type" />\fR
.br
<xsl:call-template name="provide-default" />
.br
<xsl:call-template name="provide-example" />
.LP
.in 1i
<xsl:apply-templates select="description" />
<xsl:call-template name="seealso" />
    </xsl:for-each>
</xsl:template>


<xsl:template name="seealso">
  <xsl:if test="seealso">
.P
.rj 1
\fBSee also\fR: <xsl:text />
    <xsl:for-each select="seealso">
      <xsl:text />\fI<xsl:value-of select="." />\fR<xsl:text />
      <xsl:if test="position() != last()">
        <xsl:text>, </xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:if>
</xsl:template>

<xsl:template name="provide-default">
<!--
Picks up the default from the XML.  If the `default` element
doesn't exist, or it's empty, a single '-' is provided.
-->
  <xsl:choose>
    <xsl:when test="string-length(default) &gt; 0 ">
      <xsl:text />Default: \fI<xsl:apply-templates
        select="default" />\fR<xsl:text />
    </xsl:when>
    <xsl:otherwise>
      <xsl:text />Default: \fI-\fR<xsl:text />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="provide-example">
<!--
By default, doesn't output examples for String types (mirroring the
quickref page).  But for *any* options in the XML instance that
have an `example` child, that example will be used in lieu of a
stylesheet-provided one. (Useful e.g. for `repeated-attributes`).
-->
  <xsl:choose>
    <xsl:when test="string-length(example) &gt; 0">
      <xsl:text />Example: \fI<xsl:apply-templates
          select="example" />\fR<xsl:text />
    </xsl:when>
    <xsl:otherwise>
      <xsl:text />Default: \fI-\fR<xsl:text />
    </xsl:otherwise>
  </xsl:choose>
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
