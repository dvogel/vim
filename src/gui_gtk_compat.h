#ifndef __GUI_GTK_COMPAT_H__
#define __GUI_GTK_COMPAT_H__

#ifdef USE_GTK4

#define LOG_FUNC_ENTRY fprintf(stderr, "enter:%s\n", __func__)

GdkSurface *gui_gtk_widget_get_containing_surface(GtkWidget *);
void gtk_widget_queue_draw_area(GtkWidget *, int, int, int, int);
# define gtk_widget_set_can_default(w, b) do { } while (0)
# define gtk_window_set_keep_above(w, b) do { } while (0)

gint gtk_dialog_run(GtkDialog *);
gint gtk_native_dialog_run(GtkNativeDialog *);
void gtk_entry_set_text(GtkEntry *, const gchar *);
const char *gtk_entry_get_text(GtkEntry *);


#else
// These macros allow GTK2/3 code to appear to use the GTK4 API for some widget
// operations that share a similar prototype across major versions.
# define gtk_notebook_append_page(notebook, page) gtk_container_add(GTK_CONTAINER(notebook), page)
# define gtk_box_append(box, child) gtk_box_pack_end(GTK_BOX(box), child, FALSE, FALSE, 0)
# define gtk_box_prepend(box, child) gtk_box_pack_start(GTK_BOX(box), child, FALSE, FALSE, 0)
# define gtk_container_set_border_width(cont, pixels) do { } while (0)
# define gdk_key_event_get_keyval(event) event->keyval

void gtk_widget_set_cursor(GtkWidget *, GdkCursor *);
#endif

#if !GTK_CHECK_VERSION(3,0,0)
# include <gdk/gdkkeysyms.h>
#endif

#endif
