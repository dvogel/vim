/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

#include "vim.h"

#ifdef USE_GTK4
#include <gtk/gtk.h>	// without this it compiles, but gives errors at
			// runtime!
#include "gui_gtk4_form.h"
#ifdef MSWIN
# include <gdk/gdkwin32.h>
#else
# include <gdk/gdk.h>
#endif
#include "gui_gtk_compat.h"

enum {
  RESIZE,
  LAST_SIGNAL
};

static guint form_layout_signals[LAST_SIGNAL] = { 0, };

struct _GtkVimFormLayout
{
    GtkLayoutManager parent_instance;
};

struct _GtkVimFormLayoutClass
{
    GtkLayoutManagerClass parent_class;

    void (* resize)(GtkLayoutManager *);
};


G_DEFINE_FINAL_TYPE(GtkVimFormLayout, gui_gtk_form_layout, GTK_TYPE_FIXED_LAYOUT);

    static GtkSizeRequestMode
form_layout_get_request_mode(GtkLayoutManager *layout_manager, GtkWidget *widget)
{
    return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

    static void
form_layout_allocate(GtkLayoutManager *layout_manager,
		     GtkWidget        *widget,
		     int               width,
		     int               height,
		     int               baseline)
{
    GtkFixedLayoutChild *child_info;
    GtkWidget *child;

    for (child = gtk_widget_get_first_child(widget);
	 child != NULL;
	 child = gtk_widget_get_next_sibling(child))
    {
	GtkRequisition child_req;

	if (!gtk_widget_should_layout(child))
	    continue;

	child_info = GTK_FIXED_LAYOUT_CHILD(gtk_layout_manager_get_layout_child(layout_manager, child));
	gtk_widget_get_preferred_size(child, NULL, &child_req);

	gtk_widget_allocate(child,
			    child_req.width,
			    child_req.height,
			    -1,
			    gsk_transform_ref(gtk_fixed_layout_child_get_transform(child_info)));
    }

    g_signal_emit(layout_manager, form_layout_signals[RESIZE], 0);
}

    static void
form_layout_measure(
	GtkLayoutManager *layout_manager,
	GtkWidget        *widget,
	GtkOrientation    orientation,
	int               for_size,
	int              *minimum,
	int              *natural,
	int              *minimum_baseline,
	int              *natural_baseline)
{
    GtkFixedLayoutChild *child_info;
    GtkWidget *child;
    int minimum_size = 0;
    int natural_size = 0;
    GtkOrientation opposite_orientation = (orientation == GTK_ORIENTATION_HORIZONTAL) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

    for (child = gtk_widget_get_first_child(widget);
	    child != NULL;
	    child = gtk_widget_get_next_sibling(child))
    {
	int child_min = 0, child_nat = 0;
	int child_min_opp = 0, child_nat_opp = 0;
	graphene_rect_t min_rect, nat_rect;

	if (!gtk_widget_should_layout(child))
	    continue;

	if (GTK_IS_SCROLLBAR(child))
	    continue;

	child_info = GTK_FIXED_LAYOUT_CHILD(gtk_layout_manager_get_layout_child(layout_manager, child));

	gtk_widget_measure(child, orientation, -1,
		&child_min, &child_nat,
		NULL, NULL);
	gtk_widget_measure(child, opposite_orientation, -1,
		&child_min_opp, &child_nat_opp,
		NULL, NULL);

	min_rect.origin.x = min_rect.origin.y = 0;
	nat_rect.origin.x = nat_rect.origin.y = 0;
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
	    min_rect.size.width = child_min;
	    min_rect.size.height = child_min_opp;
	    nat_rect.size.width = child_nat;
	    nat_rect.size.height = child_nat_opp;
	}
	else
	{
	    min_rect.size.width = child_min_opp;
	    min_rect.size.height = child_min;
	    nat_rect.size.width = child_nat_opp;
	    nat_rect.size.height = child_nat;
	}

	gsk_transform_transform_bounds(gtk_fixed_layout_child_get_transform(child_info), &min_rect, &min_rect);
	gsk_transform_transform_bounds(gtk_fixed_layout_child_get_transform(child_info), &nat_rect, &nat_rect);

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
	    minimum_size = MAX(minimum_size, min_rect.origin.x + min_rect.size.width);
	    natural_size = MAX(natural_size, nat_rect.origin.x + nat_rect.size.width);
	}
	else
	{
	    minimum_size = MAX(minimum_size, min_rect.origin.y + min_rect.size.height);
	    natural_size = MAX(natural_size, nat_rect.origin.y + nat_rect.size.height);
	}
    }

    minimum_size = MIN(minimum_size, natural_size);
    natural_size = MAX(minimum_size, natural_size);

    if (minimum != NULL)
	*minimum = minimum_size;
    if (natural != NULL)
	*natural = natural_size;
}

    static void
