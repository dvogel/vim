/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

/*
 * This file contains the GUI implementation that is specific to GTK4.
 */

#include "vim.h"

#ifdef USE_GRESOURCE
# include "auto/gui_gtk_gresources.h"
#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "gui_gtk4_form.h"
#include "gui_gtk4_menu.h"
#include "gui_gtk4_text_area.h"
#include "gui_gtk_compat.h"
#include "gui_gtk_common.h"

static const char *receivable_mime_types[] =
{
    "text/html",
    "text/plain;charset=utf8",
    "text/plain",
    NULL
};

// TODO: Implement FEAT_DND

#define DEFAULT_FONT	"Mono 10"

static void activate_menu_item_cb(GSimpleAction *, GVariant *, gpointer);

// FEAT_GUI_TABLINE {{{
#if defined(FEAT_GUI_TABLINE) || defined(PROTO)
static int ignore_tabline_evt = FALSE;
static GtkPopoverMenu *tabline_menu;
static int clicked_page;	    // page clicked in tab line

    static void
on_select_tab(
	GtkNotebook	*notebook UNUSED,
	gpointer	*page UNUSED,
	gint		idx,
	gpointer	data UNUSED)
{
    if (!ignore_tabline_evt)
	send_tabline_event(idx + 1);
}

    static void
on_tab_reordered(
	GtkNotebook	*notebook UNUSED,
	gpointer	*page UNUSED,
	gint		idx,
	gpointer	data UNUSED)
{
    if (ignore_tabline_evt)
	return;

    if ((tabpage_index(curtab) - 1) < idx)
	tabpage_move(idx + 1);
    else
	tabpage_move(idx);
}

    static int
tab_widget_at_cursor_position(
	gdouble x, // cursor coordinates should be relative to the GtkNotebook.
	gdouble y,
	GtkNotebook *notebook
	)
{
    graphene_point_t cursor_point;
    graphene_point_init(&cursor_point, (float)x, (float)y);

    graphene_rect_t  tab_widget_area;
    GtkWidget        tab_widget;
    GtkWidget        *page_widget;
    GtkNotebookPage  *page;
    for (int page_idx = 0; page_idx < gtk_notebook_get_n_pages(notebook); page_idx++)
    {
	page_widget = gtk_notebook_get_nth_page(notebook, page_idx);
	if (page_widget == NULL)
	    continue;

	page = gtk_notebook_get_page(notebook, page_widget);
	if (page == NULL)
	    continue;

	g_object_get(page, "tab", tab_widget);
	if (!gtk_widget_compute_bounds(&tab_widget, GTK_WIDGET(notebook), &tab_widget_area))
	    continue;

	if (graphene_rect_contains_point(&tab_widget_area, &cursor_point))
	{
	    return page_idx + 1; // GTK tabs are 0-indexed while Vim tabs are 1-indexed
	}
    }

    return 0;
}

    static void
handle_tabline_menu_click(GtkGestureClick *click,
			  gint n_press,
			  gdouble x,
			  gdouble y,
			  gpointer user_data)
{

    // When ignoring events return TRUE so that the selected page doesn't
    // change.
    if (hold_gui_events || cmdwin_type != 0)
	return;

    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(click));
    int clicked_page_nr = tab_widget_at_cursor_position(x, y, GTK_NOTEBOOK(user_data));
}

    static void
create_tabline_menu(GtkNotebook *notebook)
{
    GMenu          *menu_tree;
    GtkPopoverMenu *menu_popover;

    menu_tree = g_menu_new();
    g_menu_append(menu_tree, (const char_u *)_("Close tab"), "close-tab");
    g_menu_append(menu_tree, (const char_u *)_("New tab"), "new-tab");
    g_menu_append(menu_tree, (const char_u *)_("Open Tab..."), "open-tab");
    tabline_menu = GTK_POPOVER_MENU(gtk_popover_menu_new_from_model(G_MENU_MODEL(menu_tree)));
}

    static void
create_tabline()
{
    gui.tabline = gtk_notebook_new();
    gtk_widget_set_visible(gui.tabline, TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(gui.tabline), FALSE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(gui.tabline), FALSE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(gui.tabline), TRUE);

    g_signal_connect(G_OBJECT(gui.tabline), "switch-page",
		     G_CALLBACK(on_select_tab), NULL);
    g_signal_connect(G_OBJECT(gui.tabline), "page-reordered",
		     G_CALLBACK(on_tab_reordered), NULL);

    // TODO: Does tabline_click need to be global in order to unsubscribe these
    // signal handlers?
    GtkGestureClick *tabline_click = GTK_GESTURE_CLICK(gtk_gesture_click_new());
    gtk_widget_add_controller(GTK_WIDGET(gui.tabline),
			      GTK_EVENT_CONTROLLER(tabline_click));
    g_signal_connect(G_OBJECT(tabline_click), "pressed",
	    G_CALLBACK(handle_tabline_menu_click), G_OBJECT(gui.tabline));

    create_tabline_menu(GTK_NOTEBOOK(gui.tabline));
}

    static GtkWidget *
gui_gtk_build_tab_label(const char *label_text, GtkWidget *page)
{
    GtkWidget *label = gtk_label_new(label_text);
    gtk_widget_set_visible(label, TRUE);
    g_object_set_data(G_OBJECT(label), "tab_num", GINT_TO_POINTER(1L));
    return label;
}

/*
 * Update the labels of the tabline to reflect VIM's internal tab names.
 */
    void
gui_mch_update_tabline(void)
{
    GtkWidget	    *page;
    GtkWidget	    *tab_label; // tab_label is a GtkLabel
    tabpage_T	    *tp;
    int		    nr = 0;     // 0-based index of GTK tab
    int		    tab_num;    // 1-based index of VIM tab
    int		    curtabidx = 0;
    char_u	    *labeltext;

    if (gui.tabline == NULL)
	return;

    gui_gtk_ignore_tabline_event(TRUE);

    // Add a label for each tab page.  They all contain the same text area.
    for (tp = first_tabpage; tp != NULL; tp = tp->tp_next, ++nr)
    {
	if (tp == curtab)
	    curtabidx = nr;

	tab_num = nr + 1;

	page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(gui.tabline), nr);
	if (page == NULL)
	{
	    // Add notebook page
	    page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	    gtk_box_set_homogeneous(GTK_BOX(page), FALSE);
	    gtk_widget_show(page);

	    tab_label = gui_gtk_build_tab_label("-Empty-", page);
	    gtk_notebook_insert_page(GTK_NOTEBOOK(gui.tabline),
		    page,
		    tab_label,
		    nr++);
	    gtk_notebook_set_tab_label(GTK_NOTEBOOK(gui.tabline), page, tab_label);
	    gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(gui.tabline),
		    page,
		    TRUE);
	}

	tab_label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(gui.tabline), page);
	g_object_set_data(G_OBJECT(tab_label), "tab_num",
		GINT_TO_POINTER(tab_num));
	get_tabline_label(tp, FALSE);
	labeltext = CONVERT_TO_UTF8(NameBuff);
	gtk_label_set_text(GTK_LABEL(tab_label), (const char *)labeltext);
	CONVERT_TO_UTF8_FREE(labeltext);

	get_tabline_label(tp, TRUE);
	labeltext = CONVERT_TO_UTF8(NameBuff);
	gtk_widget_set_tooltip_text(tab_label, (const gchar *)labeltext);
	CONVERT_TO_UTF8_FREE(labeltext);
    }

    // Remove any old labels.
    while (gtk_notebook_get_nth_page(GTK_NOTEBOOK(gui.tabline), nr) != NULL)
	gtk_notebook_remove_page(GTK_NOTEBOOK(gui.tabline), nr);

    if (gtk_notebook_get_current_page(GTK_NOTEBOOK(gui.tabline)) != curtabidx)
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gui.tabline), curtabidx);

    // Make sure everything is in place before drawing text.
    gui_mch_update();

    gui_gtk_ignore_tabline_event(FALSE);
}
#endif
// }}}

    guicolor_T
gui_mch_get_rgb(guicolor_T pixel)
{
    return (long_u)pixel;
}

    GtkSettings*
gui_gtk_get_settings()
{
    return gtk_settings_get_for_display(gdk_display_get_default());
}

    static gboolean
drawarea_reconstruct_surface(GObject *aliased_drawarea,
			     GParamSpec *unused1,
			     gpointer unused2)
{
    // In GTK3 the configure event would signal when some of the properties we
    // care about remained the same. In GTK4 we rely on property signals which
    // won't fire if the value has not changed because GTK takes pains to avoid
    // that.

    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(gui.drawarea), gtk_widget_get_width(gui.drawarea));
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(gui.drawarea), gtk_widget_get_height(gui.drawarea));

    if (gui.surface != NULL)
	cairo_surface_destroy(gui.surface);

    gui.surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					     gtk_widget_get_width(gui.drawarea),
					     gtk_widget_get_height(gui.drawarea));
    gui.force_redraw = 1;
    gui_redraw(0, 0, screen_Rows, screen_Columns);
    gtk_widget_queue_draw(gui.drawarea);
    return TRUE;
}

/*
 * Callback routine for the "close-request" signal on the toplevel window.
 * Tries to exit vim gracefully, or refuses to exit with changed buffers.
 */
    static gint
close_request_cb(GtkWidget *widget UNUSED,
		 gpointer data UNUSED)
{
    gui_shell_closed();
    return TRUE;
}

// FEAT_MOUSESHAPE {{{

#if defined(FEAT_MOUSESHAPE) || defined(PROTO)
static const char * mshape_css_names[] =
{
    "default",                  // GDK_LEFT_PTR
    "blank",                    // GDK_CURSOR_IS_PIXMAP TODO: should this be "none"?
    "text",                     // GDK_XTERM
    "ns-resize",                // GDK_SB_V_DOUBLE_ARROW
    "nwse-resize",              // GDK_SIZING
    "ew-resize",                // GDK_SB_H_DOUBLE_ARROW
    "ew-resize",                // GDK_SIZING
    "progress",                 // GDK_WATCH
    "not-allowed",              // GDK_X_CURSOR
    "crosshair",                // GDK_CROSSHAIR
    "pointer",                  // GDK_HAND1
    "pointer",                  // GDK_HAND2
    "pointer",                  // GDK_PENCIL (no good option for this one)
    "help",                     // GDK_QUESTION_ARROW
    "default",                  // GDK_RIGHT_PTR (no css analogue)
    "default",                  // GDK_CENTER_PTR (no css analogue)
    "default"                   // GDK_LEFT_PTR
};

// The last set mouse pointer shape is remembered, to be used when it goes
// from hidden to not hidden.
static int last_shape = 0;

    void
mch_set_mouse_shape(int shape)
{
    GdkCursor	   *c;

    if (gtk_widget_get_realized(gui.drawarea) == FALSE)
	return;

    if ((shape < 0) || (shape >= (int)ARRAY_LENGTH(mshape_css_names)))
	return;

    if (shape == MSHAPE_HIDE || gui.pointer_hidden)
	c = gdk_cursor_new_from_name("none", NULL);
    else
    {
	c = gdk_cursor_new_from_name(mshape_css_names[shape], NULL);
	last_shape = shape;
    }
    gtk_widget_set_cursor(gui.drawarea, c);
    g_object_unref(G_OBJECT(c));
}
#endif // FEAT_MOUSESHAPE

