/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

#include "vim.h"

#if defined(USE_GTK4)

#include "gui_gtk4_text_area.h"

struct _GtkVimTextArea
{
    GtkWidget parent_instance;
    GtkPopoverMenu **context_menus;
    size_t context_menu_count;
};

struct _GtkVimTextAreaClass
{
    GtkWidgetClass parent_class;
};

G_DEFINE_FINAL_TYPE(GtkVimTextArea, gui_gtk_text_area, GTK_TYPE_DRAWING_AREA)

static void gui_gtk_text_area_class_init(GtkVimTextAreaClass *);
static void gui_gtk_text_area_init(GtkVimTextArea *);

    void
text_area_dispose(GObject *object)
{
    GtkVimTextArea *self = GTKVIM_TEXT_AREA(object);
    for (size_t idx = 0; idx < self->context_menu_count; idx++)
    {
	gtk_widget_unparent(GTK_WIDGET(self->context_menus[idx]));
	g_object_unref(G_OBJECT(self->context_menus[idx]));
    }
}

    static void
text_area_measure(
	GtkWidget	    *widget,
	GtkOrientation   orientation,
	int		     for_size,
	int		    *minimum,
	int		    *natural,
	int		    *minimum_baseline,
	int		    *natural_baseline)
{
  GtkDrawingArea *self = GTK_DRAWING_AREA(widget);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
  {
      *minimum = 1;
      *natural = gtk_drawing_area_get_content_width(self);
  }
  else
  {
      *minimum = 1;
      *natural = gtk_drawing_area_get_content_height(self);
  }

  if (*minimum > *natural)
      *natural = *minimum;
}

    static void
gui_gtk_text_area_class_init(GtkVimTextAreaClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = text_area_dispose;
    widget_class->measure = text_area_measure;
}

    static void
gui_gtk_text_area_init(GtkVimTextArea *self)
{
    self->context_menus = NULL;
    self->context_menu_count = 0;

    gtk_widget_set_can_focus(GTK_WIDGET(self), TRUE);
    gtk_widget_set_focusable(GTK_WIDGET(self), TRUE);
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(self), 1);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(self), 1);
    gtk_widget_set_name(GTK_WIDGET(self), "vim-text-area");
}

    GtkVimTextArea *
gui_gtk_text_area_new(void)
{
    return g_object_new(GTKVIM_TYPE_TEXT_AREA, NULL);
}

    void
gui_gtk_text_area_add_context_menu(GtkVimTextArea *self, vimmenu_T *menu)
{
    if (menu->id != NULL)
    {
	self->context_menu_count++;
	self->context_menus = realloc(self->context_menus, self->context_menu_count * sizeof(GtkPopoverMenu *));
	self->context_menus[self->context_menu_count - 1] = GTK_POPOVER_MENU(menu->id);
	g_object_ref_sink(G_OBJECT(menu->id));
	gtk_widget_set_parent(GTK_WIDGET(menu->id), GTK_WIDGET(self));
    }
}

    void
gui_gtk_text_area_remove_context_menu(GtkVimTextArea *self, vimmenu_T *menu)
{
    if (menu->id != NULL)
    {
	for (size_t idx = 0; idx < self->context_menu_count; idx++)
	{
	    if (GTK_POPOVER_MENU(menu->id) == self->context_menus[idx])
	    {
		gtk_widget_unparent(GTK_WIDGET(self->context_menus[idx]));
		for (size_t copy_idx = idx + 1; copy_idx < self->context_menu_count; copy_idx++)
		    self->context_menus[copy_idx - 1] = self->context_menus[copy_idx];
		self->context_menu_count--;
		self->context_menus = realloc(self->context_menus, self->context_menu_count);
		return;
	    }
	}
    }
}

#endif // USE_GTK4
