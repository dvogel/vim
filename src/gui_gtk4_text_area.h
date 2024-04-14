/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

/*
 * The GtkVimTextArea widget is a GtkDrawingArea with different sizing
 * mechanics. In GTK4 the standard sizing mechanics are incompatible with VIM's
 * internal layout mechanics.
 */

#if defined(USE_GTK4)

#ifndef __GUI_GTK4_TEXT_AREA_H__
#define __GUI_GTK4_TEXT_AREA_H__

# include <gtk/gtk.h>

# ifdef __cplusplus
extern "C" {
# endif

# define GTKVIM_TYPE_TEXT_AREA		       (gui_gtk_text_area_get_type ())
G_DECLARE_FINAL_TYPE(GtkVimTextArea, gui_gtk_text_area, GTKVIM, TEXT_AREA, GtkDrawingArea)

GType gui_gtk_text_area_get_type(void);
GtkVimTextArea *gui_gtk_text_area_new(void);
void gui_gtk_text_area_add_context_menu(GtkVimTextArea *, vimmenu_T *);
void gui_gtk_text_area_remove_context_menu(GtkVimTextArea *, vimmenu_T *);

# ifdef __cplusplus
}
# endif

#endif	// __GTK_FORM_H__

#endif // defined(USE_GTK4)