// }}}

// pointer utilities {{{
    static GdkDevice *
gui_gtk_get_pointer_device(GtkWidget *widget)
{
    GdkSurface * const s = gui_gtk_widget_get_containing_surface(widget);
    GdkDisplay * const dpy = gdk_surface_get_display(s);
    GdkSeat * const seat = gdk_display_get_default_seat(dpy);
    return gdk_seat_get_pointer(seat);
}

    void
gui_gtk_get_pointer(GtkWidget       *widget,
		    gint	    *x,
		    gint	    *y,
		    GdkModifierType *state)
{
    GdkSurface * const s = gui_gtk_widget_get_containing_surface(widget);
    GdkDevice * const dev = gui_gtk_get_pointer_device(widget);
    gdouble xd, yd;
    gdk_surface_get_device_position(s, dev, &xd, &yd, state);
    *x = (gint)xd;
    *y = (gint)yd;
}

/*
 * Get current mouse coordinates in text window.
 */
    void
gui_mch_getmouse(int *x, int *y)
{
    gui_gtk_get_pointer(gui.drawarea, x, y, NULL);
}

/* Since we cannot do this on Wayland, this is a no-op with GTK4.
 */
    void
gui_mch_setmouse(int x, int y)
{
    return;
}

/*
 * Use the blank mouse pointer or not.
 *
 * hide: TRUE = use blank ptr, FALSE = use parent ptr
 */
    void
gui_mch_mousehide(int hide)
{
    if (gui.pointer_hidden == hide)
	return;

    gui.pointer_hidden = hide;
    if (gui.blank_pointer != NULL)
    {
	if (hide)
	    gtk_widget_set_cursor(gui.drawarea, gui.blank_pointer);
	else
#ifdef FEAT_MOUSESHAPE
	    mch_set_mouse_shape(last_shape);
#else
	    gtk_widget_set_cursor(gui.drawarea, NULL);
#endif
    }
}
// }}}

// widget realize event handlers {{{

    static void
measure_scrollbar_size(int sb_idx)
{
    GtkWidget *sbar;

    if (!gui.which_scrollbars[sb_idx])
	return;

    if (sb_idx == SBAR_BOTTOM)
	sbar = gui.bottom_sbar.id;
    else
	sbar = firstwin->w_scrollbars[sb_idx].id;
    if (!sbar)
	return;

    if (GTK_IS_WIDGET(sbar) && gtk_widget_get_realized(sbar))
	if (gtk_orientable_get_orientation((GtkOrientable *)sbar) == GTK_ORIENTATION_HORIZONTAL)
	    gui.scrollbar_height = gtk_widget_get_height(sbar);
	else
	    gui.scrollbar_width = gtk_widget_get_width(sbar);
}

    static void
formwin_resize_cb(GtkForm* aliased_formwin,
		  gpointer user_data)
{
    gui_gtk_form_freeze((GtkForm *)gui.formwin);
    gui_resize_shell(
	    gtk_widget_get_width(gui.formwin),
	    gtk_widget_get_height(gui.formwin));
    gui_gtk_form_thaw((GtkForm *)gui.formwin);
}

    static void
drawarea_resize_cb(GtkDrawingArea* aliased_drawarea,
		   gint width,
		   gint height,
		   gpointer user_data)
{
    drawarea_reconstruct_surface((GObject *)gui.drawarea, NULL, NULL);
}


/*
 * After the drawing area comes up, we calculate all colors and create the
 * dummy blank cursor.
 *
 * Don't try to set any VIM scrollbar sizes anywhere here. I'm relying on the
 * fact that the main VIM engine doesn't take them into account anywhere.
 */
    static void
drawarea_realize_cb(GtkWidget *widget, gpointer data UNUSED)
{
    GtkWidget *sbar;
    GtkAllocation allocation;
    gui_mch_new_colors();
    drawarea_reconstruct_surface((GObject *)gui.drawarea, NULL, NULL);
    gui.blank_pointer = gdk_cursor_new_from_name("none", NULL);
    if (gui.pointer_hidden)
	gtk_widget_set_cursor(widget, gui.blank_pointer);

    measure_scrollbar_size(SBAR_LEFT);
    measure_scrollbar_size(SBAR_RIGHT);
    measure_scrollbar_size(SBAR_BOTTOM);
}

    void
gui_gtk_get_screen_geom_of_win(
	GtkWidget *widget,
	int point_x,	    // x position of window if not initialized
	int point_y,	    // y position of window if not initialized
	int *screen_x,
	int *screen_y,
	int *width,
	int *height)
{
    GdkRectangle geometry;
    GdkDisplay *disp;
    GdkMonitor *monitor;
    GdkSurface *surf = gui_gtk_widget_get_containing_surface(widget);

    if (surf == NULL)
	return;

    if (widget != NULL && gtk_widget_get_realized(widget))
	disp = gtk_widget_get_display(widget);
    else
	disp = gdk_display_get_default();

    monitor = gdk_display_get_monitor_at_surface(disp, surf);
    gdk_monitor_get_geometry(monitor, &geometry);
    *screen_x = geometry.x;
    *screen_y = geometry.y;
    *width = geometry.width;
    *height = geometry.height;
}

    void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
    int	    x, y;
    gui_gtk_get_screen_geom_of_win(gui.mainwin,
				   0, 0,
				   &x, &y,
				   screen_w, screen_h);

    // Subtract 'guiheadroom' from the height to allow some room for the
    // window manager (task list and window title bar).
    *screen_h -= p_ghr;
}

/*
 * Properly clean up on shutdown.
 */
    static void
drawarea_unrealize_cb(GtkWidget *widget UNUSED, gpointer data UNUSED)
{
    // Don't write messages to the GUI anymore
    full_screen = FALSE;

    if (gui.ascii_glyphs != NULL)
    {
	pango_glyph_string_free(gui.ascii_glyphs);
	gui.ascii_glyphs = NULL;
    }
    if (gui.ascii_font != NULL)
    {
	g_object_unref(gui.ascii_font);
	gui.ascii_font = NULL;
    }
    g_object_unref(gui.text_context);
    gui.text_context = NULL;

    if (gui.surface != NULL)
    {
	cairo_surface_destroy(gui.surface);
	gui.surface = NULL;
    }

    g_object_unref(G_OBJECT(gui.blank_pointer));
    gui.blank_pointer = NULL;
}

    static void
mainwin_realize(GtkWidget *widget UNUSED, gpointer data UNUSED)
{
#include "../runtime/vim32x32.xpm"
#include "../runtime/vim16x16.xpm"
#include "../runtime/vim48x48.xpm"
    if (vim_strchr(p_go, GO_ICON) != NULL)
    {
	GList *icons = NULL;
	icons = g_list_prepend(icons, gdk_texture_new_for_pixbuf(gdk_pixbuf_new_from_xpm_data((const char **)vim16x16)));
	icons = g_list_prepend(icons, gdk_texture_new_for_pixbuf(gdk_pixbuf_new_from_xpm_data((const char **)vim32x32)));
	icons = g_list_prepend(icons, gdk_texture_new_for_pixbuf(gdk_pixbuf_new_from_xpm_data((const char **)vim48x48)));

	// TODO: This is broken. How can I get a GdkToplevel for a GtkWindow?
	// GListModel *toplevels = gtk_window_get_toplevels();
	// int idx;
	// for (idx = 0; idx < g_list_model_get_n_items(toplevels); idx++)
	//     gdk_toplevel_set_icon_list(GDK_TOPLEVEL(g_list_model_get_item(toplevels, idx)), icons);

	g_list_foreach(icons, (GFunc)(void *)&g_object_unref, NULL);
	g_list_free(icons);
    }

    // TODO: Re-implement FEAT_CLIENTSERVER, probably based on d-bus instead of X11.
}
// }}}

// focus event handlers {{{
    static void
pointer_enter_event_cb(
	GtkEventControllerFocus *self,
	gpointer user_data UNUSED)
{
    gui_gtk_pointer_enter();
}

    static void
pointer_leave_event_cb(
	GtkEventControllerFocus *self,
	gpointer user_data UNUSED)
{
    gui_gtk_pointer_leave();
}

    static void
focus_enter_event_cb(
	GtkEventControllerFocus *self,
	gpointer user_data)
{
    gui_gtk_focus_enter(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self)));
}

    static void
focus_leave_event_cb(
	GtkEventControllerFocus *self,
	gpointer user_data UNUSED)
{
    gui_gtk_focus_leave();
}
// }}}

// motion notify handler {{{
    static void
motion_notify_event(
	GtkEventControllerMotion* self,
	gdouble x,
	gdouble y,
	gpointer user_data)
{
    gui_gtk_process_motion_notify((int)x, (int)y,
				  gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(self)));
}
// }}}

// mouse and keyboard input handlers {{{
    static gboolean
key_pressed_event_cb(
	GtkEventControllerKey *self,
	guint                 keyval,
	guint                 keycode UNUSED,
	GdkModifierType       state,
	gpointer              user_data UNUSED)
{
    gui.event_time = gtk_event_controller_get_current_event_time((GtkEventController *)self);
    return gui_gtk_process_key_press(keyval, state);
}

    static void
button_press_event_cb(
	GtkGestureClick* self,
	gint n_press,
	gdouble x,
	gdouble y,
	gpointer user_data)
{
    gui_gtk_process_button_press(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self)),
				 gtk_event_controller_get_current_event_time(GTK_EVENT_CONTROLLER(self)),
				 GDK_BUTTON_PRESS,
				 gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(self)),
				 gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(self)),
				 (int)x, (int)y);
}

    static void
button_released_event_cb(
	GtkGestureClick* self,
	gint n_press,
	gdouble x,
	gdouble y,
	gpointer user_data)
{
    gui_gtk_process_button_release(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self)),
				   gtk_event_controller_get_current_event_time(GTK_EVENT_CONTROLLER(self)),
				   GDK_BUTTON_RELEASE,
				   gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(self)),
				   gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(self)),
				   (int)x, (int)y);
}

    static gboolean
