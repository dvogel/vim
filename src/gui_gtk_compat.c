#include "vim.h"

#ifdef USE_GTK4

// TODO: How to implement this functionality without violating the LGPL of
// GTK+?

// WARNING: These are analogs of functions that were provided by GTK+ prior to
// v4. The old versions proved troublesome to implement due to re-entrancy.
// These implementations make no attempt to be re-entrant compatible. This is
// only tenable because we know the Vim GTK code is single threaded.

#include <gtk/gtk.h>
#include <gtk/gtknativedialog.h>

    GdkSurface *
gui_gtk_widget_get_containing_surface(GtkWidget *widget)
{
    GtkNative *n;
    GdkSurface *s;

    n = gtk_widget_get_native(widget);
    if (n == NULL)
	return NULL; // should not really happen

    s = gtk_native_get_surface(n);
    if (s == NULL)
	return NULL;

    return s;
}

    void
gtk_widget_queue_draw_area(
	GtkWidget *widget,
	int x UNUSED, int y UNUSED,
	int w UNUSED, int h UNUSED)
{
    // The rectangle params are accepted for compat but ignored. The entire
    // widget is redrawn.
    GdkSurface *s = gui_gtk_widget_get_containing_surface(widget);
    if (s == NULL)
		return;

    /* GdkCairoContext *surf_cr = gdk_surface_create_cairo_context(s); */
    /* cairo_t * const cr = gdk_cairo_context_cairo_create(surf_cr); */

    gdk_surface_queue_render(s);
}

typedef struct
{
    gint response_id;
    GMainLoop *loop;
} RunInfo;

    static void
shutdown_loop (RunInfo *ri)
{
    if (g_main_loop_is_running (ri->loop))
	g_main_loop_quit (ri->loop);
}

    static void
run_response_handler (
	GtkDialog *dialog,
	gint response_id,
	gpointer data)
{
    RunInfo *ri = data;
    ri->response_id = response_id;
    shutdown_loop (ri);
}

    static void
run_close_handler (GtkDialog *dialog, gpointer data)
{
    RunInfo *ri = data;
    ri->response_id = GTK_RESPONSE_CANCEL;
    shutdown_loop (ri);
}

    gint
gtk_dialog_run (GtkDialog *dialog)
{
    RunInfo ri = { GTK_RESPONSE_NONE, NULL };
    gboolean was_modal;
    gulong response_handler;
    gulong close_handler;

    g_return_val_if_fail (GTK_IS_DIALOG (dialog), -1);

    g_object_ref (dialog);

    was_modal = gtk_window_get_modal (GTK_WINDOW (dialog));
    if (!was_modal)
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    if (!gtk_widget_get_visible (GTK_WIDGET (dialog)))
	gtk_widget_show (GTK_WIDGET (dialog));

    response_handler =
	g_signal_connect (dialog,
		"response",
		G_CALLBACK (run_response_handler),
		&ri);

    close_handler =
	g_signal_connect (dialog,
		"close",
		G_CALLBACK (run_close_handler),
		&ri);

    ri.loop = g_main_loop_new (NULL, FALSE);

    g_main_loop_run (ri.loop);

    g_main_loop_unref (ri.loop);

    ri.loop = NULL;

    if (!was_modal)
	gtk_window_set_modal (GTK_WINDOW(dialog), FALSE);

    g_signal_handler_disconnect (dialog, response_handler);
    g_signal_handler_disconnect (dialog, close_handler);

    g_object_unref (dialog);

    return ri.response_id;
}


typedef struct
{
  gint response_id;
  GMainLoop *loop;
} NativeRunInfo;

    static void
run_response_cb (
	GtkNativeDialog *self,
	gint response_id,
	gpointer data)
{
    NativeRunInfo* ri = data;
    ri->response_id = response_id;
    if (ri->loop && g_main_loop_is_running (ri->loop))
	g_main_loop_quit (ri->loop);
}

    gint
gtk_native_dialog_run (GtkNativeDialog *self)
{
    NativeRunInfo ri = { GTK_RESPONSE_NONE, NULL };
    gboolean was_modal;
    guint response_handler;

    g_return_val_if_fail (GTK_IS_NATIVE_DIALOG (self), -1);

    g_object_ref (self);

    ri.response_id = GTK_RESPONSE_NONE;
    ri.loop = g_main_loop_new (NULL, FALSE);

    was_modal = gtk_native_dialog_get_modal(self);
    gtk_native_dialog_set_modal (self, TRUE);

    response_handler =
	g_signal_connect (self,
		"response",
		G_CALLBACK (run_response_cb),
		(gpointer)&ri);

    gtk_native_dialog_show (self);

    g_main_loop_run (ri.loop);

    g_signal_handler_disconnect (self, response_handler);

    g_main_loop_unref (ri.loop);
    ri.loop = NULL;

    if (!was_modal)
	gtk_native_dialog_set_modal (self, FALSE);

    g_object_unref (self);

    return ri.response_id;
}

	const char *
gtk_entry_get_text(GtkEntry *entry)
{
	GtkEntryBuffer *buf = gtk_entry_get_buffer(entry);
	gtk_entry_buffer_get_text(buf);
}

	void
gtk_entry_set_text(GtkEntry *entry, const gchar *text)
{
	GtkEntryBuffer *buf = gtk_entry_get_buffer(entry);
	gtk_entry_buffer_set_text(buf, text, -1);
}

#else // !USE_GTK4

	void
gtk_widget_set_cursor(GtkWidget *widget, GdkCursor *cursor)
{
    GdkWindow *window = gtk_widget_get_window(widget);
    if (window != NULL)
		gdk_window_set_cursor(window, cursor);
}

#endif

