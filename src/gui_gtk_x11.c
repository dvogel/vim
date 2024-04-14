/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

/*
 * Porting to GTK+ was done by:
 *
 * (C) 1998,1999,2000 by Marcin Dalecki <martin@dalecki.de>
 *
 * With GREAT support and continuous encouragements by Andy Kahn and of
 * course Bram Moolenaar!
 *
 * Support for GTK+ 2 was added by:
 *
 * (C) 2002,2003  Jason Hildebrand  <jason@peaceworks.ca>
 *		  Daniel Elstner  <daniel.elstner@gmx.net>
 *
 * Support for GTK+ 3 was added by:
 *
 * 2016  Kazunobu Kuriyama  <kazunobu.kuriyama@gmail.com>
 */

#include "vim.h"
#ifdef USE_GRESOURCE
#include "auto/gui_gtk_gresources.h"
#endif

#ifdef FEAT_GUI_GNOME
// Gnome redefines _() and N_().  Grrr...
# ifdef _
#  undef _
# endif
# ifdef N_
#  undef N_
# endif
# ifdef textdomain
#  undef textdomain
# endif
# ifdef bindtextdomain
#  undef bindtextdomain
# endif
# ifdef bind_textdomain_codeset
#  undef bind_textdomain_codeset
# endif
# if defined(FEAT_GETTEXT) && !defined(ENABLE_NLS)
#  define ENABLE_NLS	// so the texts in the dialog boxes are translated
# endif
# include <gnome.h>
# include "version.h"
// missing prototype in bonobo-dock-item.h
extern void bonobo_dock_item_set_behavior(BonoboDockItem *dock_item, BonoboDockItemBehavior beh);
#endif

#if defined(FEAT_GUI_GTK)
# if GTK_CHECK_VERSION(3,0,0)
#  include <gdk/gdkkeysyms-compat.h>
#  include <gtk/gtkx.h>
# else
#  include <gdk/gdkkeysyms.h>
# endif
# include <gdk/gdk.h>
# ifdef MSWIN
#  include <gdk/gdkwin32.h>
# else
#  include <gdk/gdkx.h>
# endif
# include <gtk/gtk.h>
# include "gui_gtk3_form.h"
# include "gui_gtk_compat.h"
# include "gui_gtk_common.h"
#endif

#ifdef HAVE_X11_SUNKEYSYM_H
# include <X11/Sunkeysym.h>
#endif

#ifdef FEAT_SOCKETSERVER
# include <glib-unix.h>

// Used to track the source for the listening socket
static uint socket_server_source_id = 0;
#endif

/*
 * Easy-to-use macro for multihead support.
 */
# define GET_X_ATOM(atom)	gdk_x11_atom_to_xatom_for_display( \
				    gtk_widget_get_display(gui.mainwin), atom)

// Selection type distinguishers
enum
{
    TARGET_TYPE_NONE,
    TARGET_UTF8_STRING,
    TARGET_STRING,
    TARGET_COMPOUND_TEXT,
    TARGET_HTML,
    TARGET_TEXT,
    TARGET_TEXT_URI_LIST,
    TARGET_TEXT_PLAIN,
    TARGET_TEXT_PLAIN_UTF8,
    TARGET_VIM,
    TARGET_VIMENC
};

/*
 * Table of selection targets supported by Vim.
 * Note: Order matters, preferred types should come first.
 */
static const GtkTargetEntry selection_targets[] =
{
    {VIMENC_ATOM_NAME,	0, TARGET_VIMENC},
    {VIM_ATOM_NAME,	0, TARGET_VIM},
    {"text/html",	0, TARGET_HTML},
    {"UTF8_STRING",	0, TARGET_UTF8_STRING},
    {"COMPOUND_TEXT",	0, TARGET_COMPOUND_TEXT},
    {"TEXT",		0, TARGET_TEXT},
    {"STRING",		0, TARGET_STRING},
    {"text/plain;charset=utf-8", 0, TARGET_TEXT_PLAIN_UTF8},
    {"text/plain",	0, TARGET_TEXT_PLAIN}
};
# define N_SELECTION_TARGETS ARRAY_LENGTH(selection_targets)

#ifdef FEAT_DND
/*
 * Table of DnD targets supported by Vim.
 * Note: Order matters, preferred types should come first.
 */
static const GtkTargetEntry dnd_targets[] =
{
    {"text/uri-list",	0, TARGET_TEXT_URI_LIST},
    {"text/html",	0, TARGET_HTML},
    {"UTF8_STRING",	0, TARGET_UTF8_STRING},
    {"STRING",		0, TARGET_STRING},
    {"text/plain",	0, TARGET_TEXT_PLAIN}
};
# define N_DND_TARGETS ARRAY_LENGTH(dnd_targets)
#endif

/*
 * "Monospace" is a standard font alias that should be present
 * on all proper Pango/fontconfig installations.
 */
# define DEFAULT_FONT	"Monospace 12"

#if defined(FEAT_GUI_GNOME) && defined(FEAT_SESSION)
# define USE_GNOME_SESSION
#endif

#if !defined(FEAT_GUI_GNOME) && !defined(USE_GTK4)
/*
 * Atoms used to communicate save-yourself from the X11 session manager. There
 * is no need to move them into the GUI struct, since they should be constant.
 */
static GdkAtom wm_protocols_atom = GDK_NONE;
static GdkAtom save_yourself_atom = GDK_NONE;
#endif


#ifndef USE_GTK4
/*
 * Atoms used to control/reference X11 selections.
 */
static GdkAtom html_atom = GDK_NONE;
static GdkAtom utf8_string_atom = GDK_NONE;
static GdkAtom vim_atom = GDK_NONE;	// Vim's own special selection format
static GdkAtom vimenc_atom = GDK_NONE;	// Vim's extended selection format
#endif



#ifdef FEAT_GUI_GNOME
/*
 * Can't use Gnome if --socketid given
 */
static int using_gnome = 0;
#else
# define using_gnome 0
#endif

// Comment out the following line to ignore code for resize history tracking.
#define TRACK_RESIZE_HISTORY
#ifdef TRACK_RESIZE_HISTORY
/*
 * Keep a short term resize history so that stale gtk responses can be
 * discarded.
 * When a gtk_window_resize() request is sent to gtk, the width/height of
 * the request is saved. Recent stale requests are kept around in a list.
 * See https://github.com/vim/vim/issues/10123
 */
# if 0  // Change to 1 to enable ch_log() calls for debugging.
#  ifdef FEAT_EVAL
#   define ENABLE_RESIZE_HISTORY_LOG
#  endif
# endif

/*
 * History item of a resize request.
 * Width and height are of gui.mainwin.
 */
typedef struct resize_history {
    int used;	    // If true, can't match for discard. Only matches once.
    int width;
    int height;
# ifdef ENABLE_RESIZE_HISTORY_LOG
    int seq;	    // for ch_log messages
# endif
    struct resize_history *next;
} resize_hist_T;

// never NULL during execution
static resize_hist_T *latest_resize_hist;
// list of stale resize requests
static resize_hist_T *old_resize_hists;

/*
 * Used when calling gtk_window_resize().
 * Create a resize request history item, put previous request on stale list.
 * Width/height are the size of the request for the gui.mainwin.
 */
    static void
alloc_resize_hist(int width, int height)
{
    // alloc a new resize hist, save current in list of old history
    resize_hist_T *prev_hist = latest_resize_hist;
    resize_hist_T *new_hist = ALLOC_CLEAR_ONE(resize_hist_T);

    new_hist->width = width;
    new_hist->height = height;
    latest_resize_hist = new_hist;

    // previous hist item becomes head of list
    prev_hist->next = old_resize_hists;
    old_resize_hists = prev_hist;

# ifdef ENABLE_RESIZE_HISTORY_LOG
    new_hist->seq = prev_hist->seq + 1;
    ch_log(NULL, "gui_gtk: New resize seq %d (%d, %d) [%d, %d]",
	   new_hist->seq, width, height, (int)Columns, (int)Rows);
# endif
}

/*
 * Free everything on the stale resize history list.
 * This list is empty when there are no outstanding resize requests.
 */
    static void
clear_resize_hists(void)
{
# ifdef ENABLE_RESIZE_HISTORY_LOG
    int		    i = 0;
# endif

    if (latest_resize_hist)
	latest_resize_hist->used = TRUE;
    while (old_resize_hists != NULL)
    {
	resize_hist_T *next_hist = old_resize_hists->next;

	vim_free(old_resize_hists);
	old_resize_hists = next_hist;
# ifdef ENABLE_RESIZE_HISTORY_LOG
	i++;
# endif
    }
# ifdef ENABLE_RESIZE_HISTORY_LOG
    ch_log(NULL, "gui_gtk: free %d hists", i);
# endif
}

// true if hist item is unused and matches w,h
# define MATCH_WIDTH_HEIGHT(hist, w, h) \
	  (!hist->used && hist->width == w && hist->height == h)

/*
 * Search the resize hist list.
 * Return true if the specified width,height match an item in the list that
 * has never matched before. Mark the matching item as used so it will
 * not match again.
 */
    static int
match_stale_width_height(int width, int height)
{
    resize_hist_T *hist = old_resize_hists;

    for (hist = old_resize_hists; hist != NULL; hist = hist->next)
	if (MATCH_WIDTH_HEIGHT(hist, width, height))
	{
# ifdef ENABLE_RESIZE_HISTORY_LOG
	    ch_log(NULL, "gui_gtk: discard seq %d, cur seq %d",
		   hist->seq, latest_resize_hist->seq);
# endif
	    hist->used = TRUE;
	    return TRUE;
	}
    return FALSE;
}

# if defined(EXITFREE)
    static void
free_all_resize_hist(void)
{
    clear_resize_hists();
    vim_free(latest_resize_hist);
}
# endif
#endif

    GtkSettings*
gui_gtk_get_settings()
{
    return gtk_settings_get_for_screen(gdk_screen_get_default());
}

#if defined(EXITFREE)
    void
gui_mch_free_all(void)
{
    vim_free(gui_argv);
#if defined(USE_GNOME_SESSION)
    vim_free(gnome_abs_restart_command);
#endif
#ifdef TRACK_RESIZE_HISTORY
    free_all_resize_hist();
#endif
}
#endif

#if !GTK_CHECK_VERSION(3,0,0)
/*
 * This should be maybe completely removed.
 * Doesn't seem possible, since check_copy_area() relies on
 * this information.  --danielk
 */
    static gint
visibility_event(GtkWidget *widget UNUSED,
		 GdkEventVisibility *event,
		 gpointer data UNUSED)
{
    gui.visibility = event->state;
    /*
     * When we do an gdk_window_copy_area(), and the window is partially
     * obscured, we want to receive an event to tell us whether it worked
     * or not.
     */
    if (gui.text_gc != NULL)
	gdk_gc_set_exposures(gui.text_gc,
			     gui.visibility != GDK_VISIBILITY_UNOBSCURED);
    return FALSE;
}
#endif // !GTK_CHECK_VERSION(3,0,0)

#if GTK_CHECK_VERSION(3,0,0)
/*
 * Redraw the corresponding portions of the screen.
 */
    static gboolean
draw_event(GtkWidget *widget UNUSED,
	   cairo_t   *cr,
	   gpointer   user_data UNUSED)
{
    return gui_gtk_draw_gui_surface_to_widget_surface(cr);
}
#endif

#if GTK_CHECK_VERSION(3,10,0)
    static gboolean
scale_factor_event(GtkWidget *widget,
		   GParamSpec* pspec UNUSED,
		   gpointer   user_data UNUSED)
{
    if (gui.surface != NULL)
	cairo_surface_destroy(gui.surface);

    int	    w, h;
    gtk_window_get_size(GTK_WINDOW(gui.mainwin), &w, &h);
    gui.surface = gdk_window_create_similar_surface(
	    gtk_widget_get_window(widget),
	    CAIRO_CONTENT_COLOR_ALPHA,
	    w, h);

    int	    usable_height = h;
    if (gtk_socket_id != 0)
	usable_height -= (gui.char_height - (gui.char_height/2)); // sic.

    gui_gtk_form_freeze(GTK_FORM(gui.formwin));
    gui.force_redraw = 1;
    gui_resize_shell(w, usable_height);
    gui_gtk_form_thaw(GTK_FORM(gui.formwin));

    return TRUE;
}
#else // !GTK_CHECK_VERSION(3,0,0)
    static gint
expose_event(GtkWidget *widget UNUSED,
	     GdkEventExpose *event,
	     gpointer data UNUSED)
{
    // Skip this when the GUI isn't set up yet, will redraw later.
    if (gui.starting)
	return FALSE;

    out_flush();		// make sure all output has been processed
    gui_redraw(event->area.x, event->area.y,
	       event->area.width, event->area.height);

    // Clear the border areas if needed
    if (event->area.x < FILL_X(0))
	gdk_window_clear_area(gui.drawarea->window, 0, 0, FILL_X(0), 0);
    if (event->area.y < FILL_Y(0))
	gdk_window_clear_area(gui.drawarea->window, 0, 0, 0, FILL_Y(0));
    if (event->area.x > FILL_X(Columns))
	gdk_window_clear_area(gui.drawarea->window,
			      FILL_X((int)Columns), 0, 0, 0);
    if (event->area.y > FILL_Y(Rows))
	gdk_window_clear_area(gui.drawarea->window, 0, FILL_Y((int)Rows), 0, 0);

    return FALSE;
}
#endif // !GTK_CHECK_VERSION(3,0,0)

#ifdef FEAT_CLIENTSERVER
/*
 * Handle changes to the "Comm" property
 */
    static gint
property_event(GtkWidget *widget,
	       GdkEventProperty *event,
	       gpointer data UNUSED)
{
    if (event->type == GDK_PROPERTY_NOTIFY
	    && event->state == (int)GDK_PROPERTY_NEW_VALUE
	    && GDK_WINDOW_XID(event->window) == commWindow
	    && GET_X_ATOM(event->atom) == commProperty)
    {
	XEvent xev;

	// Translate to XLib
	xev.xproperty.type = PropertyNotify;
	xev.xproperty.atom = commProperty;
	xev.xproperty.window = commWindow;
	xev.xproperty.state = PropertyNewValue;
	serverEventProc(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(widget)),
								      &xev, 0);
    }
    return FALSE;
}
#endif // defined(FEAT_CLIENTSERVER)


/////////////////////////////////////////////////////////////////////////////
// Focus handlers:

    static gint
enter_notify_event(GtkWidget *widget UNUSED,
		   GdkEventCrossing *event UNUSED,
		   gpointer data UNUSED)
{
    gui_gtk_pointer_enter();
    return FALSE;
}

    static gint
leave_notify_event(GtkWidget *widget UNUSED,
		   GdkEventCrossing *event UNUSED,
		   gpointer data UNUSED)
{
    gui_gtk_pointer_leave();
    return FALSE;
}

    static gint
focus_in_event(GtkWidget *widget,
	       GdkEventFocus *event UNUSED,
	       gpointer data UNUSED)
{
    gui_gtk_focus_enter(widget);
    return TRUE;
}

    static gint
focus_out_event(GtkWidget *widget UNUSED,
		GdkEventFocus *event UNUSED,
		gpointer data UNUSED)
{
    gui_gtk_focus_leave();
    return TRUE;
}

/*
 * Main keyboard handler:
 */
    static gint
key_press_event(GtkWidget *widget UNUSED,
		GdkEventKey *event,
		gpointer data UNUSED)
{
    gui.event_time = event->time;
#ifdef FEAT_XIM
    if (xim_queue_key_press_event(event, TRUE))
	return TRUE;
#endif
    return gui_gtk_process_key_press(event->keyval, event->state);
}

#if defined(FEAT_XIM) || GTK_CHECK_VERSION(3,0,0)
    static gboolean
key_release_event(GtkWidget *widget UNUSED,
		  GdkEventKey *event UNUSED,
		  gpointer data UNUSED)
{
# if defined(FEAT_XIM)
    gui.event_time = event->time;
    /*
     * GTK+ 2 input methods may do fancy stuff on key release events too.
     * With the default IM for instance, you can enter any UCS code point
     * by holding down CTRL-SHIFT and typing hexadecimal digits.
     */
    return xim_queue_key_press_event(event, FALSE);
# else
    return TRUE;
# endif
}
#endif


/////////////////////////////////////////////////////////////////////////////
// Selection handlers:

// Remember when clip_lose_selection was called from here, we must not call
// gtk_selection_owner_set() then.
static int in_selection_clear_event = FALSE;

    static gint
selection_clear_event(GtkWidget		*widget UNUSED,
		      GdkEventSelection	*event,
		      gpointer		user_data UNUSED)
{
    in_selection_clear_event = TRUE;
    if (event->selection == clip_plus.gtk_sel_atom)
	clip_lose_selection(&clip_plus);
    else
	clip_lose_selection(&clip_star);
    in_selection_clear_event = FALSE;

    return TRUE;
}

#define RS_NONE	0	// selection_received_cb() not called yet
#define RS_OK	1	// selection_received_cb() called and OK
#define RS_FAIL	2	// selection_received_cb() called and failed
static int received_selection = RS_NONE;


    static void