scroll_event_cb(
	GtkEventControllerScroll* self,
	gdouble dx,
	gdouble dy,
	gpointer user_data UNUSED)
{
    int	        button = 0;  // silence gcc
    int_u       vim_modifiers;
    GtkWidget  *widget = gtk_event_controller_get_widget((GtkEventController *)self);

    if (!gtk_widget_has_focus(widget))
	gtk_widget_grab_focus(widget);

    GtkEventControllerScrollFlags scroll_flags = gtk_event_controller_scroll_get_flags(self);

    // Since we always scroll in discrete units, scrolling on both axes with
    // deltas of different magnitudes feels strange to the user. So we pretend
    // each scroll is in only one direction, prioritizing vertical scrolling
    // because it is the more common case.
    if (scroll_flags & GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES)
	if (fabs(dx) > fabs(dy))
	    dy = 0.0;
	else
	    dx = 0.0;

    if (dy != 0.0)
	button = (dy < 0.0) ? MOUSE_4 : MOUSE_5;
    else if (dx != 0.0)
	button = (dx < 0.0) ? MOUSE_7 : MOUSE_6;

    vim_modifiers = gui_gtk_modifiers_gdk2mouse(
	    gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(self)));

    gint x, y;
    GdkModifierType mod_state;
    gui_gtk_get_pointer(gui.drawarea, &x, &y, &mod_state);
    gui_send_mouse_event(button, (int)x, (int)y, FALSE, vim_modifiers);

    return TRUE;
}

// }}}

// drawing {{{

    static void
set_cairo_source_rgba_from_color(cairo_t *cr, guicolor_T color)
{
    float red, green, blue;
    red   = ((color & 0xff0000) >> 16) / 255.0;
    green = ((color & 0x00ff00) >> 8) / 255.0;
    blue  = ((color & 0x0000ff)) / 255.0;
    cairo_set_source_rgba(cr, red, green, blue, 1.0);
}

/*
 * Clear a rectangular region of the screen from text pos (row1, col1) to
 * (row2, col2) inclusive.
 */
    void
gui_mch_clear_block(int row1arg, int col1arg, int row2arg, int col2arg)
{
    if (gtk_widget_get_realized(gui.drawarea) == FALSE)
	return;

    int col1 = check_col(col1arg);
    int col2 = check_col(col2arg);
    int row1 = check_row(row1arg);
    int row2 = check_row(row2arg);

    if (gtk_widget_get_realized(gui.drawarea) == FALSE)
	return;

    {
	// Add one pixel to the far right column in case a double-stroked
	// bold glyph may sit there.
	const GdkRectangle rect = {
	    FILL_X(col1), FILL_Y(row1),
	    (col2 - col1 + 1) * gui.char_width + (col2 == Columns - 1),
	    (row2 - row1 + 1) * gui.char_height
	};
	cairo_t * const cr = cairo_create(gui.surface);
	set_cairo_source_rgba_from_color(cr, gui.back_pixel);
	cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
	cairo_fill(cr);
	cairo_destroy(cr);

	gtk_widget_queue_draw_area(gui.drawarea,
		rect.x, rect.y, rect.width, rect.height);
    }
}

    void
gui_mch_clear_all(void)
{
    const GdkRectangle rect = {
	0, 0, gtk_widget_get_width(gui.drawarea), gtk_widget_get_height(gui.drawarea),
    };
    cairo_t * const cr = cairo_create(gui.surface);
    set_cairo_source_rgba_from_color(cr, gui.back_pixel);
    cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
    cairo_fill(cr);
    cairo_destroy(cr);
    gtk_widget_queue_draw_area(gui.drawarea,
	    rect.x, rect.y, rect.width, rect.height);
}

// Flush any output to the screen
    void
gui_mch_flush(void)
{
    if (gui.mainwin != NULL && gtk_widget_get_realized(gui.mainwin))
	gdk_display_flush(gtk_widget_get_display(gui.mainwin));
}

    static void
draw_cb(
	GtkDrawingArea *area,
	cairo_t        *cr,
	int            width,
	int            height,
	gpointer       user_data UNUSED)
{
    gui_gtk_draw_gui_surface_to_widget_surface(cr);
}

    void
gui_mch_new_colors(void)
{
    if (gui.drawarea == NULL)
	return;

    if (gui.formwin == NULL)
	return;

    gchar * const css = g_strdup_printf(
	    "widget#vim-gtk-form {\n"
	    "  background-color: #%.2lx%.2lx%.2lx;\n"
	    "}\n",
	     (gui.back_pixel >> 16) & 0xff,
	     (gui.back_pixel >> 8) & 0xff,
	     gui.back_pixel & 0xff);

    GtkCssProvider *css_provider = gtk_css_provider_new();
#if GTK_CHECK_VERSION(4,12,0)
    gtk_css_provider_load_from_string(css_provider, css);
#else
    gtk_css_provider_load_from_data(css_provider, css, -1);
#endif

    gtk_style_context_add_provider_for_display(gtk_widget_get_display(gui.mainwin),
	    (GtkStyleProvider *)css_provider,
	    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_free(css);
    g_object_unref(css_provider);
}

/*
 * Delete the given number of lines from the given row, scrolling up any
 * text further down within the scroll region.
 */
    void
gui_mch_delete_lines(int row, int num_lines)
{
    gui_clear_block(
	    row,		 gui.scroll_region_left,
	    row + num_lines - 1, gui.scroll_region_right);

    const int ncols = gui.scroll_region_right - gui.scroll_region_left + 1;
    const int nrows = gui.scroll_region_bot - row + 1;
    const int src_nrows = nrows - num_lines;

    gui_gtk_surface_copy_rect(
	    FILL_X(gui.scroll_region_left), FILL_Y(row),
	    FILL_X(gui.scroll_region_left), FILL_Y(row + num_lines),
	    gui.char_width * ncols + 1,     gui.char_height * src_nrows);
    gui_clear_block(
	    gui.scroll_region_bot - num_lines + 1, gui.scroll_region_left,
	    gui.scroll_region_bot,		   gui.scroll_region_right);

    // GTK2/3 used the given parameters to limit the redrawn area. GTK4 is
    // pretty tied to modern, high-throughput displays. We already have the
    // text re-rendered into gui.surface. Therefore a full copy is likely quite
    // efficient.
    gtk_widget_queue_draw(gui.drawarea);
}

    void
gui_mch_insert_lines(int row, int num_lines)
{
    const int ncols = gui.scroll_region_right - gui.scroll_region_left + 1;
    const int nrows = gui.scroll_region_bot - row + 1;
    const int src_nrows = nrows - num_lines;
    gui_gtk_surface_copy_rect(
	    FILL_X(gui.scroll_region_left), FILL_Y(row + num_lines),
	    FILL_X(gui.scroll_region_left), FILL_Y(row),
	    gui.char_width * ncols + 1,     gui.char_height * src_nrows);
    gui_clear_block(
	    row,		 gui.scroll_region_left,
	    row + num_lines - 1, gui.scroll_region_right);
    gtk_widget_queue_draw(gui.drawarea);
}


#if defined(FEAT_SIGN_ICONS) || defined(PROTO)
    void
gui_gtk_draw_sign_pixbuf(
	GdkPixbuf *sign,
	int row, int col,
	int sign_width, int sign_height,
	int width, int height,
	int xoffset, int yoffset)
{
    // TODO: This was copied from gui_gtk_x11.c so the implementation is the
    // same as for GTK3 but we should be able to make some optimizations for
    // GTK4.
    cairo_t	    *cr;
    cairo_surface_t *bg_surf;
    cairo_t	    *bg_cr;
    cairo_surface_t *sign_surf;
    cairo_t	    *sign_cr;

    cr = cairo_create(gui.surface);

    bg_surf = cairo_surface_create_similar(gui.surface,
	    cairo_surface_get_content(gui.surface),
	    sign_width, sign_height);
    bg_cr = cairo_create(bg_surf);
    cairo_set_source_rgba(bg_cr,
	    gui.bgcolor->red, gui.bgcolor->green, gui.bgcolor->blue,
	    gui.bgcolor->alpha);
    cairo_paint(bg_cr);

    sign_surf = cairo_surface_create_similar(gui.surface,
	    cairo_surface_get_content(gui.surface),
	    sign_width, sign_height);
    sign_cr = cairo_create(sign_surf);
    gdk_cairo_set_source_pixbuf(sign_cr, sign, -xoffset, -yoffset);
    cairo_paint(sign_cr);

    cairo_set_operator(sign_cr, CAIRO_OPERATOR_DEST_OVER);
    cairo_set_source_surface(sign_cr, bg_surf, 0, 0);
    cairo_paint(sign_cr);

    cairo_set_source_surface(cr, sign_surf, FILL_X(col), FILL_Y(row));
    cairo_paint(cr);

    cairo_destroy(sign_cr);
    cairo_surface_destroy(sign_surf);
    cairo_destroy(bg_cr);
    cairo_surface_destroy(bg_surf);
    cairo_destroy(cr);

    gtk_widget_queue_draw_area(gui.drawarea,
	    FILL_X(col), FILL_Y(col), width, height);
}
#endif // FEAT_SIGN_ICONS

// }}}

// scrollbars {{{

/*
 * Take action upon scrollbar dragging.
 */
    static void
adjustment_value_changed(GtkAdjustment *adjustment, gpointer data)
{
    scrollbar_T	*sb;
    long	value;

    sb = gui_find_scrollbar((long)data);
    value = gtk_adjustment_get_value(adjustment);
    gui_drag_scrollbar(sb, value, FALSE);
}

    void
gui_mch_create_scrollbar(scrollbar_T *sb, int orient)
{
    if (orient == SBAR_HORIZ)
    {
	sb->id = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
	g_object_ref(sb->id);

	if (sb->id && GTK_IS_WIDGET(sb->id) && gtk_widget_get_realized(sb->id))
	    gui.scrollbar_height = gtk_widget_get_height(sb->id);
    }
    else if (orient == SBAR_VERT)
    {
	sb->id = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL);
	g_object_ref(sb->id);

	gui.scrollbar_width = gtk_widget_get_width(sb->id);
    }

    if (sb->id == NULL)
	return;

    GtkAdjustment *adjustment;

    gtk_widget_set_can_focus(sb->id, FALSE);
    gui_gtk_form_put(GTK_FORM(gui.formwin), sb->id, 0, 0);

    gtk_widget_add_css_class(sb->id,
	    (sb->type == SBAR_LEFT)
		? "sbar-left"
		: (sb->type == SBAR_RIGHT)
		    ? "sbar-right"
		    : "sbar-bottom");

    adjustment = gtk_scrollbar_get_adjustment(GTK_SCROLLBAR(sb->id));

    sb->handler_id = g_signal_connect(
	    G_OBJECT(adjustment), "value-changed",
	    G_CALLBACK(adjustment_value_changed),
	    GINT_TO_POINTER(sb->ident));
    gui_mch_update();
}

    void
gui_mch_destroy_scrollbar(scrollbar_T *sb)
{
    if (sb->id != NULL)
    {
	g_return_if_fail(G_IS_OBJECT(sb->id));
	gui_gtk_form_remove_child(GTK_FORM(gui.formwin), GTK_WIDGET(sb->id));
	g_object_unref(sb->id);
	sb->id = NULL;
    }
    gui_mch_update();
}

    void
gui_mch_enable_scrollbar(scrollbar_T *sb, int flag)
{
    if (sb->id == NULL)
	return;

    gtk_widget_set_visible(sb->id, flag);
}

    void
