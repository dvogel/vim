/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

/*
 * The GtkForm widget performs layout necessary to reflect VIM's internal
 * layout while remaining compatible with the GTK4. Specifically, GtkForm
 * reports synthetic minimum dimensions to it's parent widget rather than
 * measuring it's children's minimum dimensions. This allows the window to be
 * resized smaller than the current fixed size of the VIM text area.
 */

#if defined(USE_GTK4)

#ifndef __GTK4_FORM_H__
#define __GTK4_FORM_H__


# include <gtk/gtk.h>

# ifdef __cplusplus
extern "C" {
# endif

# define GTKVIM_TYPE_FORM_LAYOUT	(gui_gtk_form_layout_get_type ())
G_DECLARE_FINAL_TYPE(GtkVimFormLayout, gui_gtk_form_layout, GTKVIM, FORM_LAYOUT, GtkFixedLayout)

GtkLayoutManager *gui_gtk_form_layout_new(void);


# define GTK_TYPE_FORM		       (gui_gtk_form_get_type ())
G_DECLARE_FINAL_TYPE(GtkForm, gui_gtk_form, GTK, FORM, GtkWidget)

GType gui_gtk_form_get_type(void);

GtkForm *gui_gtk_form_new(void);

void gui_gtk_form_put(GtkForm * form, GtkWidget * widget, gint x, gint y);

void gui_gtk_form_remove_child(GtkForm * form, GtkWidget * widget);

void gui_gtk_form_move(GtkForm *form, GtkWidget * widget, gint x, gint y);

void gui_gtk_form_move_resize(GtkForm * form, GtkWidget * widget, gint x, gint y, gint w, gint h);

// These disable and enable moving and repainting respectively.  If you
// want to update the layout's offsets but do not want it to repaint
// itself, you should use these functions.

void gui_gtk_form_freeze(GtkForm *form);
void gui_gtk_form_thaw(GtkForm *form);


# ifdef __cplusplus
}
# endif

#endif	// __GTK4_FORM_H__

#endif // defined(USE_GTK4)
