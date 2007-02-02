#!/bin/sh

SMTPAUTHUSER="joe@domain.com" ./authlogger >/tmp/TESTAUTHLOGGER.$$ 2>&1 
R=$(cat /tmp/TESTAUTHLOGGER.$$)

if [ ! "${R}" = "authlogger: pid $$ - authenticated as joe@domain.com" ]
then
	echo "FAILED TEST authlogger 1"
	exit 1
fi

SMTPAUTHUSER= ./authlogger >/tmp/TESTAUTHLOGGER.$$ 2>&1 
R=$(cat /tmp/TESTAUTHLOGGER.$$)

if [ ! -z "${R}" ]
then
	echo "FAILED TEST authlogger 2"
	exit 1
fi

echo "TEST AUTHLOGGER PASSED"