gui_gtk_form_layout_class_init(GtkVimFormLayoutClass *klass)
{
    GtkLayoutManagerClass *layout_class = GTK_LAYOUT_MANAGER_CLASS(klass);
    layout_class->allocate = form_layout_allocate;
    layout_class->get_request_mode = form_layout_get_request_mode;
    layout_class->layout_child_type = GTK_TYPE_FIXED_LAYOUT_CHILD;
    layout_class->measure = form_layout_measure;

    form_layout_signals[RESIZE] =
	g_signal_new("resize",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(struct _GtkVimFormLayoutClass, resize),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 0);
}

    static void
gui_gtk_form_layout_init(GtkVimFormLayout *layout)
{
}

struct _GtkForm
{
    GtkWidget parent_instance;
    GList *children;
    int width;
    int height;
    gint freeze_count;
};

struct _GtkFormClass
{
    GtkWidgetClass parent_class;

    void (* resize)(GtkForm *);
};

G_DEFINE_FINAL_TYPE(GtkForm, gui_gtk_form, GTK_TYPE_WIDGET)

// TODO: The GtkForm is like a GtkFixed except it needs to somehow let the GtkDrawingArea resize when it resizes. Best idea so far:
// GtkForm should emit a 'resize' signal as GtkDrawingArea does. Then the init code can subscribe to that signal.

static guint form_signals[LAST_SIGNAL] = { 0, };

static void gui_gtk_form_class_init(GtkFormClass *klass);
static void gui_gtk_form_init(GtkForm *form);


/*
 * Virtual method overrides
 */

    static void
form_compute_expand(
	GtkWidget *form,
	gboolean *hexpand_p,
	gboolean *vexpand_p)
{
    *hexpand_p = TRUE;
    *vexpand_p = TRUE;
}

    static GtkSizeRequestMode