selection_received_cb(GtkWidget		*widget UNUSED,
		      GtkSelectionData	*data,
		      guint		time_ UNUSED,
		      gpointer		user_data UNUSED)
{
    Clipboard_T	    *cbd;
    char_u	    *text;
    char_u	    *tmpbuf = NULL;
    guchar	    *tmpbuf_utf8 = NULL;
    int		    len;
    int		    motion_type = MAUTO;

    if (gtk_selection_data_get_selection(data) == clip_plus.gtk_sel_atom)
	cbd = &clip_plus;
    else
	cbd = &clip_star;

    text = (char_u *)gtk_selection_data_get_data(data);
    len = gtk_selection_data_get_length(data);

    if (text == NULL || len <= 0)
    {
	received_selection = RS_FAIL;
	// clip_free_selection(cbd); ???

	return;
    }

    if (gtk_selection_data_get_data_type(data) == vim_atom)
    {
	motion_type = *text++;
	--len;
    }
    else if (gtk_selection_data_get_data_type(data) == vimenc_atom)
    {
	char_u		*enc;
	vimconv_T	conv;

	motion_type = *text++;
	--len;

	enc = text;
	text += STRLEN(text) + 1;
	len -= text - enc;

	// If the encoding of the text is different from 'encoding', attempt
	// converting it.
	conv.vc_type = CONV_NONE;
	convert_setup(&conv, enc, p_enc);
	if (conv.vc_type != CONV_NONE)
	{
	    tmpbuf = string_convert(&conv, text, &len);
	    if (tmpbuf != NULL)
		text = tmpbuf;
	    convert_setup(&conv, NULL, NULL);
	}
    }

    // gtk_selection_data_get_text() handles all the nasty details
    // and targets and encodings etc.  This rocks so hard.
    else
    {
	tmpbuf_utf8 = gtk_selection_data_get_text(data);
	if (tmpbuf_utf8 != NULL)
	{
	    len = STRLEN(tmpbuf_utf8);
	    if (input_conv.vc_type != CONV_NONE)
	    {
		tmpbuf = string_convert(&input_conv, tmpbuf_utf8, &len);
		if (tmpbuf != NULL)
		    text = tmpbuf;
	    }
	    else
		text = tmpbuf_utf8;
	}
	else if (len >= 2 && text[0] == 0xff && text[1] == 0xfe)
	{
	    vimconv_T conv;

	    // UTF-16, we get this for HTML
	    conv.vc_type = CONV_NONE;
	    convert_setup_ext(&conv, (char_u *)"utf-16le", FALSE, p_enc, TRUE);

	    if (conv.vc_type != CONV_NONE)
	    {
		text += 2;
		len -= 2;
		tmpbuf = string_convert(&conv, text, &len);
		convert_setup(&conv, NULL, NULL);
	    }
	    if (tmpbuf != NULL)
		text = tmpbuf;
	}
    }

    // Chop off any trailing NUL bytes.  OpenOffice sends these.
    while (len > 0 && text[len - 1] == NUL)
	--len;

    clip_yank_selection(motion_type, text, (long)len, cbd);
    received_selection = RS_OK;
    vim_free(tmpbuf);
    g_free(tmpbuf_utf8);
}


/*
 * Prepare our selection data for passing it to the external selection
 * client.
 */
    static void
selection_get_cb(GtkWidget	    *widget UNUSED,
		 GtkSelectionData   *selection_data,
		 guint		    info,
		 guint		    time_ UNUSED,
		 gpointer	    user_data UNUSED)
{
    char_u	    *string;
    char_u	    *tmpbuf;
    long_u	    tmplen;
    int		    length;
    int		    motion_type;
    GdkAtom	    type;
    Clipboard_T    *cbd;

    if (gtk_selection_data_get_selection(selection_data)
	    == clip_plus.gtk_sel_atom)
	cbd = &clip_plus;
    else
	cbd = &clip_star;

    if (!cbd->owned)
	return;			// Shouldn't ever happen

    if (info != (guint)TARGET_STRING
	    && (!clip_html || info != (guint)TARGET_HTML)
	    && info != (guint)TARGET_UTF8_STRING
	    && info != (guint)TARGET_VIMENC
	    && info != (guint)TARGET_VIM
	    && info != (guint)TARGET_COMPOUND_TEXT
	    && info != (guint)TARGET_TEXT_PLAIN
	    && info != (guint)TARGET_TEXT_PLAIN_UTF8
	    && info != (guint)TARGET_TEXT)
	return;

    // get the selection from the '*'/'+' register
    clip_get_selection(cbd);

    motion_type = clip_convert_selection(&string, &tmplen, cbd);
    if (motion_type < 0 || string == NULL)
	return;
    // Due to int arguments we can't handle more than G_MAXINT.  Also
    // reserve one extra byte for NUL or the motion type; just in case.
    // (Not that pasting 2G of text is ever going to work, but... ;-)
    length = MIN(tmplen, (long_u)(G_MAXINT - 1));

    if (info == (guint)TARGET_VIM)
    {
	tmpbuf = alloc(length + 1);
	if (tmpbuf != NULL)
	{
	    tmpbuf[0] = motion_type;
	    mch_memmove(tmpbuf + 1, string, (size_t)length);
	}
	// For our own format, the first byte contains the motion type
	++length;
	vim_free(string);
	string = tmpbuf;
	type = vim_atom;
    }

    else if (info == (guint)TARGET_HTML)
    {
	vimconv_T conv;

	// Since we get utf-16, we probably should set it as well.
	conv.vc_type = CONV_NONE;
	convert_setup_ext(&conv, p_enc, TRUE, (char_u *)"utf-16le", FALSE);
	if (conv.vc_type != CONV_NONE)
	{
	    tmpbuf = string_convert(&conv, string, &length);
	    convert_setup(&conv, NULL, NULL);
	    vim_free(string);
	    string = tmpbuf;
	}

	// Prepend the BOM: "fffe"
	if (string != NULL)
	{
	    tmpbuf = alloc(length + 2);
	    if (tmpbuf != NULL)
	    {
		tmpbuf[0] = 0xff;
		tmpbuf[1] = 0xfe;
		mch_memmove(tmpbuf + 2, string, (size_t)length);
		vim_free(string);
		string = tmpbuf;
		length += 2;
	    }

# if !GTK_CHECK_VERSION(3,0,0)
	    // Looks redundant even for GTK2 because these values are
	    // overwritten by gtk_selection_data_set() that follows.
	    selection_data->type = selection_data->target;
	    selection_data->format = 16;	// 16 bits per char
# endif
	    gtk_selection_data_set(selection_data, html_atom, 16,
							      string, length);
	    vim_free(string);
	}
	return;
    }
    else if (info == (guint)TARGET_VIMENC)
    {
	int l = STRLEN(p_enc);

	// contents: motion_type 'encoding' NUL text
	tmpbuf = alloc(length + l + 2);
	if (tmpbuf != NULL)
	{
	    tmpbuf[0] = motion_type;
	    STRCPY(tmpbuf + 1, p_enc);
	    mch_memmove(tmpbuf + l + 2, string, (size_t)length);
	    length += l + 2;
	    vim_free(string);
	    string = tmpbuf;
	}
	type = vimenc_atom;
    }

    // gtk_selection_data_set_text() handles everything for us.  This is
    // so easy and simple and cool, it'd be insane not to use it.
    else
    {
	if (output_conv.vc_type != CONV_NONE)
	{
	    tmpbuf = string_convert(&output_conv, string, &length);
	    vim_free(string);
	    if (tmpbuf == NULL)
		return;
	    string = tmpbuf;
	}
	// Validate the string to avoid runtime warnings
	if (g_utf8_validate((const char *)string, (gssize)length, NULL))
	{
	    gtk_selection_data_set_text(selection_data,
					(const char *)string, length);
	}
	vim_free(string);
	return;
    }

    if (string != NULL)
    {
# if !GTK_CHECK_VERSION(3,0,0)
	// Looks redundant even for GTK2 because these values are
	// overwritten by gtk_selection_data_set() that follows.
	selection_data->type = selection_data->target;
	selection_data->format = 8;	// 8 bits per char
# endif
	gtk_selection_data_set(selection_data, type, 8, string, length);
	vim_free(string);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Mouse handling callbacks


#if GTK_CHECK_VERSION(3,0,0)
    static GdkDevice *
gui_gtk_get_pointer_device(GtkWidget *widget)
{
    GdkWindow * const win = gtk_widget_get_window(widget);
    GdkDisplay * const dpy = gdk_window_get_display(win);
# if GTK_CHECK_VERSION(3,20,0)
    GdkSeat * const seat = gdk_display_get_default_seat(dpy);
    return gdk_seat_get_pointer(seat);
# else
    GdkDeviceManager * const mngr = gdk_display_get_device_manager(dpy);
    return gdk_device_manager_get_client_pointer(mngr);
# endif
}

    void
gui_gtk_get_pointer(GtkWidget       *widget,
		    gint	    *x,
		    gint	    *y,
		    GdkModifierType *state)
{
    GdkWindow * const win = gtk_widget_get_window(widget);
    GdkDevice * const dev = gui_gtk_get_pointer_device(widget);
    gdk_window_get_device_position(win, dev, x, y, state);
}

# if defined(FEAT_GUI_TABLINE)
    static GdkWindow *
gui_gtk_window_at_position(GtkWidget *widget,
			   gint      *x,
			   gint      *y)
{
    GdkDevice * const dev = gui_gtk_get_pointer_device(widget);
    return gdk_device_get_window_at_position(dev, x, y);
}
# endif
#else // !GTK_CHECK_VERSION(3,0,0)
    void
gui_gtk_get_pointer(GtkWidget       *widget,
		    gint	    *x,
		    gint	    *y,
		    GdkModifierType *state)
{
    gdk_window_get_pointer((widget)->window, x, y, state);
}
# define gui_gtk_window_at_position(wid, x, y)	gdk_window_at_pointer(x, y)
#endif


    static gint
motion_notify_event(GtkWidget *widget,
		    GdkEventMotion *event,
		    gpointer data UNUSED)
{
    if (event->is_hint)
    {
	int		x;
	int		y;
	GdkModifierType	state;

	gui_gtk_get_pointer(widget, &x, &y, &state);
	gui_gtk_process_motion_notify(x, y, state);
    }
    else
    {
	gui_gtk_process_motion_notify((int)event->x, (int)event->y,
				      (GdkModifierType)event->state);
    }

    return TRUE; // handled
}

/*
 * Mouse button handling.  Note please that we are capturing multiple click's
 * by our own timeout mechanism instead of the one provided by GTK+ itself.
 * This is due to the way the generic VIM code is recognizing multiple clicks.
 */
    static gint
button_press_event(GtkWidget *widget,
		   GdkEventButton *event,
		   gpointer data UNUSED)
{
    gui_gtk_process_button_press(widget, event->time, event->type, event->state, event->button, event->x, event->y);
    return TRUE;
}


/*
 * GTK+ abstracts scrolling via the GdkEventScroll.
 */
    static gboolean
scroll_event(GtkWidget *widget,
	     GdkEventScroll *event,
	     gpointer data UNUSED)
{
    int	    button = 0;  // silence gcc
    int_u   vim_modifiers;
#if GTK_CHECK_VERSION(3,4,0)
    static double  acc_x, acc_y;
# if !GTK_CHECK_VERSION(3,22,0)
    static guint32 last_smooth_event_time;
# endif
# define DT_X11     1
# define DT_WAYLAND 2
    static int display_type;
    if (!display_type)
	display_type = gui_mch_get_display() ? DT_X11 : DT_WAYLAND;
#endif

    if (gtk_socket_id != 0 && !gtk_widget_has_focus(widget))
	gtk_widget_grab_focus(widget);

    switch (event->direction)
    {
	case GDK_SCROLL_UP:
	    button = MOUSE_4;
	    break;
	case GDK_SCROLL_DOWN:
	    button = MOUSE_5;
	    break;
	case GDK_SCROLL_LEFT:
	    button = MOUSE_7;
	    break;
	case GDK_SCROLL_RIGHT:
	    button = MOUSE_6;
	    break;
#if GTK_CHECK_VERSION(3,4,0)
	case GDK_SCROLL_SMOOTH:
	    if (event->is_stop)
	    {
		acc_x = acc_y = 0;
		// this event tells us to stop, without an actual moving
		return FALSE;
	    }
# if GTK_CHECK_VERSION(3,22,0)
	    if (gdk_device_get_axes(event->device) & GDK_AXIS_FLAG_WHEEL)
		// this is from a wheel (as oppose to a touchpad / trackpoint)
# else
	    if (event->time - last_smooth_event_time > 50)
		// reset our accumulations after 50ms of silence
# endif
		acc_x = acc_y = 0;
	    acc_x += event->delta_x;
	    acc_y += event->delta_y;
# if !GTK_CHECK_VERSION(3,22,0)
	    last_smooth_event_time = event->time;
# endif
	    break;
#endif
	default: // This shouldn't happen
	    return FALSE;
    }

#ifdef FEAT_XIM
    // cancel any preediting
    if (im_is_preediting())
	xim_reset();
#endif

    vim_modifiers = gui_gtk_modifiers_gdk2mouse(event->state);

#if GTK_CHECK_VERSION(3,4,0)
    // on x11, despite not requested, when we copy into primary clipboard,
    // we'll get smooth events. Unsmooth ones will also come along.
    if (event->direction == GDK_SCROLL_SMOOTH && display_type == DT_WAYLAND)
    {
	while (acc_x >= 1.0)
	{ // right
	    acc_x = MAX(0.0, acc_x - 1.0);
	    gui_send_mouse_event(MOUSE_6, (int)event->x, (int)event->y,
		    FALSE, vim_modifiers);
	}
	while (acc_x <= -1.0)
	{ // left
	    acc_x = MIN(0.0, acc_x + 1.0);
	    gui_send_mouse_event(MOUSE_7, (int)event->x, (int)event->y,
		    FALSE, vim_modifiers);
	}
	while (acc_y >= 1.0)
	{ // down
	    acc_y = MAX(0.0, acc_y - 1.0);
	    gui_send_mouse_event(MOUSE_5, (int)event->x, (int)event->y,
		    FALSE, vim_modifiers);
	}
	while (acc_y <= -1.0)
	{ // up
	    acc_y = MIN(0.0, acc_y + 1.0);
	    gui_send_mouse_event(MOUSE_4, (int)event->x, (int)event->y,
		    FALSE, vim_modifiers);
	}
    }
    else if (event->direction == GDK_SCROLL_SMOOTH && display_type == DT_X11)
	// for X11 we deal with unsmooth events, and so ignore the smooth ones
	;
    else
# undef DT_X11
# undef DT_WAYLAND
#endif
	gui_send_mouse_event(button, (int)event->x, (int)event->y,
		FALSE, vim_modifiers);

    return TRUE;
}


    static gint
button_release_event(GtkWidget *widget,
		     GdkEventButton *event,
		     gpointer data UNUSED)
{
    gui_gtk_process_button_release(widget, event->time, event->type, event->state, event->button, event->x, event->y);
}


#ifdef FEAT_DND
/////////////////////////////////////////////////////////////////////////////
// Drag aNd Drop support handlers.

/*
 * Count how many items there may be and separate them with a NUL.
 * Apparently the items are separated with \r\n.  This is not documented,
 * thus be careful not to go past the end.	Also allow separation with
 * NUL characters.
 */
    static int
count_and_decode_uri_list(char_u *out, char_u *raw, int len)
{
    int		i;
    char_u	*p = out;
    int		count = 0;

    for (i = 0; i < len; ++i)
    {
	if (raw[i] == NUL || raw[i] == '\n' || raw[i] == '\r')
	{
	    if (p > out && p[-1] != NUL)
	    {
		++count;
		*p++ = NUL;
	    }
	}
	else if (raw[i] == '%' && i + 2 < len && hexhex2nr(raw + i + 1) > 0)
	{
	    *p++ = hexhex2nr(raw + i + 1);
	    i += 2;
	}
	else
	    *p++ = raw[i];
    }
    if (p > out && p[-1] != NUL)
    {
	*p = NUL;	// last item didn't have \r or \n
	++count;
    }
    return count;
}

/*
 * Parse NUL separated "src" strings.  Make it an array "outlist" form.  On
 * this process, URI which protocol is not "file:" are removed.  Return
 * length of array (less than "max").
 */
    static int
filter_uri_list(char_u **outlist, int max, char_u *src)
{
    int	i, j;

    for (i = j = 0; i < max; ++i)
    {
	outlist[i] = NULL;
	if (STRNCMP(src, "file:", 5) == 0)
	{
	    src += 5;
	    if (STRNCMP(src, "//localhost", 11) == 0)
		src += 11;
	    while (src[0] == '/' && src[1] == '/')
		++src;
	    outlist[j++] = vim_strsave(src);
	}
	src += STRLEN(src) + 1;
    }
    return j;
}

    static char_u **
parse_uri_list(int *count, char_u *data, int len)
{
    int	    n	    = 0;
    char_u  *tmp    = NULL;
    char_u  **array = NULL;

    if (data != NULL && len > 0 && (tmp = alloc(len + 1)) != NULL)
    {
	n = count_and_decode_uri_list(tmp, data, len);
	if (n > 0 && (array = ALLOC_MULT(char_u *, n)) != NULL)
	    n = filter_uri_list(array, n, tmp);
    }
    vim_free(tmp);
    *count = n;
    return array;
}

    static void
drag_handle_uri_list(GdkDragContext	*context,
		     GtkSelectionData	*data,
		     guint		time_,
		     GdkModifierType	state,
		     gint		x,
		     gint		y)
{
    char_u  **fnames;
    int	    nfiles = 0;

    fnames = parse_uri_list(&nfiles,
	    (char_u *)gtk_selection_data_get_data(data),
	    gtk_selection_data_get_length(data));

    if (fnames != NULL && nfiles > 0)
    {
	int_u   modifiers;

	gtk_drag_finish(context, TRUE, FALSE, time_); // accept

	modifiers = gui_gtk_modifiers_gdk2mouse(state);

	gui_handle_drop(x, y, modifiers, fnames, nfiles);
    }
    else
	vim_free(fnames);
}

    static void
drag_handle_text(GdkDragContext	    *context,
		 GtkSelectionData   *data,
		 guint		    time_,
		 GdkModifierType    state)
{
    char_u  dropkey[6] = {CSI, KS_MODIFIER, 0, CSI, KS_EXTRA, (char_u)KE_DROP};
    char_u  *text;
    int	    len;
    char_u  *tmpbuf = NULL;

    text = (char_u *)gtk_selection_data_get_data(data);
    len = gtk_selection_data_get_length(data);

    if (gtk_selection_data_get_data_type(data) == utf8_string_atom)
    {
	if (input_conv.vc_type != CONV_NONE)
	    tmpbuf = string_convert(&input_conv, text, &len);
	if (tmpbuf != NULL)
	    text = tmpbuf;
    }

    dnd_yank_drag_data(text, (long)len);
    gtk_drag_finish(context, TRUE, FALSE, time_); // accept
    vim_free(tmpbuf);

    dropkey[2] = gui_gtk_modifiers_gdk2vim(state);

    if (dropkey[2] != 0)
	add_to_input_buf(dropkey, (int)sizeof(dropkey));
    else
	add_to_input_buf(dropkey + 3, (int)(sizeof(dropkey) - 3));
}

/*
 * DND receiver.
 */
    static void
drag_data_received_cb(GtkWidget		*widget,
		      GdkDragContext	*context,
		      gint		x,
		      gint		y,
		      GtkSelectionData	*data,
		      guint		info,
		      guint		time_,
		      gpointer		user_data UNUSED)
{
    GdkModifierType state;

    // Guard against trash
    const guchar * const data_data = gtk_selection_data_get_data(data);
    const gint data_length = gtk_selection_data_get_length(data);
    const gint data_format = gtk_selection_data_get_format(data);

    if (data_data == NULL
	    || data_length <= 0
	    || data_format != 8
	    || data_data[data_length] != '\0')
    {
	gtk_drag_finish(context, FALSE, FALSE, time_);
	return;
    }

    // Get the current modifier state for proper distinguishment between
    // different operations later.
    gui_gtk_get_pointer(widget, NULL, NULL, &state);

    // Not sure about the role of "text/plain" here...
    if (info == (guint)TARGET_TEXT_URI_LIST)
	drag_handle_uri_list(context, data, time_, state, x, y);
    else
	drag_handle_text(context, data, time_, state);

}
#endif // FEAT_DND


#if defined(USE_GNOME_SESSION)
/*
 * GnomeClient interact callback.  Check for unsaved buffers that cannot
 * be abandoned and pop up a dialog asking the user for confirmation if
 * necessary.
 */
    static void
sm_client_check_changed_any(GnomeClient	    *client UNUSED,
			    gint	    key,
			    GnomeDialogType type UNUSED,
			    gpointer	    data UNUSED)
{
    cmdmod_T	save_cmdmod;
    gboolean	shutdown_cancelled;

    save_cmdmod = cmdmod;

# ifdef FEAT_BROWSE
    cmdmod.cmod_flags |= CMOD_BROWSE;
# endif
# if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
    cmdmod.cmod_flags |= CMOD_CONFIRM;
# endif
    /*
     * If there are changed buffers, present the user with
     * a dialog if possible, otherwise give an error message.
     */
    shutdown_cancelled = check_changed_any(FALSE, FALSE);

    exiting = FALSE;
    cmdmod = save_cmdmod;
    setcursor(); // position the cursor
    out_flush();
    /*
     * If the user hit the [Cancel] button the whole shutdown
     * will be cancelled.  Wow, quite powerful feature (:
     */
    gnome_interaction_key_return(key, shutdown_cancelled);
}

/*
 * "save_yourself" signal handler.  Initiate an interaction to ask the user
 * for confirmation if necessary.  Save the current editing session and tell
 * the session manager how to restart Vim.
 */
    static gboolean
sm_client_save_yourself(GnomeClient	    *client,
			gint		    phase UNUSED,
			GnomeSaveStyle	    save_style UNUSED,
			gboolean	    shutdown UNUSED,
			GnomeInteractStyle  interact_style,
			gboolean	    fast UNUSED,
			gpointer	    data UNUSED)
{
    static const char	suffix[] = "-session.vim";
    char		*session_file;
    unsigned int	len;
    gboolean		success;

    // Always request an interaction if possible.  check_changed_any()
    // won't actually show a dialog unless any buffers have been modified.
    // There doesn't seem to be an obvious way to check that without
    // automatically firing the dialog.  Anyway, it works just fine.
    if (interact_style == GNOME_INTERACT_ANY)
	gnome_client_request_interaction(client, GNOME_DIALOG_NORMAL,
					 &sm_client_check_changed_any,
					 NULL);
    out_flush();
    ml_sync_all(FALSE, FALSE); // preserve all swap files

    // The path is unique for each session save.  We do neither know nor care
    // which session script will actually be used later.  This decision is in
    // the domain of the session manager.
    session_file = gnome_config_get_real_path(
			gnome_client_get_config_prefix(client));
    len = strlen(session_file);

    if (len > 0 && session_file[len-1] == G_DIR_SEPARATOR)
	--len; // get rid of the superfluous trailing '/'

    session_file = g_renew(char, session_file, len + sizeof(suffix));
    memcpy(session_file + len, suffix, sizeof(suffix));

    success = write_session_file((char_u *)session_file);

    if (success)
    {
	const char  *argv[8];
	int	    i;

	// Tell the session manager how to wipe out the stored session data.
	// This isn't as dangerous as it looks, don't worry :)	session_file
	// is a unique absolute filename.  Usually it'll be something like
	// `/home/user/.gnome2/vim-XXXXXX-session.vim'.
	i = 0;
	argv[i++] = "rm";
	argv[i++] = session_file;
	argv[i] = NULL;

	gnome_client_set_discard_command(client, i, (char **)argv);

	// Tell the session manager how to restore the just saved session.
	// This is easily done thanks to Vim's -S option.  Pass the -f flag
	// since there's no need to fork -- it might even cause confusion.
	// Also pass the window role to give the WM something to match on.
	// The role is set in gui_mch_open(), thus should _never_ be NULL.
	i = 0;
	argv[i++] = gnome_restart_command;
	argv[i++] = "-f";
	argv[i++] = "-g";
	argv[i++] = "--role";
	argv[i++] = gtk_window_get_role(GTK_WINDOW(gui.mainwin));
	argv[i++] = "-S";
	argv[i++] = session_file;
	argv[i] = NULL;

	gnome_client_set_restart_command(client, i, (char **)argv);
	gnome_client_set_clone_command(client, 0, NULL);
    }

    g_free(session_file);

    return success;
}

/*
 * Called when the session manager wants us to die.  There isn't much to save
 * here since "save_yourself" has been emitted before (unless serious trouble
 * is happening).
 */
    static void
sm_client_die(GnomeClient *client UNUSED, gpointer data UNUSED)
{
    // Don't write messages to the GUI anymore
    full_screen = FALSE;

    vim_strncpy(IObuff, (char_u *)
		    _("Vim: Received \"die\" request from session manager\n"),
		    IOSIZE - 1);
    preserve_exit();
}

/*
 * Connect our signal handlers to be notified on session save and shutdown.
 */
    static void
setup_save_yourself(void)
{
    GnomeClient *client;

    client = gnome_master_client();
    if (client == NULL)
	return;

    // Must use the deprecated gtk_signal_connect() for compatibility
    // with GNOME 1.  Arrgh, zombies!
    gtk_signal_connect(GTK_OBJECT(client), "save_yourself",
	    GTK_SIGNAL_FUNC(&sm_client_save_yourself), NULL);
    gtk_signal_connect(GTK_OBJECT(client), "die",
	    GTK_SIGNAL_FUNC(&sm_client_die), NULL);
}

#else // !USE_GNOME_SESSION

# ifdef USE_XSMP
/*
 * GTK tells us that XSMP needs attention
 */
    static gboolean
local_xsmp_handle_requests(
    GIOChannel		*source UNUSED,
    GIOCondition	condition,
    gpointer		data)
{
    if (condition == G_IO_IN)
    {
	// Do stuff; maybe close connection
	if (xsmp_handle_requests() == FAIL)
	    g_io_channel_unref((GIOChannel *)data);
	return TRUE;
    }
    // Error
    g_io_channel_unref((GIOChannel *)data);
    xsmp_close();
    return TRUE;
}
# endif // USE_XSMP

/*
 * Setup the WM_PROTOCOLS to indicate we want the WM_SAVE_YOURSELF event.
 * This is an ugly use of X functions.	GTK doesn't offer an alternative.
 */
    static void
setup_save_yourself(void)
{
    Atom    *existing_atoms = NULL;
    int	    count = 0;

# ifdef USE_XSMP
    if (xsmp_icefd != -1)
    {
	/*
	 * Use XSMP is preference to legacy WM_SAVE_YOURSELF;
	 * set up GTK IO monitor
	 */
	GIOChannel *g_io = g_io_channel_unix_new(xsmp_icefd);

	g_io_add_watch(g_io, G_IO_IN | G_IO_ERR | G_IO_HUP,
				  local_xsmp_handle_requests, (gpointer)g_io);
	g_io_channel_unref(g_io);
    }
    else
# endif
    {
	// Fall back to old method

	// first get the existing value
	Display * dpy = gui_mch_get_display();
	if (!dpy)
	    return;

	GdkWindow * const mainwin_win = gtk_widget_get_window(gui.mainwin);
	if (XGetWMProtocols(dpy, GDK_WINDOW_XID(mainwin_win),
		    &existing_atoms, &count))
	{
	    Atom	*new_atoms;
	    Atom	save_yourself_xatom;
	    int	i;

	    save_yourself_xatom = GET_X_ATOM(save_yourself_atom);

	    // check if WM_SAVE_YOURSELF isn't there yet
	    for (i = 0; i < count; ++i)
		if (existing_atoms[i] == save_yourself_xatom)
		    break;

	    if (i == count)
	    {
		// allocate an Atoms array which is one item longer
		new_atoms = ALLOC_MULT(Atom, count + 1);
		if (new_atoms != NULL)
		{
		    memcpy(new_atoms, existing_atoms, count * sizeof(Atom));
		    new_atoms[count] = save_yourself_xatom;
		    XSetWMProtocols(GDK_WINDOW_XDISPLAY(mainwin_win),
			    GDK_WINDOW_XID(mainwin_win),
			    new_atoms, count + 1);
		    vim_free(new_atoms);
		}
	    }
	    XFree(existing_atoms);
	}
    }
}

/*
 * Installing a global event filter seems to be the only way to catch
 * client messages of type WM_PROTOCOLS without overriding GDK's own
 * client message event filter.  Well, that's still better than trying
 * to guess what the GDK filter had done if it had been invoked instead
 *
 * GTK2_FIXME:	This doesn't seem to work.  For some reason we never
 * receive WM_SAVE_YOURSELF even though everything is set up correctly.
 * I have the nasty feeling modern session managers just don't send this
 * deprecated message anymore.	Addition: confirmed by several people.
 *
 * The GNOME session support is much cooler anyway.  Unlike this ugly
 * WM_SAVE_YOURSELF hack it actually stores the session...  And yes,
 * it should work with KDE as well.
 */
    static GdkFilterReturn
global_event_filter(GdkXEvent *xev,
		    GdkEvent *event UNUSED,
		    gpointer data UNUSED)
{
    XEvent *xevent = (XEvent *)xev;

    if (xevent != NULL
	    && xevent->type == ClientMessage
	    && xevent->xclient.message_type == GET_X_ATOM(wm_protocols_atom)
	    && (long_u)xevent->xclient.data.l[0]
					    == GET_X_ATOM(save_yourself_atom))
    {
	out_flush();
	ml_sync_all(FALSE, FALSE); // preserve all swap files
	/*
	 * Set the window's WM_COMMAND property, to let the window manager
	 * know we are done saving ourselves.  We don't want to be
	 * restarted, thus set argv to NULL.
	 */
	XSetCommand(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.mainwin)),
		    GDK_WINDOW_XID(gtk_widget_get_window(gui.mainwin)),
		    NULL, 0);
	return GDK_FILTER_REMOVE;
    }

    return GDK_FILTER_CONTINUE;
}
#endif // !USE_GNOME_SESSION

