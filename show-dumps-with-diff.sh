#!/bin/bash

set -o errexit
shopt -s inherit_errexit

show_dump() {
  if [[ -e "$1" ]]; then
    echo
    echo "### $1:"
    cat "$1"
    echo
  fi
}

tmpout="$(mktemp)"
VIMRUNTIME=../../runtime vim --clean --cmd "call term_dumpdiff(\"failed/$1.dump\", \"dumps/$1.dump\") | w! $tmpout | qall!"
cat "$tmpout"

for dir in dumps failed; do
  show_dump "$dir/$1.dump"
done

