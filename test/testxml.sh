#! /bin/sh

#
# testxml.sh - execute all XML testcases
#
# (c) 1998-2001 (W3C) MIT, INRIA, Keio University
# See tidy.c for the copyright notice.
#
# <URL:http://tidy.sourceforge.net/>
#
# CVS Info:
#
#    $Author: creitzel $
#    $Date: 2003/02/16 19:33:12 $
#    $Revision: 1.2 $
#
# set -x

VERSION='$Id'

BUGS="427837 431956 433604 433607 433670 434100\
 480406 480701 500236 503436 537604 640474 646946"

for bugNo in ${BUGS}
do
#  echo Testing $bugNo | tee -a testall.log
  ./testone.sh $bugNo "$@" | tee -a testall.log
  mv ./tmp/out_$bugNo.html ./tmp/out_$bugNo.xml
done