gui_mch_set_scrollbar_pos(scrollbar_T *sb, int x, int y, int w, int h)
{
    char **css_classes = NULL;
    int css_idx;
    if (sb->id && GTK_IS_WIDGET(sb->id) && gtk_widget_get_realized(sb->id))
	if (sb->type == SBAR_BOTTOM)
	{
	    gui.scrollbar_height = gtk_widget_get_height(sb->id);
	    gui_gtk_form_move_resize(GTK_FORM(gui.formwin), sb->id, x, y, w, gui.scrollbar_height);
	}
	else
	{
	    gui.scrollbar_width = gtk_widget_get_width(sb->id);
	    gui_gtk_form_move_resize(GTK_FORM(gui.formwin), sb->id, x, y, gui.scrollbar_width, h);
	}
}

    int
gui_mch_get_scrollbar_ypadding(void)
{
    int ypad = gtk_widget_get_allocated_height(gui.formwin)
	     - gtk_widget_get_allocated_height(gui.drawarea)
	     - gui.scrollbar_height;
    return (ypad < 0) ? 0 : ypad;
}

    int
gui_mch_get_scrollbar_xpadding(void)
{
    int xpad = gtk_widget_get_allocated_width(gui.formwin)
	     - gtk_widget_get_allocated_width(gui.drawarea)
	     - gui.scrollbar_width;

    if (gui.which_scrollbars[SBAR_LEFT] && gui.which_scrollbars[SBAR_RIGHT])
	xpad -= gui.scrollbar_width;

    return (xpad < 0) ? 0 : xpad;
}

// }}}

/* The FEAT_XCLIPBOARD code is a mess, so we need to make this a no-op.
 */
    Display *
gui_mch_get_display(void)
{
    return NULL;
}

/* This hook was used in older GTK versions to support GNOME sessions. Since
 * that is a relic of the past, this is a no-op with GTK4.
 */
    void
gui_mch_forked(void)
{
    return;
}

    static void
load_runtime_user_css_file(char_u *path, void *cookie UNUSED)
{
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GFile *infil = g_file_new_for_path((const char *)path);
    gtk_css_provider_load_from_file(css_provider, infil);
    gtk_style_context_add_provider_for_display(gtk_widget_get_display(gui.mainwin),
	    (GtkStyleProvider *)css_provider,
	    GTK_STYLE_PROVIDER_PRIORITY_USER);
    smsg("Loaded CSS file: %s\n", path);
}

    static void
load_all_runtime_user_css_files()
{
    do_in_runtimepath("gvim-gtk.css", DIP_ALL, load_runtime_user_css_file, NULL);
}

/*
 * Initialize the GUI.	Create all the windows, set up all the callbacks etc.
 * Returns OK for success, FAIL when the GUI can't be started.
 */
    int
gui_mch_init(void)
{
    GtkWidget *vbox;

    VIM_CLEAR(gui_argv);

    g_set_application_name("Vim-dev");

    /*
     * Force UTF-8 output no matter what the value of 'encoding' is.
     * did_set_string_option() in option.c prohibits changing 'termencoding'
     * to something else than UTF-8 if the GUI is in use.
     */
    set_option_value_give_err((char_u *)"termencoding",
						     0L, (char_u *)"utf-8", 0);

    // Initialize values
    gui.border_width = 2;
    gui.border_offset = gui.border_width;
    gui.scrollbar_width = SB_DEFAULT_WIDTH;
    gui.scrollbar_height = SB_DEFAULT_WIDTH;

    gui.fgcolor = g_new(GdkRGBA, 1);
    gui.bgcolor = g_new(GdkRGBA, 1);
    gui.spcolor = g_new(GdkRGBA, 1);

    // Set default foreground and background colors.
    gui.norm_pixel = gui.def_norm_pixel;
    gui.back_pixel = gui.def_back_pixel;

    if (gtk_socket_id != 0)
    {
	g_warning("VIM is using GTK4, which lacks support for GTK sockets.");
	gtk_socket_id = 0;
    }

    gui.mainwin = gtk_window_new();
    gtk_widget_set_name(gui.mainwin, "vim-main-window");

    gui_gtk_rebuild_text_context();

    g_signal_connect(G_OBJECT(gui.mainwin), "close-request",
		     G_CALLBACK(&close_request_cb), NULL);

    g_signal_connect(G_OBJECT(gui.mainwin), "realize",
		     G_CALLBACK(&mainwin_realize), NULL);

    // TODO: In GTK3 the screen-changed signal was used to reconstruct the
    // Pango text context. This should probably be done using the GDK Surface
    // notify::scale-factor, enter-monitor, and leave-monitor signals.

    // TODO: Use GtkShortcutController to enable keyboard shortcuts that GTK3
    // enabled with GtkAccelGroup.

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
    gtk_window_set_child(GTK_WINDOW(gui.mainwin), vbox);
    gtk_widget_set_visible(vbox, TRUE);

#ifdef FEAT_MENU
    // TODO: Implement GMenuModel for the VIM menu data structure.
    GMenuModel *main_menu = G_MENU_MODEL(g_menu_new());
    gui.menubar = gtk_popover_menu_bar_new_from_model(main_menu);
    gtk_widget_set_name(gui.menubar, "vim-menubar");
    gtk_widget_set_visible(gui.menubar, TRUE);
    gtk_box_append(GTK_BOX(vbox), gui.menubar);

    // Avoid that GTK takes <F10> away from us.
    {
	// TODO: This setting seems to no longer be available in GTK4
	// GtkSettings *gtk_settings = gui_gtk_get_settings();
	// g_object_set(gtk_settings, "gtk-menu-bar-accel", NULL, NULL);
    }
#endif // FEAT_MENU

#ifdef FEAT_TOOLBAR
    gui.toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_set_homogeneous(GTK_BOX(gui.toolbar), FALSE);
    if (vim_strchr(p_go, GO_TOOLBAR) != NULL
	    && (toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS)))
	gtk_widget_show(gui.toolbar);
    gtk_box_append(GTK_BOX(vbox), gui.toolbar);
#endif // FEAT_TOOLBAR

#ifdef FEAT_GUI_TABLINE
    create_tabline();
    gtk_box_append(GTK_BOX(vbox), gui.tabline);
#endif // FEAT_GUI_TABLINE

    gui.formwin = GTK_WIDGET(gui_gtk_form_new());
    gtk_widget_set_hexpand(GTK_WIDGET(gui.formwin), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(gui.formwin), TRUE);
    gtk_widget_set_visible(gui.formwin, TRUE);

    gui.surface = NULL;

    gui.drawarea = (GtkWidget *)gui_gtk_text_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(gui.drawarea), &draw_cb, NULL, NULL);
    gtk_widget_set_visible(gui.drawarea, TRUE);
    gtk_widget_set_visible(gui.formwin, TRUE);
    gui_gtk_form_put(GTK_FORM(gui.formwin), gui.drawarea, 0, 0);
    gtk_box_append(GTK_BOX(vbox), gui.formwin);

    {
	GdkDisplay *disp = gtk_widget_get_display(gui.mainwin);
	clip_star.gtk_clipboard = gdk_display_get_primary_clipboard(disp);
	clip_plus.gtk_clipboard = gdk_display_get_clipboard(disp);
    }

    gui.focus_events = GTK_EVENT_CONTROLLER_FOCUS(gtk_event_controller_focus_new());
    gtk_widget_add_controller(gui.mainwin, GTK_EVENT_CONTROLLER(gui.focus_events));

    gui.key_events = GTK_EVENT_CONTROLLER_KEY(gtk_event_controller_key_new());
    gtk_widget_add_controller(gui.mainwin, GTK_EVENT_CONTROLLER(gui.key_events));

    gui.motion_events = GTK_EVENT_CONTROLLER_MOTION(gtk_event_controller_motion_new());
    gtk_widget_add_controller(gui.drawarea, GTK_EVENT_CONTROLLER(gui.motion_events));

    g_signal_connect(G_OBJECT(gui.key_events), "key-pressed",
		     G_CALLBACK(key_pressed_event_cb), NULL);
    /*
     * Only install these enter/leave callbacks when 'p' in 'guioptions'.
     * Only needed for some window managers.
     */
    if (vim_strchr(p_go, GO_POINTER) != NULL)
    {
	g_signal_connect(G_OBJECT(gui.motion_events), "leave",
			 G_CALLBACK(pointer_leave_event_cb), NULL);
	g_signal_connect(G_OBJECT(gui.motion_events), "enter",
			 G_CALLBACK(pointer_enter_event_cb), NULL);
    }
    g_signal_connect(G_OBJECT(gui.motion_events), "motion",
		     G_CALLBACK(motion_notify_event), NULL);

    g_signal_connect(G_OBJECT(gui.drawarea), "realize",
		     G_CALLBACK(drawarea_realize_cb), NULL);
    g_signal_connect(G_OBJECT(gui.drawarea), "unrealize",
		     G_CALLBACK(drawarea_unrealize_cb), NULL);

    g_signal_connect(G_OBJECT(gui.focus_events), "leave",
	    G_CALLBACK(focus_leave_event_cb), NULL);
    g_signal_connect(G_OBJECT(gui.focus_events), "enter",
	    G_CALLBACK(focus_enter_event_cb), NULL);

    GtkGestureClick *draw_area_click_events = (GtkGestureClick *)gtk_gesture_click_new();
    g_signal_connect(G_OBJECT(draw_area_click_events), "pressed",
		     G_CALLBACK(button_press_event_cb), gui.drawarea);
    g_signal_connect(G_OBJECT(draw_area_click_events), "released",
		     G_CALLBACK(button_released_event_cb), NULL);
    gtk_widget_add_controller(GTK_WIDGET(gui.drawarea),
			      GTK_EVENT_CONTROLLER(draw_area_click_events));

    GtkEventControllerScroll *draw_area_scroll_events =
	(GtkEventControllerScroll *)gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES
								    | GTK_EVENT_CONTROLLER_SCROLL_DISCRETE);
    g_signal_connect(G_OBJECT(draw_area_scroll_events), "scroll",
		     G_CALLBACK(scroll_event_cb), NULL);
    gtk_widget_add_controller(GTK_WIDGET(gui.drawarea),
			      GTK_EVENT_CONTROLLER(draw_area_scroll_events));

    gui.in_focus = FALSE;

    {
	GtkSettings *gtk_settings = gui_gtk_get_settings();

	g_signal_connect(gtk_settings, "notify::gtk-xft-dpi",
			 G_CALLBACK(gui_gtk_settings_xft_dpi_changed_cb), NULL);
    }

    load_all_runtime_user_css_files();

    {
	GVariantType *ptype = g_variant_type_new("t");
	GSimpleAction *menu_item_action = g_simple_action_new("activate-menu-item", ptype);
	g_simple_action_set_enabled(menu_item_action, TRUE);
	g_signal_connect(G_OBJECT(menu_item_action), "activate",
			 G_CALLBACK(activate_menu_item_cb), NULL);

	gui.menu_actions = g_simple_action_group_new();
	g_simple_action_group_insert(gui.menu_actions, G_ACTION(menu_item_action));
	gtk_widget_insert_action_group(gui.mainwin, "vim", G_ACTION_GROUP(gui.menu_actions));
    }

    gtk_window_set_focus(GTK_WINDOW(gui.mainwin), GTK_WIDGET(gui.drawarea));
    return OK;
}