#if defined(FEAT_SOCKETSERVER)

/*
 * Callback for new events from the socket server listening socket
 */
    static int
socket_server_poll_in(int fd UNUSED, GIOCondition cond, void *user_data UNUSED)
{
    if (cond & G_IO_IN)
	socket_server_accept_client();
    else if (cond & (G_IO_ERR | G_IO_HUP))
    {
	socket_server_uninit();
	return FALSE;
    }

    return TRUE;
}

/*
 * Initialize socket server for use in the GUI (does not actually initialize the
 * socket server, only attaches a source).
 */
    void
gui_gtk_init_socket_server(void)
{
    if (socket_server_source_id > 0)
	return;
    // Register source for file descriptor to global default context
    socket_server_source_id = g_unix_fd_add(socket_server_get_fd(),
	    G_IO_IN | G_IO_ERR | G_IO_HUP, socket_server_poll_in, NULL);
}

/*
 * Remove the source for the socket server listening socket.
 */
    void
gui_gtk_uninit_socket_server(void)
{
    if (socket_server_source_id > 0)
    {
	g_source_remove(socket_server_source_id);
	socket_server_source_id = 0;
    }
}

#endif

/*
 * Setup the window icon & xcmdsrv comm after the main window has been realized.
 */
    static void
mainwin_realize(GtkWidget *widget UNUSED, gpointer data UNUSED)
{
#include "../runtime/vim16x16.xpm"
#include "../runtime/vim32x32.xpm"
#include "../runtime/vim48x48.xpm"

    GdkWindow * const mainwin_win = gtk_widget_get_window(gui.mainwin);

    // When started with "--echo-wid" argument, write window ID on stdout.
    if (echo_wid_arg)
    {
	if (gui_mch_get_display())
	    printf("WID: %ld\n", (long)GDK_WINDOW_XID(mainwin_win));
	else
	    printf("WID: 0\n");
	fflush(stdout);
    }

    // GTK4 delegates all application icon functionality to the desktop
    // environment. TODO: Fix VIM packaging to set the appropriate icon
    // metadata, particularly for GNOME/FreeDesktop.
    if (vim_strchr(p_go, GO_ICON) != NULL)
    {
	GtkIconTheme *icon_theme;

	icon_theme = gtk_icon_theme_get_default();

	if (icon_theme && gtk_icon_theme_has_icon(icon_theme, "gvim"))
	{
	    gtk_window_set_icon_name(GTK_WINDOW(gui.mainwin), "gvim");
	}
	else
	{
	    /*
	     * Add an icon to the main window. For fun and convenience of the user.
	     */
	    GList *icons = NULL;

	    icons = g_list_prepend(icons, gdk_pixbuf_new_from_xpm_data((const char **)vim16x16));
	    icons = g_list_prepend(icons, gdk_pixbuf_new_from_xpm_data((const char **)vim32x32));
	    icons = g_list_prepend(icons, gdk_pixbuf_new_from_xpm_data((const char **)vim48x48));

	    gtk_window_set_icon_list(GTK_WINDOW(gui.mainwin), icons);

	    // TODO: is this type cast OK?
	    g_list_foreach(icons, (GFunc)(void *)&g_object_unref, NULL);
	    g_list_free(icons);
	}
	g_object_unref(icon_theme);
    }

#if !defined(USE_GNOME_SESSION)
    // Register a handler for WM_SAVE_YOURSELF with GDK's low-level X I/F
    gdk_window_add_filter(NULL, &global_event_filter, NULL);
#endif
    // Setup to indicate to the window manager that we want to catch the
    // WM_SAVE_YOURSELF event.	For GNOME, this connects to the session
    // manager instead.
# if defined(USE_GNOME_SESSION)
    if (using_gnome)
# endif
	setup_save_yourself();

#ifdef FEAT_CLIENTSERVER
    if (clientserver_method == CLIENTSERVER_METHOD_X11 && gui_mch_get_display())
    {
	if (serverName == NULL && serverDelayedStartName != NULL)
	{
	    // This is a :gui command in a plain vim with no previous server
	    commWindow = GDK_WINDOW_XID(mainwin_win);

	    (void)serverRegisterName(GDK_WINDOW_XDISPLAY(mainwin_win),
				    serverDelayedStartName);
	}
	else
	{
	    /*
	    * Cannot handle "XLib-only" windows with gtk event routines, we'll
	    * have to change the "server" registration to that of the main window
	    * If we have not registered a name yet, remember the window.
	    */
	    serverChangeRegisteredWindow(GDK_WINDOW_XDISPLAY(mainwin_win),
					GDK_WINDOW_XID(mainwin_win));
	}
	gtk_widget_add_events(gui.mainwin, GDK_PROPERTY_CHANGE_MASK);
	g_signal_connect(G_OBJECT(gui.mainwin), "property-notify-event",
			G_CALLBACK(property_event), NULL);
    }
#endif
}


    static GdkCursor *
create_blank_pointer(void)
{
    GdkCursor	*cursor;

#if GTK_CHECK_VERSION(3,0,0)
    GdkDisplay  *disp = gtk_widget_get_display(gui.mainwin);
    cursor = gdk_cursor_new_from_name(disp, "none");
#else
    // Create a pseudo blank pointer, which is in fact one pixel by one pixel
    // in size.
    GdkWindow	*root_window = NULL;
    GdkColor	color = { 0, 0, 0, 0 };
    GdkPixmap	*blank_mask;
    char	blank_data[] = { 0x0 };
    root_window = gtk_widget_get_root_window(gui.mainwin);
    blank_mask = gdk_bitmap_create_from_data(root_window, blank_data, 1, 1);
    cursor = gdk_cursor_new_from_pixmap(blank_mask, blank_mask,
					&color, &color, 0, 0);
    gdk_bitmap_unref(blank_mask);
#endif

    return cursor;
}

    static void
mainwin_screen_changed_cb(GtkWidget  *widget,
			  GdkScreen  *previous_screen UNUSED,
			  gpointer   data UNUSED)
{
    if (!gtk_widget_has_screen(widget))
	return;

    /*
     * Recreate the invisible mouse cursor.
     */
    if (gui.blank_pointer != NULL)
#if GTK_CHECK_VERSION(3,0,0)
	g_object_unref(G_OBJECT(gui.blank_pointer));
#else
	gdk_cursor_unref(gui.blank_pointer);
#endif

    gui.blank_pointer = create_blank_pointer();

    if (gui.pointer_hidden && gtk_widget_get_window(gui.drawarea) != NULL)
	gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea),
		gui.blank_pointer);

    gui_gtk_rebuild_text_context();

    if (gui.norm_font != NULL)
    {
	gui_mch_init_font(p_guifont, FALSE);
	gui_set_shellsize(TRUE, FALSE, RESIZE_BOTH);
    }
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

#ifdef FEAT_XIM
    xim_init();
#endif
    gui_mch_new_colors();
#if GTK_CHECK_VERSION(3,0,0)
    gui.surface = gdk_window_create_similar_surface(
	    gtk_widget_get_window(widget),
	    CAIRO_CONTENT_COLOR_ALPHA,
	    gtk_widget_get_allocated_width(widget),
	    gtk_widget_get_allocated_height(widget));
#else
    gui.text_gc = gdk_gc_new(gui.drawarea->window);
