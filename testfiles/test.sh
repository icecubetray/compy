#!/bin/sh

CASE="$@"

../bin/Debug/school-compressor -c -i "${CASE}.vec" -o "${CASE}.vec.sca" --log="${CASE}.sca.log"
../bin/Debug/school-compressor -d -i "${CASE}.vec.sca" -o "${CASE}.vec.sca.out" --log="${CASE}.sca.out.log"

if diff "${CASE}.vec" "${CASE}.vec.sca.out"; then
	echo success;
else
	echo failed;
fi;
