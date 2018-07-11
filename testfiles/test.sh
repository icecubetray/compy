#!/bin/sh

CASE="$@"

../bin/compy -c -i "${CASE}.vec" -o "${CASE}.vec.sca" --log="${CASE}.sca.log"
../bin/compy -d -i "${CASE}.vec.sca" -o "${CASE}.vec.sca.out" --log="${CASE}.sca.out.log"

if diff "${CASE}.vec" "${CASE}.vec.sca.out"; then
	echo success;
else
	echo failed;
fi;
