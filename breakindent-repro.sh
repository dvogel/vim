#!/bin/bash

set -o errexit
shopt -s inherit_errexit

export RUN_COUNT="${RUN_COUNT:-50}"
export TEST_FILTER='\(Test_breakindent20_cpo_n_nextpage\|Test_cursor_position_with_showbreak\)'
export DISPLAY=:99 make test_breakindent
export GUI_FLAG=-g
export LIBGL_ALWAYS_SOFTWARE=true
export LIBGL_ALWAYS_INDIRECT=true

rm junk.txt
for x in `seq 1 $RUN_COUNT`; do
  ( echo; echo; echo "################ $x #################"; echo) | tee -a junk.txt
  if ! make test_breakindent; then
    break;
  fi;
done
