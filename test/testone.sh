#! /bin/sh

# set -x

echo Testing $1
INHTML=./input/in_$1.html
CFGFILE=./input/cfg_$1.txt
TIDYHTML=./tmp/out_$1.html
OUTHTML=./output/out_$1.html

MSGFILE=./tmp/msg_$1.txt
DIFFOUT=./tmp/diff_$1.txt

REPORTWARN=$2
shift
if [ $REPORTWARN ]
then
  shift
fi

# Remove any pre-exising test outputs
for INFIL in $MSGFILE $TIDYHTML $DIFFOUT
do
  if [ -f $INFIL ]
  then
    rm $INFIL
  fi
done

# If no test specific config file, use default.
if [ ! -f $CFGFILE ]
then
  CFGFILE=./input/cfg_default.txt
fi

../tidy -config $CFGFILE "$@" $INHTML > $TIDYHTML 2> $MSGFILE
STATUS=$?

if [ $STATUS -gt 1 ]
then
  cat $MSGFILE
  exit $STATUS
fi

if [ $REPORTWARN ] 
then
  if [ $STATUS -gt 0 ]
  then
    cat $MSGFILE
    exit $STATUS
  fi
fi


if [ ! -s $TIDYHTML ]
then
  cat err.txt
  exit 1
fi

if [ -f $OUTHTML ]
then
  diff $TIDYHTML $OUTHTML > $DIFFOUT

  # Not a valid shell test
  if [ -s diff.txt ]
  then
    cat err.txt
    cat diff.txt
    exit 1
  fi
fi

exit 0