/*
 * Open the GUI window which was created by a call to gui_mch_init().
 */
    int
gui_mch_open(void)
{
    guicolor_T fg_pixel = INVALCOLOR;
    guicolor_T bg_pixel = INVALCOLOR;
    guint		pixel_width;
    guint		pixel_height;

    pixel_width = (guint)(gui_get_base_width() + Columns * gui.char_width);
    pixel_height = (guint)(gui_get_base_height() + Rows * gui.char_height);
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(gui.drawarea), pixel_width);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(gui.drawarea), pixel_height);

    if (foreground_argument != NULL)
	fg_pixel = gui_get_color((char_u *)foreground_argument);
    if (fg_pixel == INVALCOLOR)
	fg_pixel = gui_get_color((char_u *)"Black");

    if (background_argument != NULL)
	bg_pixel = gui_get_color((char_u *)background_argument);
    if (bg_pixel == INVALCOLOR)
	bg_pixel = gui_get_color((char_u *)"White");

    if (found_reverse_arg)
    {
	gui.def_norm_pixel = bg_pixel;
	gui.def_back_pixel = fg_pixel;
    }
    else
    {
	gui.def_norm_pixel = fg_pixel;
	gui.def_back_pixel = bg_pixel;
    }

    // Get the colors from the "Normal" and "Menu" group (set in syntax.c or
    // in a vimrc file)
    set_normal_colors();

    // Check that none of the colors are the same as the background color
    gui_check_colors();

    // Get the colors for the highlight groups (gui_check_colors() might have
    // changed them).
    highlight_gui_started();	// re-init colors and fonts

    g_signal_connect(G_OBJECT(gui.mainwin), "destroy",
		     G_CALLBACK(gui_gtk_mainwin_destroy_cb), NULL);

    g_signal_connect(gui.drawarea, "notify::display", G_CALLBACK(drawarea_reconstruct_surface), NULL);
    g_signal_connect(gui.drawarea, "resize", G_CALLBACK(drawarea_resize_cb), NULL);
    g_signal_connect(gui.formwin, "resize", G_CALLBACK(formwin_resize_cb), NULL);

#ifdef FEAT_DND
    // TODO: How to support DND on GTK4?
#endif

    // With GTK+ 2, we need to iconify the window before calling show()
    // to avoid mapping the window for a short time.
    if (found_iconic_arg)
	gui_mch_iconify();

    gtk_widget_set_visible(gui.mainwin, TRUE);

    return OK;
}

    int
gui_mch_get_winpos(int *x UNUSED, int *y UNUSED)
{
    // Because GTK4 is trying to divorce X11, it makes it really difficult to
    // do this. If this turns out to be important, we might be able to shim an
    // X11-specific workaround in here.
    return FAIL;
}

    void
gui_mch_set_winpos(int x UNUSED, int y UNUSED)
{
    // Similar to gui_mch_get_winpos, GTK4 makes this really difficult because
    // they see Wayland's lack of core functionality around global positioning
    // as the future.
    return;
}

/*
 * Return TRUE if the main window is maximized.
 */
    int
gui_mch_maximized(void)
{
    if (gui.mainwin == NULL)
	return FALSE;

    GdkSurface *surf = gui_gtk_widget_get_containing_surface(gui.mainwin);
    if (surf == NULL)
	return FALSE;
    return (gdk_toplevel_get_state(GDK_TOPLEVEL(surf)) &
	    GDK_TOPLEVEL_STATE_MAXIMIZED);
}

// text rendering {{{
    static gboolean
monospace_font_filter_func(GObject *item, gpointer user_data)
{
    PangoFontFamily *family = NULL;

    if (PANGO_IS_FONT_FAMILY(item))
	family = PANGO_FONT_FAMILY(item);
    else if (PANGO_IS_FONT_FACE(item))
	family = pango_font_face_get_family(PANGO_FONT_FACE(item));
    else
	return FALSE;

    return pango_font_family_is_monospace(family);
}

    static void
font_chosen_cb(GtkFontDialog	*dialog,
	       GAsyncResult	*res,
	       gpointer		 user_data)
{
    PangoFontDescription *newdesc = NULL;
    GError		 *error = NULL;
    newdesc = gtk_font_dialog_choose_font_finish(dialog, res, &error);
    if (newdesc == NULL)
    {
	emsg(error->message);
	g_error_free(error);
	return;
    }

    *((char_u **)user_data) = pango_font_description_to_string(newdesc);
    g_free(newdesc);
}

/*
 * Put up a font dialog and return the selected font name in allocated memory.
 * "oldval" is the previous value.  Return NULL when cancelled.
 * TODO: GTK2/3 was calling string_convert. What platform is this for? Is GTK4
 * available on those platforms?
 */
    char_u *
gui_mch_font_dialog(char_u *oldval)
{
    GtkWidget	*dialog;
    int		response;
    char_u	*fontname = NULL;
    char_u	*oldname;
    PangoFontDescription *oldfont = NULL;
    GtkFilter	*filter;

    dialog = (GtkWidget *)gtk_font_dialog_new();
    filter = GTK_FILTER(gtk_custom_filter_new((GtkCustomFilterFunc)monospace_font_filter_func, NULL, NULL));
    gtk_font_dialog_set_filter(GTK_FONT_DIALOG(dialog), filter);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gui.mainwin));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

    // TODO: GTK2/3 had a bug that required the font name to include a size.
    // This is being omitted for now to test whether GTK4 still has the same
    // bug.
    oldfont = pango_font_description_from_string(oldval);

    // The font_chosen_cb is expected to allocate memory and assign it to fontname.
    gtk_font_dialog_choose_font(GTK_FONT_DIALOG(dialog),
	GTK_WINDOW(gui.mainwin),
	oldfont,
	NULL, (GAsyncReadyCallback)font_chosen_cb, &fontname);

    /*
     * TODO: VIM's synchronous model is incompatible with the GTK4 async dialog
     * API but we'll fake it for now.
     */
    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	if (fontname != NULL)
	{
	    g_free(fontname);
	    fontname = NULL;
	}

    gtk_window_destroy(GTK_WINDOW(dialog));
    g_free(oldfont);
    return fontname;
}

    static int
get_menu_tool_width(void)
{
    int width = 0;
#ifdef FEAT_MENU
    if (gui.menubar != NULL)
	width += gtk_widget_get_width(gui.menubar);
#endif
    // TODO: Implement FEAT_MENU and FEAT_GUI_TABLINE
    return width;
}

    static int
get_menu_tool_height(void)
{
    int height = 0;
#ifdef FEAT_MENU
    if (gui.menubar != NULL)
	height += gtk_widget_get_height(gui.menubar);
#endif
    // TODO: Implement FEAT_MENU and FEAT_GUI_TABLINE
    return height;
}

/*
 * Called when the font changed while the window is maximized or GO_KEEPWINSIZE
 * is set.  Compute the new Rows and Columns.  This is like resizing the
 * window.
 */
    void
gui_mch_newfont(void)
{
    int w, h;
    w = gtk_widget_get_size(gui.formwin, GTK_ORIENTATION_HORIZONTAL);
    h = gtk_widget_get_size(gui.formwin, GTK_ORIENTATION_VERTICAL);
    w -= get_menu_tool_width();
    h -= get_menu_tool_height();
    gui_resize_shell(w, h);
}

    void
gui_mch_flash(int msec)
{
    // TODO: This was ifndef'd out for GTK3. Probably useful to implement though.
}

// }}}

    void
gui_mch_iconify(void)
{
    gdk_toplevel_minimize(GDK_TOPLEVEL(gui.mainwin));
}

#if defined(FEAT_EVAL) || defined(PROTO)
/*
 * Bring the Vim window to the foreground.
 */
    void
gui_mch_set_foreground(void)
{
    // Apparently GTK3 found this approach unreliable. However GTK4 lacks the
    // pieces to use the alternative solution used with GTK3. TODO: Test this.
    gtk_window_present((GtkWindow *)gui.mainwin);
    gui_may_flush();
}
#endif

// TODO: Pick up here with gui_mch_set_shellsize
    void
gui_mch_set_shellsize(int width, int height,
		      int min_width UNUSED,  int min_height UNUSED,
		      int base_width UNUSED, int base_height UNUSED,
		      int direction UNUSED)
{
    // give GTK+ a chance to put all widget's into place
    gui_mch_update();

    // TODO: Are the 'width' and 'height' values really supposed to be the
    // shell window size? The other GUI code seems to treat them as the size of
    // the text area.
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(gui.drawarea), width);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(gui.drawarea), height);
}

#if defined(EXITFREE)
    void
gui_mch_free_all(void)
{
    vim_free(gui_argv);
}
#endif

    void
gui_mch_exit(int rc UNUSED)
{
    // Clean up, unless we don't want to invoke free().
    if (gui.mainwin != NULL && !really_exiting)
	gtk_window_destroy(GTK_WINDOW(gui.mainwin));
}

// clipboard {{{

#define RS_NONE	0	// selection_received_cb() not called yet
#define RS_OK	1	// selection_received_cb() called and OK
#define RS_FAIL	2	// selection_received_cb() called and failed
static int received_selection = RS_NONE;

    static void
selection_received_cb(
	GObject* source_object,
	GAsyncResult *res,
	gpointer user_data)
{
#define GTK4_CLIPBOARD_SCRATCH_LEN 1024
    garray_T buf;
    char_u *scratch = alloc(GTK4_CLIPBOARD_SCRATCH_LEN);
    gssize bytes_read = 0;
    const char *mime_type = NULL;
    GError *error = NULL;
    Clipboard_T *cbd = (Clipboard_T *)user_data;

    ga_init2(&buf, 1, GTK4_CLIPBOARD_SCRATCH_LEN);
    received_selection = RS_FAIL;

    // TODO: Sometimes the CLIPBOARD has these formats... why?
    // CLIPBOARD FORMATS: gchararray text/html text/_moz_htmlcontext text/_moz_htmlinfo text/plain;charset=utf-8 text/plain text/x-moz-url-priv
    // CLIPBOARD FORMATS: gchararray text/html text/_moz_htmlcontext text/_moz_htmlinfo text/plain;charset=utf-8 text/plain text/x-moz-url-priv
    // Sometimes the PRIMARY has no formats. Should we bail out early?
    GdkContentFormats *formats = gdk_clipboard_get_formats(cbd->gtk_clipboard);
    if (gdk_content_formats_is_empty(formats) == TRUE)
    {
	emsg("Clipboard advertises no data formats.");
	goto cleanup;
    }
    else
    {
	semsg("Clipboard has formats: %s", gdk_content_formats_to_string(formats));
    }

    const char *text = gdk_clipboard_read_text_finish(cbd->gtk_clipboard, res, &error);
    if (error != NULL)
    {
	semsg("Error reading clipboard: %s", error->message);
	g_error_free(error);
	goto cleanup;
    }

    ga_grow(&buf, strlen(text));
    ga_concat_len(&buf, text, strlen(text));
    ga_append(&buf, NUL);
    clip_yank_selection(MAUTO, buf.ga_data, (long)g_utf8_strlen(buf.ga_data, -1), cbd);
    received_selection = RS_OK;

cleanup:
    ga_clear(&buf);
    vim_free(scratch);
#undef GTK4_CLIPBOARD_SCRATCH_LEN
}

