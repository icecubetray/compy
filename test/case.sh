#!/bin/bash

ABS_DIR="$(cd -- "$(dirname -- "$0")" && pwd)";
CASE_NAME="${@}";
CASE="${ABS_DIR}/files/${CASE_NAME}";

"${ABS_DIR}/../bin/compy" -c -i "${CASE}.vec" -o "${CASE}.vec.sca" --log="${CASE}.vec.sca.log"
"${ABS_DIR}/../bin/compy" -d -i "${CASE}.vec.sca" -o "${CASE}.vec.sca.out" --log="${CASE}.vec.sca.out.log"

if diff "${CASE}.vec" "${CASE}.vec.sca.out"; then
	echo "${CASE_NAME}: successful";
else
	echo "${CASE_NAME}: failed";
fi;
