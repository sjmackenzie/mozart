#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZC=./ozc.sh
#

: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${OZMAFILE="-b $SRCDIR/ozc.ozm"}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}

if test -z "$OZEMULATOR"
then
    for d in \
	$BUILDTOP/platform/emulator \
	$BUILDTOP/platform/emulator/$OZPLATFORM \
	$OZPREFIX/platform/$OZPLATFORM
    do
	if test -x $d/oz.emulator.bin
	then
	    OZEMULATOR=$d/oz.emulator.bin
	    break
	fi
    done
fi

if test -z "$OZEMULATOR"
then
    echo Fatal error: no oz.emulator.bin found
    exit 1
fi

OZINIT=${OZMAINIT}
export OZINIT

echo "Using OZEMULATOR: $OZEMULATOR"
echo "Using OZMAFILE: $OZMAFILE"

exec $OZEMULATOR $OZMAFILE -- "$@"