#endif

    gui.blank_pointer = create_blank_pointer();
    if (gui.pointer_hidden)
	gdk_window_set_cursor(gtk_widget_get_window(widget), gui.blank_pointer);

    // get the actual size of the scrollbars, if they are realized
    sbar = firstwin->w_scrollbars[SBAR_LEFT].id;
    if (!sbar || (!gui.which_scrollbars[SBAR_LEFT]
				    && firstwin->w_scrollbars[SBAR_RIGHT].id))
	sbar = firstwin->w_scrollbars[SBAR_RIGHT].id;
    if (GTK_IS_WIDGET(sbar))
    {
	gtk_widget_get_allocation(sbar, &allocation);
	if (sbar && gtk_widget_get_realized(sbar) && allocation.width)
	    gui.scrollbar_width = allocation.width;

	sbar = gui.bottom_sbar.id;
	if (sbar && gtk_widget_get_realized(sbar) && allocation.height)
	    gui.scrollbar_height = allocation.height;
    }
}

/*
 * Properly clean up on shutdown.
 */
    static void
drawarea_unrealize_cb(GtkWidget *widget UNUSED, gpointer data UNUSED)
{
    // Don't write messages to the GUI anymore
    full_screen = FALSE;

#ifdef FEAT_XIM
    im_shutdown();
#endif
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

#if GTK_CHECK_VERSION(3,0,0)
    if (gui.surface != NULL)
    {
	cairo_surface_destroy(gui.surface);
	gui.surface = NULL;
    }
#else
    g_object_unref(gui.text_gc);
    gui.text_gc = NULL;
#endif

#if GTK_CHECK_VERSION(3,0,0)
    g_object_unref(G_OBJECT(gui.blank_pointer));
#else
    gdk_cursor_unref(gui.blank_pointer);
#endif
    gui.blank_pointer = NULL;
}

#if GTK_CHECK_VERSION(3,22,2)
    static void
drawarea_style_updated_cb(GtkWidget *widget UNUSED,
			 gpointer data UNUSED)
#else
    static void
drawarea_style_set_cb(GtkWidget	*widget UNUSED,
		      GtkStyle	*previous_style UNUSED,
		      gpointer	data UNUSED)
#endif
{
    gui_mch_new_colors();
}


#if GTK_CHECK_VERSION(3,0,0)
    static gboolean
drawarea_configure_event_cb(GtkWidget	      *widget,
			    GdkEventConfigure *event,
			    gpointer	       data UNUSED)
{
    static int cur_width = 0;
    static int cur_height = 0;

    g_return_val_if_fail(event
	    && event->width >= 1 && event->height >= 1, TRUE);

# if GTK_CHECK_VERSION(3,22,2) && !GTK_CHECK_VERSION(3,22,4)
    // As of 3.22.2, GdkWindows have started distributing configure events to
    // their "native" children (https://git.gnome.org/browse/gtk+/commit/?h=gtk-3-22&id=12579fe71b3b8f79eb9c1b80e429443bcc437dd0).
    //
    // As can be seen from the implementation of move_native_children() and
    // configure_native_child() in gdkwindow.c, those functions actually
    // propagate configure events to every child, failing to distinguish
    // "native" one from non-native one.
    //
    // Naturally, configure events propagated to here like that are fallacious
    // and, as a matter of fact, they trigger a geometric collapse of
    // gui.drawarea in fullscreen and maximized modes.
    //
    // To filter out such nuisance events, we are making use of the fact that
    // the field send_event of such GdkEventConfigures is set to FALSE in
    // configure_native_child().
    //
    // Obviously, this is a terrible hack making GVim depend on GTK's
    // implementation details.  Therefore, watch out any relevant internal
    // changes happening in GTK in the feature (sigh).
    //
    // Follow-up
    // After a few weeks later, the GdkWindow change mentioned above was
    // reverted (https://git.gnome.org/browse/gtk+/commit/?h=gtk-3-22&id=f70039cb9603a02d2369fec4038abf40a1711155).
    // The corresponding official release is 3.22.4.
    if (event->send_event == FALSE)
	return TRUE;
# endif

    if (event->width == cur_width && event->height == cur_height)
	return TRUE;

    cur_width = event->width;
    cur_height = event->height;

    if (gui.surface != NULL)
	cairo_surface_destroy(gui.surface);

    gui.surface = gdk_window_create_similar_surface(
	    gtk_widget_get_window(widget),
	    CAIRO_CONTENT_COLOR_ALPHA,
	    event->width, event->height);

    gtk_widget_queue_draw(widget);

    return TRUE;
}
#endif

/*
 * Callback routine for the "delete_event" signal on the toplevel window.
 * Tries to exit vim gracefully, or refuses to exit with changed buffers.
 */
    static gint
delete_event_cb(GtkWidget *widget UNUSED,
		GdkEventAny *event UNUSED,
		gpointer data UNUSED)
{
    gui_shell_closed();
    return TRUE;
}

#if defined(FEAT_MENU) || defined(FEAT_TOOLBAR) || defined(FEAT_GUI_TABLINE)
    static int
get_item_dimensions(GtkWidget *widget, GtkOrientation orientation)
{
# ifdef FEAT_GUI_GNOME
    GtkOrientation item_orientation = GTK_ORIENTATION_HORIZONTAL;

    if (using_gnome && widget != NULL)
    {
	GtkWidget *parent;
	BonoboDockItem *dockitem;

	parent	 = gtk_widget_get_parent(widget);
	if (G_TYPE_FROM_INSTANCE(parent) == BONOBO_TYPE_DOCK_ITEM)
	{
	    // Only menu & toolbar are dock items.  Could tabline be?
	    // Seem to be only the 2 defined in GNOME
	    widget = parent;
	    dockitem = BONOBO_DOCK_ITEM(widget);

	    if (dockitem == NULL || dockitem->is_floating)
		return 0;
	    item_orientation = bonobo_dock_item_get_orientation(dockitem);
	}
    }
# else
#  define item_orientation GTK_ORIENTATION_HORIZONTAL
# endif

    if (widget != NULL
	    && item_orientation == orientation
	    && gtk_widget_get_realized(widget)
	    && gtk_widget_get_visible(widget))
    {
# if GTK_CHECK_VERSION(3,0,0) || !defined(FEAT_GUI_GNOME)
	GtkAllocation allocation;

	gtk_widget_get_allocation(widget, &allocation);
	return allocation.height;
# else
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	    return widget->allocation.height;
	else
	    return widget->allocation.width;
# endif
    }
    return 0;
}
#endif

    static int
get_menu_tool_width(void)
{
    int width = 0;

#ifdef FEAT_GUI_GNOME // these are never vertical without GNOME
# ifdef FEAT_MENU
    width += get_item_dimensions(gui.menubar, GTK_ORIENTATION_VERTICAL);
# endif
# ifdef FEAT_TOOLBAR
    width += get_item_dimensions(gui.toolbar, GTK_ORIENTATION_VERTICAL);
# endif
# ifdef FEAT_GUI_TABLINE
    if (gui.tabline != NULL)
	width += get_item_dimensions(gui.tabline, GTK_ORIENTATION_VERTICAL);
# endif
#endif

    return width;
}

    static int
get_menu_tool_height(void)
{
    int height = 0;

#ifdef FEAT_MENU
    height += get_item_dimensions(gui.menubar, GTK_ORIENTATION_HORIZONTAL);
#endif
#ifdef FEAT_TOOLBAR
    height += get_item_dimensions(gui.toolbar, GTK_ORIENTATION_HORIZONTAL);
#endif
#ifdef FEAT_GUI_TABLINE
    if (gui.tabline != NULL)
	height += get_item_dimensions(gui.tabline, GTK_ORIENTATION_HORIZONTAL);
#endif

    return height;
}

// This controls whether we can set the real window hints at
// start-up when in a GtkPlug.
// 0 = normal processing (default)
// 1 = init. hints set, no-one's tried to reset since last check
// 2 = init. hints set, attempt made to change hints
static int init_window_hints_state = 0;

    void
gui_gtk_update_window_manager_hints(int force_width, int force_height)
{
    static int old_width  = 0;
    static int old_height = 0;
    static int old_min_width  = 0;
    static int old_min_height = 0;
    static int old_char_width  = 0;
    static int old_char_height = 0;

    int width;
    int height;
    int min_width;
    int min_height;

    // At start-up, don't try to set the hints until the initial
    // values have been used (those that dictate our initial size)
    // Let forced (i.e., correct) values through always.
    if (!(force_width && force_height)  &&  init_window_hints_state > 0)
    {
	// Don't do it!
	init_window_hints_state = 2;
	return;
    }

    // This also needs to be done when the main window isn't there yet,
    // otherwise the hints don't work.
    width  = gui_get_base_width();
    height = gui_get_base_height();
# ifdef FEAT_MENU
    height += tabline_height() * gui.char_height;
# endif
    width  += get_menu_tool_width();
    height += get_menu_tool_height();

    // GtkSockets use GtkPlug's [gui,mainwin] min-size hints to determine
    // their actual widget size.  When we set our size ourselves (e.g.,
    // 'set columns=' or init. -geom) we briefly set the min. to the size
    // we wish to be instead of the legitimate minimum so that we actually
    // resize correctly.
    if (force_width && force_height)
    {
	min_width  = force_width;
	min_height = force_height;
    }
    else
    {
	min_width  = width  + MIN_COLUMNS * gui.char_width;
	min_height = height + MIN_LINES   * gui.char_height;
    }

    // Avoid an expose event when the size didn't change.
    if (width != old_width
	    || height != old_height
	    || min_width != old_min_width
	    || min_height != old_min_height
	    || gui.char_width != old_char_width
	    || gui.char_height != old_char_height)
    {
	GdkGeometry	geometry;
	GdkWindowHints	geometry_mask;

	geometry.width_inc   = gui.char_width;
	geometry.height_inc  = gui.char_height;
	geometry.base_width  = width;
	geometry.base_height = height;
	geometry.min_width   = min_width;
	geometry.min_height  = min_height;
	geometry_mask	     = GDK_HINT_BASE_SIZE|GDK_HINT_RESIZE_INC
			       |GDK_HINT_MIN_SIZE;
	// Using gui.formwin as geometry widget doesn't work as expected
	// with GTK+ 2 -- dunno why.  Presumably all the resizing hacks
	// in Vim confuse GTK+.  For GTK 3 the second argument should be NULL
	// to make the width/height inc works, despite the docs saying
	// something else.
	gtk_window_set_geometry_hints(GTK_WINDOW(gui.mainwin), NULL,
				      &geometry, geometry_mask);
	old_width       = width;
	old_height      = height;
	old_min_width   = min_width;
	old_min_height  = min_height;
	old_char_width  = gui.char_width;
	old_char_height = gui.char_height;
    }
}

#ifdef FEAT_TOOLBAR

/*
 * This extra effort wouldn't be necessary if we only used stock icons in the
 * toolbar, as we do for all builtin icons.  But user-defined toolbar icons
 * shouldn't be treated differently, thus we do need this.
 */
    static void
icon_size_changed_foreach(GtkWidget *widget, gpointer user_data)
{
    if (GTK_IS_IMAGE(widget))
    {
	GtkImage *image = (GtkImage *)widget;

# if GTK_CHECK_VERSION(3,10,0)
	if (gtk_image_get_storage_type(image) == GTK_IMAGE_ICON_NAME)
	{
	    const GtkIconSize icon_size = GPOINTER_TO_INT(user_data);
	    const gchar *icon_name;

	    gtk_image_get_icon_name(image, &icon_name, NULL);
	    image = (GtkImage *)gtk_image_new_from_icon_name(
							 icon_name, icon_size);
	}
# else
	// User-defined icons are stored in a GtkIconSet
	if (gtk_image_get_storage_type(image) == GTK_IMAGE_ICON_SET)
	{
	    GtkIconSet	*icon_set;
	    GtkIconSize	icon_size;

	    gtk_image_get_icon_set(image, &icon_set, &icon_size);
	    icon_size = (GtkIconSize)(long)user_data;

	    gtk_icon_set_ref(icon_set);
	    gtk_image_set_from_icon_set(image, icon_set, icon_size);
	    gtk_icon_set_unref(icon_set);
	}
# endif
    }
    else if (GTK_IS_CONTAINER(widget))
    {
	gtk_container_foreach((GtkContainer *)widget,
			      &icon_size_changed_foreach,
			      user_data);
    }
}

    static void
set_toolbar_style(GtkToolbar *toolbar)
{
    GtkToolbarStyle style;
    GtkIconSize	    size;
    GtkIconSize	    oldsize;

    if ((toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS | TOOLBAR_HORIZ))
		      == (TOOLBAR_TEXT | TOOLBAR_ICONS | TOOLBAR_HORIZ))
	style = GTK_TOOLBAR_BOTH_HORIZ;
    else if ((toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS))
		      == (TOOLBAR_TEXT | TOOLBAR_ICONS))
	style = GTK_TOOLBAR_BOTH;
    else if (toolbar_flags & TOOLBAR_TEXT)
	style = GTK_TOOLBAR_TEXT;
    else
	style = GTK_TOOLBAR_ICONS;

    gtk_toolbar_set_style(toolbar, style);
# if !GTK_CHECK_VERSION(3,0,0)
    gtk_toolbar_set_tooltips(toolbar, (toolbar_flags & TOOLBAR_TOOLTIPS) != 0);
# endif

    switch (tbis_flags)
    {
	case TBIS_TINY:	    size = GTK_ICON_SIZE_MENU;		break;
	case TBIS_SMALL:    size = GTK_ICON_SIZE_SMALL_TOOLBAR;	break;
	case TBIS_MEDIUM:   size = GTK_ICON_SIZE_BUTTON;	break;
	case TBIS_LARGE:    size = GTK_ICON_SIZE_LARGE_TOOLBAR;	break;
	case TBIS_HUGE:     size = GTK_ICON_SIZE_DND;		break;
	case TBIS_GIANT:    size = GTK_ICON_SIZE_DIALOG;	break;
	default:	    size = GTK_ICON_SIZE_INVALID;	break;
    }
    oldsize = gtk_toolbar_get_icon_size(toolbar);

    if (size == GTK_ICON_SIZE_INVALID)
    {
	// Let global user preferences decide the icon size.
	gtk_toolbar_unset_icon_size(toolbar);
	size = gtk_toolbar_get_icon_size(toolbar);
    }
    if (size != oldsize)
    {
	gtk_container_foreach(GTK_CONTAINER(toolbar),
			      &icon_size_changed_foreach,
			      GINT_TO_POINTER((int)size));
    }
    gtk_toolbar_set_icon_size(toolbar, size);
}

#endif // FEAT_TOOLBAR

#if defined(FEAT_GUI_TABLINE)
# if GTK_CHECK_VERSION(3,0,0)
static GtkPopoverMenu *tabline_menu;
# else
static GtkWidget *tabline_menu;
# endif
# if !GTK_CHECK_VERSION(3,0,0)
static GtkTooltips *tabline_tooltip;
# endif
static int clicked_page;	    // page clicked in tab line



/*
 * Create a menu for the tab line.
 */

/*
 * Handle selecting an item in the tab line popup menu.
 */
    static void
tabline_menu_handler(GtkMenuItem *item UNUSED, gpointer user_data)
{
    // Add the string cmd into input buffer
    send_tabline_menu_event(clicked_page, (int)(long)user_data);
}

    static void
add_tabline_menu_item(GtkWidget *menu, char_u *text, int resp)
{
    GtkWidget	*item;
    char_u	*utf_text;

    utf_text = CONVERT_TO_UTF8(text);
    item = gtk_menu_item_new_with_label((const char *)utf_text);
    gtk_widget_show(item);
    CONVERT_TO_UTF8_FREE(utf_text);

    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(G_OBJECT(item), "activate",
	    G_CALLBACK(tabline_menu_handler),
	    GINT_TO_POINTER(resp));
}

    static gboolean
on_tabline_menu(GtkWidget *widget, GdkEvent *event)
{
    // Was this button press event ?
    if (event->type == GDK_BUTTON_PRESS)
    {
	GdkEventButton *bevent = (GdkEventButton *)event;
	int		x = bevent->x;
	int		y = bevent->y;
	GtkWidget	*tabwidget;
	GdkWindow	*tabwin;

	// When ignoring events return TRUE so that the selected page doesn't
	// change.
	if (hold_gui_events || cmdwin_type != 0)
	    return TRUE;

	tabwin = gui_gtk_window_at_position(gui.mainwin, &x, &y);

	gdk_window_get_user_data(tabwin, (gpointer)&tabwidget);
	clicked_page = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(tabwidget),
							 "tab_num"));

	// If the event was generated for 3rd button popup the menu.
	if (bevent->button == 3)
	{
# if GTK_CHECK_VERSION(3,22,2)
	    gtk_menu_popup_at_pointer(GTK_MENU(widget), event);
# else
	    gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
						bevent->button, bevent->time);
# endif
	    // We handled the event.
	    return TRUE;
	}
	else if (bevent->button == 1)
	{
	    if (clicked_page == 0)
	    {
		// Click after all tabs moves to next tab page.  When "x" is
		// small guess it's the left button.
		send_tabline_event(x < 50 ? -1 : 0);
	    }
	}
	else if (bevent->button == 2)
	{
	    if (clicked_page != 0)
		// Middle mouse click on tabpage label closes that tab.
		send_tabline_menu_event(clicked_page, TABLINE_MENU_CLOSE);
	}
    }

    // We didn't handle the event.
    return FALSE;
}

    static void
create_tabline_menu(GtkNotebook *notebook)
{
    GtkWidget *menu;

    menu = gtk_menu_new();
    add_tabline_menu_item(menu, (char_u *)_("Close tab"), TABLINE_MENU_CLOSE);
    add_tabline_menu_item(menu, (char_u *)_("New tab"), TABLINE_MENU_NEW);
    add_tabline_menu_item(menu, (char_u *)_("Open Tab..."), TABLINE_MENU_OPEN);

    g_signal_connect_swapped(G_OBJECT(notebook), "button-press-event",
	    G_CALLBACK(on_tabline_menu), G_OBJECT(tabline_menu));

# if GTK_CHECK_VERSION(3,0,0)
    tabline_menu = GTK_POPOVER_MENU(menu);
# else
    tabline_menu = GTK_WIDGET(menu);
# endif
}

