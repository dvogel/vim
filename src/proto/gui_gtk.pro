/* gui_gtk.c */
GtkWidget *gui_gtk_load_menu_iconfile(char_u *name, GtkIconSize icon_size);
GtkIconTheme *gui_gtk_icon_theme_for_main_window(void);
void gui_gtk_register_stock_icons(void);
void gui_mch_add_menu(vimmenu_T *menu, int idx);
void gui_mch_add_menu_item(vimmenu_T *menu, int idx);
void gui_gtk_set_mnemonics(int enable);
void gui_mch_toggle_tearoffs(int enable);
void gui_mch_menu_set_tip(vimmenu_T *menu);
void gui_mch_destroy_menu(vimmenu_T *menu);
void gui_mch_set_scrollbar_pos(scrollbar_T *sb, int x, int y, int w, int h);
int gui_mch_get_scrollbar_xpadding(void);
int gui_mch_get_scrollbar_ypadding(void);
void gui_mch_create_scrollbar(scrollbar_T *sb, int orient);
void gui_mch_destroy_scrollbar(scrollbar_T *sb);
void gui_mch_show_popupmenu(vimmenu_T *menu);
void gui_make_popup(char_u *path_name, int mouse_pos);
void gui_gtk_find_replace_dialog_create(char_u *arg, int do_replace);
/* vim: set ft=c : */
