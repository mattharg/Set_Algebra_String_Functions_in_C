#!/bin/ksh
ME="Expected Result";

USAGE='Usage:  $0 [-n] num "Commands"
  Where -n means dont halt on errors, and
  num is the expected return code, and
  "Commands" are the commands to execute
';
##set -x;
print; # Nice Spacing...

if [ "$1" == "-n" ] ; then
	shift;
	print "$ME: Advise: Not Halting (returning non-zero)";
	DONT_HALT="true";
fi;

expectedReturn="$1"; shift;
cmmds="$*";
eval $cmmds;
RT=$?;
if [ "$RT" != "$expectedReturn" ] ; then
	print "$ME: *************Error*************: Expected: $expectedReturn; Received: $RT";
	print "$ME: *************Error*************: Performing: $cmmds";
	print; print;
	if [ -z "$DONT_HALT" ] ; then
		return 1;
	fi
	return 0;
else
	print "$ME: Info: OK: Expected Result Received: $ExpectedResult";
	return 0;
fi