/*
 * Handle selecting one of the tabs.
 */
    static void
on_select_tab(
	GtkNotebook	*notebook UNUSED,
	gpointer	*page UNUSED,
	gint		idx,
	gpointer	data UNUSED)
{
    if (!gui_gtk_should_ignore_tabline_event())
	send_tabline_event(idx + 1);
}

# if GTK_CHECK_VERSION(2,10,0)
/*
 * Handle reordering the tabs (using D&D).
 */
    static void
on_tab_reordered(
	GtkNotebook	*notebook UNUSED,
	gpointer	*page UNUSED,
	gint		idx,
	gpointer	data UNUSED)
{
    if (gui_gtk_should_ignore_tabline_event())
	return;

    if ((tabpage_index(curtab) - 1) < idx)
	tabpage_move(idx + 1);
    else
	tabpage_move(idx);
}
# endif

    static GtkWidget *
gui_gtk_build_tab_label(const char *label_text, GtkWidget *page)
{
    GtkWidget *label;
    GtkWidget *event_box;

    label = gtk_label_new(label_text);
    gtk_widget_set_visible(label, TRUE);
#if !GTK_CHECK_VERSION(3,14,0)
    gtk_misc_set_padding(GTK_MISC(label), 2, 2);
#endif
    event_box = gtk_event_box_new();
    gtk_widget_set_visible(event_box, TRUE);
    g_object_set_data(G_OBJECT(event_box), "tab_num", GINT_TO_POINTER(1L));
    gtk_container_add(GTK_CONTAINER(event_box), label);
    return event_box;
}

/*
 * Update the labels of the tabline.
 */
    void
gui_mch_update_tabline(void)
{
    GtkWidget	    *page;
    GtkWidget	    *label;     // label is a GtkLabel
    GtkWidget	    *tab_label; // tab_label is either GtkEventBox or the same GtkLabel as label.
    tabpage_T	    *tp;
    int		    nr = 0;
    int		    tab_num;
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
# if GTK_CHECK_VERSION(3,2,0)
	    page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	    gtk_box_set_homogeneous(GTK_BOX(page), FALSE);
# else
	    page = gtk_vbox_new(FALSE, 0);
# endif
	    gtk_widget_show(page);
	    tab_label = gui_gtk_build_tab_label("-Empty-", page);
# if !GTK_CHECK_VERSION(3,14,0)
	    gtk_misc_set_padding(GTK_MISC(label), 2, 2);
# endif
	    gtk_notebook_insert_page(GTK_NOTEBOOK(gui.tabline),
		    page,
		    tab_label,
		    nr++);
	    gtk_notebook_set_tab_label(GTK_NOTEBOOK(gui.tabline), page, tab_label);
# if GTK_CHECK_VERSION(2,10,0)
	    gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(gui.tabline),
		    page,
		    TRUE);
# endif
	}

	tab_label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(gui.tabline), page);
	g_object_set_data(G_OBJECT(tab_label), "tab_num",
		GINT_TO_POINTER(tab_num));
	label = gtk_bin_get_child(GTK_BIN(tab_label));
	get_tabline_label(tp, FALSE);
	labeltext = CONVERT_TO_UTF8(NameBuff);
	gtk_label_set_text(GTK_LABEL(label), (const char *)labeltext);
	CONVERT_TO_UTF8_FREE(labeltext);

	get_tabline_label(tp, TRUE);
	labeltext = CONVERT_TO_UTF8(NameBuff);
# if GTK_CHECK_VERSION(3,0,0)
	gtk_widget_set_tooltip_text(tab_label, (const gchar *)labeltext);
# else
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tabline_tooltip), tab_label,
			     (const char *)labeltext, NULL);
# endif
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

#endif // FEAT_GUI_TABLINE


#ifndef USE_GTK4
/*
 * Add selection targets for PRIMARY and CLIPBOARD selections.
 */
    void
gui_gtk_set_selection_targets(GdkAtom selection)
{
    int		    i, j = 0;
    static int	    n_targets = N_SELECTION_TARGETS;
    static GtkTargetEntry  targets[N_SELECTION_TARGETS];

    if (targets[0].target == NULL)
    {
	for (i = 0; i < (int)N_SELECTION_TARGETS; ++i)
	{
	    // OpenOffice tries to use TARGET_HTML and fails when we don't
	    // return something, instead of trying another target. Therefore only
	    // offer TARGET_HTML when it works.
	    if (!clip_html && selection_targets[i].info == TARGET_HTML)
		n_targets--;
	    else
		targets[j++] = selection_targets[i];
	}
    }

    gtk_selection_clear_targets(gui.drawarea, selection);
    gtk_selection_add_targets(gui.drawarea, selection,
			      targets, n_targets);
}

# ifdef FEAT_DND
/*
 * Set up for receiving DND items.
 */
    void
gui_gtk_set_dnd_targets(void)
{
    int		    i, j = 0;
    int		    n_targets = N_DND_TARGETS;
    GtkTargetEntry  targets[N_DND_TARGETS];

    for (i = 0; i < (int)N_DND_TARGETS; ++i)
    {
	if (!clip_html && dnd_targets[i].info == TARGET_HTML)
	    n_targets--;
	else
	    targets[j++] = dnd_targets[i];
    }

    gtk_drag_dest_unset(gui.drawarea);
    gtk_drag_dest_set(gui.drawarea,
		      GTK_DEST_DEFAULT_ALL,
		      targets, n_targets,
		      GDK_ACTION_COPY | GDK_ACTION_MOVE);
}
# endif
#endif

/*
 * Initialize the GUI.	Create all the windows, set up all the callbacks etc.
 * Returns OK for success, FAIL when the GUI can't be started.
 */
    int
gui_mch_init(void)
{
    GtkWidget *vbox;

#ifdef FEAT_GUI_GNOME
    // Initialize the GNOME libraries.	gnome_program_init()/gnome_init()
    // exits on failure, but that's a non-issue because we already called
    // gtk_init_check() in gui_mch_init_check().
    if (using_gnome)
    {
	gnome_program_init(VIMPACKAGE, VIM_VERSION_SHORT,
			   LIBGNOMEUI_MODULE, gui_argc, gui_argv, NULL);
# if defined(LC_NUMERIC)
	{
	    char *p = setlocale(LC_NUMERIC, NULL);

	    // Make sure strtod() uses a decimal point, not a comma. Gnome
	    // init may change it.
	    if (p == NULL || strcmp(p, "C") != 0)
	       setlocale(LC_NUMERIC, "C");
	}
# endif
    }
#endif
    VIM_CLEAR(gui_argv);

#if GLIB_CHECK_VERSION(2,1,3)
    // Set the human-readable application name
    g_set_application_name("Vim");
#endif
    /*
     * Force UTF-8 output no matter what the value of 'encoding' is.
     * did_set_string_option() in option.c prohibits changing 'termencoding'
     * to something else than UTF-8 if the GUI is in use.
     */
    set_option_value_give_err((char_u *)"termencoding",
						     0L, (char_u *)"utf-8", 0);

#ifdef FEAT_TOOLBAR
    gui_gtk_register_stock_icons();
#endif
    // FIXME: Need to install the classic icons and a gtkrc.classic file.
    // The hard part is deciding install locations and the Makefile magic.
#if !GTK_CHECK_VERSION(3,0,0)
# if 0
    gtk_rc_parse("gtkrc");
# endif
#endif

    // Initialize values
    gui.border_width = 2;
    gui.scrollbar_width = SB_DEFAULT_WIDTH;
    gui.scrollbar_height = SB_DEFAULT_WIDTH;
#if GTK_CHECK_VERSION(3,0,0)
    gui.fgcolor = g_new(GdkRGBA, 1);
    gui.bgcolor = g_new(GdkRGBA, 1);
    gui.spcolor = g_new(GdkRGBA, 1);
#else
    // LINTED: avoid warning: conversion to 'unsigned long'
    gui.fgcolor = g_new0(GdkColor, 1);
    // LINTED: avoid warning: conversion to 'unsigned long'
    gui.bgcolor = g_new0(GdkColor, 1);
    // LINTED: avoid warning: conversion to 'unsigned long'
    gui.spcolor = g_new0(GdkColor, 1);
#endif

#ifndef USE_GTK4
    // Initialise atoms
    html_atom = gdk_atom_intern("text/html", FALSE);
    utf8_string_atom = gdk_atom_intern("UTF8_STRING", FALSE);
#endif

    // Set default foreground and background colors.
    gui.norm_pixel = gui.def_norm_pixel;
    gui.back_pixel = gui.def_back_pixel;

    // The GtkSocket and GtkPlug functionality was removed from GTK4 because
    // Wayland does have an XEmbed analogue.
    if (gtk_socket_id != 0)
    {
	GtkWidget *plug;

	// Use GtkSocket from another app.
	plug = gtk_plug_new_for_display(gdk_display_get_default(),
					gtk_socket_id);
	if (plug != NULL && gtk_plug_get_socket_window(GTK_PLUG(plug)) != NULL)
	{
	    gui.mainwin = plug;
	}
	else
	{
	    g_warning("Connection to GTK+ socket (ID %u) failed",
		      (unsigned int)gtk_socket_id);
	    // Pretend we never wanted it if it failed (get own window)
	    gtk_socket_id = 0;
	}
    }

    if (gtk_socket_id == 0)
    {
#ifdef FEAT_GUI_GNOME
	if (using_gnome)
	{
	    gui.mainwin = gnome_app_new("Vim", NULL);
# ifdef USE_XSMP
	    // Use the GNOME save-yourself functionality now.
	    xsmp_close();
# endif
	}
	else
#endif
	    gui.mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    }

    gtk_widget_set_name(gui.mainwin, "vim-main-window");

    gui_gtk_rebuild_text_context();

    gtk_container_set_border_width(GTK_CONTAINER(gui.mainwin), 0);
    gtk_widget_add_events(gui.mainwin, GDK_VISIBILITY_NOTIFY_MASK);
    g_signal_connect(G_OBJECT(gui.mainwin), "delete-event",
		     G_CALLBACK(&delete_event_cb), NULL);


    g_signal_connect(G_OBJECT(gui.mainwin), "realize",
		     G_CALLBACK(&mainwin_realize), NULL);

    g_signal_connect(G_OBJECT(gui.mainwin), "screen-changed",
		     G_CALLBACK(&mainwin_screen_changed_cb), NULL);

    gui.accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(gui.mainwin), gui.accel_group);

    // A vertical box holds the menubar, toolbar and main text window.
#if GTK_CHECK_VERSION(3,2,0)
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
#else
    vbox = gtk_vbox_new(FALSE, 0);
#endif

#ifdef FEAT_GUI_GNOME
    if (using_gnome)
    {
# if defined(FEAT_MENU)
	// automagically restore menubar/toolbar placement
	gnome_app_enable_layout_config(GNOME_APP(gui.mainwin), TRUE);
# endif
	gnome_app_set_contents(GNOME_APP(gui.mainwin), vbox);
    }
    else
#endif
    {
	gtk_container_add(GTK_CONTAINER(gui.mainwin), vbox);
	gtk_widget_show(vbox);
    }

#ifdef FEAT_MENU
    /*
     * Create the menubar and handle
     */
    gui.menubar = gtk_menu_bar_new();
    gtk_widget_set_name(gui.menubar, "vim-menubar");

    // Avoid that GTK takes <F10> away from us.
    {
	GtkSettings *gtk_settings = gui_gtk_get_settings();
	g_object_set(gtk_settings, "gtk-menu-bar-accel", NULL, NULL);
    }


# ifdef FEAT_GUI_GNOME
    if (using_gnome)
    {
	BonoboDockItem *dockitem;

	gnome_app_set_menus(GNOME_APP(gui.mainwin), GTK_MENU_BAR(gui.menubar));
	dockitem = gnome_app_get_dock_item_by_name(GNOME_APP(gui.mainwin),
						   GNOME_APP_MENUBAR_NAME);
	// We don't want the menu to float.
	bonobo_dock_item_set_behavior(dockitem,
		bonobo_dock_item_get_behavior(dockitem)
				       | BONOBO_DOCK_ITEM_BEH_NEVER_FLOATING);
	gui.menubar_h = GTK_WIDGET(dockitem);
    }
    else
# endif	// FEAT_GUI_GNOME
    {
	// Always show the menubar, otherwise <F10> doesn't work.  It may be
	// disabled in gui_init() later.
	gtk_widget_show(gui.menubar);
	gtk_box_prepend(GTK_BOX(vbox), gui.menubar);
    }
#endif	// FEAT_MENU

#ifdef FEAT_TOOLBAR
    /*
     * Create the toolbar and handle
     */
    // some aesthetics on the toolbar
# if defined(USE_GTK3) || defined(USE_GTK4)
    // TODO: Add GTK+ 3 code here using GtkCssProvider if necessary.
    // N.B.  Since the default value of GtkToolbar::button-relief is
    // GTK_RELIEF_NONE, there's no need to specify that, probably.
# else
    gtk_rc_parse_string(
	    "style \"vim-toolbar-style\" {\n"
	    "  GtkToolbar::button_relief = GTK_RELIEF_NONE\n"
	    "}\n"
	    "widget \"*.vim-toolbar\" style \"vim-toolbar-style\"\n");
# endif
    gui.toolbar = gtk_toolbar_new();
    gtk_widget_set_name(gui.toolbar, "vim-toolbar");
    set_toolbar_style(GTK_TOOLBAR(gui.toolbar));

# ifdef FEAT_GUI_GNOME
    if (using_gnome)
    {
	BonoboDockItem *dockitem;

	gnome_app_set_toolbar(GNOME_APP(gui.mainwin), GTK_TOOLBAR(gui.toolbar));
	dockitem = gnome_app_get_dock_item_by_name(GNOME_APP(gui.mainwin),
						   GNOME_APP_TOOLBAR_NAME);
	gui.toolbar_h = GTK_WIDGET(dockitem);
	// When the toolbar is floating it gets stuck.  So long as that isn't
	// fixed let's disallow floating.
	bonobo_dock_item_set_behavior(dockitem,
		bonobo_dock_item_get_behavior(dockitem)
				       | BONOBO_DOCK_ITEM_BEH_NEVER_FLOATING);
	gtk_container_set_border_width(GTK_CONTAINER(gui.toolbar), 0);
    }
    else
# endif	// FEAT_GUI_GNOME
    {
	if (vim_strchr(p_go, GO_TOOLBAR) != NULL
		&& (toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS)))
	    gtk_widget_show(gui.toolbar);
	gtk_box_prepend(GTK_BOX(vbox), gui.toolbar);
    }
#endif // FEAT_TOOLBAR

#ifdef FEAT_GUI_TABLINE
    /*
     * Use a Notebook for the tab pages labels.  The labels are hidden by
     * default.
     */
    gui.tabline = gtk_notebook_new();
    gtk_widget_show(gui.tabline);
    gtk_box_prepend(GTK_BOX(vbox), gui.tabline);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(gui.tabline), FALSE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(gui.tabline), FALSE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(gui.tabline), TRUE);
# if !GTK_CHECK_VERSION(3,0,0)
    gtk_notebook_set_tab_border(GTK_NOTEBOOK(gui.tabline), FALSE);
# endif

# if !GTK_CHECK_VERSION(3,0,0)
    tabline_tooltip = gtk_tooltips_new();
    gtk_tooltips_enable(GTK_TOOLTIPS(tabline_tooltip));
# endif

    {
	GtkWidget *page, *tab_label;

	// Add the first tab.
# if GTK_CHECK_VERSION(3,2,0)
	page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(page), FALSE);
# else
	page = gtk_vbox_new(FALSE, 0);
# endif
	gtk_widget_show(page);
	tab_label = gui_gtk_build_tab_label("-Empty-", page);
	gtk_notebook_append_page(GTK_NOTEBOOK(gui.tabline), page);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(gui.tabline), page, tab_label);
# if GTK_CHECK_VERSION(2,10,0)
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(gui.tabline), page, TRUE);
# endif
    }

    g_signal_connect(G_OBJECT(gui.tabline), "switch-page",
		     G_CALLBACK(on_select_tab), NULL);
# if GTK_CHECK_VERSION(2,10,0)
    g_signal_connect(G_OBJECT(gui.tabline), "page-reordered",
		     G_CALLBACK(on_tab_reordered), NULL);
# endif

    create_tabline_menu(GTK_NOTEBOOK(gui.tabline));
    // GtkEventControllerFocus *tabline_focus_events = (GtkEventControllerFocus *)gtk_event_controller_focus_new();
    // gtk_widget_add_controller(gui.tabline, GTK_EVENT_CONTROLLER(tabline_focus_events));

#endif // FEAT_GUI_TABLINE

    gui.formwin = GTK_WIDGET(gui_gtk_form_new());
    gtk_widget_set_visible(gui.formwin, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(gui.formwin), 0);

#if !GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_events(gui.formwin, GDK_EXPOSURE_MASK);
#endif
#if GTK_CHECK_VERSION(3,22,2)
    gtk_widget_set_name(gui.formwin, "vim-gtk-form");
#endif

    gui.drawarea = gtk_drawing_area_new();
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_hexpand(GTK_WIDGET(gui.formwin), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(gui.formwin), TRUE);
    gtk_widget_set_hexpand(GTK_WIDGET(gui.drawarea), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(gui.drawarea), TRUE);

    gui.surface = NULL;
#endif

    // Determine which events we will filter.
    gint event_mask =
	GDK_EXPOSURE_MASK |
	GDK_ENTER_NOTIFY_MASK |
	GDK_LEAVE_NOTIFY_MASK |
	GDK_BUTTON_PRESS_MASK |
	GDK_BUTTON_RELEASE_MASK |
	GDK_KEY_PRESS_MASK |
	GDK_KEY_RELEASE_MASK |
	GDK_POINTER_MOTION_MASK |
	GDK_POINTER_MOTION_HINT_MASK;
# if GTK_CHECK_VERSION(3,4,0)
    if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
    {
	// for X11, if we were using smooth scroll events, we
	// would get an scroll without deltas on the very first user scroll* and
	// get both "unsmooth" scroll and smooth scroll events after
	// copying into the primary selection
	//
	// *: https://bugzilla.gnome.org/show_bug.cgi?id=675959
	event_mask |= GDK_SCROLL_MASK;
    }
    else
    {
	// for Wayland, touchpads don't generate "unsmooth" scroll events. Both
	// touchpads and wheels generate smooth scroll events expectedly.
	event_mask |= GDK_SMOOTH_SCROLL_MASK;
    }