/*
 * Get the current selection and put it in the clipboard register.
 */
    void
clip_mch_request_selection(Clipboard_T *cbd)
{
    // TODO: What is the encoding of the text in a GTK4 clipboard. Does it
    // differ if the backend is X11 vs Wayland?

    gdk_clipboard_read_text_async(
	    cbd->gtk_clipboard,
	    NULL,
	    &selection_received_cb,
	    (gpointer)cbd);


    received_selection = RS_NONE;
    time_t start = time(NULL);
    while (received_selection == RS_NONE && time(NULL) < start + 1)
	g_main_context_iteration(NULL, TRUE);	// wait for selection_received_cb

    if (received_selection == RS_NONE)
	semsg("No content from clipboard after waiting 1 second.");
}

/*
 * Disown the selection. However GTK4 hides clipboard selection ownership. So
 * mainly do nothing.
 */
    void
clip_mch_lose_selection(Clipboard_T *cbd UNUSED)
{
    gui_mch_update();
}

/*
 * Own the selection and return OK if it worked. However GTK4 hides clipboard
 * selection ownership. So mainly do nothing.
 */
    int
clip_mch_own_selection(Clipboard_T *cbd)
{
    // Since GTK4 hides clipboard selection ownership, even when using the X11
    // backend, always return OK to ensure that Vim's internal selection model
    // considers us the owner. This mainly matters to prevent vim from stalling
    // while waiting for a non-existant clipboard owner to deliver content while
    // Vim reads the "+ register.
    return OK;
}

/*
 * Send the current selection to the clipboard.
 */
    void
clip_mch_set_selection(Clipboard_T *cbd)
{
    // TODO: For the first pass, the clipboard only supports text. We likely
    // need to support TARGET_VIM and TARGET_VIMENC.

    char_u	    *string;
    char_u	    *tmpbuf;
    long_u	    length;
    int		    copylen;
    int		    motion_type;

    // get the selection from the '*'/'+' register
    clip_get_selection(cbd);
    motion_type = clip_convert_selection(&string, &length, cbd);
    if (motion_type < 0 || string == NULL)
	return;

    if (output_conv.vc_type != CONV_NONE)
    {
	tmpbuf = string_convert(&output_conv, string, NULL);
	vim_free(string);
	if (tmpbuf == NULL)
	    return;
	string = tmpbuf;
    }

    // Validate the string to avoid runtime warnings
    if (g_utf8_validate((const char *)string, (gssize)length, NULL))
    {
	gdk_clipboard_set_text(cbd->gtk_clipboard, string);
    }
    else
    {
	// TODO: This should probably provide a warning. It does not do so in the GTK3 code though.
    }
    vim_free(string);
}

    int
clip_gtk_owner_exists(Clipboard_T *cbd)
{
    return FALSE;
}

// }}}

// FEAT_TOOLBAR {{{

#if defined(FEAT_TOOLBAR) || defined(PROTO)
    void
gui_mch_menu_set_tip(vimmenu_T *menu)
{
    // In GTK4 the tooltips for toolbar items are setup statically in
    // add_toolbar_item.
}

    void
gui_mch_show_toolbar(int showit)
{
    if (gui.toolbar == NULL)
	return;

    if (!showit != !gtk_widget_get_visible(GTK_WIDGET(gui.toolbar)))
    {
	gtk_widget_set_visible(GTK_WIDGET(gui.toolbar), showit);
    }
}

    static void
toolbar_item_clicked_cb(
	GtkButton *self,
	gpointer user_data)
{
    vimmenu_T *menu = (vimmenu_T *)user_data;
    if (menu != NULL)
	gui_menu_cb(menu);
}

    static void
add_toolbar_item(vimmenu_T *menu, int idx)
{
    if (gui.toolbar == NULL)
	return;

    if (menu_is_separator(menu->name))
    {
	menu->id = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    }
    else
    {
	char_u *text = CONVERT_TO_UTF8(menu->dname);

	int direction = (toolbar_flags & TOOLBAR_HORIZ) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
	GtkWidget *layout = gtk_box_new(direction, 0);
	GtkWidget *btn = gtk_button_new();
	gtk_widget_set_can_focus(btn, FALSE);
	gtk_button_set_has_frame(GTK_BUTTON(btn), FALSE);
	if (toolbar_flags & TOOLBAR_ICONS)
	{
	    GtkWidget *icon = gui_gtk_create_menu_icon(menu, -1);
	    gtk_box_append(GTK_BOX(layout), icon);
	}
	if (toolbar_flags & TOOLBAR_TEXT)
	{
	    GtkWidget *lbl = gtk_label_new(text);
	    gtk_box_append(GTK_BOX(layout), lbl);
	}
	gtk_button_set_child(GTK_BUTTON(btn), layout);

	if (toolbar_flags & TOOLBAR_TOOLTIPS)
	{
	    char_u *tooltip = CONVERT_TO_UTF8(menu->strings[MENU_INDEX_TIP]);
	    if (tooltip != NULL)
	    {
		if (!utf_valid_string(tooltip, NULL))
		    CONVERT_TO_UTF8_FREE(tooltip);
		gtk_widget_set_tooltip_text(GTK_WIDGET(btn), tooltip);
		CONVERT_TO_UTF8_FREE(tooltip);
	    }
	}

	g_signal_connect(G_OBJECT(btn), "clicked",
			 G_CALLBACK(toolbar_item_clicked_cb), (gpointer)menu);
	CONVERT_TO_UTF8_FREE(text);
	menu->id = btn;
    }

    if (menu->id != NULL)
	gtk_box_append(GTK_BOX(gui.toolbar), menu->id);
}

#endif // FEAT_TOOLBAR

// }}}

// FEAT_MENU {{{

    static vimmenu_T *
walk_up_to_root_menu(vimmenu_T *menu)
{
    vimmenu_T *cur_menu = menu;

    while (cur_menu != NULL)
    {
	if (cur_menu->parent == NULL)
	{
	    return cur_menu;
	}
	cur_menu = cur_menu->parent;
    };

    return NULL;
}

    void
gui_mch_toggle_tearoffs(int enable UNUSED)
{
    // Do nothing
}

    void
gui_mch_show_popupmenu(vimmenu_T *menu)
{
    GdkRectangle cursor_loc = {
	TEXT_X(curwin->w_wincol + curwin->w_wcol),
	TEXT_Y(W_WINROW(curwin) + curwin->w_wrow),
	1,
	1
    };
    gtk_popover_set_pointing_to(GTK_POPOVER(menu->id), &cursor_loc);
    gtk_popover_popup(GTK_POPOVER(menu->id));
    gtk_widget_grab_focus(gui.drawarea);
    gui_redraw(0, 0, screen_Rows, screen_Columns);
    gtk_widget_queue_draw(gui.drawarea);
    gui_mch_update();
}

/*
 * Enable or disable accelerators for the toplevel menus.
 */
    void
gui_gtk_set_mnemonics(int enable)
{
    // TODO: Implement this
}

/*
 * Destroy the machine specific menu widget.
 */
    void
gui_mch_destroy_menu(vimmenu_T *menu)
{
#ifdef FEAT_TOOLBAR
    if ((menu->parent != NULL) && menu_is_toolbar(menu->parent->name))
    {
	if (menu->id != NULL)
	{
	    gtk_box_remove(GTK_BOX(gui.toolbar), menu->id);
	}
	return;
    }
#endif

    if (menu->gobj_model_self != NULL)
    {
	g_object_unref(menu->gobj_model_self);
	menu->gobj_model_self = NULL;
    }

    // TODO: This is commented out because I'm pretty sure the window destroy
    // cascades down the hierarchy, meaning for GTK4 we don't have to clean up
    // each menu widget individually. -- How can I verify this though?
    // if (menu->id != NULL)
    // {
	// vimmenu_T *menu_root = walk_up_to_root_menu(menu);
	// if (menu_root != NULL)
	// {
	    // if (menu_is_menubar(menu_root->name))
	    // {
		// // gtk_popover_menu_bar_remove_child(GTK_POPOVER_MENU_BAR(gui.menubar), menu->id);
	    // }
	    // else
	    // {
		// // This should be automatically handled by GtkVimTextArea for context menus.
		// // gtk_popover_menu_remove_child(GTK_POPOVER_MENU(menu_root->id), menu->id);
	    // }
	// }
    // }
}

    void
gui_mch_enable_menu(int showit)
{
    if (!gui.starting)
	gtk_widget_set_visible(gui.menubar, (gboolean)showit);
}

/*
 * Make menu item hidden or not hidden.
 */
    void
gui_mch_menu_hidden(vimmenu_T *menu, int hidden)
{
    if (menu->id == NULL)
	return;
    gtk_widget_set_visible(menu->id, (gboolean)hidden);
    gui_mch_update();
}

    static GtkWidget *
menu_item_new(vimmenu_T *menu, const char *text)
{
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *label = gtk_label_new_with_mnemonic(text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_box_append(GTK_BOX(layout), label);

    GtkWidget *spacer = gtk_label_new("    ");
    gtk_widget_set_hexpand(spacer, FALSE);
    gtk_box_append(GTK_BOX(layout), spacer);

    if (menu->actext != NULL && menu->actext[0] != NUL)
    {
	char_u *accel_text = CONVERT_TO_UTF8(menu->actext);
	GtkWidget *accel_label = gtk_label_new((const char *)accel_text);
	gtk_label_set_xalign(GTK_LABEL(accel_label), 1.0);
	gtk_label_set_justify(GTK_LABEL(accel_label), GTK_JUSTIFY_RIGHT);
	gtk_widget_set_hexpand(accel_label, false);
	gtk_box_append(GTK_BOX(layout), accel_label);
	CONVERT_TO_UTF8_FREE(accel_text);
    }

    GtkWidget *button = gtk_button_new();
    gtk_widget_add_css_class(button, "vim-menu-item-button");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), "vim.activate-menu-item");
    gtk_actionable_set_action_target(GTK_ACTIONABLE(button), "t", (guintptr)menu);

    gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
    gtk_button_set_child(GTK_BUTTON(button), layout);
    return button;
}

    static gboolean
menu_position_within_parent(vimmenu_T *menu, int *out_position)
{
    if ((menu == NULL) || (menu->parent == NULL))
	return FALSE;

    vimmenu_T *walk = menu->parent->children;
    int position = 0;
    while ((walk != NULL) && (walk != menu))
    {
	position++;
	walk = walk->next;
    }

    if (walk == NULL)
	return FALSE;

    *out_position = position;
    return TRUE;
}

