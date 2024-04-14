#!/usr/bin/env bash

list_functions() {
    grep -h -E '^(gui_)?mch_' "$@" | sed -r -e 's/\s+UNUSED//' | uniq | sort
}

# diff -u --color=always <(grep -h -E '^(gui_)?mch_' gui_gtk_x11.c gui_gtk.c | sed -r -e 's/\s+UNUSED//' | uniq | sort) <(grep -E '^(gui_)?mch_' gui_gtk4.c | sed -r -e 's/\s+UNUSED//' | uniq | sort)

diff -u --color=always <(list_functions src/gui_gtk_x11.c src/gui_gtk.c) <(list_functions src/gui_gtk4.c)