# else
    event_mask |= GDK_SCROLL_MASK;
# endif
    gtk_widget_set_events(gui.drawarea, event_mask);

    gtk_widget_show(gui.drawarea);
    gui_gtk_form_put(GTK_FORM(gui.formwin), gui.drawarea, 0, 0);
    gtk_widget_show(gui.formwin);
    gtk_box_pack_start(GTK_BOX(vbox), gui.formwin, TRUE, TRUE, 0);

    // For GtkSockets, key-presses must go to the focus widget (drawarea)
    // and not the window.
    g_signal_connect((gtk_socket_id == 0) ? G_OBJECT(gui.mainwin)
					  : G_OBJECT(gui.drawarea),
		       "key-press-event",
		       G_CALLBACK(key_press_event), NULL);

#if defined(FEAT_XIM) || GTK_CHECK_VERSION(3,0,0)
    // Also forward key release events for the benefit of GTK+ 2 input
    // modules.  Try CTRL-SHIFT-xdigits to enter a Unicode code point.
    g_signal_connect((gtk_socket_id == 0) ? G_OBJECT(gui.mainwin)
					  : G_OBJECT(gui.drawarea),
		     "key-release-event",
		     G_CALLBACK(&key_release_event), NULL);
#endif
    g_signal_connect(G_OBJECT(gui.drawarea), "realize",
		     G_CALLBACK(drawarea_realize_cb), NULL);
    g_signal_connect(G_OBJECT(gui.drawarea), "unrealize",
		     G_CALLBACK(drawarea_unrealize_cb), NULL);
#if GTK_CHECK_VERSION(3,0,0)
    g_signal_connect(G_OBJECT(gui.drawarea), "configure-event",
	    G_CALLBACK(drawarea_configure_event_cb), NULL);
#endif

#if GTK_CHECK_VERSION(3,22,2)
    g_signal_connect_after(G_OBJECT(gui.drawarea), "style-updated",
			   G_CALLBACK(&drawarea_style_updated_cb), NULL);
#else
    g_signal_connect_after(G_OBJECT(gui.drawarea), "style-set",
			   G_CALLBACK(&drawarea_style_set_cb), NULL);
#endif

#if !GTK_CHECK_VERSION(3,0,0)
    gui.visibility = GDK_VISIBILITY_UNOBSCURED;
#endif

#if !defined(USE_GNOME_SESSION)
    wm_protocols_atom = gdk_atom_intern("WM_PROTOCOLS", FALSE);
    save_yourself_atom = gdk_atom_intern("WM_SAVE_YOURSELF", FALSE);
#endif

    if (gtk_socket_id != 0)
	// make sure keyboard input can go to the drawarea
	gtk_widget_set_can_focus(gui.drawarea, TRUE);

    /*
     * Set clipboard specific atoms
     */
    vim_atom = gdk_atom_intern(VIM_ATOM_NAME, FALSE);
    vimenc_atom = gdk_atom_intern(VIMENC_ATOM_NAME, FALSE);
    clip_star.gtk_sel_atom = GDK_SELECTION_PRIMARY;
    clip_plus.gtk_sel_atom = gdk_atom_intern("CLIPBOARD", FALSE);

    /*
     * Start out by adding the configured border width into the border offset.
     */
    gui.border_offset = gui.border_width;

#if GTK_CHECK_VERSION(3,0,0)
    g_signal_connect(G_OBJECT(gui.drawarea), "draw",
		     G_CALLBACK(draw_event), NULL);
#else
    gtk_signal_connect(GTK_OBJECT(gui.mainwin), "visibility_notify_event",
		       GTK_SIGNAL_FUNC(visibility_event), NULL);
    gtk_signal_connect(GTK_OBJECT(gui.drawarea), "expose_event",
		       GTK_SIGNAL_FUNC(expose_event), NULL);
#endif

    /*
     * Only install these enter/leave callbacks when 'p' in 'guioptions'.
     * Only needed for some window managers.
     */
    if (vim_strchr(p_go, GO_POINTER) != NULL)
    {
	g_signal_connect(G_OBJECT(gui.drawarea), "leave-notify-event",
			 G_CALLBACK(leave_notify_event), NULL);
	g_signal_connect(G_OBJECT(gui.drawarea), "enter-notify-event",
			 G_CALLBACK(enter_notify_event), NULL);
    }

    // Real windows can get focus ... GtkPlug, being a mere container can't,
    // only its widgets.  Arguably, this could be common code and we do not
    // use the window focus at all, but let's be safe.
    if (gtk_socket_id == 0)
    {
	g_signal_connect(G_OBJECT(gui.mainwin), "focus-out-event",
			 G_CALLBACK(focus_out_event), NULL);
	g_signal_connect(G_OBJECT(gui.mainwin), "focus-in-event",
			 G_CALLBACK(focus_in_event), NULL);
    }
    else
    {
	g_signal_connect(G_OBJECT(gui.drawarea), "focus-out-event",
			 G_CALLBACK(focus_out_event), NULL);
	g_signal_connect(G_OBJECT(gui.drawarea), "focus-in-event",
			 G_CALLBACK(focus_in_event), NULL);
#ifdef FEAT_GUI_TABLINE
	fprintf(stderr, "FEAT_GUI_TABLINE is enabled\n");
	exit(1);
	g_signal_connect(G_OBJECT(gui.tabline), "focus-out-event",
			 G_CALLBACK(focus_out_event), NULL);
	g_signal_connect(G_OBJECT(gui.tabline), "focus-in-event",
			 G_CALLBACK(focus_in_event), NULL);
#endif // FEAT_GUI_TABLINE
    }

    g_signal_connect(G_OBJECT(gui.drawarea), "motion-notify-event",
		     G_CALLBACK(motion_notify_event), NULL);
    g_signal_connect(G_OBJECT(gui.drawarea), "button-press-event",
		     G_CALLBACK(button_press_event), NULL);
    g_signal_connect(G_OBJECT(gui.drawarea), "button-release-event",
		     G_CALLBACK(button_release_event), NULL);
    g_signal_connect(G_OBJECT(gui.drawarea), "scroll-event",
		     G_CALLBACK(scroll_event), NULL);

    // Pretend we don't have input focus, we will get an event if we do.
    gui.in_focus = FALSE;

    // Handle changes to the "Xft/DPI" setting.
    {
	GtkSettings *gtk_settings = gui_gtk_get_settings();

	g_signal_connect(gtk_settings, "notify::gtk-xft-dpi",
			   G_CALLBACK(gui_gtk_settings_xft_dpi_changed_cb), NULL);
    }

    return OK;
}

#if defined(USE_GNOME_SESSION)
/*
 * This is called from gui_start() after a fork() has been done.
 * We have to tell the session manager our new PID.
 */
    void
gui_mch_forked(void)
{
    if (!using_gnome)
	return;

    GnomeClient *client;

    client = gnome_master_client();

    if (client != NULL)
	gnome_client_set_process_id(client, getpid());
}
#endif // USE_GNOME_SESSION

#if GTK_CHECK_VERSION(3,0,0)

    static void
set_cairo_source_rgba_from_color(cairo_t *cr, guicolor_T color)
{
    const GdkRGBA rgba = color_to_rgba(color);
    cairo_set_source_rgba(cr, rgba.red, rgba.green, rgba.blue, rgba.alpha);
}
#endif // GTK_CHECK_VERSION(3,0,0)

/*
 * Called when the foreground or background color has been changed.
 * This used to change the graphics contexts directly but we are
 * currently manipulating them where desired.
 */
    void
gui_mch_new_colors(void)
{
    if (gui.drawarea == NULL)
	return;

#if GTK_CHECK_VERSION(3,22,2)
    if (gui.formwin == NULL)
	return;
#endif

    if (gtk_widget_get_window(gui.drawarea) == NULL)
	return;

#if !GTK_CHECK_VERSION(3,22,2)
    GdkWindow * const da_win = gtk_widget_get_window(gui.drawarea);
#endif
#if GTK_CHECK_VERSION(3,22,2)
    GtkStyleContext * const context =
				 gtk_widget_get_style_context(gui.formwin);
    GtkCssProvider * const provider = gtk_css_provider_new();
    gchar * const css = g_strdup_printf(
	    "widget#vim-gtk-form {\n"
	    "  background-color: #%.2lx%.2lx%.2lx;\n"
	    "}\n",
	     (gui.back_pixel >> 16) & 0xff,
	     (gui.back_pixel >> 8) & 0xff,
	     gui.back_pixel & 0xff);

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider(context,
	    GTK_STYLE_PROVIDER(provider), G_MAXUINT);

    g_free(css);
    g_object_unref(provider);
#elif GTK_CHECK_VERSION(3,4,0) // !GTK_CHECK_VERSION(3,22,2)
    GdkRGBA rgba;

    rgba = color_to_rgba(gui.back_pixel);
    {
	cairo_pattern_t * const pat = cairo_pattern_create_rgba(
		rgba.red, rgba.green, rgba.blue, rgba.alpha);
	if (pat != NULL)
	{
	    gdk_window_set_background_pattern(da_win, pat);
	    cairo_pattern_destroy(pat);
	}
	else
	    gdk_window_set_background_rgba(da_win, &rgba);
    }
#else // !GTK_CHECK_VERSION(3,4,0)
    GdkColor color = { 0, 0, 0, 0 };

    color.pixel = gui.back_pixel;
    gdk_window_set_background(da_win, &color);
#endif // !GTK_CHECK_VERSION(3,22,2)
}

/*
 * This signal informs us about the need to rearrange our sub-widgets.
 */
    static gint
form_configure_event(GtkWidget *widget UNUSED,
		     GdkEventConfigure *event,
		     gpointer data UNUSED)
{
    int	    usable_height = event->height;
#ifdef TRACK_RESIZE_HISTORY
    // Resize requests are made for gui.mainwin;
    // get its dimensions for searching if this event
    // is a response to a vim request.
    int	    w, h;
    gtk_window_get_size(GTK_WINDOW(gui.mainwin), &w, &h);

# ifdef ENABLE_RESIZE_HISTORY_LOG
    ch_log(NULL, "gui_gtk: form_configure_event: (%d, %d) [%d, %d]",
	   w, h, (int)Columns, (int)Rows);
# endif

    // Look through history of recent vim resize requests.
    // If this event matches:
    //	    - "latest resize hist" We're caught up;
    //		clear the history and process this event.
    //		If history is, old to new, 100, 99, 100, 99. If this event is
    //		99 for the stale, it is matched against the current. History
    //		is cleared, we may bounce, but no worse than before.
    //	    - "older/stale hist" If match an unused event in history,
    //		then discard this event, and mark the matching event as used.
    //	    - "no match" Figure it's a user resize event, clear history.
    // NOTE: clear history is default, then all incoming events are processed

    if (!MATCH_WIDTH_HEIGHT(latest_resize_hist, w, h)
					     && match_stale_width_height(w, h))
	// discard stale event
	return TRUE;
    clear_resize_hists();
#endif

#if GTK_CHECK_VERSION(3,22,2) && !GTK_CHECK_VERSION(3,22,4)
    // As of 3.22.2, GdkWindows have started distributing configure events to
    // their "native" children (https://git.gnome.org/browse/gtk+/commit/?h=gtk-3-22&id=12579fe71b3b8f79eb9c1b80e429443bcc437dd0).
    //
    // As can be seen from the implementation of move_native_children() and
    // configure_native_child() in gdkwindow.c, those functions actually
    // propagate configure events to every child, failing to distinguish
    // "native" one from non-native one.
    //
    // Naturally, configure events propagated to here like that are fallacious
    // and, as a matter of fact, they trigger a geometric collapse of
    // gui.formwin.
    //
    // To filter out such fallacious events, check if the given event is the
    // one that was sent out to the right place. Ignore it if not.
    //
    // Follow-up
    // After a few weeks later, the GdkWindow change mentioned above was
    // reverted (https://git.gnome.org/browse/gtk+/commit/?h=gtk-3-22&id=f70039cb9603a02d2369fec4038abf40a1711155).
    // The corresponding official release is 3.22.4.
    if (event->window != gtk_widget_get_window(gui.formwin))
	return TRUE;
#endif

    // When in a GtkPlug, we can't guarantee valid heights (as a round
    // no. of char-heights), so we have to manually sanitise them.
    // Widths seem to sort themselves out, don't ask me why.
    if (gtk_socket_id != 0)
	usable_height -= (gui.char_height - (gui.char_height/2)); // sic.

    gui_gtk_form_freeze(GTK_FORM(gui.formwin));
    gui_resize_shell(event->width, usable_height);
    gui_gtk_form_thaw(GTK_FORM(gui.formwin));

    return TRUE;
}

    void
gui_gtk_get_screen_geom_of_win(
	GtkWidget *wid,
	int point_x,	    // x position of window if not initialized
	int point_y,	    // y position of window if not initialized
	int *screen_x,
	int *screen_y,
	int *width,
	int *height)
{
    GdkRectangle geometry;
    GdkWindow *win = gtk_widget_get_window(wid);
#if GTK_CHECK_VERSION(3,22,0)
    GdkDisplay *dpy;
    GdkMonitor *monitor;

    if (wid != NULL && gtk_widget_get_realized(wid))
	dpy = gtk_widget_get_display(wid);
    else
	dpy = gdk_display_get_default();

    if (win != NULL)
	monitor = gdk_display_get_monitor_at_window(dpy, win);
    else
	monitor = gdk_display_get_monitor_at_point(dpy, point_x, point_y);
    if (monitor != NULL)
	gdk_monitor_get_geometry(monitor, &geometry);
#else
    GdkScreen* screen;
    int monitor;

    if (wid != NULL && gtk_widget_has_screen(wid))
	screen = gtk_widget_get_screen(wid);
    else
	screen = gdk_screen_get_default();
    if (win != NULL)
	monitor = gdk_screen_get_monitor_at_window(screen, win);
    else
	monitor = gdk_screen_get_monitor_at_point(screen, point_x, point_y);
    gdk_screen_get_monitor_geometry(screen, monitor, &geometry);
#endif // !GTK_CHECK_VERSION(3,22,0)
    *screen_x = geometry.x;
    *screen_y = geometry.y;
    *width = geometry.width;
    *height = geometry.height;
}

/*
 * The screen size is used to make sure the initial window doesn't get bigger
 * than the screen.  This subtracts some room for menubar, toolbar and window
 * decorations.
 */
    static void
gui_gtk_get_screen_dimensions(
	int point_x,
	int point_y,
	int *screen_w,
	int *screen_h)
{
    int	    x, y;

    gui_gtk_get_screen_geom_of_win(gui.mainwin, point_x, point_y,
						   &x, &y, screen_w, screen_h);

    // Subtract 'guiheadroom' from the height to allow some room for the
    // window manager (task list and window title bar).
    *screen_h -= p_ghr;

    /*
     * FIXME: dirty trick: Because the gui_get_base_height() doesn't include
     * the toolbar and menubar for GTK, we subtract them from the screen
     * height, so that the window size can be made to fit on the screen.
     * This should be completely changed later.
     */
    *screen_w -= get_menu_tool_width();
    *screen_h -= get_menu_tool_height();
}

    void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
    gui_gtk_get_screen_dimensions(0, 0, screen_w, screen_h);
}


/*
 * Bit of a hack to ensure we start GtkPlug windows with the correct window
 * hints (and thus the required size from -geom), but that after that we
 * put the hints back to normal (the actual minimum size) so we may
 * subsequently be resized smaller.  GtkSocket (the parent end) uses the
 * plug's window 'min hints to set *its* minimum size, but that's also the
 * only way we have of making ourselves bigger (by set lines/columns).
 * Thus set hints at start-up to ensure correct init. size, then a
 * second after the final attempt to reset the real minimum hints (done by
 * scrollbar init.), actually do the standard hints and stop the timer.
 * We'll not let the default hints be set while this timer's active.
 */
    static gboolean