#if 0
    static GtkWidget *
gui_gtk_load_menu_iconfile(char_u *name)
{
    GtkWidget	    *image = NULL;
    // In GTK4 actual pixel sizes are determined by themes via CSS. Therefore
    // the pixbuf data should not be constrained to a specific pixel size.
    int		     pixel_size = -1;

    GdkPixbuf * const pixbuf
	= gdk_pixbuf_new_from_file_at_scale((const char *)name,
		pixel_size, pixel_size, TRUE, NULL);
    if (pixbuf != NULL)
    {
	image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
    }

    if (image == NULL)
	image = gtk_image_new_from_icon_name("image-missing");
    return image;
}
#endif

    static void
activate_menu_item_cb(
	GSimpleAction *action,
	GVariant *param,
	gpointer user_data UNUSED)
{
    vimmenu_T *menu = (vimmenu_T *)(guintptr)g_variant_get_uint64(param);
    if (menu != NULL)
	gui_menu_cb(menu);
}

    void
gui_mch_add_menu_item(vimmenu_T *menu, int idx)
{
    vimmenu_T *parent = menu->parent;
    menu->id = NULL;
    menu->submenu_id = NULL;
    menu->item_action = NULL;

    menu->gtk_attr_label = NULL;
    menu->gtk_attr_action = NULL;
    menu->gtk_attr_target = NULL;
    menu->gtk_attr_custom = NULL;

#ifdef FEAT_TOOLBAR
    if (menu_is_toolbar(parent->name))
    {
	add_toolbar_item(menu, idx);
	return;
    }
#endif

    // No parent, must be a non-menubar menu
    if (parent == NULL || parent->gobj_model_self == NULL)
	return;

    vimmenu_T *menu_root = walk_up_to_root_menu(menu);
    if (menu_root == NULL)
    {
	semsg("Could not find root menu from %s", menu->name);
	return;
    }

    // int use_mnemonic = (p_wak[0] != 'n');
    // char_u *text = gui_gtk_translate_mnemonic_tag(menu->name, use_mnemonic);

    // RANT: The GMenu API provided by glib seems very tied to the idea of
    // static menus that are usually defined via an XML definition that is fed
    // to GtkBuilder. The way GTK3 and later use that API is overly abstract,
    // attempting to hide the fact that the menu is ultimately a tree of
    // widgets. In order to (de-)activate the widget for a menu item we need to
    // (de-)activate the associated action and we need to do so by name. This
    // requires having a unique action name for each menu item -- despite the
    // same callback being used for every action. The Vim menu item names
    // contain characters that the GTK menu system forbids. Vim also places the
    // same vimmenu_T struct at multiple locations within the menu tree, which
    // means we cannot use the address of the struct alone to disambiguate the
    // names. This is what motivates the call to menu_position_within_parent()
    // call below. I'm sorry it is slow.
    int position = -1;
    if (menu_position_within_parent(menu, &position) == FALSE)
	return;

    if (menu_is_separator(menu->name))
    {
	menu->id = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	menu->gtk_attr_custom = g_variant_ref_sink(g_variant_new("s", menu->name));
	if (menu_is_menubar(menu_root->name))
	{
	    gtk_popover_menu_bar_add_child(GTK_POPOVER_MENU_BAR(gui.menubar), menu->id, menu->name);
	}
	else
	{
	    gtk_popover_menu_add_child(GTK_POPOVER_MENU(menu_root->id), menu->id, menu->name);
	}

	goto end;
    }

    char action_name_buf[44];
    GVariantType *ptype = g_variant_type_new("t");
    snprintf(action_name_buf, sizeof(action_name_buf), "vim.activate-menu-item-%d-%lx", position, (guintptr)menu);
    menu->item_action = g_simple_action_new(&action_name_buf[4], ptype);
    g_simple_action_set_enabled(menu->item_action, TRUE);
    g_signal_connect(G_OBJECT(menu->item_action), "activate",
		     G_CALLBACK(activate_menu_item_cb), NULL);
    {
	// Retain a copy of the relevant attributes in gvariant form for efficient menu iteration by GTK.
	char_u *text = gui_gtk_translate_mnemonic_tag(menu->name, TRUE);
	menu->gtk_attr_label = g_variant_ref_sink(g_variant_new("s", text));
	vim_free(text);

	if (menu->item_action != NULL)
	{
	    g_simple_action_set_enabled(menu->item_action, TRUE);
	    GValue prop_name = G_VALUE_INIT;
	    g_object_get_property(G_OBJECT(menu->item_action), "name", &prop_name);
	    char action_name_buf[44];
	    snprintf(action_name_buf, sizeof(action_name_buf), "vim.%s", g_value_get_string(&prop_name));
	    menu->gtk_attr_action = g_variant_ref_sink(g_variant_new("s", action_name_buf));
	    menu->gtk_attr_target = g_variant_ref_sink(g_variant_new("t", (guintptr)menu));
	}

	if (menu->id != NULL)
	{
	    menu->gtk_attr_custom = g_variant_ref_sink(g_variant_new("s", menu->name));
	}
    }
    g_simple_action_group_insert(gui.menu_actions, G_ACTION(menu->item_action));

end:
    if (position != -1)
	g_menu_model_items_changed(G_MENU_MODEL(menu->parent->gobj_model_self), position, 0, 1);
}

    void
gui_mch_add_menu(vimmenu_T *menu, int idx)
{
    menu->gobj_model_self = g_object_new(GTKVIM_TYPE_MENU, NULL);
    gui_gtk_menu_set_vim_menu(menu->gobj_model_self, menu);
    menu->id = NULL;
    menu->submenu_id = NULL;

    if (menu->name[0] == ']' || menu_is_popup(menu->name))
    {
	menu->id = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu->gobj_model_self));
	gui_gtk_text_area_add_context_menu(GTKVIM_TEXT_AREA(gui.drawarea), menu);
	return;
    }

    int use_mnemonic = (p_wak[0] != 'n');
    char_u *text = gui_gtk_translate_mnemonic_tag(menu->name, use_mnemonic);
    GMenuItem *submenu_item = g_menu_item_new_submenu(text, G_MENU_MODEL(menu->gobj_model_self));

    {
	// Retain a copy of the relevant attributes in gvariant form for efficient menu iteration by GTK.
	menu->gtk_attr_label = g_variant_ref_sink(g_variant_new("s", text));
	menu->gtk_attr_action = NULL;
	menu->gtk_attr_target = NULL;
	menu->gtk_attr_custom = NULL;
    }

    if (menu->parent == NULL)
    {
	if (menu_is_menubar(menu->name))
	{
	    g_menu_insert_item(G_MENU(gtk_popover_menu_bar_get_menu_model(GTK_POPOVER_MENU_BAR(gui.menubar))), idx, submenu_item);
	}
    }
    g_free(submenu_item);

    int position = -1;
    if (menu_position_within_parent(menu, &position) == TRUE)
	g_menu_model_items_changed(G_MENU_MODEL(menu->parent->gobj_model_self), position, 0, 1);
}

/*
 * This is called when ':popup <path_name>' is run from ex mode.
 */
    void
gui_make_popup(char_u *path_name, int mouse_pos)
{
    vimmenu_T	*menu = gui_find_menu(path_name);
    if (menu == NULL)
	return;

    // TODO: In GTK3 the popup_mouse_pos variable is used to affect the
    // location but in GTK4 the cursor position can be quirky. Can this be
    // re-implemented?

    gui_mch_show_popupmenu(menu);
}

/*
 * Make a menu item appear either active or not active (grey or not grey).
 */
    void
gui_mch_menu_grey(vimmenu_T *menu, int grey)
{
    if ((menu->parent == NULL) || (menu->parent->children == NULL))
	return;

    if (menu->item_action == NULL)
	return;

    g_simple_action_set_enabled(menu->item_action, (grey == 0) ? TRUE : FALSE);
}

// }}}

// find and replace dialog {{{

/*
 * We don't create it twice.
 */

typedef struct _SharedFindReplace
{
    GtkWidget *dialog;	// the main dialog widget
    GtkWidget *wword;	// 'Whole word only' check button
    GtkWidget *mcase;	// 'Match case' check button
    GtkWidget *up;	// search direction 'Up' radio button
    GtkWidget *down;	// search direction 'Down' radio button
    GtkWidget *what;	// 'Find what' entry text widget
    GtkWidget *with;	// 'Replace with' entry text widget
    GtkWidget *find;	// 'Find Next' action button
    GtkWidget *replace;	// 'Replace With' action button
    GtkWidget *all;	// 'Replace All' action button
    GtkEventControllerKey *key_events
} SharedFindReplace;

static SharedFindReplace find_widgets = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static SharedFindReplace repl_widgets = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

    static gboolean
find_replace_dialog_direction_is_down(SharedFindReplace *frdp)
{
    return gtk_check_button_get_active(GTK_CHECK_BUTTON(frdp->down));
}

/*
 * Callback for actions of the find and replace dialogs
 */
    static void
find_replace_cb(GtkWidget *widget UNUSED, gpointer data)
{
    int			flags;
    char_u		*find_text;
    char_u		*repl_text;
    gboolean		direction_down;
    SharedFindReplace	*sfr;

    flags = (int)(long)data;	    // avoid a lint warning here

    // Get the search/replace strings from the dialog
    if (flags == FRD_FINDNEXT)
    {
	repl_text = NULL;
	sfr = &find_widgets;
    }
    else
    {
	repl_text = (char_u *)gtk_entry_get_text(GTK_ENTRY(repl_widgets.with));
	sfr = &repl_widgets;
    }

    find_text = (char_u *)gtk_entry_get_text(GTK_ENTRY(sfr->what));
    direction_down = find_replace_dialog_direction_is_down(sfr);

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(sfr->wword)))
	flags |= FRD_WHOLE_WORD;
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(sfr->mcase)))
	flags |= FRD_MATCH_CASE;

    repl_text = CONVERT_FROM_UTF8(repl_text);
    find_text = CONVERT_FROM_UTF8(find_text);
    gui_do_findrepl(flags, find_text, repl_text, direction_down);
    CONVERT_FROM_UTF8_FREE(repl_text);
    CONVERT_FROM_UTF8_FREE(find_text);
}

    static int
find_key_press_event(
    GtkEventControllerKey *events,
    guint                 keyval,
    gint                  keycode,
    GdkModifierType       state,
    SharedFindReplace     *frdp
)
{
    // If the user is holding one of the key modifiers we will just bail out,
    // thus preserving the possibility of normal focus traversal.
    if (state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))
	return FALSE;

    // the Escape key synthesizes a cancellation action
    if (keyval == GDK_KEY_Escape)
    {
	gtk_widget_set_visible(frdp->dialog, FALSE);

	return TRUE;
    }

    // It would be delightful if it where possible to do search history
    // operations on the K_UP and K_DOWN keys here.

    return FALSE;
}

    static void
entry_activate_cb(GtkWidget *widget UNUSED, gpointer data)
{
    gtk_widget_grab_focus(GTK_WIDGET(data));
}

    static void
