#!/bin/sh

CASE="$@"

../bin/compy -c -i "${CASE}.vec" -o "${CASE}.vec.sca" --log="${CASE}.vec.sca.log"
../bin/compy -d -i "${CASE}.vec.sca" -o "${CASE}.vec.sca.out" --log="${CASE}.vec.sca.out.log"

if diff "${CASE}.vec" "${CASE}.vec.sca.out"; then
	echo success;
else
	echo failed;
fi;