check_startup_plug_hints(gpointer data UNUSED)
{
    if (init_window_hints_state == 1)
    {
	// Safe to use normal hints now
	init_window_hints_state = 0;
	gui_gtk_update_window_manager_hints(0, 0);
	return FALSE;   // stop timer
    }

    // Keep on trying
    init_window_hints_state = 1;
    return TRUE;
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

    /*
     * Allow setting a window role on the command line, or invent one
     * if none was specified.  This is mainly useful for GNOME session
     * support; allowing the WM to restore window placement.
     */
#if defined(HAVE_X11) && defined(WANT_X11)
    if (x11_role_argument != NULL)
    {
	gtk_window_set_role(GTK_WINDOW(gui.mainwin), x11_role_argument);
    }
    else
    {
	char *role;

	// Invent a unique-enough ID string for the role
	role = g_strdup_printf("vim-%u-%u-%u",
			       (unsigned)mch_get_pid(),
			       (unsigned)g_random_int(),
			       (unsigned)time(NULL));

	gtk_window_set_role(GTK_WINDOW(gui.mainwin), role);
	g_free(role);
    }

    if (gui_win_x != -1 && gui_win_y != -1)
	gtk_window_move(GTK_WINDOW(gui.mainwin), gui_win_x, gui_win_y);

    // Determine user specified geometry, if present.
    if (gui.geom != NULL)
    {
	int		mask;
	unsigned int	w, h;
	int		x = 0;
	int		y = 0;

	mask = XParseGeometry((char *)gui.geom, &x, &y, &w, &h);

	if (mask & WidthValue)
	    Columns = w;
	if (mask & HeightValue)
	{
	    if (p_window > (long)h - 1 || !option_was_set((char_u *)"window"))
		p_window = h - 1;
	    Rows = h;
	}
	limit_screen_size();

	pixel_width = (guint)(gui_get_base_width() + Columns * gui.char_width);
	pixel_height = (guint)(gui_get_base_height() + Rows * gui.char_height);

	pixel_width  += get_menu_tool_width();
	pixel_height += get_menu_tool_height();

	if (mask & (XValue | YValue))
	{
	    int ww, hh;

# ifdef FEAT_GUI_GTK
	    gui_gtk_get_screen_dimensions(x, y, &ww, &hh);
# else
	    gui_mch_get_screen_dimensions(&ww, &hh);
# endif
	    hh += p_ghr + get_menu_tool_height();
	    ww += get_menu_tool_width();
	    if (mask & XNegative)
		x += ww - pixel_width;
	    if (mask & YNegative)
		y += hh - pixel_height;
	    gtk_window_move(GTK_WINDOW(gui.mainwin), x, y);
	}
	VIM_CLEAR(gui.geom);

	// From now until everyone's stopped trying to set the window hints
	// to their correct minimum values, stop them being set as we need
	// them to remain at our required size for the parent GtkSocket to
	// give us the right initial size.
	if (gtk_socket_id != 0  &&  (mask & WidthValue || mask & HeightValue))
	{
	    gui_gtk_update_window_manager_hints(pixel_width, pixel_height);
	    init_window_hints_state = 1;
	    gui_gtk_timeout_add(1000, check_startup_plug_hints, NULL);
	}
    }
#else
    if (x11_role_argument != NULL)
	mch_msg(_("X11 window roles are not supported."));
#endif

    pixel_width = (guint)(gui_get_base_width() + Columns * gui.char_width);
    pixel_height = (guint)(gui_get_base_height() + Rows * gui.char_height);

    // For GTK2 changing the size of the form widget doesn't cause window
    // resizing.
    if (gtk_socket_id == 0)
	gtk_window_resize(GTK_WINDOW(gui.mainwin), pixel_width, pixel_height);
    gui_gtk_update_window_manager_hints(0, 0);

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

    /*
     * Notify the fixed area about the need to resize the contents of the
     * gui.formwin, which we use for random positioning of the included
     * components.
     *
     * We connect this signal deferred finally after anything is in place,
     * since this is intended to handle resizements coming from the window
     * manager upon us and should not interfere with what VIM is requesting
     * upon startup.
     */
#ifdef TRACK_RESIZE_HISTORY
    latest_resize_hist = ALLOC_CLEAR_ONE(resize_hist_T);
#endif
    g_signal_connect(G_OBJECT(gui.formwin), "configure-event",
				       G_CALLBACK(form_configure_event), NULL);
#if GTK_CHECK_VERSION(3,10,0)
    g_signal_connect(G_OBJECT(gui.formwin), "notify::scale-factor",
		     G_CALLBACK(scale_factor_event), NULL);
#endif

#ifdef FEAT_DND
    // TODO: How do support DND on GTK4?
    // Set up for receiving DND items.
    gui_gtk_set_dnd_targets();

    g_signal_connect(G_OBJECT(gui.drawarea), "drag-data-received",
				      G_CALLBACK(drag_data_received_cb), NULL);
#endif

	// With GTK+ 2, we need to iconify the window before calling show()
	// to avoid mapping the window for a short time.
	if (found_iconic_arg && gtk_socket_id == 0)
	    gui_mch_iconify();

    {
#if defined(FEAT_GUI_GNOME) && defined(FEAT_MENU)
	unsigned long menu_handler = 0;
# ifdef FEAT_TOOLBAR
	unsigned long tool_handler = 0;
# endif
	/*
	 * Urgh hackish :/  For some reason BonoboDockLayout always forces a
	 * show when restoring the saved layout configuration.	We can't just
	 * hide the widgets again after gtk_widget_show(gui.mainwin) since it's
	 * a toplevel window and thus will be realized immediately.  Instead,
	 * connect signal handlers to hide the widgets just after they've been
	 * marked visible, but before the main window is realized.
	 */
	if (using_gnome && vim_strchr(p_go, GO_MENUS) == NULL)
	    menu_handler = g_signal_connect_after(gui.menubar_h, "show",
						  G_CALLBACK(&gtk_widget_hide),
						  NULL);
# ifdef FEAT_TOOLBAR
	if (using_gnome && vim_strchr(p_go, GO_TOOLBAR) == NULL
		&& (toolbar_flags & (TOOLBAR_TEXT | TOOLBAR_ICONS)))
	    tool_handler = g_signal_connect_after(gui.toolbar_h, "show",
						  G_CALLBACK(&gtk_widget_hide),
						  NULL);
# endif
#endif
	gtk_widget_show(gui.mainwin);

#if defined(FEAT_GUI_GNOME) && defined(FEAT_MENU)
	if (menu_handler != 0)
	    g_signal_handler_disconnect(gui.menubar_h, menu_handler);
# ifdef FEAT_TOOLBAR
	if (tool_handler != 0)
	    g_signal_handler_disconnect(gui.toolbar_h, tool_handler);
# endif
#endif
    }

    /*
     * Add selection handler functions.
     */
    g_signal_connect(G_OBJECT(gui.drawarea), "selection-clear-event",
		     G_CALLBACK(selection_clear_event), NULL);
    g_signal_connect(G_OBJECT(gui.drawarea), "selection-received",
		     G_CALLBACK(selection_received_cb), NULL);

    g_signal_connect(G_OBJECT(gui.drawarea), "selection-get",
		     G_CALLBACK(selection_get_cb), NULL);

    return OK;
}

/*
 * Clean up for when exiting Vim.
 */
    void
gui_mch_exit(int rc UNUSED)
{
    // Clean up, unless we don't want to invoke free().
    if (gui.mainwin != NULL && !really_exiting)
	gtk_widget_destroy(gui.mainwin);
}

/*
 * Get the position of the top left corner of the window.
 */
    int
gui_mch_get_winpos(int *x, int *y)
{
    if (gui_mch_get_display())
    {
	gtk_window_get_position(GTK_WINDOW(gui.mainwin), x, y);
	return OK;
    }
    return FAIL;
}

/*
 * Set the position of the top left corner of the window to the given
 * coordinates.
 */
    void
gui_mch_set_winpos(int x, int y)
{
    gtk_window_move(GTK_WINDOW(gui.mainwin), x, y);
}

#if !GTK_CHECK_VERSION(3,0,0)
# if 0
static int resize_idle_installed = FALSE;
/*
 * Idle handler to force resize.  Used by gui_mch_set_shellsize() to ensure
 * the shell size doesn't exceed the window size, i.e. if the window manager
 * ignored our size request.  Usually this happens if the window is maximized.
 *
 * FIXME: It'd be nice if we could find a little more orthodox solution.
 * See also the remark below in gui_mch_set_shellsize().
 *
 * DISABLED: When doing ":set lines+=1" this function would first invoke
 * gui_resize_shell() with the old size, then the normal callback would
 * report the new size through form_configure_event().  That caused the window
 * layout to be messed up.
 */
    static gboolean
force_shell_resize_idle(gpointer data)
{
    if (gui.mainwin != NULL
	    && GTK_WIDGET_REALIZED(gui.mainwin)
	    && GTK_WIDGET_VISIBLE(gui.mainwin))
    {
	int width;
	int height;

	gtk_window_get_size(GTK_WINDOW(gui.mainwin), &width, &height);

	width  -= get_menu_tool_width();
	height -= get_menu_tool_height();

	gui_resize_shell(width, height);
    }

    resize_idle_installed = FALSE;
    return FALSE; // don't call me again
}
# endif
#endif // !GTK_CHECK_VERSION(3,0,0)

/*
 * Return TRUE if the main window is maximized.
 */
    int
gui_mch_maximized(void)
{
    if (gui.mainwin == NULL)
	return FALSE;

    GdkWindow *w = gtk_widget_get_window(gui.mainwin);
    if (w == NULL)
	return FALSE;
    return (gdk_window_get_state(w) & GDK_WINDOW_STATE_MAXIMIZED);
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
    gtk_window_get_size(GTK_WINDOW(gui.mainwin), &w, &h);
    w -= get_menu_tool_width();
    h -= get_menu_tool_height();
    gui_resize_shell(w, h);
}

/*
 * Set the windows size.
 */
    void
gui_mch_set_shellsize(int width, int height,
		      int min_width UNUSED,  int min_height UNUSED,
		      int base_width UNUSED, int base_height UNUSED,
		      int direction UNUSED)
{
    // give GTK+ a chance to put all widget's into place
    gui_mch_update();

    // this will cause the proper resizement to happen too
    if (gtk_socket_id == 0)
	gui_gtk_update_window_manager_hints(0, 0);

    // With GTK+ 2, changing the size of the form widget doesn't resize
    // the window.  So let's do it the other way around and resize the
    // main window instead.
    width  += get_menu_tool_width();
    height += get_menu_tool_height();

#ifdef TRACK_RESIZE_HISTORY
    alloc_resize_hist(width, height); // track the resize request
#endif
    if (gtk_socket_id == 0)
	gtk_window_resize(GTK_WINDOW(gui.mainwin), width, height);
    else
	gui_gtk_update_window_manager_hints(width, height);

# if !GTK_CHECK_VERSION(3,0,0)
#  if 0
    if (!resize_idle_installed)
    {
	g_idle_add_full(GDK_PRIORITY_EVENTS + 10,
			&force_shell_resize_idle, NULL, NULL);
	resize_idle_installed = TRUE;
    }
#  endif
# endif // !GTK_CHECK_VERSION(3,0,0)
    /*
     * Wait until all events are processed to prevent a crash because the
     * real size of the drawing area doesn't reflect Vim's internal ideas.
     *
     * This is a bit of a hack, since Vim is a terminal application with a GUI
     * on top, while the GUI expects to be the boss.
     */
    gui_mch_update();
}

#if defined(FEAT_MENU)
    void
gui_mch_enable_menu(int showit)
{
    GtkWidget *widget;

# ifdef FEAT_GUI_GNOME
    if (using_gnome)
	widget = gui.menubar_h;
    else
# endif
	widget = gui.menubar;

    // Do not disable the menu while starting up, otherwise F10 doesn't work.
    if (!showit != !gtk_widget_get_visible(widget) && !gui.starting)
    {
	if (showit)
	    gtk_widget_show(widget);
	else
	    gtk_widget_hide(widget);

	gui_gtk_update_window_manager_hints(0, 0);
    }
}
#endif // FEAT_MENU

#if defined(FEAT_TOOLBAR)
    void
gui_mch_show_toolbar(int showit)
{
    GtkWidget *widget;

    if (gui.toolbar == NULL)
	return;

# ifdef FEAT_GUI_GNOME
    if (using_gnome)
	widget = gui.toolbar_h;
    else
# endif
	widget = gui.toolbar;

    if (showit)
	set_toolbar_style(GTK_TOOLBAR(gui.toolbar));

    if (!showit != !gtk_widget_get_visible(widget))
    {
	if (showit)
	    gtk_widget_show(widget);
	else
	    gtk_widget_hide(widget);

	gui_gtk_update_window_manager_hints(0, 0);
    }
}
#endif // FEAT_TOOLBAR

#if GTK_CHECK_VERSION(3,0,0)
// Callback function used in gui_mch_font_dialog()
    static gboolean
font_filter(const PangoFontFamily *family,
	    const PangoFontFace   *face UNUSED,
	    gpointer		   data UNUSED)
{
    return pango_font_family_is_monospace((PangoFontFamily *)family);
}
#endif

/*
 * Put up a font dialog and return the selected font name in allocated memory.
 * "oldval" is the previous value.  Return NULL when cancelled.
 * This should probably go into gui_gtk.c.  Hmm.
 * FIXME:
 * The GTK2 font selection dialog has no filtering API.  So we could either
 * a) implement our own (possibly copying the code from somewhere else) or
 * b) just live with it.
 */
    char_u *
gui_mch_font_dialog(char_u *oldval)
{
    GtkWidget	*dialog;
    int		response;
    char_u	*fontname = NULL;
    char_u	*oldname;

#if GTK_CHECK_VERSION(3,2,0)
    dialog = gtk_font_chooser_dialog_new(NULL, NULL);
    gtk_font_chooser_set_filter_func(GTK_FONT_CHOOSER(dialog), font_filter,
	    NULL, NULL);
#else
    dialog = gtk_font_selection_dialog_new(NULL);
#endif

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gui.mainwin));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

    if (oldval != NULL && oldval[0] != NUL)
    {
	if (output_conv.vc_type != CONV_NONE)
	    oldname = string_convert(&output_conv, oldval, NULL);
	else
	    oldname = oldval;

	// Annoying bug in GTK (or Pango): if the font name does not include a
	// size, zero is used.  Use default point size ten.
	if (!vim_isdigit(oldname[STRLEN(oldname) - 1]))
	{
	    char_u	*p = vim_strnsave(oldname, STRLEN(oldname) + 3);

	    if (p != NULL)
	    {
		STRCPY(p + STRLEN(p), " 10");
		if (oldname != oldval)
		    vim_free(oldname);
		oldname = p;
	    }
	}

#if GTK_CHECK_VERSION(3,2,0)
	gtk_font_chooser_set_font(
		GTK_FONT_CHOOSER(dialog), (const gchar *)oldname);
#else
	gtk_font_selection_dialog_set_font_name(
		GTK_FONT_SELECTION_DIALOG(dialog), (const char *)oldname);
#endif

	if (oldname != oldval)
	    vim_free(oldname);
    }
    else
#if GTK_CHECK_VERSION(3,2,0)
	gtk_font_chooser_set_font(
		GTK_FONT_CHOOSER(dialog), DEFAULT_FONT);
#else
	gtk_font_selection_dialog_set_font_name(
		GTK_FONT_SELECTION_DIALOG(dialog), DEFAULT_FONT);
#endif

    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK)
    {
	char *name;

#if GTK_CHECK_VERSION(3,2,0)
	name = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
#else
	name = gtk_font_selection_dialog_get_font_name(
			    GTK_FONT_SELECTION_DIALOG(dialog));
#endif
	if (name != NULL)
	{
	    char_u  *p;

	    // Apparently some font names include a comma, need to escape
	    // that, because in 'guifont' it separates names.
	    p = vim_strsave_escaped((char_u *)name, (char_u *)",");
	    g_free(name);
	    if (p != NULL && input_conv.vc_type != CONV_NONE)
	    {
		fontname = string_convert(&input_conv, p, NULL);
		vim_free(p);
	    }
	    else
		fontname = p;
	}
    }

    if (response != GTK_RESPONSE_NONE)
	gtk_widget_destroy(dialog);

    return fontname;
}

#if defined(FEAT_EVAL) && !defined(USE_GTK4)
/*
 * Return the text window-id and display.  Only required for X-based GUI's
 */
    int
gui_get_x11_windis(Window *win, Display **dis)
{
    Display * dpy = gui_mch_get_display();
    if (dpy)
    {
	*dis = dpy;
	*win = GDK_WINDOW_XID(gtk_widget_get_window(gui.mainwin));
	return OK;
    }

    *dis = NULL;
    *win = 0;
    return FAIL;
}
#endif

    Display *
gui_mch_get_display(void)
{
    if (gui.mainwin != NULL && gtk_widget_get_window(gui.mainwin) != NULL
#if GTK_CHECK_VERSION(3,0,0)
	    && GDK_IS_X11_DISPLAY(gtk_widget_get_display(gui.mainwin))
#endif
	)
	return GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.mainwin));
    else
	return NULL;
}

    void
gui_mch_flash(int msec)
{
#if GTK_CHECK_VERSION(3,0,0)
    // TODO Replace GdkGC with Cairo
    (void)msec;
#else
    GdkGCValues	values;
    GdkGC	*invert_gc;

    if (gui.drawarea->window == NULL)
	return;

    values.foreground.pixel = gui.norm_pixel ^ gui.back_pixel;
    values.background.pixel = gui.norm_pixel ^ gui.back_pixel;
    values.function = GDK_XOR;
    invert_gc = gdk_gc_new_with_values(gui.drawarea->window,
				       &values,
				       GDK_GC_FOREGROUND |
				       GDK_GC_BACKGROUND |
				       GDK_GC_FUNCTION);
    gdk_gc_set_exposures(invert_gc,
			 gui.visibility != GDK_VISIBILITY_UNOBSCURED);
    /*
     * Do a visual beep by changing back and forth in some undetermined way,
     * the foreground and background colors.  This is due to the fact that
     * there can't be really any prediction about the effects of XOR on
     * arbitrary X11 servers. However this seems to be enough for what we
     * intend it to do.
     */
    gdk_draw_rectangle(gui.drawarea->window, invert_gc,
		       TRUE,
		       0, 0,
		       FILL_X((int)Columns) + gui.border_offset,
		       FILL_Y((int)Rows) + gui.border_offset);

    gui_mch_flush();
    ui_delay((long)msec, TRUE);	// wait so many msec

    gdk_draw_rectangle(gui.drawarea->window, invert_gc,
		       TRUE,
		       0, 0,
		       FILL_X((int)Columns) + gui.border_offset,
		       FILL_Y((int)Rows) + gui.border_offset);

    gdk_gc_destroy(invert_gc);
#endif
}

/*
 * Iconify the GUI window.
 */
    void
gui_mch_iconify(void)
{
    gtk_window_iconify(GTK_WINDOW(gui.mainwin));
}

#if defined(FEAT_EVAL)
/*
 * Bring the Vim window to the foreground.
 */
    void
gui_mch_set_foreground(void)
{
    // Just calling gtk_window_present() used to work in the past, but now this
    // sequence appears to be needed:
    // - Show the window on top of others.
    // - Present the window (also shows it above others).
    // - Do not the window on top of others (otherwise it would be stuck there).
    gtk_window_set_keep_above(GTK_WINDOW(gui.mainwin), TRUE);
    gui_may_flush();
    gtk_window_present(GTK_WINDOW(gui.mainwin));
    gui_may_flush();
    gtk_window_set_keep_above(GTK_WINDOW(gui.mainwin), FALSE);
    gui_may_flush();
}
#endif


// TODO: Start moving drawing routines to _common.c ... I think

/////////////////////////////////////////////////////////////////////////////
// Output drawing routines.
//


// Flush any output to the screen
    void
gui_mch_flush(void)
{
    if (gui.mainwin != NULL && gtk_widget_get_realized(gui.mainwin))
#if GTK_CHECK_VERSION(2,4,0)
	gdk_display_flush(gtk_widget_get_display(gui.mainwin));
#else
	gdk_display_sync(gtk_widget_get_display(gui.mainwin));
#endif
}

/*
 * Clear a rectangular region of the screen from text pos (row1, col1) to
 * (row2, col2) inclusive.
 */
    void