form_get_request_mode(GtkWidget *form)
{
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

    static void
form_resize_cb(GtkLayoutManager *layout, gpointer *form_p)
{
    GtkForm *form = GTK_FORM(form_p);
    if (GTK_FORM(form)->freeze_count == 0)
    {
	g_signal_emit(form, form_signals[RESIZE], 0);
    }
    else
    {
	// fprintf(stderr, "%s:squelching resize event\n", __func__);
    }
}

    static void
form_dispose(GObject *object)
{
    GtkWidget *child;

    while ((child = gtk_widget_get_first_child(GTK_WIDGET(object))))
	gui_gtk_form_remove_child(GTK_FORM(object), child);

    G_OBJECT_CLASS(gui_gtk_form_parent_class)->dispose(object);
}

// Public interface

    GtkForm *
gui_gtk_form_new(void)
{
    GtkForm *form;

    form = g_object_new(GTK_TYPE_FORM, NULL);

    return form;
}

    void
gui_gtk_form_put(
	GtkForm    *form,
	GtkWidget  *child_widget,
	gint	   x,
	gint	   y)
{
    g_return_if_fail(GTK_IS_FORM(form));
    g_return_if_fail(GTK_IS_WIDGET(child_widget));
    g_return_if_fail(gtk_widget_get_parent(child_widget) == NULL);

    gtk_widget_set_parent(child_widget, GTK_WIDGET(form));
    gtk_widget_set_visible(child_widget, TRUE);
    gui_gtk_form_move(form, child_widget, x, y);
}

    void
gui_gtk_form_remove_child(
	GtkForm *form,
	GtkWidget *child_widget)
{
    GList *tmp_list;

    g_return_if_fail(GTK_IS_FORM(form));
    g_return_if_fail(GTK_IS_WIDGET(child_widget));
    g_return_if_fail(gtk_widget_get_parent(child_widget) == GTK_WIDGET(form));

    gtk_widget_unparent(child_widget);
}

    void
gui_gtk_form_move(
	GtkForm	   *form,
	GtkWidget  *child_widget,
	gint	   x,
	gint	   y)
{
    GtkFixedLayout      *layout = NULL;
    GtkFixedLayoutChild *child_info = NULL;
    GskTransform        *transform = NULL;

    g_return_if_fail(GTK_IS_FORM(form));
    g_return_if_fail(GTK_IS_WIDGET(child_widget));
    g_return_if_fail(gtk_widget_get_parent(child_widget) == GTK_WIDGET(form));

    layout = GTK_FIXED_LAYOUT(gtk_widget_get_layout_manager(GTK_WIDGET(form)));
    g_return_if_fail(layout != NULL);

    child_info = GTK_FIXED_LAYOUT_CHILD(gtk_layout_manager_get_layout_child(GTK_LAYOUT_MANAGER(layout), child_widget));

    transform = gsk_transform_translate(transform, &GRAPHENE_POINT_INIT((gdouble)x, (gdouble)y));
    gtk_fixed_layout_child_set_transform(child_info, transform);
    gsk_transform_unref(transform);
}

    void
gui_gtk_form_move_resize(GtkForm *form, GtkWidget *widget,
			 gint x, gint y, gint w, gint h)
{
    GtkAllocation size_alloc = { x, y, w, h };
    gtk_widget_size_allocate(widget, &size_alloc, 0);
    /* gui_gtk_form_move(form, widget, x, y); */
}

    void
gui_gtk_form_freeze(GtkForm *form)
{
    g_return_if_fail(GTK_IS_FORM(form));

    ++form->freeze_count;
}

    void
gui_gtk_form_thaw(GtkForm *form)
{
    g_return_if_fail(GTK_IS_FORM(form));

    if (!form->freeze_count)
	return;

    if (!(--form->freeze_count))
    {
	gtk_widget_queue_draw(GTK_WIDGET(form));
    }
}

// Basic Object handling procedures

    static void
gui_gtk_form_class_init(GtkFormClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = form_dispose;
    widget_class->compute_expand = form_compute_expand;
    widget_class->get_request_mode = form_get_request_mode;
    // widget_class->size_allocate = form_size_allocate;
    gtk_widget_class_set_layout_manager_type(widget_class, GTKVIM_TYPE_FORM_LAYOUT);

    form_signals[RESIZE] =
	g_signal_new("resize",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(struct _GtkFormClass, resize),
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 0);
}

    static void
gui_gtk_form_init(GtkForm *form)
{
    GtkLayoutManager *layout;
    form->children = NULL;
    form->freeze_count = 0;
    gtk_widget_add_css_class(GTK_WIDGET(form), "gtk-form");
    gtk_widget_set_hexpand(GTK_WIDGET(form), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(form), TRUE);

    layout = gtk_widget_get_layout_manager(GTK_WIDGET(form));
    g_signal_connect(G_OBJECT(layout), "resize", G_CALLBACK(form_resize_cb), form);

    gtk_widget_set_name(GTK_WIDGET(form), "vim-gtk-form");
}


#endif // USE_GTK4
