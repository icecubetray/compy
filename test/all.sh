#!/bin/sh


ABS_DIR="$(cd -- "$(dirname -- "$0")" && pwd)";


for CASE in `find "${ABS_DIR}/files" \( -type f -or -type l \) -name '*.vec' | sed 's/.*\///' | sed 's/\(.*\)\.vec/\1/'`; do
	"${ABS_DIR}/case.sh" "$CASE"
done;
