/* gui_gtk_compat.c */
GdkSurface *gui_gtk_widget_get_containing_surface(GtkWidget *widget);
void gtk_widget_queue_draw_area(GtkWidget *widget, int x, int y, int w, int h);
gint gtk_dialog_run(GtkDialog *dialog);
gint gtk_native_dialog_run(GtkNativeDialog *self);
const char *gtk_entry_get_text(GtkEntry *entry);
void gtk_entry_set_text(GtkEntry *entry, const gchar *text);
/* vim: set ft=c : */
