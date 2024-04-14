#ifndef __GUI_GTK_COMPAT_H__
#define __GUI_GTK_COMPAT_H__

#ifndef USE_GTK4
// These macros allow GTK2/3 code to appear to use the GTK4 API for some widget
// operations that share a similar prototype across major versions.
# define gtk_notebook_append_page(notebook, page) gtk_container_add(GTK_CONTAINER(notebook), page)
# define gtk_box_prepend(box, child) gtk_box_pack_start(GTK_BOX(box), child, FALSE, FALSE, 0)
# define gtk_container_set_border_width(cont, pixels) do { } while(0)
# define gdk_key_event_get_keyval(event) event->keyval
#endif

#endif
