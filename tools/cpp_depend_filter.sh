#!/bin/sh

remove_line_folds() {
  sed -e 's+\\$++' \
    | tr -d '\n' \
    | sed -e 's/[ ]\{2,\}/ /g'
}

reindent_lines() {
  sed -e 's/^/ /' \
    | tail -c +2
}

reescape_eol() {
  # head removes the final line ending and the backslach escape character just
  # before it. The echo simply recreates the line ending.
  sed -e 's+$+\\+' \
    | head --bytes=-1
  echo
}

refold_lines() {
  # This initial fold is at 75 characters because Makefile sets tw=78 and the
  # folded lines will be indented by 1 space and most lines will have a space
  # and backslash appended to escape the line ending.
  fold -s -w75 \
    | reindent_lines \
    | reescape_eol
}

prepend_objects_dir() {
  sed -e 's+^\([^ ]*\.o\)+objects/'"$1"'\1+'
}

remove_proto_depends() {
  sed -e 's+proto/[^ ]*\.pro[ ]*++g'
}

add_osdef_before_vim_h() {
  sed -e 's+: \(.*\)vim.h+: auto/osdef.h \1vim.h+'
}

remove_line_folds \
  | add_osdef_before_vim_h \
  | prepend_objects_dir "$1" \
  | remove_proto_depends \
  | refold_lines
