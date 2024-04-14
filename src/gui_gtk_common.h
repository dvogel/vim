/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *				Motif support by Robert Webb
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

#ifndef __GUI_GTK_COMMON__H__
#define __GUI_GTK_COMMON__H__

typedef gboolean timeout_cb_type;
guint gui_gtk_timeout_add(int, timeout_cb_type (*callback)(gpointer), int *);
void gui_gtk_timeout_remove(guint);

extern char **gui_argv;

extern const char *x11_role_argument;
extern const char *gnome_restart_command;
extern char *gnome_abs_restart_command;
extern int found_iconic_arg;

int gui_gtk_modifiers_gdk2mouse(guint);
int gui_gtk_modifiers_gdk2vim(guint);
void gui_gtk_process_button_press(GtkWidget *, guint32, GdkEventType, GdkModifierType, guint, int, int);
void gui_gtk_process_button_release(GtkWidget *, guint32, GdkEventType, GdkModifierType, guint, int, int);
void gui_gtk_process_motion_notify(int, int, GdkModifierType);
gint gui_gtk_process_key_press(guint, guint);

# if GTK_CHECK_VERSION(3,0,0)
GdkRGBA color_to_rgba(guicolor_T);
# endif
gboolean gui_gtk_draw_gui_surface_to_widget_surface(cairo_t *);
void gui_gtk_surface_copy_rect(int, int, int,  int, int,  int);
void gui_gtk_rebuild_text_context();

void gui_gtk_focus_leave();
void gui_gtk_focus_enter(GtkWidget *);
void gui_gtk_pointer_leave();
void gui_gtk_pointer_enter();

void gui_gtk_settings_xft_dpi_changed_cb(GtkSettings *, GParamSpec *, gpointer);
void gui_gtk_mainwin_destroy_cb(GObject *, gpointer);
void gui_mch_set_text_area_pos(int, int, int, int);

const char *gui_gtk_convert_localized_message(char_u **, const char *);

#ifdef FEAT_GUI_TABLINE
gboolean gui_gtk_should_ignore_tabline_event();
void gui_gtk_ignore_tabline_event(gboolean);
#endif

#ifdef FEAT_TOOLBAR
GtkWidget *gui_gtk_create_menu_icon(vimmenu_T *, GtkIconSize);
#endif

#ifdef FEAT_MENU
char_u *gui_gtk_translate_mnemonic_tag(char_u *, int);
#endif

#endif
