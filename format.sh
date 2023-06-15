#!/usr/bin/env bash

WORK_DIR=$(dirname "$(realpath "$0")")
cd "$WORK_DIR" || exit 1

while read -r filename; do
    clang-format -i $filename
done < <(tree -aif --noreport | grep -v third_party | grep -E "\.cpp$|\.h$")