gui_mch_clear_block(int row1arg, int col1arg, int row2arg, int col2arg)
{

    int col1 = check_col(col1arg);
    int col2 = check_col(col2arg);
    int row1 = check_row(row1arg);
    int row2 = check_row(row2arg);

#if GTK_CHECK_VERSION(3,0,0)
    if (gtk_widget_get_realized(gui.drawarea) == FALSE)
	return;
#else
    GdkColor color;

    if (gui.drawarea->window == NULL)
	return;

    color.pixel = gui.back_pixel;
#endif

#if GTK_CHECK_VERSION(3,0,0)
    {
	// Add one pixel to the far right column in case a double-stroked
	// bold glyph may sit there.
	const GdkRectangle rect = {
	    FILL_X(col1), FILL_Y(row1),
	    (col2 - col1 + 1) * gui.char_width + (col2 == Columns - 1),
	    (row2 - row1 + 1) * gui.char_height
	};
	cairo_t * const cr = cairo_create(gui.surface);
# if GTK_CHECK_VERSION(3,22,2)
	set_cairo_source_rgba_from_color(cr, gui.back_pixel);
# else
	GdkWindow * const win = gtk_widget_get_window(gui.drawarea);
	cairo_pattern_t * const pat = gdk_window_get_background_pattern(win);
	if (pat != NULL)
	    cairo_set_source(cr, pat);
	else
	    set_cairo_source_rgba_from_color(cr, gui.back_pixel);
# endif
	cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
	cairo_fill(cr);
	cairo_destroy(cr);

	gtk_widget_queue_draw_area(gui.drawarea,
		rect.x, rect.y, rect.width, rect.height);
    }
#else // !GTK_CHECK_VERSION(3,0,0)
    gdk_gc_set_foreground(gui.text_gc, &color);

    // Clear one extra pixel at the far right, for when bold characters have
    // spilled over to the window border.
    gdk_draw_rectangle(gui.drawarea->window, gui.text_gc, TRUE,
		       FILL_X(col1), FILL_Y(row1),
		       (col2 - col1 + 1) * gui.char_width
						      + (col2 == Columns - 1),
		       (row2 - row1 + 1) * gui.char_height);
#endif // !GTK_CHECK_VERSION(3,0,0)
}


#if GTK_CHECK_VERSION(3,0,0)
    static void
gui_gtk_window_clear(GdkWindow *win)
{
    const GdkRectangle rect = {
	0, 0, gdk_window_get_width(win), gdk_window_get_height(win)
    };
    cairo_t * const cr = cairo_create(gui.surface);
# if GTK_CHECK_VERSION(3,22,2)
    set_cairo_source_rgba_from_color(cr, gui.back_pixel);
# else
    cairo_pattern_t * const pat = gdk_window_get_background_pattern(win);
    if (pat != NULL)
	cairo_set_source(cr, pat);
    else
	set_cairo_source_rgba_from_color(cr, gui.back_pixel);
# endif
    cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
    cairo_fill(cr);
    cairo_destroy(cr);

    gtk_widget_queue_draw_area(gui.drawarea,
	    rect.x, rect.y, rect.width, rect.height);
}
#else
# define gui_gtk_window_clear(win)  gdk_window_clear(win)
#endif

    void
gui_mch_clear_all(void)
{
    if (gtk_widget_get_window(gui.drawarea) != NULL)
	gui_gtk_window_clear(gtk_widget_get_window(gui.drawarea));
}

#if !GTK_CHECK_VERSION(3,0,0)
/*
 * Redraw any text revealed by scrolling up/down.
 */
    static void
check_copy_area(void)
{
    GdkEvent	*event;
    int		expose_count;

    if (gui.visibility != GDK_VISIBILITY_PARTIAL)
	return;

    // Avoid redrawing the cursor while scrolling or it'll end up where
    // we don't want it to be.	I'm not sure if it's correct to call
    // gui_dont_update_cursor() at this point but it works as a quick
    // fix for now.
    gui_dont_update_cursor(TRUE);

    do
    {
	// Wait to check whether the scroll worked or not.
	event = gdk_event_get_graphics_expose(gui.drawarea->window);

	if (event == NULL)
	    break; // received NoExpose event

	gui_redraw(event->expose.area.x, event->expose.area.y,
		   event->expose.area.width, event->expose.area.height);

	expose_count = event->expose.count;
	gdk_event_free(event);
    }
    while (expose_count > 0); // more events follow

    gui_can_update_cursor();
}
#endif // !GTK_CHECK_VERSION(3,0,0)


/*
 * Delete the given number of lines from the given row, scrolling up any
 * text further down within the scroll region.
 */
    void
gui_mch_delete_lines(int row, int num_lines)
{
#if GTK_CHECK_VERSION(3,0,0)
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
    gtk_widget_queue_draw_area(gui.drawarea,
	    FILL_X(gui.scroll_region_left), FILL_Y(row),
	    gui.char_width * ncols + 1,	gui.char_height * nrows);
#else
    if (gui.visibility == GDK_VISIBILITY_FULLY_OBSCURED)
	return;			// Can't see the window

    gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
    gdk_gc_set_background(gui.text_gc, gui.bgcolor);

    // copy one extra pixel, for when bold has spilled over
    gdk_window_copy_area(gui.drawarea->window, gui.text_gc,
	    FILL_X(gui.scroll_region_left), FILL_Y(row),
	    gui.drawarea->window,
	    FILL_X(gui.scroll_region_left),
	    FILL_Y(row + num_lines),
	    gui.char_width * (gui.scroll_region_right
					    - gui.scroll_region_left + 1) + 1,
	    gui.char_height * (gui.scroll_region_bot - row - num_lines + 1));

    gui_clear_block(gui.scroll_region_bot - num_lines + 1,
						       gui.scroll_region_left,
		    gui.scroll_region_bot, gui.scroll_region_right);
    check_copy_area();
#endif // !GTK_CHECK_VERSION(3,0,0)
}

/*
 * Insert the given number of lines before the given row, scrolling down any
 * following text within the scroll region.
 */
    void
gui_mch_insert_lines(int row, int num_lines)
{
#if GTK_CHECK_VERSION(3,0,0)
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
    gtk_widget_queue_draw_area(gui.drawarea,
	    FILL_X(gui.scroll_region_left), FILL_Y(row),
	    gui.char_width * ncols + 1,	gui.char_height * nrows);
#else
    if (gui.visibility == GDK_VISIBILITY_FULLY_OBSCURED)
	return;			// Can't see the window

    gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
    gdk_gc_set_background(gui.text_gc, gui.bgcolor);

    // copy one extra pixel, for when bold has spilled over
    gdk_window_copy_area(gui.drawarea->window, gui.text_gc,
	    FILL_X(gui.scroll_region_left), FILL_Y(row + num_lines),
	    gui.drawarea->window,
	    FILL_X(gui.scroll_region_left), FILL_Y(row),
	    gui.char_width * (gui.scroll_region_right
					    - gui.scroll_region_left + 1) + 1,
	    gui.char_height * (gui.scroll_region_bot - row - num_lines + 1));

    gui_clear_block(row, gui.scroll_region_left,
				row + num_lines - 1, gui.scroll_region_right);
    check_copy_area();
#endif // !GTK_CHECK_VERSION(3,0,0)
}

/*
 * Get the current selection and put it in the clipboard register.
 */
    void
clip_mch_request_selection(Clipboard_T *cbd)
{
    GdkAtom	target;
    unsigned	i;
    time_t	start;

    for (i = 0; i < N_SELECTION_TARGETS; ++i)
    {
	if (!clip_html && selection_targets[i].info == TARGET_HTML)
	    continue;
	received_selection = RS_NONE;
	target = gdk_atom_intern(selection_targets[i].target, FALSE);

	gtk_selection_convert(gui.drawarea,
			      cbd->gtk_sel_atom, target,
			      (guint32)GDK_CURRENT_TIME);

	// Hack: Wait up to three seconds for the selection.  A hang was
	// noticed here when using the netrw plugin combined with ":gui"
	// during the FocusGained event.
	start = time(NULL);
	while (received_selection == RS_NONE && time(NULL) < start + 3)
	    g_main_context_iteration(NULL, TRUE);	// wait for selection_received_cb

	if (received_selection != RS_FAIL)
	    return;
    }

    if (gui_mch_get_display())
	// Final fallback position - use the X CUT_BUFFER0 store
	yank_cut_buffer0(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(gui.mainwin)),
		cbd);
}

/*
 * Disown the selection.
 */
    void
clip_mch_lose_selection(Clipboard_T *cbd UNUSED)
{
    if (in_selection_clear_event)
	return;

    gtk_selection_owner_set(NULL, cbd->gtk_sel_atom, gui.event_time);
    gui_mch_update();
}

/*
 * Own the selection and return OK if it worked.
 */
    int
clip_mch_own_selection(Clipboard_T *cbd)
{
    // If we're blocking autocmds, we are filling the register to offer the
    // selection (inside selection-get)
    if (is_autocmd_blocked())
	return OK;

    int success;

    success = gtk_selection_owner_set(gui.drawarea, cbd->gtk_sel_atom,
				      gui.event_time);
    // don't update on every visual selection change
    if (!(cbd->owned && VIsual_active))
	gui_gtk_set_selection_targets(cbd->gtk_sel_atom);
    gui_mch_update();
    return (success) ? OK : FAIL;
}

/*
 * Send the current selection to the clipboard.  Do nothing for X because we
 * will fill in the selection only when requested by another app.
 */
    void
clip_mch_set_selection(Clipboard_T *cbd UNUSED)
{
}

#if defined(FEAT_XCLIPBOARD) && defined(USE_SYSTEM)
    int
clip_gtk_owner_exists(Clipboard_T *cbd)
{
    return gdk_selection_owner_get(cbd->gtk_sel_atom) != NULL;
}
#endif


#if defined(FEAT_MENU)
/*
 * Make a menu item appear either active or not active (grey or not grey).
 */
    void
gui_mch_menu_grey(vimmenu_T *menu, int grey)
{
    if (menu->id == NULL)
	return;

    if (menu_is_separator(menu->name))
	grey = TRUE;

    gui_mch_menu_hidden(menu, FALSE);
    // Be clever about bitfields versus true booleans here!
    if (!gtk_widget_get_sensitive(menu->id) == !grey)
    {
	gtk_widget_set_sensitive(menu->id, !grey);
	gui_mch_update();
    }
}

/*
 * Make menu item hidden or not hidden.
 */
    void
gui_mch_menu_hidden(vimmenu_T *menu, int hidden)
{
    if (menu->id == 0)
	return;

    if (hidden)
    {
	if (gtk_widget_get_visible(menu->id))
	{
	    gtk_widget_hide(menu->id);
	    gui_mch_update();
	}
    }
    else
    {
	if (!gtk_widget_get_visible(menu->id))
	{
	    gtk_widget_show(menu->id);
	    gui_mch_update();
	}
    }
}
#endif // FEAT_MENU

/*
 * Scrollbar stuff.
 */
    void
gui_mch_enable_scrollbar(scrollbar_T *sb, int flag)
{
    if (sb->id == NULL)
	return;

    gtk_widget_set_visible(sb->id, flag);
    gui_gtk_update_window_manager_hints(0, 0);
}


/*
 * Return the RGB value of a pixel as long.
 */
    guicolor_T
gui_mch_get_rgb(guicolor_T pixel)
{
#if GTK_CHECK_VERSION(3,0,0)
    return (long_u)pixel;
#else
    GdkColor color;

    gdk_colormap_query_color(gtk_widget_get_colormap(gui.drawarea),
			     (unsigned long)pixel, &color);

    return (guicolor_T)(
	    (((unsigned)color.red   & 0xff00) << 8)
	 |  ((unsigned)color.green & 0xff00)
	 | (((unsigned)color.blue  & 0xff00) >> 8));
#endif
}

/*
 * Get current mouse coordinates in text window.
 */
    void
gui_mch_getmouse(int *x, int *y)
{
    gui_gtk_get_pointer(gui.drawarea, x, y, NULL);
}

    void
gui_mch_setmouse(int x, int y)
{
    // Sorry for the Xlib call, but we can't avoid it, since there is no
    // internal GDK mechanism present to accomplish this.  (and for good
    // reason...)
    Display * dpy = gui_mch_get_display();
    if (dpy)
	XWarpPointer(dpy, (Window)0,
		GDK_WINDOW_XID(gtk_widget_get_window(gui.drawarea)),
		0, 0, 0U, 0U, x, y);
}


#ifdef FEAT_MOUSESHAPE
// The last set mouse pointer shape is remembered, to be used when it goes
// from hidden to not hidden.
static int last_shape = 0;
#endif

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
	    gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea), NULL);
#endif
    }
}

#if defined(FEAT_MOUSESHAPE)

# if GTK_CHECK_VERSION(3,0,0)
static const char * mshape_css_names[] =
{
    "default",                  // arrow aka GDK_LEFT_PTR
    "blank",                    // blank aka GDK_CURSOR_IS_PIXMAP
    "text",                     // beam aka GDK_XTERM
    "ns-resize",                // updown aka GDK_SB_V_DOUBLE_ARROW
    "nwse-resize",              // udsizing aka GDK_SIZING
    "ew-resize",                // leftright aka GDK_SB_H_DOUBLE_ARROW
    "ew-resize",                // lrsizing aka GDK_SIZING
    "progress",                 // busy aka GDK_WATCH
    "not-allowed",              // no aka GDK_X_CURSOR
    "crosshair",                // crosshair aka GDK_CROSSHAIR
    "pointer",                  // hand1 aka GDK_HAND1
    "pointer",                  // hand2 aka GDK_HAND2
    "default",                  // pencil aka GDK_PENCIL (no css analogue)
    "help",                     // question aka GDK_QUESTION_ARROW
    "default",                  // right-arrow aka GDK_RIGHT_PTR (no css analogue)
    "default",                  // up-arrow aka GDK_CENTER_PTR (no css analogue)
    "default"                   // GDK_LEFT_PTR (no css analogue)
};
# else
// Table for shape IDs.  Keep in sync with the mshape_names[] table in
// misc2.c!
static const int mshape_ids[] =
{
    GDK_LEFT_PTR,		// arrow
    GDK_CURSOR_IS_PIXMAP,	// blank
    GDK_XTERM,			// beam
    GDK_SB_V_DOUBLE_ARROW,	// updown
    GDK_SIZING,			// udsizing
    GDK_SB_H_DOUBLE_ARROW,	// leftright
    GDK_SIZING,			// lrsizing
    GDK_WATCH,			// busy
    GDK_X_CURSOR,		// no
    GDK_CROSSHAIR,		// crosshair
    GDK_HAND1,			// hand1
    GDK_HAND2,			// hand2
    GDK_PENCIL,			// pencil
    GDK_QUESTION_ARROW,		// question
    GDK_RIGHT_PTR,		// right-arrow
    GDK_CENTER_PTR,		// up-arrow
    GDK_LEFT_PTR		// last one
};
# endif // GTK_CHECK_VERSION(3,0,0)

    void
mch_set_mouse_shape(int shape)
{
    GdkCursor	   *c;
    int		   id;                    // Only id or css_name is used.
    GdkDisplay     *disp;
# if GTK_CHECK_VERSION(3,0,0)
    const char     *css_name = "default";
# endif

    if (gtk_widget_get_realized(gui.drawarea) == FALSE)
	return;

    if (shape == MSHAPE_HIDE || gui.pointer_hidden)
    {
	disp = gtk_widget_get_display(gui.drawarea);
	c = gdk_cursor_new_from_name(disp, "none");
	gtk_widget_set_cursor(gui.drawarea, c);
    }
    else
    {
	if (shape >= MSHAPE_NUMBERED)
	{
	    id = shape - MSHAPE_NUMBERED;
	    if (id >= GDK_LAST_CURSOR)
		id = GDK_LEFT_PTR;
	    else
		id &= ~1;	// they are always even (why?)
	}
# if GTK_CHECK_VERSION(3,0,0)
	else if (shape < (int)ARRAY_LENGTH(mshape_css_names))
	    css_name = mshape_css_names[shape];
# else
	else if (shape < (int)ARRAY_LENGTH(mshape_ids))
	    id = mshape_ids[shape];
# endif
	else
	    return;
# if GTK_CHECK_VERSION(3,0,0)
	disp = gtk_widget_get_display(gui.drawarea);
	c = gdk_cursor_new_from_name(disp, css_name);
# else
	c = gdk_cursor_new_for_display(
		gtk_widget_get_display(gui.drawarea), (GdkCursorType)id);
# endif
	gdk_window_set_cursor(gtk_widget_get_window(gui.drawarea), c);
# if GTK_CHECK_VERSION(3,0,0)
	g_object_unref(G_OBJECT(c));
# else
	gdk_cursor_destroy(c); // Unref, actually.  Bloody GTK+ 1.
# endif
    }
    if (shape != MSHAPE_HIDE)
	last_shape = shape;
}
#endif // FEAT_MOUSESHAPE

#if defined(FEAT_SIGN_ICONS)
    void
gui_gtk_draw_sign_pixbuf(GdkPixbuf *sign,
			 int row, int col,
			 int sign_width, int sign_height,
			 int width, int height,
			 int xoffset, int yoffset)
{
# if GTK_CHECK_VERSION(3,0,0)
    {
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
# else // !GTK_CHECK_VERSION(3,0,0)
    gdk_gc_set_foreground(gui.text_gc, gui.bgcolor);

    gdk_draw_rectangle(gui.drawarea->window,
	    gui.text_gc,
	    TRUE,
	    FILL_X(col),
	    FILL_Y(row),
	    sign_width,
	    sign_height);

    gdk_pixbuf_render_to_drawable_alpha(sign,
	    gui.drawarea->window,
	    MAX(0, xoffset),
	    MAX(0, yoffset),
	    FILL_X(col) - MIN(0, xoffset),
	    FILL_Y(row) - MIN(0, yoffset),
	    MIN(width,	sign_width),
	    MIN(height, sign_height),
	    GDK_PIXBUF_ALPHA_BILEVEL,
	    127,
	    GDK_RGB_DITHER_NORMAL,
	    0, 0);
# endif // !GTK_CHECK_VERSION(3,0,0)
}

#endif // FEAT_SIGN_ICONS