add_button_to_dialog(
    GtkWidget *area,
    GtkWidget *button,
    gboolean  sensitive
)
{
    gtk_widget_set_sensitive(button, sensitive);
    gtk_box_prepend(GTK_BOX(area), button);
}

/*
 * Returns the number of characters in GtkEntry.
 */
    static unsigned long
entry_get_text_length(GtkEntry *entry)
{
    g_return_val_if_fail(entry != NULL, 0);
    g_return_val_if_fail(GTK_IS_ENTRY(entry) == TRUE, 0);
    return gtk_entry_buffer_get_length(gtk_entry_get_buffer(entry));
}

    static GtkWidget *
create_image_button(const char *label)
{
    char_u	*text;
    GtkWidget	*box;
    GtkWidget	*alignment;
    GtkWidget	*button;

    text = CONVERT_TO_UTF8((char_u *)label);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_box_set_homogeneous(GTK_BOX(box), FALSE);
    gtk_box_prepend(GTK_BOX(box), gtk_label_new((const char *)text));

    CONVERT_TO_UTF8_FREE(text);

    gtk_widget_set_halign(GTK_WIDGET(box), GTK_ALIGN_CENTER);
    gtk_widget_set_valign(GTK_WIDGET(box), GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(GTK_WIDGET(box), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(box), TRUE);

    alignment = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_prepend(GTK_BOX(alignment), box);

    button = gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(button), alignment);
    gtk_widget_set_visible(button, TRUE);

    return button;
}

    void
gui_gtk_find_replace_dialog_create(char_u *arg, int do_replace)
{
    GtkWidget	*hbox;		// main top down box
    GtkWidget	*actionarea;
    GtkWidget	*table;
    GtkWidget	*tmp;
    GtkWidget	*vbox;
    gboolean	sensitive;
    SharedFindReplace *frdp;
    char_u	*entry_text;
    int		wword = FALSE;
    int		mcase = !p_ic;
    char_u	*conv_buffer = NULL;
#   define CONV(message) gui_gtk_convert_localized_message(&conv_buffer, (message))

    frdp = (do_replace) ? (&repl_widgets) : (&find_widgets);

    // Get the search string to use.
    entry_text = get_find_dialog_text(arg, &wword, &mcase);

    if (entry_text != NULL && output_conv.vc_type != CONV_NONE)
    {
	char_u *old_text = entry_text;
	entry_text = string_convert(&output_conv, entry_text, NULL);
	vim_free(old_text);
    }

    /*
     * If the dialog already exists, just raise it.
     */
    if (frdp->dialog)
    {
	if (entry_text != NULL)
	{
	    gtk_entry_set_text(GTK_ENTRY(frdp->what), (char *)entry_text);
	    gtk_check_button_set_active(GTK_CHECK_BUTTON(frdp->wword),
							     (gboolean)wword);
	    gtk_check_button_set_active(GTK_CHECK_BUTTON(frdp->mcase),
							     (gboolean)mcase);
	}
	gtk_window_present(GTK_WINDOW(frdp->dialog));

	// For :promptfind dialog, always give keyboard focus to 'what' entry.
	// For :promptrepl dialog, give it to 'with' entry if 'what' has a
	// non-empty entry; otherwise, to 'what' entry.
	gtk_widget_grab_focus(frdp->what);
	if (do_replace && entry_get_text_length(GTK_ENTRY(frdp->what)) > 0)
	    gtk_widget_grab_focus(frdp->with);

	vim_free(entry_text);
	return;
    }

    frdp->dialog = gtk_dialog_new();
    gtk_widget_set_name(GTK_WIDGET(frdp->dialog), "find-and-replace-dialog");
    frdp->key_events = GTK_EVENT_CONTROLLER_KEY(gtk_event_controller_key_new());

    gtk_window_set_transient_for(GTK_WINDOW(frdp->dialog), GTK_WINDOW(gui.mainwin));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(frdp->dialog), TRUE);

    if (do_replace)
    {
	gtk_window_set_title(GTK_WINDOW(frdp->dialog),
			     CONV(_("VIM - Search and Replace...")));
    }
    else
    {
	gtk_window_set_title(GTK_WINDOW(frdp->dialog),
			     CONV(_("VIM - Search...")));
    }

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);
    gtk_widget_set_name(hbox, "find-and-replace-field-layout");
    {
	GtkWidget * const dialog_vbox
	    = gtk_dialog_get_content_area(GTK_DIALOG(frdp->dialog));
	gtk_box_append(GTK_BOX(dialog_vbox), hbox);
    }

    if (do_replace)
	table = gtk_grid_new();
    else
	table = gtk_grid_new();
    gtk_widget_set_hexpand(table, TRUE);
    gtk_box_prepend(GTK_BOX(hbox), table);

    tmp = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_vexpand(GTK_WIDGET(tmp), TRUE);
    gtk_box_append(GTK_BOX(hbox), tmp);

    tmp = gtk_label_new(CONV(_("Find what:")));
    gtk_label_set_xalign(GTK_LABEL(tmp), 0.0);
    gtk_label_set_yalign(GTK_LABEL(tmp), 0.5);
    gtk_grid_attach(GTK_GRID(table), tmp, 0, 0, 2, 1);
    frdp->what = gtk_entry_new();
    sensitive = (entry_text != NULL && entry_text[0] != NUL);
    if (entry_text != NULL)
	gtk_entry_set_text(GTK_ENTRY(frdp->what), (char *)entry_text);
    g_signal_connect(G_OBJECT(frdp->key_events), "key-pressed",
		     G_CALLBACK(find_key_press_event),
		     (gpointer) frdp);
    gtk_grid_attach(GTK_GRID(table), frdp->what, 2, 0, 5, 1);

    if (do_replace)
    {
	tmp = gtk_label_new(CONV(_("Replace with:")));
	gtk_label_set_xalign(GTK_LABEL(tmp), 0.0);
	gtk_label_set_yalign(GTK_LABEL(tmp), 0.5);
	gtk_grid_attach(GTK_GRID(table), tmp, 0, 1, 2, 1);
	frdp->with = gtk_entry_new();
	g_signal_connect(G_OBJECT(frdp->with), "activate",
			 G_CALLBACK(find_replace_cb),
			 GINT_TO_POINTER(FRD_R_FINDNEXT));

    // TODO: How should key presses be handled for the text inputs?

	gtk_grid_attach(GTK_GRID(table), frdp->with, 2, 1, 5, 1);

	/*
	 * Make the entry activation only change the input focus onto the
	 * with item.
	 */
	g_signal_connect(G_OBJECT(frdp->what), "activate",
			 G_CALLBACK(entry_activate_cb), frdp->with);
    }
    else
    {
	/*
	 * Make the entry activation do the search.
	 */
	g_signal_connect(G_OBJECT(frdp->what), "activate",
			 G_CALLBACK(find_replace_cb),
			 GINT_TO_POINTER(FRD_FINDNEXT));
    }

    // whole word only button
    frdp->wword = gtk_check_button_new_with_label(CONV(_("Match whole word only")));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(frdp->wword),
							(gboolean)wword);
    if (do_replace)
	gtk_grid_attach(GTK_GRID(table), frdp->wword, 0, 2, 5, 1);
    else
	gtk_grid_attach(GTK_GRID(table), frdp->wword, 0, 3, 5, 1);

    // match case button
    frdp->mcase = gtk_check_button_new_with_label(CONV(_("Match case")));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(frdp->mcase),
							(gboolean)mcase);

    if (do_replace)
	gtk_grid_attach(GTK_GRID(table), frdp->mcase, 0, 3, 5, 1);
    else
	gtk_grid_attach(GTK_GRID(table), frdp->mcase, 0, 4, 5, 1);

    tmp = gtk_frame_new(CONV(_("Direction")));
    if (do_replace)
	gtk_grid_attach(GTK_GRID(table), tmp, 5, 2, 2, 4);
    else
	gtk_grid_attach(GTK_GRID(table), tmp, 5, 2, 1, 3);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
    gtk_frame_set_child(GTK_FRAME(tmp), vbox);

    // 'Up' and 'Down' buttons
    frdp->up = gtk_check_button_new_with_label(CONV(_("Up")));
    gtk_box_prepend(GTK_BOX(vbox), frdp->up);
    gtk_widget_set_vexpand(frdp->up, TRUE);

    frdp->down = gtk_check_button_new_with_label(CONV(_("Down")));
    gtk_check_button_set_group(
	    GTK_CHECK_BUTTON(frdp->down),
	    GTK_CHECK_BUTTON(frdp->up));
    gtk_box_prepend(GTK_BOX(vbox), frdp->down);
    gtk_widget_set_vexpand(frdp->down, TRUE);

    // vbox to hold the action buttons
    actionarea = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(GTK_WIDGET(actionarea), "find-and-replace-action-layout");
    gtk_box_append(GTK_BOX(hbox), actionarea);

    // 'Find Next' button
    frdp->find = create_image_button(_("Find Next"));
    gtk_widget_set_sensitive(frdp->find, sensitive);

    g_signal_connect(G_OBJECT(frdp->find), "clicked",
		     G_CALLBACK(find_replace_cb),
		     (do_replace) ? GINT_TO_POINTER(FRD_R_FINDNEXT)
				  : GINT_TO_POINTER(FRD_FINDNEXT));

    add_button_to_dialog(actionarea, frdp->find, FALSE);

    if (do_replace)
    {
	// 'Replace' button
	frdp->replace = create_image_button(_("Replace"));
	add_button_to_dialog(actionarea, frdp->replace, sensitive);
	gtk_widget_set_can_default(frdp->find, TRUE);
	g_signal_connect(G_OBJECT(frdp->replace), "clicked",
			 G_CALLBACK(find_replace_cb),
			 GINT_TO_POINTER(FRD_REPLACE));

	// 'Replace All' button
	frdp->all = create_image_button(_("Replace All"));
	add_button_to_dialog(actionarea, frdp->all, sensitive);
	g_signal_connect(G_OBJECT(frdp->all), "clicked",
			 G_CALLBACK(find_replace_cb),
			 GINT_TO_POINTER(FRD_REPLACEALL));
    }

    // 'Cancel' button
    tmp = gtk_button_new_with_mnemonic(_("_Close"));
    gtk_widget_set_can_default(tmp, TRUE);
    gtk_box_append(GTK_BOX(actionarea), tmp);
    g_signal_connect_swapped(G_OBJECT(tmp),
			     "clicked", G_CALLBACK(gtk_widget_hide),
			     G_OBJECT(frdp->dialog));
    g_signal_connect(G_OBJECT(frdp->dialog),
	    "close-request", G_CALLBACK(gtk_widget_hide), G_OBJECT(frdp->dialog));

    // Suppress automatic show of the unused action area
    // TODO: In GTK3 we could hide the unused action area. What is possible in GTK4?
    gtk_widget_set_visible(GTK_WIDGET(frdp->dialog), TRUE);

    vim_free(entry_text);
    vim_free(conv_buffer);
#undef CONV
}

// }}}

