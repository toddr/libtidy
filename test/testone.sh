#! /bin/sh

#
# testone.sh - execute a single testcase
#
# (c) 1998-2003 (W3C) MIT, ERCIM, Keio University
# See tidy.c for the copyright notice.
#
# <URL:http://tidy.sourceforge.net/>
#
# CVS Info:
#
#    $Author: creitzel $
#    $Date: 2003/03/19 18:33:12 $
#    $Revision: 1.7 $
#
# set -x

VERSION='$Id'

echo Testing $1

set +f

TESTNO=$1
EXPECTED=$2
TIDY=../bin/tidy
INFILES=./input/in_$1.*ml
CFGFILE=./input/cfg_$1.txt

TIDYFILE=./tmp/out_$1.html
MSGFILE=./tmp/msg_$1.txt

unset HTML_TIDY

shift
shift

# Remove any pre-exising test outputs
for INFIL in $MSGFILE $TIDYFILE
do
  if [ -f $INFIL ]
  then
    rm $INFIL
  fi
done

for INFILE in $INFILES
do
    if [ -r $INFILE ]
    then
      break
    fi
done

# If no test specific config file, use default.
if [ ! -f $CFGFILE ]
then
  CFGFILE=./input/cfg_default.txt
fi

$TIDY -f $MSGFILE -config $CFGFILE "$@" --tidy-mark no -o $TIDYFILE $INFILE
STATUS=$?

if [ $STATUS -ne $EXPECTED ]
then
  cat $MSGFILE
  exit 1
fi

exit 0

