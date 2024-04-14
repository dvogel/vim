#!/bin/bash

VIMRUNTIME="$PWD/runtime"
export VIMRUNTIME

declare -a prog_args
prog_args=(-f -g -N --clean -U gtk4_vimrc)

if [[ "$1" == "--debug" ]]; then
  shift
  gdbscript="$1"
  shift
  prog_args+=("$@")
  gdb ./src/vim -x "$gdbscript" -ex "run ${prog_args[*]}"
else
	./src/vim "${prog_args[@]}"
fi

