/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

/* This file contains functions that are used by both the GTK2/3 and GTK4
 * implementations.
 */

#include "vim.h"
#if GTK_CHECK_VERSION(4,0,0)
# include "gui_gtk4_form.h"
#else
# include "gui_gtk3_form.h"
#endif
#include "gui_gtk_compat.h"
#ifdef USE_GRESOURCE
#include "auto/gui_gtk_gresources.h"
#endif

// forward declarations for gui_gtk4.c and gui_gtk_x11.c implementations {{{
GtkSettings* gui_gtk_get_settings();
void gui_gtk_get_pointer(GtkWidget *, gint *, gint *, GdkModifierType *);
void gui_gtk_update_window_manager_hints(int, int);
void gui_gtk_draw_sign_pixbuf(GdkPixbuf *, int, int, int, int, int, int, int, int);
// }}}

// glib timeout wrappers {{{

typedef gboolean timeout_cb_type;

/*
 * Start a timer that will invoke the specified callback.
 * Returns the ID of the timer.
 */
    guint
gui_gtk_timeout_add(int time, timeout_cb_type (*callback)(gpointer), int *flagp)
{
    return g_timeout_add((guint)time, (GSourceFunc)callback, flagp);
}

    void
gui_gtk_timeout_remove(guint timer)
{
    g_source_remove(timer);
}
// }}}

#define LOG_FUNC_ENTRY fprintf(stderr, "enter:%s\n", __func__)

/*
 * Keycodes recognized by vim.
 * NOTE: when changing this, the table in gui_x11.c probably needs the same
 * change!
 */
static struct _special_key
{
    guint key_sym;
    char_u code0;
    char_u code1;
}
const special_keys[] =
{
    {GDK_KEY_Up,		'k', 'u'},
    {GDK_KEY_Down,		'k', 'd'},
    {GDK_KEY_Left,		'k', 'l'},
    {GDK_KEY_Right,		'k', 'r'},
    {GDK_KEY_F1,		'k', '1'},
    {GDK_KEY_F2,		'k', '2'},
    {GDK_KEY_F3,		'k', '3'},
    {GDK_KEY_F4,		'k', '4'},
    {GDK_KEY_F5,		'k', '5'},
    {GDK_KEY_F6,		'k', '6'},
    {GDK_KEY_F7,		'k', '7'},
    {GDK_KEY_F8,		'k', '8'},
    {GDK_KEY_F9,		'k', '9'},
    {GDK_KEY_F10,		'k', ';'},
    {GDK_KEY_F11,		'F', '1'},
    {GDK_KEY_F12,		'F', '2'},
    {GDK_KEY_F13,		'F', '3'},
    {GDK_KEY_F14,		'F', '4'},
    {GDK_KEY_F15,		'F', '5'},
    {GDK_KEY_F16,		'F', '6'},
    {GDK_KEY_F17,		'F', '7'},
    {GDK_KEY_F18,		'F', '8'},
    {GDK_KEY_F19,		'F', '9'},
    {GDK_KEY_F20,		'F', 'A'},
    {GDK_KEY_F21,		'F', 'B'},
    {GDK_KEY_Pause,		'F', 'B'}, // Pause == F21 according to netbeans.txt
    {GDK_KEY_F22,		'F', 'C'},
    {GDK_KEY_F23,		'F', 'D'},
    {GDK_KEY_F24,		'F', 'E'},
    {GDK_KEY_F25,		'F', 'F'},
    {GDK_KEY_F26,		'F', 'G'},
    {GDK_KEY_F27,		'F', 'H'},
    {GDK_KEY_F28,		'F', 'I'},
    {GDK_KEY_F29,		'F', 'J'},
    {GDK_KEY_F30,		'F', 'K'},
    {GDK_KEY_F31,		'F', 'L'},
    {GDK_KEY_F32,		'F', 'M'},
    {GDK_KEY_F33,		'F', 'N'},
    {GDK_KEY_F34,		'F', 'O'},
    {GDK_KEY_F35,		'F', 'P'},
#ifdef SunXK_F36
    {SunXK_F36,			'F', 'Q'},
    {SunXK_F37,			'F', 'R'},
#endif
    {GDK_KEY_Help,		'%', '1'},
    {GDK_KEY_Undo,		'&', '8'},
    {GDK_KEY_BackSpace,		'k', 'b'},
    {GDK_KEY_Insert,		'k', 'I'},
    {GDK_KEY_Delete,		'k', 'D'},
    {GDK_KEY_3270_BackTab,	'k', 'B'},
    {GDK_KEY_Clear,		'k', 'C'},
    {GDK_KEY_Home,		'k', 'h'},
    {GDK_KEY_End,		'@', '7'},
    {GDK_KEY_Prior,		'k', 'P'},
    {GDK_KEY_Next,		'k', 'N'},
    {GDK_KEY_Print,		'%', '9'},
    // Keypad keys:
    {GDK_KEY_KP_Left,		'k', 'l'},
    {GDK_KEY_KP_Right,		'k', 'r'},
    {GDK_KEY_KP_Up,		'k', 'u'},
    {GDK_KEY_KP_Down,		'k', 'd'},
    {GDK_KEY_KP_Insert,	KS_EXTRA, (char_u)KE_KINS},
    {GDK_KEY_KP_Delete,	KS_EXTRA, (char_u)KE_KDEL},
    {GDK_KEY_KP_Home,		'K', '1'},
    {GDK_KEY_KP_End,		'K', '4'},
    {GDK_KEY_KP_Prior,		'K', '3'},  // page up
    {GDK_KEY_KP_Next,		'K', '5'},  // page down

    {GDK_KEY_KP_Add,		'K', '6'},
    {GDK_KEY_KP_Subtract,	'K', '7'},
    {GDK_KEY_KP_Divide,		'K', '8'},
    {GDK_KEY_KP_Multiply,	'K', '9'},
    {GDK_KEY_KP_Enter,		'K', 'A'},
    {GDK_KEY_KP_Decimal,	'K', 'B'},

    {GDK_KEY_KP_0,		'K', 'C'},
    {GDK_KEY_KP_1,		'K', 'D'},
    {GDK_KEY_KP_2,		'K', 'E'},
    {GDK_KEY_KP_3,		'K', 'F'},
    {GDK_KEY_KP_4,		'K', 'G'},
    {GDK_KEY_KP_5,		'K', 'H'},
    {GDK_KEY_KP_6,		'K', 'I'},
    {GDK_KEY_KP_7,		'K', 'J'},
    {GDK_KEY_KP_8,		'K', 'K'},
    {GDK_KEY_KP_9,		'K', 'L'},

    // End of list marker:
    {0, 0, 0}
};

// TODO: I think this can become static and drop the gui_gtk_ prefix after the
// scrolling event handlers are refactored.
    int
gui_gtk_modifiers_gdk2mouse(guint state)
{
    int modifiers = 0;

    if (state & GDK_SHIFT_MASK)
	modifiers |= MOUSE_SHIFT;
    if (state & GDK_CONTROL_MASK)
	modifiers |= MOUSE_CTRL;
#if GTK_CHECK_VERSION(4,0,0)
    if (state & GDK_ALT_MASK)
	modifiers |= MOUSE_ALT;
#else
    if (state & GDK_MOD1_MASK)
	modifiers |= MOUSE_ALT;
#endif

    return modifiers;
}

    int
gui_gtk_modifiers_gdk2vim(guint state)
{
    int modifiers = 0;

    if (state & GDK_SHIFT_MASK)
	modifiers |= MOD_MASK_SHIFT;
    if (state & GDK_CONTROL_MASK)
	modifiers |= MOD_MASK_CTRL;
#if GTK_CHECK_VERSION(4,0,0)
    if (state & GDK_ALT_MASK)
	modifiers |= MOD_MASK_ALT;
#else
    if (state & GDK_MOD1_MASK)
	modifiers |= MOD_MASK_ALT;
#endif
#if GTK_CHECK_VERSION(2,10,0)
    if (state & GDK_META_MASK)
	modifiers |= MOD_MASK_META;
    if (state & GDK_SUPER_MASK)
	modifiers |= MOD_MASK_CMD;
#else
    if (state & GDK_MOD4_MASK)
	modifiers |= MOD_MASK_CMD;
#endif

    return modifiers;
}

/*
 * Catch up with any queued events.  This may put keyboard input into the input
 * buffer, call resize call-backs, trigger timers etc.  If there is nothing in
 * the X11 event queue (& no timers pending), then we return immediately.
 */
    void
gui_mch_update(void)
{
    while (g_main_context_pending(NULL)
	    && !vim_is_input_buf_full())
	g_main_context_iteration(NULL, TRUE);
}

/*
 * Flags for command line options table below.
 */
#define ARG_FONT	1
#define ARG_GEOMETRY	2
#define ARG_REVERSE	3
#define ARG_NOREVERSE	4
#define ARG_BACKGROUND	5
#define ARG_FOREGROUND	6
#define ARG_ICONIC	7
#define ARG_ROLE	8
#define ARG_NETBEANS	9
#define ARG_XRM		10	// ignored
#define ARG_MENUFONT	11	// ignored
#define ARG_INDEX_MASK	0x00ff
#define ARG_HAS_VALUE	0x0100	// a value is expected after the argument
#define ARG_NEEDS_GUI	0x0200	// need to initialize the GUI for this
#define ARG_FOR_GTK	0x0400	// argument is handled by GTK+ or GNOME
#define ARG_COMPAT_LONG	0x0800	// accept -foo but substitute with --foo
#define ARG_KEEP	0x1000	// don't remove argument from argv[]

/*
 * This table holds all the GTK GUI command line options allowed.  This includes
 * the standard ones so that we can skip them when Vim is started without the
 * GUI (but the GUI might start up later).
 *
 * When changing this, also update doc/gui_x11.txt and the usage message!!!
 */
typedef struct
{
    const char	    *name;
    unsigned int    flags;
}
cmdline_option_T;

static const cmdline_option_T cmdline_options[] =
{
    // We handle these options ourselves
    {"-fn",		ARG_FONT|ARG_HAS_VALUE},
    {"-font",		ARG_FONT|ARG_HAS_VALUE},
    {"-geom",		ARG_GEOMETRY|ARG_HAS_VALUE},
    {"-geometry",	ARG_GEOMETRY|ARG_HAS_VALUE},
    {"-rv",		ARG_REVERSE},
    {"-reverse",	ARG_REVERSE},
    {"+rv",		ARG_NOREVERSE},
    {"+reverse",	ARG_NOREVERSE},
    {"-bg",		ARG_BACKGROUND|ARG_HAS_VALUE},
    {"-background",	ARG_BACKGROUND|ARG_HAS_VALUE},
    {"-fg",		ARG_FOREGROUND|ARG_HAS_VALUE},
    {"-foreground",	ARG_FOREGROUND|ARG_HAS_VALUE},
    {"-iconic",		ARG_ICONIC},
    {"--role",		ARG_ROLE|ARG_HAS_VALUE},
#ifdef FEAT_NETBEANS_INTG
    {"-nb",		ARG_NETBEANS},	      // non-standard value format
    {"-xrm",		ARG_XRM|ARG_HAS_VALUE},		// not implemented
    {"-mf",		ARG_MENUFONT|ARG_HAS_VALUE},	// not implemented
    {"-menufont",	ARG_MENUFONT|ARG_HAS_VALUE},	// not implemented
#endif
    // Arguments handled by GTK (and GNOME) internally.
    {"--g-fatal-warnings",	ARG_FOR_GTK},
    {"--gdk-debug",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--gdk-no-debug",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--gtk-debug",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--gtk-no-debug",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--gtk-module",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--sync",			ARG_FOR_GTK},
    {"--display",		ARG_FOR_GTK|ARG_HAS_VALUE|ARG_COMPAT_LONG},
    {"--name",			ARG_FOR_GTK|ARG_HAS_VALUE|ARG_COMPAT_LONG},
    {"--class",			ARG_FOR_GTK|ARG_HAS_VALUE|ARG_COMPAT_LONG},
    {"--screen",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--gxid-host",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--gxid-port",		ARG_FOR_GTK|ARG_HAS_VALUE},
#ifdef FEAT_GUI_GNOME
    {"--load-modules",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--sm-client-id",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--sm-config-prefix",	ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--sm-disable",		ARG_FOR_GTK},
    {"--oaf-ior-fd",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--oaf-activate-iid",	ARG_FOR_GTK|ARG_HAS_VALUE},
    {"--oaf-private",		ARG_FOR_GTK},
    {"--enable-sound",		ARG_FOR_GTK},
    {"--disable-sound",		ARG_FOR_GTK},
    {"--espeaker",		ARG_FOR_GTK|ARG_HAS_VALUE},
    {"-?",			ARG_FOR_GTK|ARG_NEEDS_GUI},
    {"--help",			ARG_FOR_GTK|ARG_NEEDS_GUI|ARG_KEEP},
    {"--usage",			ARG_FOR_GTK|ARG_NEEDS_GUI},
# if 0 // conflicts with Vim's own --version argument
    {"--version",		ARG_FOR_GTK|ARG_NEEDS_GUI},
# endif
    {"--disable-crash-dialog",	ARG_FOR_GTK},
#endif
    {NULL, 0}
};

static int    gui_argc = 0;
char	    **gui_argv = NULL;

const char *x11_role_argument = NULL;
#if defined(USE_GNOME_SESSION)
const char *gnome_restart_command = NULL;
char *gnome_abs_restart_command = NULL;
#endif
int found_iconic_arg = FALSE;


/*
 * Check if the GUI can be started.  Called before gvimrc is sourced and
 * before fork().
 * Return OK or FAIL.
 */
    int
gui_mch_early_init_check(int give_message)
{
    char_u *p, *q;

    // Guess that when $DISPLAY isn't set the GUI can't start.
    p = mch_getenv((char_u *)"DISPLAY");
    q = mch_getenv((char_u *)"WAYLAND_DISPLAY");
    if ((p == NULL || *p == NUL) && (q == NULL || *q == NUL))
    {
	gui.dying = TRUE;
	if (give_message)
	    emsg(_((char *)e_cannot_open_display));
	return FAIL;
    }
    return OK;
}

/*
 * Check if the GUI can be started.  Called before gvimrc is sourced but after
 * fork().
 * Return OK or FAIL.
 */
    int
gui_mch_init_check(void)
{
#ifdef USE_GRESOURCE
    static int res_registered = FALSE;

    if (!res_registered)
    {
	// Call this function in the GUI process; otherwise, the resources
	// won't be available.  Don't call it twice.
	res_registered = TRUE;
	gui_gtk_register_resource();
    }
#endif

#if GTK_CHECK_VERSION(3,10,0) && !GTK_CHECK_VERSION(4,0,0)
    // Vim currently assumes that Gtk means X11, so it cannot use native Gtk
    // support for other backends such as Wayland.
    //
    // Use an environment variable to enable unfinished Wayland support.
    if (getenv("GVIM_ENABLE_WAYLAND") == NULL)
	gdk_set_allowed_backends("x11");
#endif

#ifdef FEAT_GUI_GNOME
    if (gtk_socket_id == 0)
	using_gnome = 1;
#endif

    // This defaults to argv[0], but we want it to match the name of the
    // shipped gvim.desktop so that Vim's windows can be associated with this
    // file.
    g_set_prgname("gvim");

    // Don't use gtk_init() or gnome_init(), it exits on failure.
    if (!gtk_init_check(
#if !GTK_CHECK_VERSION(4,0,0)
		&gui_argc, &gui_argv
#endif
		))
    {
	gui.dying = TRUE;
	emsg(_((char *)e_cannot_open_display));
	return FAIL;
    }

    return OK;
}

/*
 * Parse the GUI related command-line arguments.  Any arguments used are
 * deleted from argv, and *argc is decremented accordingly.  This is called
 * when vim is started, whether or not the GUI has been started.
 */
    void
gui_mch_prepare(int *argc, char **argv)
{
    const cmdline_option_T  *option;
    int			    i	= 0;
    int			    len = 0;

#if defined(USE_GNOME_SESSION)
    /*
     * Determine the command used to invoke Vim, to be passed as restart
     * command to the session manager.	If argv[0] contains any directory
     * components try building an absolute path, otherwise leave it as is.
     */
    restart_command = argv[0];

    if (strchr(argv[0], G_DIR_SEPARATOR) != NULL)
    {
	char_u buf[MAXPATHL];

	if (mch_FullName((char_u *)argv[0], buf, (int)sizeof(buf), TRUE) == OK)
	{
	    gnome_abs_restart_command = (char *)vim_strsave(buf);
	    restart_command = gnome_abs_restart_command;
	}
    }
#endif

    /*
     * Move all the entries in argv which are relevant to GTK+ and GNOME
     * into gui_argv.  Freed later in gui_mch_init().
     */
    gui_argc = 0;
    gui_argv = ALLOC_MULT(char *, *argc + 1);

    g_return_if_fail(gui_argv != NULL);

    gui_argv[gui_argc++] = argv[i++];

    while (i < *argc)
    {
	// Don't waste CPU cycles on non-option arguments.
	if (argv[i][0] != '-' && argv[i][0] != '+')
	{
	    ++i;
	    continue;
	}

	// Look for argv[i] in cmdline_options[] table.
	for (option = &cmdline_options[0]; option->name != NULL; ++option)
	{
	    len = strlen(option->name);

	    if (strncmp(argv[i], option->name, len) == 0)
	    {
		if (argv[i][len] == '\0')
		    break;
		// allow --foo=bar style
		if (argv[i][len] == '=' && (option->flags & ARG_HAS_VALUE))
		    break;
#ifdef FEAT_NETBEANS_INTG
		// darn, -nb has non-standard syntax
		if (vim_strchr((char_u *)":=", argv[i][len]) != NULL
			&& (option->flags & ARG_INDEX_MASK) == ARG_NETBEANS)
		    break;
#endif
	    }
	    else if ((option->flags & ARG_COMPAT_LONG)
			&& strcmp(argv[i], option->name + 1) == 0)
	    {
		// Replace the standard X arguments "-name" and "-display"
		// with their GNU-style long option counterparts.
		argv[i] = (char *)option->name;
		break;
	    }
	}
	if (option->name == NULL) // no match
	{
	    ++i;
	    continue;
	}

	if (option->flags & ARG_FOR_GTK)
	{
	    // Move the argument into gui_argv, which
	    // will later be passed to gtk_init_check()
	    gui_argv[gui_argc++] = argv[i];
	}
	else
	{
	    char *value = NULL;

	    // Extract the option's value if there is one.
	    // Accept both "--foo bar" and "--foo=bar" style.
	    if (option->flags & ARG_HAS_VALUE)
	    {
		if (argv[i][len] == '=')
		    value = &argv[i][len + 1];
		else if (i + 1 < *argc && strcmp(argv[i + 1], "--") != 0)
		    value = argv[i + 1];
	    }

	    // Check for options handled by Vim itself
	    switch (option->flags & ARG_INDEX_MASK)
	    {
		case ARG_REVERSE:
		    found_reverse_arg = TRUE;
		    break;
		case ARG_NOREVERSE:
		    found_reverse_arg = FALSE;
		    break;
		case ARG_FONT:
		    font_argument = value;
		    break;
		case ARG_GEOMETRY:
		    if (value != NULL)
			gui.geom = vim_strsave((char_u *)value);
		    break;
		case ARG_BACKGROUND:
		    background_argument = value;
		    break;
		case ARG_FOREGROUND:
		    foreground_argument = value;
		    break;
		case ARG_ICONIC:
		    found_iconic_arg = TRUE;
		    break;
		case ARG_ROLE:
		    x11_role_argument = value; // used later in gui_mch_open()
		    break;
#ifdef FEAT_NETBEANS_INTG
		case ARG_NETBEANS:
		    gui.dofork = FALSE; // don't fork() when starting GUI
		    netbeansArg = argv[i];
		    break;
#endif
		default:
		    break;
	    }
	}

	// These arguments make gnome_program_init() print a message and exit.
	// Must start the GUI for this, otherwise ":gui" will exit later!
	// Only when the GUI can start.
	if ((option->flags & ARG_NEEDS_GUI)
				      && gui_mch_early_init_check(FALSE) == OK)
	    gui.starting = TRUE;

	if (option->flags & ARG_KEEP)
	    ++i;
	else
	{
	    // Remove the flag from the argument vector.
	    if (--*argc > i)
	    {
		int n_strip = 1;

		// Move the argument's value as well, if there is one.
		if ((option->flags & ARG_HAS_VALUE)
			&& argv[i][len] != '='
			&& strcmp(argv[i + 1], "--") != 0)
		{
		    ++n_strip;
		    --*argc;
		    if (option->flags & ARG_FOR_GTK)
			gui_argv[gui_argc++] = argv[i + 1];
		}

		if (*argc > i)
		    mch_memmove(&argv[i], &argv[i + n_strip],
						(*argc - i) * sizeof(char *));
	    }
	    argv[*argc] = NULL;
	}
    }

    gui_argv[gui_argc] = NULL;
}

// Flush any output to the screen
    void
gui_mch_flush(void)
{
    if (gui.mainwin != NULL && gtk_widget_get_realized(gui.mainwin))
	gdk_display_flush(gtk_widget_get_display(gui.mainwin));
}

// cursor blinking {{{
/*
 * This is a simple state machine:
 * BLINK_NONE	not blinking at all
 * BLINK_OFF	blinking, cursor is not shown
 * BLINK_ON	blinking, cursor is shown
 */

#define BLINK_NONE  0
#define BLINK_OFF   1
#define BLINK_ON    2

static int blink_state = BLINK_NONE;
static long_u blink_waittime = 700;
static long_u blink_ontime = 400;
static long_u blink_offtime = 250;
static guint blink_timer = 0;

    int
gui_mch_is_blinking(void)
{
    return blink_state != BLINK_NONE;
}

    int
gui_mch_is_blink_off(void)
{
    return blink_state == BLINK_OFF;
}

    void
gui_mch_set_blinking(long waittime, long on, long off)
{
    blink_waittime = waittime;
    blink_ontime = on;
    blink_offtime = off;
}

/*
 * Stop the cursor blinking.  Show the cursor if it wasn't shown.
 */
    void
gui_mch_stop_blink(int may_call_gui_update_cursor)
{
    if (blink_timer)
    {
	gui_gtk_timeout_remove(blink_timer);
	blink_timer = 0;
    }
    if (blink_state == BLINK_OFF && may_call_gui_update_cursor)
    {
	gui_update_cursor(TRUE, FALSE);
#if !GTK_CHECK_VERSION(3,0,0)
	gui_mch_flush();
#endif
    }
    blink_state = BLINK_NONE;
}

    static timeout_cb_type
blink_cb(gpointer data UNUSED)
{
    if (blink_state == BLINK_ON)
    {
	gui_undraw_cursor();
	blink_state = BLINK_OFF;
	blink_timer = gui_gtk_timeout_add(blink_offtime, blink_cb, NULL);
    }
    else
    {
	gui_update_cursor(TRUE, FALSE);
	blink_state = BLINK_ON;
	blink_timer = gui_gtk_timeout_add(blink_ontime, blink_cb, NULL);
    }
#if !GTK_CHECK_VERSION(3,0,0)
    gui_mch_flush();
#endif

    return FALSE;		// don't happen again
}

/*
 * Start the cursor blinking.  If it was already blinking, this restarts the
 * waiting time and shows the cursor.
 */
    void
gui_mch_start_blink(void)
{
    if (blink_timer)
    {
	gui_gtk_timeout_remove(blink_timer);
	blink_timer = 0;
    }
    // Only switch blinking on if none of the times is zero
    if (blink_waittime && blink_ontime && blink_offtime && gui.in_focus)
    {
	blink_timer = gui_gtk_timeout_add(blink_waittime, blink_cb, NULL);
	blink_state = BLINK_ON;
	gui_update_cursor(TRUE, FALSE);
#if !GTK_CHECK_VERSION(3,0,0)
	gui_mch_flush();
#endif
    }
}
// }}}

/*
 * Function called when window already closed.
 * We can't do much more here than to trying to preserve what had been done,
 * since the window is already inevitably going away.
 */
    void
gui_gtk_mainwin_destroy_cb(GObject *object UNUSED, gpointer data UNUSED)
{
    // Don't write messages to the GUI anymore
    full_screen = FALSE;

    gui.mainwin  = NULL;
    gui.drawarea = NULL;

    if (!exiting) // only do anything if the destroy was unexpected
    {
	vim_strncpy(IObuff,
		(char_u *)_("Vim: Main window unexpectedly destroyed\n"),
		IOSIZE - 1);
	preserve_exit();
    }
#ifdef USE_GRESOURCE
    gui_gtk_unregister_resource();
#endif
}

/*
 * Unmaximize the main window
 */
    void
gui_mch_unmaximize(void)
{
    if (gui.mainwin != NULL)
	gtk_window_unmaximize(GTK_WINDOW(gui.mainwin));
}


    void
gui_gtk_pointer_enter()
{
    if (blink_state == BLINK_NONE)
	gui_mch_start_blink();

    // make sure keyboard input goes there
    if (gtk_socket_id == 0 || !gtk_widget_has_focus(gui.drawarea))
	gtk_widget_grab_focus(gui.drawarea);
}

    void
gui_gtk_pointer_leave()
{
    if (blink_state != BLINK_NONE)
	gui_mch_stop_blink(TRUE);
}

    void
gui_gtk_focus_enter(GtkWidget *widget)
{
    g_return_if_fail(GTK_IS_WIDGET(widget));

    gui_focus_change(TRUE);

    if (blink_state == BLINK_NONE)
	gui_mch_start_blink();

    gtk_widget_grab_focus(gui.drawarea);
}

    void
gui_gtk_focus_leave()
{
    gui_focus_change(FALSE);

    if (blink_state != BLINK_NONE)
	gui_mch_stop_blink(TRUE);
}

    void
gui_mch_set_text_area_pos(int x, int y, int w, int h)
{
    gui_gtk_form_move_resize(GTK_FORM(gui.formwin), gui.drawarea, x, y, w, h);
}

// mouse and keyboard handling {{{

static guint		motion_repeat_timer  = 0;
static int		motion_repeat_offset = FALSE;
static timeout_cb_type	motion_repeat_timer_cb(gpointer);
static guint		dragging_button_state = 0;
static guint		mouse_click_timer = 0;
static int		mouse_timed_out = TRUE;

    void
gui_gtk_process_motion_notify(int x, int y, GdkModifierType state)
{
    int	    button;
    int_u   vim_modifiers;
    GtkAllocation allocation;

    // Need to add GDK_BUTTON1_MASK state when dragging a touch.
    state |= dragging_button_state;

    button = (state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK |
		       GDK_BUTTON3_MASK | GDK_BUTTON4_MASK |
		       GDK_BUTTON5_MASK))
	      ? MOUSE_DRAG : ' ';

    // If our pointer is currently hidden, then we should show it.
    gui_mch_mousehide(FALSE);

    // Just moving the rodent above the drawing area without any button
    // being pressed.
    if (button != MOUSE_DRAG)
    {
	gui_mouse_moved(x, y);
	return;
    }

    // translate modifier coding between the main engine and GTK
    vim_modifiers = gui_gtk_modifiers_gdk2mouse((guint)state);

    // inform the editor engine about the occurrence of this event
    gui_send_mouse_event(button, x, y, FALSE, vim_modifiers);

    /*
     * Auto repeat timer handling.
     */
    gtk_widget_get_allocation(gui.drawarea, &allocation);

    if (x < 0 || y < 0
	    || x >= allocation.width
	    || y >= allocation.height)
    {

	int dx;
	int dy;
	int offshoot;
	int delay = 10;

	// Calculate the maximal distance of the cursor from the drawing area.
	// (offshoot can't become negative here!).
	dx = x < 0 ? -x : x - allocation.width;
	dy = y < 0 ? -y : y - allocation.height;

	offshoot = dx > dy ? dx : dy;

	// Make a linearly decaying timer delay with a threshold of 5 at a
	// distance of 127 pixels from the main window.
	//
	// One could think endlessly about the most ergonomic variant here.
	// For example it could make sense to calculate the distance from the
	// drags start instead...
	//
	// Maybe a parabolic interpolation would suite us better here too...
	if (offshoot > 127)
	{
	    // 5 appears to be somehow near to my perceptual limits :-).
	    delay = 5;
	}
	else
	{
	    delay = (130 * (127 - offshoot)) / 127 + 5;
	}

	// shoot again
	if (!motion_repeat_timer)
	    motion_repeat_timer = gui_gtk_timeout_add(delay, motion_repeat_timer_cb,
									 NULL);
    }
}

/*
 * Timer used to recognize multiple clicks of the mouse button.
 */
    static timeout_cb_type
motion_repeat_timer_cb(gpointer data UNUSED)
{
    int		    x;
    int		    y;
    GdkModifierType state;

    gui_gtk_get_pointer(gui.drawarea, &x, &y, &state);

    if (!(state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK |
		   GDK_BUTTON3_MASK | GDK_BUTTON4_MASK |
		   GDK_BUTTON5_MASK)))
    {
	motion_repeat_timer = 0;
	return FALSE;
    }

    // If there already is a mouse click in the input buffer, wait another
    // time (otherwise we would create a backlog of clicks)
    if (vim_used_in_input_buf() > 10)
	return TRUE;

    motion_repeat_timer = 0;

    /*
     * Fake a motion event.
     * Trick: Pretend the mouse moved to the next character on every other
     * event, otherwise drag events will be discarded, because they are still
     * in the same character.
     */
    if (motion_repeat_offset)
	x += gui.char_width;

    motion_repeat_offset = !motion_repeat_offset;
    gui_gtk_process_motion_notify(x, y, state);

    // Don't happen again.  We will get reinstalled in the synthetic event
    // if needed -- thus repeating should still work.
    return FALSE;
}

/*
 * Timer used to recognize multiple clicks of the mouse button
 */
    static timeout_cb_type
mouse_click_timer_cb(gpointer data)
{
    // we don't use this information currently
    int *timed_out = (int *) data;

    *timed_out = TRUE;
    return FALSE;		// don't happen again
}

    void
gui_gtk_process_button_press(
	GtkWidget	*widget,
	guint32		 event_time,
	GdkEventType     event_type,
	GdkModifierType  event_modifiers_state,
	guint		 event_button,
	int		 x,
	int		 y)
{
    int button;
    int repeated_click = FALSE;
    int_u vim_modifiers;

    gui.event_time = event_time;

    // Make sure we have focus now we've been selected
    if (gtk_socket_id != 0 && !gtk_widget_has_focus(widget))
	gtk_widget_grab_focus(widget);

    /*
     * Don't let additional events about multiple clicks send by GTK to us
     * after the initial button press event confuse us.
     */
    if (event_type != GDK_BUTTON_PRESS)
	return;

    // Handle multiple clicks
    if (!mouse_timed_out && mouse_click_timer)
    {
	gui_gtk_timeout_remove(mouse_click_timer);
	mouse_click_timer = 0;
	repeated_click = TRUE;
    }

    mouse_timed_out = FALSE;
    mouse_click_timer = gui_gtk_timeout_add(p_mouset, mouse_click_timer_cb,
							     &mouse_timed_out);

    switch (event_button)
    {
	// Keep in sync with gui_x11.c.
	// Buttons 4-7 are handled in scroll_event()
	case 1:
		button = MOUSE_LEFT;
		// needed for touch-drag
		dragging_button_state |= GDK_BUTTON1_MASK;
		break;
	case 2: button = MOUSE_MIDDLE; break;
	case 3: button = MOUSE_RIGHT; break;
	case 8: button = MOUSE_X1; break;
	case 9: button = MOUSE_X2; break;
	default:
	    return;		// Unknown button
    }

#ifdef FEAT_XIM
    // cancel any preediting
    if (im_is_preediting())
	xim_reset();
#endif

    vim_modifiers = gui_gtk_modifiers_gdk2mouse((guint)event_modifiers_state);

    gui_send_mouse_event(button, x, y, repeated_click, vim_modifiers);
}

    void
gui_gtk_process_button_release(GtkWidget       *widget UNUSED,
			       guint32		event_time,
			       GdkEventType	event_type UNUSED, // TODO: should this be removed since it is unused?
			       GdkModifierType  event_modifiers_state,
			       guint		event_button,
			       int		x,
			       int		y)
{
    int_u vim_modifiers;

    gui.event_time = event_time;

    // Remove any motion "machine gun" timers used for automatic further
    // extension of allocation areas if outside of the applications window
    // area .
    if (motion_repeat_timer)
    {
	gui_gtk_timeout_remove(motion_repeat_timer);
	motion_repeat_timer = 0;
    }

    vim_modifiers = gui_gtk_modifiers_gdk2mouse((guint)event_modifiers_state);

    gui_send_mouse_event(MOUSE_RELEASE, x, y, FALSE, vim_modifiers);

    switch (event_button)
    {
	case 1:  // MOUSE_LEFT
	    dragging_button_state = 0;
	    break;
    }

}

    static timeout_cb_type
input_timer_cb(gpointer data)
{
    int *timed_out = (int *) data;

    // Just inform the caller about the occurrence of it
    *timed_out = TRUE;

    return FALSE;		// don't happen again
}

#ifdef FEAT_JOB_CHANNEL
    static timeout_cb_type
channel_poll_cb(gpointer data UNUSED)
{
    // Using an event handler for a channel that may be disconnected does
    // not work, it hangs.  Instead poll for messages.
    channel_handle_events(TRUE);
    parse_queued_messages();

    return TRUE;		// repeat
}
#endif

/*
 * GUI input routine called by gui_wait_for_chars().  Waits for a character
 * from the keyboard.
 *  wtime == -1     Wait forever.
 *  wtime == 0	    This should never happen.
 *  wtime > 0	    Wait wtime milliseconds for a character.
 * Returns OK if a character was found to be available within the given time,
 * or FAIL otherwise.
 */
    int
gui_mch_wait_for_chars(long wtime)
{
    int		focus;
    guint	timer;
    static int	timed_out;
    int		retval = FAIL;
#ifdef FEAT_JOB_CHANNEL
    guint	channel_timer = 0;
#endif

    timed_out = FALSE;

    // This timeout makes sure that we will return if no characters arrived in
    // time. If "wtime" is zero just use one.
    if (wtime >= 0)
	timer = gui_gtk_timeout_add(wtime == 0 ? 1L : wtime,
						   input_timer_cb, &timed_out);
    else
	timer = 0;

#ifdef FEAT_JOB_CHANNEL
    // If there is a channel with the keep_open flag we need to poll for input
    // on them.
    if (channel_any_keep_open())
	channel_timer = gui_gtk_timeout_add(20, channel_poll_cb, NULL);
#endif

    focus = gui.in_focus;

    do
    {
	// Stop or start blinking when focus changes
	if (gui.in_focus != focus)
	{
	    if (gui.in_focus)
		gui_mch_start_blink();
	    else
		gui_mch_stop_blink(TRUE);
	    focus = gui.in_focus;
	}

#ifdef MESSAGE_QUEUE
# ifdef FEAT_TIMERS
	did_add_timer = FALSE;
# endif
	parse_queued_messages();
# ifdef FEAT_TIMERS
	if (did_add_timer)
	    // Need to recompute the waiting time.
	    goto theend;
# endif
#endif

	/*
	 * Loop in GTK+ processing  until a timeout or input occurs.
	 * Skip this if input is available anyway (can happen in rare
	 * situations, sort of race condition).
	 */
	if (!input_available())
	    g_main_context_iteration(NULL, TRUE);

	// Got char, return immediately
	if (input_available())
	{
	    retval = OK;
	    goto theend;
	}
    } while (wtime < 0 || !timed_out);

    /*
     * Flush all eventually pending (drawing) events.
     */
    gui_mch_update();

theend:
    if (timer != 0 && !timed_out)
	gui_gtk_timeout_remove(timer);
#ifdef FEAT_JOB_CHANNEL
    if (channel_timer != 0)
	gui_gtk_timeout_remove(channel_timer);
#endif

    return retval;
}

/*
 * Return OK if the key with the termcap name "name" is supported.
 */
    int
gui_mch_haskey(char_u *name)
{
    int i;

    for (i = 0; special_keys[i].key_sym != 0; i++)
	if (name[0] == special_keys[i].code0
		&& name[1] == special_keys[i].code1)
	    return OK;
    return FAIL;
}

/*
 * Translate a GDK key value to UTF-8 independently of the current locale.
 * The output is written to string, which must have room for at least 6 bytes
 * plus the NUL terminator.  Returns the length in bytes.
 *
 * event->string is evil; see here why:
 * http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventKey
 */
    static int
keyval_to_string(unsigned int keyval, char_u *string)
{
    int	    len;
    guint32 uc;

    uc = gdk_keyval_to_unicode(keyval);
    if (uc != 0)
    {
	// Translate a normal key to UTF-8.  This doesn't work for dead
	// keys of course, you _have_ to use an input method for that.
	len = utf_char2bytes((int)uc, string);
    }
    else
    {
	// Translate keys which are represented by ASCII control codes in Vim.
	// There are only a few of those; most control keys are translated to
	// special terminal-like control sequences.
	len = 1;
	switch (keyval)
	{
	    case GDK_KEY_Tab: case GDK_KEY_KP_Tab: case GDK_KEY_ISO_Left_Tab:
		string[0] = TAB;
		break;
	    case GDK_KEY_Linefeed:
		string[0] = NL;
		break;
	    case GDK_KEY_Return: case GDK_KEY_ISO_Enter: case GDK_KEY_3270_Enter:
		string[0] = CAR;
		break;
	    case GDK_KEY_Escape:
		string[0] = ESC;
		break;
	    default:
		len = 0;
		break;
	}
    }
    string[len] = NUL;

    return len;
}

    gint
gui_gtk_process_key_press(
	guint key_sym,
	guint state)
{
    // For GTK+ 2 we know for sure how large the string might get.
    // (That is, up to 6 bytes + NUL + CSI escapes + safety measure.)
    char_u	string[32], string2[32];
    int		len;
    int		i;
    int		modifiers;
    int		key;
    char_u	*s, *d;
    int		ctrl_prefix_added = 0;

#ifdef SunXK_F36
    /*
     * These keys have bogus lookup strings, and trapping them here is
     * easier than trying to XRebindKeysym() on them with every possible
     * combination of modifiers.
     */
    if (key_sym == SunXK_F36 || key_sym == SunXK_F37)
	len = 0;
    else
#endif
    {
	len = keyval_to_string(key_sym, string2);

	// Careful: convert_input() doesn't handle the NUL character.
	// No need to convert pure ASCII anyway, thus the len > 1 check.
	if (len > 1 && input_conv.vc_type != CONV_NONE)
	    len = convert_input(string2, len, sizeof(string2));

	s = string2;
	d = string;
	for (i = 0; i < len; ++i)
	{
	    *d++ = s[i];
	    if (d[-1] == CSI && d + 2 < string + sizeof(string))
	    {
		// Turn CSI into K_CSI.
		*d++ = KS_EXTRA;
		*d++ = (int)KE_CSI;
	    }
	}
	len = d - string;
    }

    // Shift-Tab results in Left_Tab, but we want <S-Tab>
    if (key_sym == GDK_KEY_ISO_Left_Tab)
    {
	key_sym = GDK_KEY_Tab;
	state |= GDK_SHIFT_MASK;
    }

#ifdef FEAT_MENU
    // If there is a menu and 'wak' is "yes", or 'wak' is "menu" and the key
    // is a menu shortcut, we ignore everything with the ALT modifier.
    if (
# if GTK_CHECK_VERSION(4,0,0)
	(state & GDK_ALT_MASK)
# else
	(state & GDK_MOD1_MASK)
# endif
	    && gui.menu_is_active
	    && (*p_wak == 'y'
		|| (*p_wak == 'm'
		    && len == 1
		    && gui_is_menu_shortcut(string[0]))))
	// For GTK2 we return false to signify that we haven't handled the
	// keypress, so that gtk will handle the mnemonic or accelerator.
	return FALSE;
#endif

    // We used to apply Alt/Meta to the key here (Mod1Mask), but that is now
    // done later, the same as it happens for the terminal.  Hopefully that
    // works for everybody...

    // Check for special keys.	Also do this when len == 1 (key has an ASCII
    // value) to detect backspace, delete and keypad keys.
    if (len == 0 || len == 1)
    {
	for (i = 0; special_keys[i].key_sym != 0; i++)
	{
	    if (special_keys[i].key_sym == key_sym)
	    {
		string[0] = CSI;
		string[1] = special_keys[i].code0;
		string[2] = special_keys[i].code1;
		len = -3;
		break;
	    }
	}
    }

#ifdef GDK_KEY_dead_circumflex
    // Belgian Ctrl+[ workaround
    if (len == 0 && key_sym == GDK_KEY_dead_circumflex)
    {
	string[0] = CSI;
	string[1] = KS_MODIFIER;
	string[2] = MOD_MASK_CTRL;
	string[3] = '[';
	len = 4;
	add_to_input_buf(string, len);
	// workaround has to return here, otherwise our fake string[] entries
	// are confusing code downstream
	return TRUE;
    }
#endif

    if (len == 0)   // Unrecognized key
	return TRUE;

    // For some keys a shift modifier is translated into another key code.
    if (len == -3)
	key = TO_SPECIAL(string[1], string[2]);
    else
    {
	string[len] = NUL;
	key = mb_ptr2char(string);
    }

    // Handle modifiers.
    modifiers = gui_gtk_modifiers_gdk2vim(state);

    // Recognize special keys.
    key = simplify_key(key, &modifiers);
    if (key == CSI)
	key = K_CSI;
    if (IS_SPECIAL(key))
    {
	string[0] = CSI;
	string[1] = K_SECOND(key);
	string[2] = K_THIRD(key);
	len = 3;
    }
    else
    {
	// Some keys need adjustment when the Ctrl modifier is used.
	key = may_adjust_key_for_ctrl(modifiers, key);

	// May remove the Shift modifier if it's included in the key.
	modifiers = may_remove_shift_modifier(modifiers, key);

	len = mb_char2bytes(key, string);
    }

    if (modifiers != 0)
    {
	string2[0] = CSI;
	string2[1] = KS_MODIFIER;
	string2[2] = modifiers;
	add_to_input_buf(string2, 3);
	if (modifiers == 0x4)
	    ctrl_prefix_added = 1;
    }

    // Check if the key interrupts.
    {
	int int_ch = check_for_interrupt(key, modifiers);

	if (int_ch != NUL)
	{
	    trash_input_buf();
	    string[0] = int_ch;
	    len = 1;
	}
    }

    // workaround for German keyboard, where instead of '[' char we have code
    // sequence of bytes 195, 188 (UTF-8 for "u-umlaut")
    if (ctrl_prefix_added && len == 2
		    && ((int)string[0]) == 195
		    && ((int)string[1]) == 188)
    {
	string[0] = 91; // ASCII('[')
	len = 1;
    }
    add_to_input_buf(string, len);

    // blank out the pointer if necessary
    if (p_mh)
	gui_mch_mousehide(TRUE);

    return TRUE;
}

// }}}

// text rendering {{{

    void
gui_gtk_rebuild_text_context()
{
    // Create a new PangoContext for this screen, and initialize it
    // with the current font if necessary.
    if (gui.text_context != NULL)
	g_object_unref(gui.text_context);

    gui.text_context = gtk_widget_create_pango_context(gui.mainwin);
    pango_context_set_base_dir(gui.text_context, PANGO_DIRECTION_LTR);
}

/*
 * Adjust gui.char_height (after 'linespace' was changed).
 */
    int
gui_mch_adjust_charheight(void)
{
    PangoFontMetrics	*metrics;
    int			ascent;
    int			descent;

    metrics = pango_context_get_metrics(gui.text_context, gui.norm_font,
				pango_context_get_language(gui.text_context));
    ascent  = pango_font_metrics_get_ascent(metrics);
    descent = pango_font_metrics_get_descent(metrics);

    pango_font_metrics_unref(metrics);

    // Round up when the value is more than about 1/16 of a pixel above a whole
    // pixel (12.0624 becomes 12, 12.07 becomes 13).  Then add 'linespace'.
    gui.char_height = (ascent + descent + (PANGO_SCALE * 15) / 16)
						   / PANGO_SCALE + p_linespace;
    // LINTED: avoid warning: bitwise operation on signed value
    gui.char_ascent = PANGO_PIXELS(ascent + p_linespace * PANGO_SCALE / 2);

    // A not-positive value of char_height may crash Vim.  Only happens
    // if 'linespace' is negative (which does make sense sometimes).
    gui.char_ascent = MAX(gui.char_ascent, 0);
    gui.char_height = MAX(gui.char_height, gui.char_ascent + 1);

    return OK;
}

/*
 * Some monospace fonts don't support a bold weight, and fall back
 * silently to the regular weight.  But this is no good since our text
 * drawing function can emulate bold by overstriking.  So let's try
 * to detect whether bold weight is actually available and emulate it
 * otherwise.
 *
 * Note that we don't need to check for italic style since Xft can
 * emulate italic on its own, provided you have a proper fontconfig
 * setup.  We wouldn't be able to emulate it in Vim anyway.
 */
    static void
get_styled_font_variants(void)
{
    PangoFontDescription    *bold_font_desc;
    PangoFont		    *plain_font;
    PangoFont		    *bold_font;

    gui.font_can_bold = FALSE;

    plain_font = pango_context_load_font(gui.text_context, gui.norm_font);

    if (plain_font == NULL)
	return;

    bold_font_desc = pango_font_description_copy_static(gui.norm_font);
    pango_font_description_set_weight(bold_font_desc, PANGO_WEIGHT_BOLD);

    bold_font = pango_context_load_font(gui.text_context, bold_font_desc);
    /*
     * The comparison relies on the unique handle nature of a PangoFont*,
     * i.e. it's assumed that a different PangoFont* won't refer to the
     * same font.  Seems to work, and failing here isn't critical anyway.
     */
    if (bold_font != NULL)
    {
	gui.font_can_bold = (bold_font != plain_font);
	g_object_unref(bold_font);
    }

    pango_font_description_free(bold_font_desc);
    if (bold_font != NULL && gui.font_can_bold)
	g_object_unref(plain_font);
}

static PangoEngineShape *default_shape_engine = NULL;

/*
 * Create a map from ASCII characters in the range [32,126] to glyphs
 * of the current font.  This is used by gui_gtk_draw_string() to skip
 * the itemize and shaping process for the most common case.
 */
    static void
ascii_glyph_table_init(void)
{
    char_u	    ascii_chars[2 * 128];
    PangoAttrList   *attr_list;
    GList	    *item_list;
    int		    i;

    if (gui.ascii_glyphs != NULL)
	pango_glyph_string_free(gui.ascii_glyphs);
    if (gui.ascii_font != NULL)
	g_object_unref(gui.ascii_font);

    gui.ascii_glyphs = NULL;
    gui.ascii_font   = NULL;

    // For safety, fill in question marks for the control characters.
    // Put a space between characters to avoid shaping.
    for (i = 0; i < 128; ++i)
    {
	if (i >= 32 && i < 127)
	    ascii_chars[2 * i] = i;
	else
	    ascii_chars[2 * i] = '?';
	ascii_chars[2 * i + 1] = ' ';
    }

    attr_list = pango_attr_list_new();
    item_list = pango_itemize(gui.text_context, (const char *)ascii_chars,
			      0, sizeof(ascii_chars), attr_list, NULL);

    if (item_list != NULL && item_list->next == NULL) // play safe
    {
	PangoItem   *item;
	int	    width;

	item  = (PangoItem *)item_list->data;
	width = gui.char_width * PANGO_SCALE;

	// Remember the shape engine used for ASCII.
	default_shape_engine = item->analysis.shape_engine;

	gui.ascii_font = item->analysis.font;
	g_object_ref(gui.ascii_font);

	gui.ascii_glyphs = pango_glyph_string_new();

	pango_shape((const char *)ascii_chars, sizeof(ascii_chars),
		    &item->analysis, gui.ascii_glyphs);

	g_return_if_fail(gui.ascii_glyphs->num_glyphs == sizeof(ascii_chars));

	for (i = 0; i < gui.ascii_glyphs->num_glyphs; ++i)
	{
	    PangoGlyphGeometry *geom;

	    geom = &gui.ascii_glyphs->glyphs[i].geometry;
	    geom->x_offset += MAX(0, width - geom->width) / 2;
	    geom->width = width;
	}
    }

    // TODO: is this type cast OK?
    g_list_foreach(item_list, (GFunc)(void *)&pango_item_free, NULL);
    g_list_free(item_list);
    pango_attr_list_unref(attr_list);
}

/*
 * Check if a given font is a CJK font. This is done in a very crude manner. It
 * just see if U+04E00 for zh and ja and U+AC00 for ko are covered in a given
 * font. Consequently, this function cannot  be used as a general purpose check
 * for CJK-ness for which fontconfig APIs should be used.  This is only used by
 * gui_mch_init_font() to deal with 'CJK fixed width fonts'.
 */
    static int
is_cjk_font(PangoFontDescription *font_desc)
{
    static const char * const cjk_langs[] =
	{"zh_CN", "zh_TW", "zh_HK", "ja", "ko"};

    PangoFont	*font;
    unsigned	i;
    int		is_cjk = FALSE;

    font = pango_context_load_font(gui.text_context, font_desc);

    if (font == NULL)
	return FALSE;

    for (i = 0; !is_cjk && i < G_N_ELEMENTS(cjk_langs); ++i)
    {
	PangoCoverage	*coverage;
	gunichar	uc;

	// Valgrind reports a leak for pango_language_from_string(), but the
	// documentation says "This is owned by Pango and should not be freed".
	coverage = pango_font_get_coverage(
		font, pango_language_from_string(cjk_langs[i]));

	if (coverage != NULL)
	{
	    uc = (cjk_langs[i][0] == 'k') ? 0xAC00 : 0x4E00;
	    is_cjk = (pango_coverage_get(coverage, uc) == PANGO_COVERAGE_EXACT);
#if PANGO_VERSION_CHECK(1, 52, 0)
	    g_object_unref(coverage);
#else
	    pango_coverage_unref(coverage);
#endif
	}
    }

    g_object_unref(font);

    return is_cjk;
}

/*
 * "Monospace" is a standard font alias that should be present
 * on all proper Pango/fontconfig installations.
 */
# define DEFAULT_FONT	"Monospace 10"

/*
 * Initialize Vim to use the font or fontset with the given name.
 * Return FAIL if the font could not be loaded, OK otherwise.
 */
    int
gui_mch_init_font(char_u *font_name, int fontset UNUSED)
{
    PangoFontDescription    *font_desc;
    PangoLayout		    *layout;
    int			    width;

    // If font_name is NULL, this means to use the default, which should
    // be present on all proper Pango/fontconfig installations.
    if (font_name == NULL)
	font_name = (char_u *)DEFAULT_FONT;

    font_desc = gui_mch_get_font(font_name, FALSE);

    if (font_desc == NULL)
	return FAIL;

    gui_mch_free_font(gui.norm_font);
    gui.norm_font = font_desc;

    pango_context_set_font_description(gui.text_context, font_desc);

    layout = pango_layout_new(gui.text_context);
    pango_layout_set_text(layout, "MW", 2);
    pango_layout_get_size(layout, &width, NULL);
    /*
     * Set char_width to half the width obtained from pango_layout_get_size()
     * for CJK fixed_width/bi-width fonts.  An unpatched version of Xft leads
     * Pango to use the same width for both non-CJK characters (e.g. Latin
     * letters and numbers) and CJK characters.  This results in 's p a c e d
     * o u t' rendering when a CJK 'fixed width' font is used. To work around
     * that, divide the width returned by Pango by 2 if cjk_width is equal to
     * width for CJK fonts.
     *
     * For related bugs, see:
     * http://bugzilla.gnome.org/show_bug.cgi?id=106618
     * http://bugzilla.gnome.org/show_bug.cgi?id=106624
     *
     * With this, for all four of the following cases, Vim works fine:
     *	   guifont=CJK_fixed_width_font
     *	   guifont=Non_CJK_fixed_font
     *	   guifont=Non_CJK_fixed_font,CJK_Fixed_font
     *	   guifont=Non_CJK_fixed_font guifontwide=CJK_fixed_font
     */
    if (is_cjk_font(gui.norm_font))
    {
	int cjk_width;

	// Measure the text extent of U+4E00 and U+4E8C
	pango_layout_set_text(layout, "\344\270\200\344\272\214", -1);
	pango_layout_get_size(layout, &cjk_width, NULL);

	if (width == cjk_width)  // Xft not patched
	    width /= 2;
    }
    g_object_unref(layout);

    gui.char_width = (width / 2 + PANGO_SCALE - 1) / PANGO_SCALE;

    // A zero width may cause a crash.	Happens for semi-invalid fontsets.
    if (gui.char_width <= 0)
	gui.char_width = 8;

    gui_mch_adjust_charheight();

    // Set the fontname, which will be used for information purposes
    hl_set_font_name(font_name);

    get_styled_font_variants();
    ascii_glyph_table_init();

    // Avoid unnecessary overhead if 'guifontwide' is equal to 'guifont'.
    if (gui.wide_font != NULL
	&& pango_font_description_equal(gui.norm_font, gui.wide_font))
    {
	pango_font_description_free(gui.wide_font);
	gui.wide_font = NULL;
    }

    if (gui_mch_maximized())
    {
	// Update lines and columns in accordance with the new font, keep the
	// window maximized.
	gui_mch_newfont();
    }
    else
    {
# if !GTK_CHECK_VERSION(4,0,0)
	// Preserve the logical dimensions of the screen.
	gui_gtk_update_window_manager_hints(0, 0);
# endif
    }

    return OK;
}

/*
 * Get a reference to the font "name".
 * Return zero for failure.
 */
    GuiFont
gui_mch_get_font(char_u *name, int report_error)
{
    PangoFontDescription    *font;

    // can't do this when GUI is not running
    if (!gui.in_use || name == NULL)
	return NULL;

    if (output_conv.vc_type != CONV_NONE)
    {
	char_u *buf;

	buf = string_convert(&output_conv, name, NULL);
	if (buf != NULL)
	{
	    font = pango_font_description_from_string((const char *)buf);
	    vim_free(buf);
	}
	else
	    font = NULL;
    }
    else
	font = pango_font_description_from_string((const char *)name);

    if (font != NULL)
    {
	PangoFont *real_font;

	// pango_context_load_font() bails out if no font size is set
	if (pango_font_description_get_size(font) <= 0)
	    pango_font_description_set_size(font, 10 * PANGO_SCALE);

	real_font = pango_context_load_font(gui.text_context, font);

	if (real_font == NULL)
	{
	    pango_font_description_free(font);
	    font = NULL;
	}
	else
	    g_object_unref(real_font);
    }

    if (font == NULL)
    {
	if (report_error)
	    semsg(_((char *)e_unknown_font_str), name);
	return NULL;
    }

    return font;
}

#if defined(FEAT_EVAL) || defined(PROTO)
/*
 * Return the name of font "font" in allocated memory.
 */
    char_u *
gui_mch_get_fontname(GuiFont font, char_u *name UNUSED)
{
    if (font != NOFONT)
    {
	char	*pangoname = pango_font_description_to_string(font);

	if (pangoname != NULL)
	{
	    char_u	*s = vim_strsave((char_u *)pangoname);

	    g_free(pangoname);
	    return s;
	}
    }
    return NULL;
}
#endif

/*
 * If a font is not going to be used, free its structure.
 */
    void
gui_mch_free_font(GuiFont font)
{
    if (font != NOFONT)
	pango_font_description_free(font);
}

/*
 * Cmdline expansion for setting 'guifont' / 'guifontwide'. Will enumerate
 * through all fonts for completion. When setting 'guifont' it will only show
 * monospace fonts as it's unlikely other fonts would be useful.
 */
    void
gui_mch_expand_font(optexpand_T *args, void *param, int (*add_match)(char_u *val))
{
    PangoFontFamily	**font_families = NULL;
    int			n_families = 0;
    int			wide = *(int *)param;

    if (args->oe_include_orig_val && *args->oe_opt_value == NUL && !wide)
    {
	// If guifont is empty, and we want to fill in the orig value, suggest
	// the default so the user can modify it.
	if (add_match((char_u *)DEFAULT_FONT) != OK)
	    return;
    }

    pango_context_list_families(
	    gui.text_context,
	    &font_families,
	    &n_families);

    for (int i = 0; i < n_families; i++)
    {
	if (!wide && !pango_font_family_is_monospace(font_families[i]))
	    continue;

	const char* fam_name = pango_font_family_get_name(font_families[i]);
	if (input_conv.vc_type != CONV_NONE)
	{
	    char_u *buf = string_convert(&input_conv, (char_u*)fam_name, NULL);
	    if (buf != NULL)
	    {
		if (add_match(buf) != OK)
		{
		    vim_free(buf);
		    break;
		}
		vim_free(buf);
	    }
	    else
		break;
	}
	else
	{
	    if (add_match((char_u *)fam_name) != OK)
		break;
	}
    }

    g_free(font_families);
}

/*
 * Return the Pixel value (color) for the given color name.
 *
 * Return INVALCOLOR for error.
 */
    guicolor_T
gui_mch_get_color(char_u *name)
{
    guicolor_T color = INVALCOLOR;

    if (!gui.in_use)		// can't do this when GUI not running
	return color;

    if (name != NULL)
	color = gui_get_color_cmn(name);

#if GTK_CHECK_VERSION(3,0,0)
    return color;
#else
    if (color == INVALCOLOR)
	return INVALCOLOR;

    return gui_mch_get_rgb_color(
	    (color & 0xff0000) >> 16,
	    (color & 0x00ff00) >> 8,
	     color & 0x0000ff);
#endif
}

#if GTK_CHECK_VERSION(3,0,0)
    GdkRGBA
color_to_rgba(guicolor_T color)
{
    GdkRGBA rgba;
    rgba.red   = ((color & 0xff0000) >> 16) / 255.0;
    rgba.green = ((color & 0x00ff00) >> 8) / 255.0;
    rgba.blue  = ((color & 0x0000ff)) / 255.0;
    rgba.alpha = 1.0;
    return rgba;
}
#endif

/*
 * Return the Pixel value (color) for the given RGB values.
 * Return INVALCOLOR for error.
 */
    guicolor_T
gui_mch_get_rgb_color(int r, int g, int b)
{
#if GTK_CHECK_VERSION(3,0,0)
    return gui_get_rgb_color_cmn(r, g, b);
#else
    GdkColor gcolor;
    int ret;

    gcolor.red = (guint16)(r / 255.0 * 65535 + 0.5);
    gcolor.green = (guint16)(g / 255.0 * 65535 + 0.5);
    gcolor.blue = (guint16)(b / 255.0 * 65535 + 0.5);

    ret = gdk_colormap_alloc_color(gtk_widget_get_colormap(gui.drawarea),
	    &gcolor, FALSE, TRUE);

    return ret != 0 ? (guicolor_T)gcolor.pixel : INVALCOLOR;
#endif
}

/*
 * Set the current text foreground color.
 */
    void
gui_mch_set_fg_color(guicolor_T color)
{
#if GTK_CHECK_VERSION(3,0,0)
    *gui.fgcolor = color_to_rgba(color);
#else
    gui.fgcolor->pixel = (unsigned long)color;
#endif
}

/*
 * Set the current text background color.
 */
    void
gui_mch_set_bg_color(guicolor_T color)
{
#if GTK_CHECK_VERSION(3,0,0)
    *gui.bgcolor = color_to_rgba(color);
#else
    gui.bgcolor->pixel = (unsigned long)color;
#endif
}

/*
 * Set the current text special color.
 */
    void
gui_mch_set_sp_color(guicolor_T color)
{
#if GTK_CHECK_VERSION(3,0,0)
    *gui.spcolor = color_to_rgba(color);
#else
    gui.spcolor->pixel = (unsigned long)color;
#endif
}

/*
 * Function-like convenience macro for the sake of efficiency.
 */
#define INSERT_PANGO_ATTR(Attribute, AttrList, Start, End)  \
    G_STMT_START{					    \
	PangoAttribute *tmp_attr_;			    \
	tmp_attr_ = (Attribute);			    \
	tmp_attr_->start_index = (Start);		    \
	tmp_attr_->end_index = (End);			    \
	pango_attr_list_insert((AttrList), tmp_attr_);	    \
    }G_STMT_END

    static void
apply_wide_font_attr(char_u *s, int len, PangoAttrList *attr_list)
{
    char_u  *start = NULL;
    char_u  *p;
    int	    uc;

    for (p = s; p < s + len; p += utf_byte2len(*p))
    {
	uc = utf_ptr2char(p);

	if (start == NULL)
	{
	    if (uc >= 0x80 && utf_char2cells(uc) == 2)
		start = p;
	}
	else if (uc < 0x80 // optimization shortcut
		 || (utf_char2cells(uc) != 2 && !utf_iscomposing(uc)))
	{
	    INSERT_PANGO_ATTR(pango_attr_font_desc_new(gui.wide_font),
			      attr_list, start - s, p - s);
	    start = NULL;
	}
    }

    if (start != NULL)
	INSERT_PANGO_ATTR(pango_attr_font_desc_new(gui.wide_font),
			  attr_list, start - s, len);
}

    static int
count_cluster_cells(char_u *s, PangoItem *item,
		    PangoGlyphString* glyphs, int i,
		    int *cluster_width,
		    int *last_glyph_rbearing)
{
    char_u  *p;
    int	    next;	// glyph start index of next cluster
    int	    start, end; // string segment of current cluster
    int	    width;	// real cluster width in Pango units
    int	    uc;
    int	    cellcount = 0;

    width = glyphs->glyphs[i].geometry.width;

    for (next = i + 1; next < glyphs->num_glyphs; ++next)
    {
	if (glyphs->glyphs[next].attr.is_cluster_start)
	    break;
	else if (glyphs->glyphs[next].geometry.width > width)
	    width = glyphs->glyphs[next].geometry.width;
    }

    start = item->offset + glyphs->log_clusters[i];
    end   = item->offset + ((next < glyphs->num_glyphs) ?
			    glyphs->log_clusters[next] : item->length);

    for (p = s + start; p < s + end; p += utf_byte2len(*p))
    {
	uc = utf_ptr2char(p);
	if (uc < 0x80)
	    ++cellcount;
	else if (!utf_iscomposing(uc))
	    cellcount += utf_char2cells(uc);
    }

    if (last_glyph_rbearing != NULL
	    && cellcount > 0 && next == glyphs->num_glyphs)
    {
	PangoRectangle ink_rect;
	/*
	 * If a certain combining mark had to be taken from a non-monospace
	 * font, we have to compensate manually by adapting x_offset according
	 * to the ink extents of the previous glyph.
	 */
	pango_font_get_glyph_extents(item->analysis.font,
				     glyphs->glyphs[i].glyph,
				     &ink_rect, NULL);

	if (PANGO_RBEARING(ink_rect) > 0)
	    *last_glyph_rbearing = PANGO_RBEARING(ink_rect);
    }

    if (cellcount > 0)
	*cluster_width = width;

    return cellcount;
}

/*
 * If there are only combining characters in the cluster, we cannot just
 * change the width of the previous glyph since there is none.	Therefore
 * some guesswork is needed.
 *
 * If ink_rect.x is negative Pango apparently has taken care of the composing
 * by itself.  Actually setting x_offset = 0 should be sufficient then, but due
 * to problems with composing from different fonts we still need to fine-tune
 * x_offset to avoid ugliness.
 *
 * If ink_rect.x is not negative, force overstriking by pointing x_offset to
 * the position of the previous glyph.	Apparently this happens only with old
 * X fonts which don't provide the special combining information needed by
 * Pango.
 */
    static void
setup_zero_width_cluster(PangoItem *item, PangoGlyphInfo *glyph,
			 int last_cellcount, int last_cluster_width,
			 int last_glyph_rbearing)
{
    PangoRectangle  ink_rect;
    PangoRectangle  logical_rect;
    int		    width;

    width = last_cellcount * gui.char_width * PANGO_SCALE;
    glyph->geometry.x_offset = -width + MAX(0, width - last_cluster_width) / 2;
    glyph->geometry.width = 0;

    pango_font_get_glyph_extents(item->analysis.font,
				 glyph->glyph,
				 &ink_rect, &logical_rect);
    if (ink_rect.x < 0)
    {
	glyph->geometry.x_offset += last_glyph_rbearing;
	glyph->geometry.y_offset  = logical_rect.height
		- (gui.char_height - p_linespace) * PANGO_SCALE;
    }
    else
	// If the accent width is smaller than the cluster width, position it
	// in the middle.
	glyph->geometry.x_offset = -width + MAX(0, width - ink_rect.width) / 2;
}

#if GTK_CHECK_VERSION(3,0,0)
    static void
draw_glyph_string(int row, int col, int num_cells, int flags,
		  PangoFont *font, PangoGlyphString *glyphs,
		  cairo_t *cr)
#else
    static void
draw_glyph_string(int row, int col, int num_cells, int flags,
		  PangoFont *font, PangoGlyphString *glyphs)
#endif
{
    if (!(flags & DRAW_TRANSP))
    {
#if GTK_CHECK_VERSION(3,0,0)
	cairo_set_source_rgba(cr,
		gui.bgcolor->red, gui.bgcolor->green, gui.bgcolor->blue,
		gui.bgcolor->alpha);
	cairo_rectangle(cr,
			FILL_X(col), FILL_Y(row),
			num_cells * gui.char_width, gui.char_height);
	cairo_fill(cr);
#else
	gdk_gc_set_foreground(gui.text_gc, gui.bgcolor);

	gdk_draw_rectangle(gui.drawarea->window,
			   gui.text_gc,
			   TRUE,
			   FILL_X(col),
			   FILL_Y(row),
			   num_cells * gui.char_width,
			   gui.char_height);
#endif
    }

#if GTK_CHECK_VERSION(3,0,0)
    cairo_set_source_rgba(cr,
	    gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
	    gui.fgcolor->alpha);
    cairo_move_to(cr, TEXT_X(col), TEXT_Y(row));
    pango_cairo_show_glyph_string(cr, font, glyphs);
#else
    gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);

    gdk_draw_glyphs(gui.drawarea->window,
		    gui.text_gc,
		    font,
		    TEXT_X(col),
		    TEXT_Y(row),
		    glyphs);
#endif

    // redraw the contents with an offset of 1 to emulate bold
    if ((flags & DRAW_BOLD) && !gui.font_can_bold)
#if GTK_CHECK_VERSION(3,0,0)
    {
	cairo_set_source_rgba(cr,
		gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
		gui.fgcolor->alpha);
	cairo_move_to(cr, TEXT_X(col) + 1, TEXT_Y(row));
	pango_cairo_show_glyph_string(cr, font, glyphs);
    }
#else
	gdk_draw_glyphs(gui.drawarea->window,
			gui.text_gc,
			font,
			TEXT_X(col) + 1,
			TEXT_Y(row),
			glyphs);
#endif
}

/*
 * Draw underline and undercurl at the bottom of the character cell.
 */
#if GTK_CHECK_VERSION(3,0,0)
    static void
draw_under(int flags, int row, int col, int cells, cairo_t *cr)
#else
    static void
draw_under(int flags, int row, int col, int cells)
#endif
{
    int			i;
    int			offset;
    static const int	val[8] = {1, 0, 0, 0, 1, 2, 2, 2 };
    int			y = FILL_Y(row + 1) - 1;

    // Undercurl: draw curl at the bottom of the character cell.
    if (flags & DRAW_UNDERC)
    {
#if GTK_CHECK_VERSION(3,0,0)
	cairo_set_line_width(cr, 1.0);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_source_rgba(cr,
		gui.spcolor->red, gui.spcolor->green, gui.spcolor->blue,
		gui.spcolor->alpha);
	cairo_move_to(cr, FILL_X(col) + 1, y - 2 + 0.5);
	for (i = FILL_X(col) + 1; i < FILL_X(col + cells); ++i)
	{
	    offset = val[i % 8];
	    cairo_line_to(cr, i, y - offset + 0.5);
	}
	cairo_stroke(cr);
#else
	gdk_gc_set_foreground(gui.text_gc, gui.spcolor);
	for (i = FILL_X(col); i < FILL_X(col + cells); ++i)
	{
	    offset = val[i % 8];
	    gdk_draw_point(gui.drawarea->window, gui.text_gc, i, y - offset);
	}
	gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
#endif
    }

    // Draw a strikethrough line
    if (flags & DRAW_STRIKE)
    {
#if GTK_CHECK_VERSION(3,0,0)
	cairo_set_line_width(cr, 1.0);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_source_rgba(cr,
		gui.spcolor->red, gui.spcolor->green, gui.spcolor->blue,
		gui.spcolor->alpha);
	cairo_move_to(cr, FILL_X(col), y + 1 - gui.char_height/2 + 0.5);
	cairo_line_to(cr, FILL_X(col + cells), y + 1 - gui.char_height/2 + 0.5);
	cairo_stroke(cr);
#else
	gdk_gc_set_foreground(gui.text_gc, gui.spcolor);
	gdk_draw_line(gui.drawarea->window, gui.text_gc,
		      FILL_X(col), y + 1 - gui.char_height/2,
		      FILL_X(col + cells), y + 1 - gui.char_height/2);
	gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
#endif
    }

    // Underline: draw a line at the bottom of the character cell.
    if (flags & DRAW_UNDERL)
    {
	// When p_linespace is 0, overwrite the bottom row of pixels.
	// Otherwise put the line just below the character.
	if (p_linespace > 1)
	    y -= p_linespace - 1;
#if GTK_CHECK_VERSION(3,0,0)
	cairo_set_line_width(cr, 1.0);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_source_rgba(cr,
		gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
		gui.fgcolor->alpha);
	cairo_move_to(cr, FILL_X(col), y + 0.5);
	cairo_line_to(cr, FILL_X(col + cells), y + 0.5);
	cairo_stroke(cr);
#else
	gdk_draw_line(gui.drawarea->window, gui.text_gc,
		      FILL_X(col), y,
		      FILL_X(col + cells) - 1, y);
#endif
    }
}

    int
gui_gtk_draw_string_ext(
	int	row,
	int	col,
	char_u	*s,
	int	len,
	int	flags,
	int	force_pango)
{
    GdkRectangle	area;		    // area for clip mask
    PangoGlyphString	*glyphs;	    // glyphs of current item
    int			column_offset = 0;  // column offset in cells
    int			i;
#if GTK_CHECK_VERSION(3,0,0)
    cairo_t		*cr;
#endif

    /*
     * Restrict all drawing to the current screen line in order to prevent
     * fuzzy font lookups from messing up the screen.
     */
    area.x = gui.border_offset;
    area.y = FILL_Y(row);
    area.width	= gui.num_cols * gui.char_width;
    area.height = gui.char_height;

#if GTK_CHECK_VERSION(3,0,0)
    cr = cairo_create(gui.surface);
    cairo_rectangle(cr, area.x, area.y, area.width, area.height);
    cairo_clip(cr);
#else
    gdk_gc_set_clip_origin(gui.text_gc, 0, 0);
    gdk_gc_set_clip_rectangle(gui.text_gc, &area);
#endif

    glyphs = pango_glyph_string_new();

    /*
     * Optimization hack:  If possible, skip the itemize and shaping process
     * for pure ASCII strings.	This optimization is particularly effective
     * because Vim draws space characters to clear parts of the screen.
     */
    if (!(flags & DRAW_ITALIC)
	    && !((flags & DRAW_BOLD) && gui.font_can_bold)
	    && gui.ascii_glyphs != NULL
	    && !force_pango)
    {
	char_u *p;

	for (p = s; p < s + len; ++p)
	    if (*p & 0x80)
		goto not_ascii;

	pango_glyph_string_set_size(glyphs, len);

	for (i = 0; i < len; ++i)
	{
	    glyphs->glyphs[i] = gui.ascii_glyphs->glyphs[2 * s[i]];
	    glyphs->log_clusters[i] = i;
	}

#if GTK_CHECK_VERSION(3,0,0)
	draw_glyph_string(row, col, len, flags, gui.ascii_font, glyphs, cr);
#else
	draw_glyph_string(row, col, len, flags, gui.ascii_font, glyphs);
#endif

	column_offset = len;
    }
    else
not_ascii:
    {
	PangoAttrList	*attr_list;
	GList		*item_list;
	int		cluster_width;
	int		last_glyph_rbearing;
	int		cells = 0;  // cells occupied by current cluster

	// Safety check: pango crashes when invoked with invalid utf-8
	// characters.
	if (!utf_valid_string(s, s + len))
	{
	    column_offset = len;
	    goto skipitall;
	}

	// original width of the current cluster
	cluster_width = PANGO_SCALE * gui.char_width;

	// right bearing of the last non-composing glyph
	last_glyph_rbearing = PANGO_SCALE * gui.char_width;

	attr_list = pango_attr_list_new();

	// If 'guifontwide' is set then use that for double-width characters.
	// Otherwise just go with 'guifont' and let Pango do its thing.
	if (gui.wide_font != NULL)
	    apply_wide_font_attr(s, len, attr_list);

	if ((flags & DRAW_BOLD) && gui.font_can_bold)
	    INSERT_PANGO_ATTR(pango_attr_weight_new(PANGO_WEIGHT_BOLD),
			      attr_list, 0, len);
	if (flags & DRAW_ITALIC)
	    INSERT_PANGO_ATTR(pango_attr_style_new(PANGO_STYLE_ITALIC),
			      attr_list, 0, len);
	/*
	 * Break the text into segments with consistent directional level
	 * and shaping engine.	Pure Latin text needs only a single segment,
	 * so there's no need to worry about the loop's efficiency.  Better
	 * try to optimize elsewhere, e.g. reducing exposes and stuff :)
	 */
	item_list = pango_itemize(gui.text_context,
				  (const char *)s, 0, len, attr_list, NULL);

	while (item_list != NULL)
	{
	    PangoItem	*item;
	    int		item_cells = 0; // item length in cells

	    item = (PangoItem *)item_list->data;
	    item_list = g_list_delete_link(item_list, item_list);
	    /*
	     * Increment the bidirectional embedding level by 1 if it is not
	     * even.  An odd number means the output will be RTL, but we don't
	     * want that since Vim handles right-to-left text on its own.  It
	     * would probably be sufficient to just set level = 0, but you can
	     * never know :)
	     *
	     * Unfortunately we can't take advantage of Pango's ability to
	     * render both LTR and RTL at the same time.  In order to support
	     * that, Vim's main screen engine would have to make use of Pango
	     * functionality.
	     */
	    item->analysis.level = (item->analysis.level + 1) & (~1U);

	    // HACK: Overrule the shape engine, we don't want shaping to be
	    // done, because drawing the cursor would change the display.
	    item->analysis.shape_engine = default_shape_engine;

#ifdef HAVE_PANGO_SHAPE_FULL
	    pango_shape_full((const char *)s + item->offset, item->length,
		    (const char *)s, len, &item->analysis, glyphs);
#else
	    pango_shape((const char *)s + item->offset, item->length,
			&item->analysis, glyphs);
#endif
	    /*
	     * Fixed-width hack: iterate over the array and assign a fixed
	     * width to each glyph, thus overriding the choice made by the
	     * shaping engine.	We use utf_char2cells() to determine the
	     * number of cells needed.
	     *
	     * Also perform all kind of dark magic to get composing
	     * characters right (and pretty too of course).
	     */
	    for (i = 0; i < glyphs->num_glyphs; ++i)
	    {
		PangoGlyphInfo *glyph;

		glyph = &glyphs->glyphs[i];

		if (glyph->attr.is_cluster_start)
		{
		    int cellcount;

		    cellcount = count_cluster_cells(
			s, item, glyphs, i, &cluster_width,
			(item_list != NULL) ? &last_glyph_rbearing : NULL);

		    if (cellcount > 0)
		    {
			int width;

			width = cellcount * gui.char_width * PANGO_SCALE;
			glyph->geometry.x_offset +=
					    MAX(0, width - cluster_width) / 2;
			glyph->geometry.width = width;
		    }
		    else
		    {
			// If there are only combining characters in the
			// cluster, we cannot just change the width of the
			// previous glyph since there is none.	Therefore
			// some guesswork is needed.
			setup_zero_width_cluster(item, glyph, cells,
						 cluster_width,
						 last_glyph_rbearing);
		    }

		    item_cells += cellcount;
		    cells = cellcount;
		}
		else if (i > 0)
		{
		    int width;

		    // There is a previous glyph, so we deal with combining
		    // characters the canonical way.
		    // In some circumstances Pango uses a positive x_offset,
		    // then use the width of the previous glyph for this one
		    // and set the previous width to zero.
		    // Otherwise we get a negative x_offset, Pango has already
		    // positioned the combining char, keep the widths as they
		    // are.
		    // For both adjust the x_offset to position the glyph in
		    // the middle.
		    if (glyph->geometry.x_offset >= 0)
		    {
			glyphs->glyphs[i].geometry.width =
					 glyphs->glyphs[i - 1].geometry.width;
			glyphs->glyphs[i - 1].geometry.width = 0;
		    }
		    width = cells * gui.char_width * PANGO_SCALE;
		    glyph->geometry.x_offset +=
					    MAX(0, width - cluster_width) / 2;
		}
		else // i == 0 "cannot happen"
		{
		    glyph->geometry.width = 0;
		}
	    }

	    //// Aaaaand action! **
#if GTK_CHECK_VERSION(3,0,0)
	    draw_glyph_string(row, col + column_offset, item_cells,
			      flags, item->analysis.font, glyphs,
			      cr);
#else
	    draw_glyph_string(row, col + column_offset, item_cells,
			      flags, item->analysis.font, glyphs);
#endif

	    pango_item_free(item);

	    column_offset += item_cells;
	}

	pango_attr_list_unref(attr_list);
    }

skipitall:
    // Draw underline and undercurl.
#if GTK_CHECK_VERSION(3,0,0)
    draw_under(flags, row, col, column_offset, cr);
#else
    draw_under(flags, row, col, column_offset);
#endif

    pango_glyph_string_free(glyphs);

#if GTK_CHECK_VERSION(4,0,0)
    gtk_widget_queue_draw(gui.drawarea);
#elif GTK_CHECK_VERSION(3,0,0)
    cairo_destroy(cr);
    gtk_widget_queue_draw_area(gui.drawarea, area.x, area.y,
	    area.width, area.height);
#else
    gdk_gc_set_clip_rectangle(gui.text_gc, NULL);
#endif

    return column_offset;
}

    int
gui_gtk_draw_string(int row, int col, char_u *s, int len, int flags)
{
    char_u	*conv_buf = NULL;   // result of UTF-8 conversion
    char_u	*new_conv_buf;
    int		convlen;
    char_u	*sp, *bp;
    int		plen;
    int		len_sum;	// return value needs to add up since we are
				// printing substrings
    int		byte_sum;	// byte position in string
    char_u	*cs;		// current *s pointer
    int		needs_pango;	// look ahead, 0=ascii 1=unicode/ligatures
    int		should_need_pango = FALSE;
    int		slen;
    int		is_ligature;
    int		is_utf8;
    char_u	backup_ch;

    if (gui.text_context == NULL || !gtk_widget_get_realized(gui.drawarea))
	return len;

    if (output_conv.vc_type != CONV_NONE)
    {
	/*
	 * Convert characters from 'encoding' to 'termencoding', which is set
	 * to UTF-8 by gui_mch_init().	did_set_string_option() in option.c
	 * prohibits changing this to something else than UTF-8 if the GUI is
	 * in use.
	 */
	convlen = len;
	conv_buf = string_convert(&output_conv, s, &convlen);
	g_return_val_if_fail(conv_buf != NULL, len);

	// Correct for differences in char width: some chars are
	// double-wide in 'encoding' but single-wide in utf-8.  Add a space to
	// compensate for that.
	for (sp = s, bp = conv_buf; sp < s + len && bp < conv_buf + convlen; )
	{
	    plen = utf_ptr2len(bp);
	    if ((*mb_ptr2cells)(sp) == 2 && utf_ptr2cells(bp) == 1)
	    {
		new_conv_buf = alloc(convlen + 2);
		if (new_conv_buf == NULL)
		    return len;
		plen += bp - conv_buf;
		mch_memmove(new_conv_buf, conv_buf, plen);
		new_conv_buf[plen] = ' ';
		mch_memmove(new_conv_buf + plen + 1, conv_buf + plen,
							  convlen - plen + 1);
		vim_free(conv_buf);
		conv_buf = new_conv_buf;
		++convlen;
		bp = conv_buf + plen;
		plen = 1;
	    }
	    sp += (*mb_ptr2len)(sp);
	    bp += plen;
	}
	s = conv_buf;
	len = convlen;
    }

    /*
     * Ligature support and complex utf-8 char optimization:
     * String received to output to screen can print using pre-cached glyphs
     * (fast) or Pango (slow). Ligatures and multibype utf-8 must use Pango.
     * Since we receive mixed content string, split it into logical segments
     * that are guaranteed to go through glyphs as much as possible. Since
     * single ligature char prints as ascii, print it that way.
     */
    len_sum = 0;    // return value needs to add up since we are printing
		    // substrings
    byte_sum = 0;
    cs = s;
    // First char decides starting needs_pango mode, 0=ascii 1=utf8/ligatures.
    // Even if it is ligature char, two chars or more make ligature.
    // Ascii followed by utf8 is also going through pango.
    is_utf8 = (*cs & 0x80);
    is_ligature = gui.ligatures_map[*cs] && (len > 1);
    if (is_ligature)
	is_ligature = gui.ligatures_map[*(cs + 1)];
    if (!is_utf8 && len > 1)
	is_utf8 = (*(cs + 1) & 0x80) != 0;
    needs_pango = is_utf8 || is_ligature;

    // split string into ascii and non-ascii (ligatures + utf-8) substrings,
    // print glyphs or use Pango
    while (cs < s + len)
    {
	slen = 0;
	while (slen < (len - byte_sum))
	{
	    is_ligature = gui.ligatures_map[*(cs + slen)];
	    // look ahead, single ligature char between ascii is ascii
	    if (is_ligature && !needs_pango)
	    {
		if ((slen + 1) < (len - byte_sum))
		    is_ligature = gui.ligatures_map[*(cs + slen + 1)];
		else
		    is_ligature = 0;
	    }
	    is_utf8 = *(cs + slen) & 0x80;
	    // ascii followed by utf8 could be combining
	    // if so send it through pango
	    if ((!is_utf8) && ((slen + 1) < (len - byte_sum)))
		is_utf8 = (*(cs + slen + 1) & 0x80);
	    should_need_pango = (is_ligature || is_utf8);
	    if (needs_pango != should_need_pango) // mode switch
		break;
	    if (needs_pango)
	    {
		if (is_ligature)
		{
		    slen++; // ligature char by char
		}
		else // is_utf8
		{
		    if ((*(cs + slen) & 0xC0) == 0x80)
		    {
			// a continuation, find next 0xC0 != 0x80 but don't
			// include it
			while ((slen < (len - byte_sum))
					    && ((*(cs + slen) & 0xC0) == 0x80))
			{
			    slen++;
			}
		    }
		    else if ((*(cs + slen) & 0xE0) == 0xC0)
		    {
			// + one byte utf8
			slen++;
		    }
		    else if ((*(cs + slen) & 0xF0) == 0xE0)
		    {
			// + two bytes utf8
			slen += 2;
		    }
		    else if ((*(cs + slen) & 0xF8) == 0xF0)
		    {
			// + three bytes utf8
			slen += 3;
		    }
		    else
		    {
			// this should not happen, try moving forward, Pango
			// will catch it
			slen++;
		    }
		}
	    }
	    else
	    {
		slen++; // ascii
	    }
	}

	if (slen < len)
	{
	    // temporarily zero terminate substring, print, restore char, wrap
	    backup_ch = *(cs + slen);
	    *(cs + slen) = NUL;
	}
	len_sum += gui_gtk_draw_string_ext(row, col + len_sum, cs, slen, flags,
					   needs_pango);
	if (slen < len)
	    *(cs + slen) = backup_ch;
	cs += slen;
	byte_sum += slen;
	needs_pango = should_need_pango;
    }

    gtk_widget_queue_draw(gui.drawarea);
    gtk_widget_queue_draw(gui.formwin);
    vim_free(conv_buf);
    return len_sum;
}

    void
gui_mch_beep(void)
{
    GdkDisplay *display;

    if (gui.mainwin != NULL && gtk_widget_get_realized(gui.mainwin))
	display = gtk_widget_get_display(gui.mainwin);
    else
	display = gdk_display_get_default();

    if (display != NULL)
	gdk_display_beep(display);
}

/*
 * Invert a rectangle from row r, column c, for nr rows and nc columns.
 */
    void
gui_mch_invert_rectangle(int r, int c, int nr, int nc)
{
#if GTK_CHECK_VERSION(3,0,0)
    const GdkRectangle rect = {
	FILL_X(c), FILL_Y(r), nc * gui.char_width, nr * gui.char_height
    };
    cairo_t * const cr = cairo_create(gui.surface);

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
# if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1,9,2)
    cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
# else
    // Give an implementation for older cairo versions if necessary.
# endif
    cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
    cairo_fill(cr);

    cairo_destroy(cr);

# if GTK_CHECK_VERSION(4,0,0)
    gtk_widget_queue_draw(gui.drawarea);
# else
    gtk_widget_queue_draw_area(gui.drawarea, rect.x, rect.y,
	    rect.width, rect.height);
# endif
#else
    GdkGCValues values;
    GdkGC *invert_gc;

    if (gui.drawarea->window == NULL)
	return;

    values.function = GDK_INVERT;
    invert_gc = gdk_gc_new_with_values(gui.drawarea->window,
				       &values,
				       GDK_GC_FUNCTION);
    gdk_gc_set_exposures(invert_gc, gui.visibility !=
						   GDK_VISIBILITY_UNOBSCURED);
    gdk_draw_rectangle(gui.drawarea->window, invert_gc,
		       TRUE,
		       FILL_X(c), FILL_Y(r),
		       (nc) * gui.char_width, (nr) * gui.char_height);
    gdk_gc_destroy(invert_gc);
#endif
}

/*
 * Draw a cursor without focus.
 */
    void
gui_mch_draw_hollow_cursor(guicolor_T color)
{
    int		i = 1;
#if GTK_CHECK_VERSION(3,0,0)
    cairo_t    *cr;
#endif

    if (gtk_widget_get_realized(gui.drawarea) == FALSE)
	return;

#if GTK_CHECK_VERSION(3,0,0)
    cr = cairo_create(gui.surface);
#endif

    gui_mch_set_fg_color(color);

#if GTK_CHECK_VERSION(3,0,0)
    cairo_set_source_rgba(cr,
	    gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
	    gui.fgcolor->alpha);
#else
    gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
#endif
    if (mb_lefthalve(gui.row, gui.col))
	i = 2;
#if GTK_CHECK_VERSION(3,0,0)
    cairo_set_line_width(cr, 1.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_rectangle(cr,
	    FILL_X(gui.col) + 0.5, FILL_Y(gui.row) + 0.5,
	    i * gui.char_width - 1, gui.char_height - 1);
    cairo_stroke(cr);
    cairo_destroy(cr);
#else
    gdk_draw_rectangle(gui.drawarea->window, gui.text_gc,
	    FALSE,
	    FILL_X(gui.col), FILL_Y(gui.row),
	    i * gui.char_width - 1, gui.char_height - 1);
#endif
}

/*
 * Draw part of a cursor, "w" pixels wide, and "h" pixels high, using
 * color "color".
 */
    void
gui_mch_draw_part_cursor(int w, int h, guicolor_T color)
{
    if (gtk_widget_get_realized(gui.drawarea) == FALSE)
	return;

    gui_mch_set_fg_color(color);

#if GTK_CHECK_VERSION(3,0,0)
    {
	cairo_t *cr;

	cr = cairo_create(gui.surface);
	cairo_set_source_rgba(cr,
		gui.fgcolor->red, gui.fgcolor->green, gui.fgcolor->blue,
		gui.fgcolor->alpha);
	cairo_rectangle(cr,
# ifdef FEAT_RIGHTLEFT
	    // vertical line should be on the right of current point
	    CURSOR_BAR_RIGHT ? FILL_X(gui.col + 1) - w :
# endif
	    FILL_X(gui.col), FILL_Y(gui.row) + gui.char_height - h,
	    w, h);
	cairo_fill(cr);
	cairo_destroy(cr);
    }
#else // !GTK_CHECK_VERSION(3,0,0)
    gdk_gc_set_foreground(gui.text_gc, gui.fgcolor);
    gdk_draw_rectangle(gui.drawarea->window, gui.text_gc,
	    TRUE,
# ifdef FEAT_RIGHTLEFT
	    // vertical line should be on the right of current point
	    CURSOR_BAR_RIGHT ? FILL_X(gui.col + 1) - w :
# endif
	    FILL_X(gui.col),
	    FILL_Y(gui.row) + gui.char_height - h,
	    w, h);
#endif // !GTK_CHECK_VERSION(3,0,0)
}

#if GTK_CHECK_VERSION(3,0,0)
    void
gui_gtk_surface_copy_rect(int dest_x, int dest_y,
			  int src_x,  int src_y,
			  int width,  int height)
{
    cairo_t * const cr = cairo_create(gui.surface);

    cairo_rectangle(cr, dest_x, dest_y, width, height);
    cairo_clip(cr);
    cairo_push_group(cr);
    cairo_set_source_surface(cr, gui.surface, dest_x - src_x, dest_y - src_y);
    cairo_paint(cr);
    cairo_pop_group_to_source(cr);
    cairo_paint(cr);

    cairo_destroy(cr);
}
#endif

/*
 * Handle changes to the "Xft/DPI" setting
 */
    void
gui_gtk_settings_xft_dpi_changed_cb(GtkSettings *gtk_settings UNUSED,
				    GParamSpec *pspec UNUSED,
				    gpointer data UNUSED)
{
    gui_gtk_rebuild_text_context();

    if (gui.norm_font != NULL)
    {
	// force default font
	gui_mch_init_font(*p_guifont == NUL ? NULL : p_guifont, FALSE);
	gui_set_shellsize(TRUE, FALSE, RESIZE_BOTH);
    }
}

#if defined(FEAT_SIGN_ICONS) || defined(PROTO)
/*
 * Signs are currently always 2 chars wide.  With GTK+ 2, the image will be
 * scaled down if the current font is not big enough, or scaled up if the image
 * size is less than 3/4 of the maximum sign size.
 */
# define SIGN_WIDTH  (2 * gui.char_width)
# define SIGN_HEIGHT (gui.char_height)
# define SIGN_ASPECT ((double)SIGN_HEIGHT / (double)SIGN_WIDTH)

    void
gui_mch_drawsign(int row, int col, int typenr)
{
    GdkPixbuf *sign;

    sign = (GdkPixbuf *)sign_get_image(typenr);

    if ((sign == NULL) || (gui.drawarea == NULL)
	    || (gtk_widget_get_realized(gui.drawarea) == TRUE))
	return;

    int width;
    int height;
    int xoffset;
    int yoffset;
    int need_scale;

    width  = gdk_pixbuf_get_width(sign);
    height = gdk_pixbuf_get_height(sign);
    /*
     * Decide whether we need to scale.  Allow one pixel of border
     * width to be cut off, in order to avoid excessive scaling for
     * tiny differences in font size.
     * Do scale to fit the height to avoid gaps because of linespacing.
     */
    need_scale = (width > SIGN_WIDTH + 2
	    || height != SIGN_HEIGHT
	    || (width < 3 * SIGN_WIDTH / 4
		&& height < 3 * SIGN_HEIGHT / 4));
    if (need_scale)
    {
	double  aspect;
	int	    w = width;
	int	    h = height;

	// Keep the original aspect ratio
	aspect = (double)height / (double)width;
	width  = (double)SIGN_WIDTH * SIGN_ASPECT / aspect;
	width  = MIN(width, SIGN_WIDTH);
	if (((double)(MAX(height, SIGN_HEIGHT)) /
		    (double)(MIN(height, SIGN_HEIGHT))) < 1.15)
	{
	    // Change the aspect ratio by at most 15% to fill the
	    // available space completely.
	    height = (double)SIGN_HEIGHT * SIGN_ASPECT / aspect;
	    height = MIN(height, SIGN_HEIGHT);
	}
	else
	    height = (double)width * aspect;

	if (w == width && h == height)
	{
	    // no change in dimensions; don't decrease reference counter
	    // (below)
	    need_scale = FALSE;
	}
	else
	{
	    // This doesn't seem to be worth caching, and doing so would
	    // complicate the code quite a bit.
	    sign = gdk_pixbuf_scale_simple(sign, width, height,
		    GDK_INTERP_BILINEAR);
	    if (sign == NULL)
		return; // out of memory
	}
    }

    // The origin is the upper-left corner of the pixmap.  Therefore
    // these offset may become negative if the pixmap is smaller than
    // the 2x1 cells reserved for the sign icon.
    xoffset = (width  - SIGN_WIDTH)  / 2;
    yoffset = (height - SIGN_HEIGHT) / 2;

    gui_gtk_draw_sign_pixbuf(sign, row, col, SIGN_WIDTH, SIGN_HEIGHT, width, height, xoffset, yoffset);
#if GTK_CHECK_VERSION(4,0,0)
    gtk_widget_queue_draw(gui.drawarea);
#endif

    if (need_scale)
	g_object_unref(sign);
}

    void *
gui_mch_register_sign(char_u *signfile)
{
    if (signfile[0] != NUL && signfile[0] != '-' && gui.in_use)
    {
	GdkPixbuf   *sign;
	GError	    *error = NULL;
	char_u	    *message;

	sign = gdk_pixbuf_new_from_file((const char *)signfile, &error);

	if (error == NULL)
	    return sign;

	message = (char_u *)error->message;

	if (message != NULL && input_conv.vc_type != CONV_NONE)
	    message = string_convert(&input_conv, message, NULL);

	if (message != NULL)
	{
	    // The error message is already translated and will be more
	    // descriptive than anything we could possibly do ourselves.
	    semsg("E255: %s", message);

	    if (input_conv.vc_type != CONV_NONE)
		vim_free(message);
	}
	g_error_free(error);
    }

    return NULL;
}
    void
gui_mch_destroy_sign(void *sign)
{
    if (sign != NULL)
	g_object_unref(sign);
}
#endif

// }}}

#if GTK_CHECK_VERSION(3,0,0)
    gboolean
gui_gtk_draw_gui_surface_to_widget_surface(cairo_t *cr)
{
    // Skip this when the GUI isn't set up yet, will redraw later.
    if (gui.starting)
	return FALSE;

    out_flush();		// make sure all output has been processed
				// for GTK+ 3, may induce other draw events.

    cairo_set_source_surface(cr, gui.surface, 0, 0);

    {
	cairo_rectangle_list_t *list = NULL;

	list = cairo_copy_clip_rectangle_list(cr);
	if (list->status != CAIRO_STATUS_CLIP_NOT_REPRESENTABLE)
	{
	    int i;

	    for (i = 0; i < list->num_rectangles; i++)
	    {
		const cairo_rectangle_t *rect = &list->rectangles[i];
		cairo_rectangle(cr, rect->x, rect->y,
						    rect->width, rect->height);
		cairo_fill(cr);
	    }
	}
	else
	{
	    fprintf(stderr, "error:%s:CAIRO_STATUS_CLIP_NOT_REPRESENTABLE\n", __func__);
	}
	cairo_rectangle_list_destroy(list);
    }

    return FALSE;
}
#endif

#ifdef FEAT_GUI_TABLINE
static gboolean ignore_tabline_evt = FALSE;

    gboolean
gui_gtk_should_ignore_tabline_event()
{
    return ignore_tabline_evt;
}

    void
gui_gtk_ignore_tabline_event(gboolean newsetting)
{
    ignore_tabline_evt = newsetting;
}


/*
 * Show or hide the tabline.
 */
    void
gui_mch_show_tabline(int showit)
{
    if (gui.tabline == NULL)
	return;

    if (!showit != !gtk_notebook_get_show_tabs(GTK_NOTEBOOK(gui.tabline)))
    {
	// Note: this may cause a resize event
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(gui.tabline), showit);
# if !GTK_CHECK_VERSION(4,0,0)
	gui_gtk_update_window_manager_hints(0, 0);
# endif
	if (showit)
	    gtk_widget_set_can_focus(GTK_WIDGET(gui.tabline), FALSE);
    }

    gui_mch_update();
}

/*
 * Set the current tab to "nr".  First tab is 1.
 */
    void
gui_mch_set_curtab(int nr)
{
    if (gui.tabline == NULL)
	return;

    ignore_tabline_evt = TRUE;
    if (gtk_notebook_get_current_page(GTK_NOTEBOOK(gui.tabline)) != nr - 1)
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gui.tabline), nr - 1);
    ignore_tabline_evt = FALSE;
}

/*
 * Return TRUE when tabline is displayed.
 */
    int
gui_mch_showing_tabline(void)
{
    return gui.tabline != NULL
		     && gtk_notebook_get_show_tabs(GTK_NOTEBOOK(gui.tabline));
}
#endif

    void
gui_mch_settitle(char_u *title, char_u *icon UNUSED)
{
    if (title != NULL && output_conv.vc_type != CONV_NONE)
       title = string_convert(&output_conv, title, NULL);

    gtk_window_set_title(GTK_WINDOW(gui.mainwin), (const char *)title);

    if (output_conv.vc_type != CONV_NONE)
       vim_free(title);
}

#if defined(FEAT_GUI_DARKTHEME) || defined(PROTO)
    void
gui_mch_set_dark_theme(int dark)
{
# if GTK_CHECK_VERSION(3,0,0)
    GtkSettings *gtk_settings = gui_gtk_get_settings();
    g_object_set(gtk_settings, "gtk-application-prefer-dark-theme", (gboolean)dark, NULL);
# endif
}
#endif // FEAT_GUI_DARKTHEME


// FEAT_TOOLBAR {{{
#ifdef FEAT_TOOLBAR

/*
 * Table from BuiltIn## icon indices to GTK+ stock IDs.  Order must exactly
 * match toolbar_names[] in menu.c!  All stock icons including the "vim-*"
 * ones can be overridden in your gtkrc file.
 */
# if GTK_CHECK_VERSION(3,10,0)
static const char * const menu_themed_names[] =
{
    /* 00 */ "document-new",		// sub. GTK_STOCK_NEW
    /* 01 */ "document-open",		// sub. GTK_STOCK_OPEN
    /* 02 */ "document-save",		// sub. GTK_STOCK_SAVE
    /* 03 */ "edit-undo",		// sub. GTK_STOCK_UNDO
    /* 04 */ "edit-redo",		// sub. GTK_STOCK_REDO
    /* 05 */ "edit-cut",		// sub. GTK_STOCK_CUT
    /* 06 */ "edit-copy",		// sub. GTK_STOCK_COPY
    /* 07 */ "edit-paste",		// sub. GTK_STOCK_PASTE
    /* 08 */ "document-print",		// sub. GTK_STOCK_PRINT
    /* 09 */ "help-browser",		// sub. GTK_STOCK_HELP
    /* 10 */ "edit-find",		// sub. GTK_STOCK_FIND
#  if GTK_CHECK_VERSION(3,14,0)
    // Use the file names in gui_gtk_res.xml, cutting off the extension.
    // Similar changes follow.
    /* 11 */ "stock_vim_save_all",
    /* 12 */ "stock_vim_session_save",
    /* 13 */ "stock_vim_session_new",
    /* 14 */ "stock_vim_session_load",
#  else
    /* 11 */ "vim-save-all",
    /* 12 */ "vim-session-save",
    /* 13 */ "vim-session-new",
    /* 14 */ "vim-session-load",
#  endif
    /* 15 */ "system-run",		// sub. GTK_STOCK_EXECUTE
    /* 16 */ "edit-find-replace",	// sub. GTK_STOCK_FIND_AND_REPLACE
    /* 17 */ "window-close",		// sub. GTK_STOCK_CLOSE, FIXME: fuzzy
#  if GTK_CHECK_VERSION(3,14,0)
    /* 18 */ "stock_vim_window_maximize",
    /* 19 */ "stock_vim_window_minimize",
    /* 20 */ "stock_vim_window_split",
    /* 21 */ "stock_vim_shell",
#  else
    /* 18 */ "vim-window-maximize",
    /* 19 */ "vim-window-minimize",
    /* 20 */ "vim-window-split",
    /* 21 */ "vim-shell",
#  endif
    /* 22 */ "go-previous",		// sub. GTK_STOCK_GO_BACK
    /* 23 */ "go-next",			// sub. GTK_STOCK_GO_FORWARD
#  if GTK_CHECK_VERSION(3,14,0)
    /* 24 */ "stock_vim_find_help",
#  else
    /* 24 */ "vim-find-help",
#  endif
    /* 25 */ "gtk-convert",		// sub. GTK_STOCK_CONVERT
    /* 26 */ "go-jump",			// sub. GTK_STOCK_JUMP_TO
#  if GTK_CHECK_VERSION(3,14,0)
    /* 27 */ "stock_vim_build_tags",
    /* 28 */ "stock_vim_window_split_vertical",
    /* 29 */ "stock_vim_window_maximize_width",
    /* 30 */ "stock_vim_window_minimize_width",
#  else
    /* 27 */ "vim-build-tags",
    /* 28 */ "vim-window-split-vertical",
    /* 29 */ "vim-window-maximize-width",
    /* 30 */ "vim-window-minimize-width",
#  endif
    /* 31 */ "application-exit",	// GTK_STOCK_QUIT
};
# else // !GTK_CHECK_VERSION(3,10,0)
static const char * const menu_stock_ids[] =
{
    /* 00 */ GTK_STOCK_NEW,
    /* 01 */ GTK_STOCK_OPEN,
    /* 02 */ GTK_STOCK_SAVE,
    /* 03 */ GTK_STOCK_UNDO,
    /* 04 */ GTK_STOCK_REDO,
    /* 05 */ GTK_STOCK_CUT,
    /* 06 */ GTK_STOCK_COPY,
    /* 07 */ GTK_STOCK_PASTE,
    /* 08 */ GTK_STOCK_PRINT,
    /* 09 */ GTK_STOCK_HELP,
    /* 10 */ GTK_STOCK_FIND,
    /* 11 */ "vim-save-all",
    /* 12 */ "vim-session-save",
    /* 13 */ "vim-session-new",
    /* 14 */ "vim-session-load",
    /* 15 */ GTK_STOCK_EXECUTE,
    /* 16 */ GTK_STOCK_FIND_AND_REPLACE,
    /* 17 */ GTK_STOCK_CLOSE,		// FIXME: fuzzy
    /* 18 */ "vim-window-maximize",
    /* 19 */ "vim-window-minimize",
    /* 20 */ "vim-window-split",
    /* 21 */ "vim-shell",
    /* 22 */ GTK_STOCK_GO_BACK,
    /* 23 */ GTK_STOCK_GO_FORWARD,
    /* 24 */ "vim-find-help",
    /* 25 */ GTK_STOCK_CONVERT,
    /* 26 */ GTK_STOCK_JUMP_TO,
    /* 27 */ "vim-build-tags",
    /* 28 */ "vim-window-split-vertical",
    /* 29 */ "vim-window-maximize-width",
    /* 30 */ "vim-window-minimize-width",
    /* 31 */ GTK_STOCK_QUIT
};
# endif // !GTK_CHECK_VERSION(3,10,0)

# ifdef USE_GRESOURCE
#  if !GTK_CHECK_VERSION(3,10,0)
typedef struct IconNames {
    const char *icon_name;
    const char *file_name;
} IconNames;

static IconNames stock_vim_icons[] = {
    { "vim-build-tags", "stock_vim_build_tags.png" },
    { "vim-find-help", "stock_vim_find_help.png" },
    { "vim-save-all", "stock_vim_save_all.png" },
    { "vim-session-load", "stock_vim_session_load.png" },
    { "vim-session-new", "stock_vim_session_new.png" },
    { "vim-session-save", "stock_vim_session_save.png" },
    { "vim-shell", "stock_vim_shell.png" },
    { "vim-window-maximize", "stock_vim_window_maximize.png" },
    { "vim-window-maximize-width", "stock_vim_window_maximize_width.png" },
    { "vim-window-minimize", "stock_vim_window_minimize.png" },
    { "vim-window-minimize-width", "stock_vim_window_minimize_width.png" },
    { "vim-window-split", "stock_vim_window_split.png" },
    { "vim-window-split-vertical", "stock_vim_window_split_vertical.png" },
    { NULL, NULL }
};
#  endif
# endif // USE_G_RESOURCE

    int
lookup_menu_iconfile(char_u *iconfile, char_u *dest)
{
    expand_env(iconfile, dest, MAXPATHL);

    if (mch_isFullName(dest))
    {
	return vim_fexists(dest);
    }
    else
    {
	static const char   suffixes[][4] = {"png", "xpm", "bmp"};
	char_u		    buf[MAXPATHL];
	unsigned int	    i;

	for (i = 0; i < G_N_ELEMENTS(suffixes); ++i)
	    if (gui_find_bitmap(dest, buf, (char *)suffixes[i]) == OK)
	    {
		STRCPY(dest, buf);
		return TRUE;
	    }

	return FALSE;
    }
}

    GtkWidget *
load_menu_iconfile(char_u *name, GtkIconSize icon_size)
{
    GtkWidget	    *image = NULL;
# if GTK_CHECK_VERSION(3,10,0)
    int		     pixel_size = -1;

#ifdef USE_GTK4
    // In GTK4 actual pixel sizes are determined by themes via CSS. Therefore
    // the pixbuf data should not be constrained to a specific pixel size.
    pixel_size = -1;
#else
    switch (icon_size)
    {
	case GTK_ICON_SIZE_MENU:
	    pixel_size = 16;
	    break;
	case GTK_ICON_SIZE_SMALL_TOOLBAR:
	    pixel_size = 16;
	    break;
	case GTK_ICON_SIZE_LARGE_TOOLBAR:
	    pixel_size = 24;
	    break;
	case GTK_ICON_SIZE_BUTTON:
	    pixel_size = 16;
	    break;
	case GTK_ICON_SIZE_DND:
	    pixel_size = 32;
	    break;
	case GTK_ICON_SIZE_DIALOG:
	    pixel_size = 48;
	    break;
	case GTK_ICON_SIZE_INVALID:
	    // FALLTHROUGH
	default:
	    pixel_size = 0;
	    break;
    }
#endif

    if (pixel_size > 0 || pixel_size == -1)
    {
	GdkPixbuf * const pixbuf
	    = gdk_pixbuf_new_from_file_at_scale((const char *)name,
		pixel_size, pixel_size, TRUE, NULL);
	if (pixbuf != NULL)
	{
	    image = gtk_image_new_from_pixbuf(pixbuf);
	    g_object_unref(pixbuf);
	}
    }
    if (image == NULL)
#ifdef USE_GTK4
	image = gtk_image_new_from_icon_name("image-missing");
#else
	image = gtk_image_new_from_icon_name("image-missing", icon_size);
#endif

    return image;
# else // !GTK_CHECK_VERSION(3,10,0)
    GtkIconSet	    *icon_set;
    GtkIconSource   *icon_source;

    /*
     * Rather than loading the icon directly into a GtkImage, create
     * a new GtkIconSet and put it in there.  This way we can easily
     * scale the toolbar icons on the fly when needed.
     */
    icon_set = gtk_icon_set_new();
    icon_source = gtk_icon_source_new();

    gtk_icon_source_set_filename(icon_source, (const char *)name);
    gtk_icon_set_add_source(icon_set, icon_source);

    image = gtk_image_new_from_icon_set(icon_set, icon_size);

    gtk_icon_source_free(icon_source);
    gtk_icon_set_unref(icon_set);

    return image;
# endif // !GTK_CHECK_VERSION(3,10,0)
}

    GtkWidget *
gui_gtk_themed_menu_icon(vimmenu_T *menu, GtkIconSize icon_size)
{
    GtkWidget *image = NULL;
# if GTK_CHECK_VERSION(3,10,0)
    const char *icon_name = NULL;
    const int   n_names = G_N_ELEMENTS(menu_themed_names);

    if (menu->iconidx >= 0 && menu->iconidx < n_names)
	icon_name = menu_themed_names[menu->iconidx];
    if (icon_name == NULL)
	icon_name = "image-missing";

#  if GTK_CHECK_VERSION(4,0,0)
	image = gtk_image_new_from_icon_name(icon_name);
#  else
	image = gtk_image_new_from_icon_name(icon_name, icon_size);
#  endif
# endif
    return image;
}

    GtkWidget *
gui_gtk_stock_menu_icon(vimmenu_T *menu, GtkIconSize icon_size)
{
    GtkWidget *image = NULL;
# if !GTK_CHECK_VERSION(3,10,0)
    const char  *stock_id;
    const int   n_ids = G_N_ELEMENTS(menu_stock_ids);

    if (menu->iconidx >= 0 && menu->iconidx < n_ids)
	stock_id = menu_stock_ids[menu->iconidx];
    else
	stock_id = GTK_STOCK_MISSING_IMAGE;

    image = gtk_image_new_from_stock(stock_id, icon_size);
# endif
    return image;
}

    GtkWidget *
gui_gtk_create_menu_icon(vimmenu_T *menu, GtkIconSize icon_size)
{
    GtkWidget	*image = NULL;
    char_u	buf[MAXPATHL];

    // First use a specified "icon=" argument.
    if (menu->iconfile != NULL && lookup_menu_iconfile(menu->iconfile, buf))
	image = load_menu_iconfile(buf, icon_size);

    // If not found and not builtin specified try using the menu name.
    if (image == NULL && !menu->icon_builtin
				     && lookup_menu_iconfile(menu->name, buf))
	image = load_menu_iconfile(buf, icon_size);

    // Still not found?  Then use a builtin icon, a blank one as fallback.
    if (image == NULL)
	image = gui_gtk_themed_menu_icon(menu, icon_size);

    if (image == NULL)
	image = gui_gtk_stock_menu_icon(menu, icon_size);

    return image;
}

#endif // FEAT_TOOLBAR
// }}}

// FEAT_MENU {{{

#if defined(FEAT_MENU)
/*
 * This is called after setting all the menus to grey/hidden or not.
 */
    void
gui_mch_draw_menubar(void)
{
    // just make sure that the visual changes get effect immediately
    gui_mch_update();
}

/*
 * Translate Vim's mnemonic tagging to GTK+ style and convert to UTF-8
 * if necessary.  The caller must vim_free() the returned string.
 *
 *	Input	Output
 *	_	__
 *	&&	&
 *	&	_	stripped if use_mnemonic == FALSE
 *	<Tab>		end of menu label text
 */
    char_u *
gui_gtk_translate_mnemonic_tag(char_u *name, int use_mnemonic)
{
    char_u  *buf;
    char_u  *psrc;
    char_u  *pdest;
    int	    n_underscores = 0;

    name = CONVERT_TO_UTF8(name);
    if (name == NULL)
	return NULL;

    for (psrc = name; *psrc != NUL && *psrc != TAB; ++psrc)
	if (*psrc == '_')
	    ++n_underscores;

    buf = alloc(psrc - name + n_underscores + 1);
    if (buf != NULL)
    {
	pdest = buf;
	for (psrc = name; *psrc != NUL && *psrc != TAB; ++psrc)
	{
	    if (*psrc == '_')
	    {
		*pdest++ = '_';
		*pdest++ = '_';
	    }
	    else if (*psrc != '&')
	    {
		*pdest++ = *psrc;
	    }
	    else if (*(psrc + 1) == '&')
	    {
		*pdest++ = *psrc++;
	    }
	    else if (use_mnemonic)
	    {
		*pdest++ = '_';
	    }
	}
	*pdest = NUL;
    }

    CONVERT_TO_UTF8_FREE(name);
    return buf;
}
#endif

// }}}

/*
 * Scrollbar stuff.
 */
    void
gui_mch_set_scrollbar_thumb(scrollbar_T *sb, long val, long size, long max)
{
    if (sb->id == NULL)
	return;

    GtkAdjustment *adjustment;

    // ignore events triggered by moving the thumb (happens in GTK 3)
    ++hold_gui_events;

#if GTK_CHECK_VERSION(4,0,0)
    adjustment = gtk_scrollbar_get_adjustment(GTK_SCROLLBAR(sb->id));
#else
    adjustment = gtk_range_get_adjustment(GTK_RANGE(sb->id));
#endif

    gtk_adjustment_set_lower(adjustment, 0.0);
    gtk_adjustment_set_value(adjustment, val);
    gtk_adjustment_set_upper(adjustment, max + 1);
    gtk_adjustment_set_page_size(adjustment, size);
    gtk_adjustment_set_page_increment(adjustment,
	    size < 3L ? 1L : size - 2L);
    gtk_adjustment_set_step_increment(adjustment, 1.0);

    g_signal_handler_block(G_OBJECT(adjustment), (gulong)sb->handler_id);

    --hold_gui_events;

#if !GTK_CHECK_VERSION(3,18,0)
    gtk_adjustment_changed(adjustment);
#endif

    g_signal_handler_unblock(G_OBJECT(adjustment),
	    (gulong)sb->handler_id);
}

extern void gui_gtk_find_replace_dialog_create(char_u *, int);

    void
gui_mch_find_dialog(exarg_T *eap)
{
    if (gui.in_use)
	gui_gtk_find_replace_dialog_create(eap->arg, FALSE);
}

    void
gui_mch_replace_dialog(exarg_T *eap)
{
    if (gui.in_use)
	gui_gtk_find_replace_dialog_create(eap->arg, TRUE);
}

/*
 * This is currently only used by find_replace_dialog_create(), and
 * I'd really like to keep it at that. In other words: don't spread
 * this nasty hack all over the code.  Think twice.
 */
    const char *
gui_gtk_convert_localized_message(char_u **buffer, const char *message)
{
    if (output_conv.vc_type == CONV_NONE)
       return message;

    vim_free(*buffer);
    *buffer = string_convert(&output_conv, (char_u *)message, NULL);

    return (const char *)*buffer;
}

/*
 * ":helpfind"
 */
    void
ex_helpfind(exarg_T *eap UNUSED)
{
    // This will fail when menus are not loaded.  Well, it's only for
    // backwards compatibility anyway.
    do_cmdline_cmd((char_u *)"emenu ToolBar.FindHelp");
}

// FEAT_GUI_DIALOG {{{

#if defined(FEAT_GUI_DIALOG) || defined(PROTO)

    static GtkWidget *
create_message_dialog(int type, char_u *title, char_u *message)
{
    GtkWidget	    *dialog;
    GtkMessageType  message_type;

    switch (type)
    {
	case VIM_ERROR:	    message_type = GTK_MESSAGE_ERROR;	 break;
	case VIM_WARNING:   message_type = GTK_MESSAGE_WARNING;	 break;
	case VIM_QUESTION:  message_type = GTK_MESSAGE_QUESTION; break;
	default:	    message_type = GTK_MESSAGE_INFO;	 break;
    }

    message = CONVERT_TO_UTF8(message);
    dialog  = gtk_message_dialog_new(GTK_WINDOW(gui.mainwin),
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     message_type,
				     GTK_BUTTONS_NONE,
				     "%s", (const char *)message);
    CONVERT_TO_UTF8_FREE(message);

    if (title != NULL)
    {
	title = CONVERT_TO_UTF8(title);
	gtk_window_set_title(GTK_WINDOW(dialog), (const char *)title);
	CONVERT_TO_UTF8_FREE(title);
    }
    else if (type == VIM_GENERIC)
    {
	gtk_window_set_title(GTK_WINDOW(dialog), "VIM");
    }

    return dialog;
}

/*
 * Split up button_string into individual button labels by inserting
 * NUL bytes.  Also replace the Vim-style mnemonic accelerator prefix
 * '&' with '_'.  button_string must point to allocated memory!
 * Return an allocated array of pointers into button_string.
 */
    static char **
split_button_string(char_u *button_string, int *n_buttons)
{
    char	    **array;
    char_u	    *p;
    unsigned int    count = 1;

    for (p = button_string; *p != NUL; ++p)
	if (*p == DLG_BUTTON_SEP)
	    ++count;

    array = ALLOC_MULT(char *, count + 1);
    count = 0;

    if (array != NULL)
    {
	array[count++] = (char *)button_string;
	for (p = button_string; *p != NUL; )
	{
	    if (*p == DLG_BUTTON_SEP)
	    {
		*p++ = NUL;
		array[count++] = (char *)p;
	    }
	    else if (*p == DLG_HOTKEY_CHAR)
		*p++ = '_';
	    else
		MB_PTR_ADV(p);
	}
	array[count] = NULL; // currently not relied upon, but doesn't hurt
    }

    *n_buttons = count;
    return array;
}

    static char **
split_button_translation(const char *message)
{
    char    **buttons = NULL;
    char_u  *str;
    int	    n_buttons = 0;
    int	    n_expected = 1;

    for (str = (char_u *)message; *str != NUL; ++str)
	if (*str == DLG_BUTTON_SEP)
	    ++n_expected;

    str = (char_u *)_(message);
    if (str != NULL)
    {
	if (output_conv.vc_type != CONV_NONE)
	    str = string_convert(&output_conv, str, NULL);
	else
	    str = vim_strsave(str);

	if (str != NULL)
	    buttons = split_button_string(str, &n_buttons);
    }
    /*
     * Uh-oh... this should never ever happen.	But we don't wanna crash
     * if the translation is broken, thus fall back to the untranslated
     * buttons string in case of emergency.
     */
    if (buttons == NULL || n_buttons != n_expected)
    {
	vim_free(buttons);
	vim_free(str);
	buttons = NULL;
	str = vim_strsave((char_u *)message);

	if (str != NULL)
	    buttons = split_button_string(str, &n_buttons);
	if (buttons == NULL)
	    vim_free(str);
    }

    return buttons;
}

    static int
button_equal(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0')
    {
	if (*a == '_' && *++a == '\0')
	    break;
	if (*b == '_' && *++b == '\0')
	    break;

	if (g_unichar_tolower(g_utf8_get_char(a))
		!= g_unichar_tolower(g_utf8_get_char(b)))
	    return FALSE;

	a = g_utf8_next_char(a);
	b = g_utf8_next_char(b);
    }

    return (*a == '\0' && *b == '\0');
}

    static void
dialog_add_buttons(GtkDialog *dialog, char_u *button_string)
{
    char    **ok;
    char    **ync;  // "yes no cancel"
    char    **buttons;
    int	    n_buttons = 0;
    int	    idx;

    button_string = vim_strsave(button_string); // must be writable
    if (button_string == NULL)
	return;

    /*
     * Yes this is ugly, I don't particularly like it either.  But doing it
     * this way has the compelling advantage that translations need not to
     * be touched at all.  See below what 'ok' and 'ync' are used for.
     */
    ok	    = split_button_translation(N_("&Ok"));
    ync     = split_button_translation(N_("&Yes\n&No\n&Cancel"));
    buttons = split_button_string(button_string, &n_buttons);

    /*
     * Yes, the buttons are in reversed order to match the GNOME 2 desktop
     * environment.  Don't hit me -- it's all about consistency.
     * Well, apparently somebody changed his mind: with GTK 2.2.4 it works the
     * other way around...
     */
    for (idx = 1; idx <= n_buttons; ++idx)
    {
	char	*label;
	char_u	*label8;

	label = buttons[idx - 1];
	/*
	 * Perform some guesswork to find appropriate stock items for the
	 * buttons.  We have to compare with a sample of the translated
	 * button string to get things right.  Yes, this is hackish :/
	 *
	 * But even the common button labels aren't necessarily translated,
	 * since anyone can create their own dialogs using Vim functions.
	 * Thus we have to check for those too.
	 */
	if (ok != NULL && ync != NULL) // almost impossible to fail
	{
# if GTK_CHECK_VERSION(3,10,0)
	    if	    (button_equal(label, ok[0]))    label = _("OK");
	    else if (button_equal(label, ync[0]))   label = _("Yes");
	    else if (button_equal(label, ync[1]))   label = _("No");
	    else if (button_equal(label, ync[2]))   label = _("Cancel");
	    else if (button_equal(label, "Ok"))     label = _("OK");
	    else if (button_equal(label, "Yes"))    label = _("Yes");
	    else if (button_equal(label, "No"))     label = _("No");
	    else if (button_equal(label, "Cancel")) label = _("Cancel");
# else
	    if	    (button_equal(label, ok[0]))    label = GTK_STOCK_OK;
	    else if (button_equal(label, ync[0]))   label = GTK_STOCK_YES;
	    else if (button_equal(label, ync[1]))   label = GTK_STOCK_NO;
	    else if (button_equal(label, ync[2]))   label = GTK_STOCK_CANCEL;
	    else if (button_equal(label, "Ok"))     label = GTK_STOCK_OK;
	    else if (button_equal(label, "Yes"))    label = GTK_STOCK_YES;
	    else if (button_equal(label, "No"))     label = GTK_STOCK_NO;
	    else if (button_equal(label, "Cancel")) label = GTK_STOCK_CANCEL;
# endif
	}
	label8 = CONVERT_TO_UTF8((char_u *)label);
	gtk_dialog_add_button(dialog, (const gchar *)label8, idx);
	CONVERT_TO_UTF8_FREE(label8);
    }

    if (ok != NULL)
	vim_free(*ok);
    if (ync != NULL)
	vim_free(*ync);
    vim_free(ok);
    vim_free(ync);
    vim_free(buttons);
    vim_free(button_string);
}

/*
 * Allow mnemonic accelerators to be activated without pressing <Alt>.
 * I'm not sure if it's a wise idea to do this.  However, the old GTK+ 1.2
 * GUI used to work this way, and I consider the impact on UI consistency
 * low enough to justify implementing this as a special Vim feature.
 */
typedef struct _DialogInfo
{
    int		ignore_enter;	    // no default button, ignore "Enter"
    int		noalt;		    // accept accelerators without Alt
    GtkDialog	*dialog;	    // Widget of the dialog
} DialogInfo;

    static gboolean
#ifdef USE_GTK4
dialog_key_press_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
#else
dialog_key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
#endif
{
    DialogInfo *di = (DialogInfo *)data;
    guint keyval = gdk_key_event_get_keyval(event);
    GdkModifierType state;
#ifdef USE_GTK4
    state = gdk_event_get_modifier_state(event);
#else
    if (gdk_event_get_state((GdkEvent *)event, &state) == FALSE)
    {
	return FALSE;
    }
#endif

    // Ignore hitting Enter (or Space) when there is no default button.
    if (di->ignore_enter && (keyval == GDK_KEY_Return || keyval == ' '))
	return TRUE;
    else    // A different key was pressed, return to normal behavior
	di->ignore_enter = FALSE;

    // Close the dialog when hitting "Esc".
    if (keyval == GDK_KEY_Escape)
    {
	gtk_dialog_response(di->dialog, GTK_RESPONSE_REJECT);
	return TRUE;
    }

#ifndef USE_GTK4
    if (di->noalt && (state & gtk_accelerator_get_default_mod_mask()) == 0)
    {
	return gtk_window_mnemonic_activate(
		   GTK_WINDOW(widget), keyval,
		   gtk_window_get_mnemonic_modifier(GTK_WINDOW(widget)));
    }
#endif

    return FALSE; // continue emission
}

    int
gui_mch_dialog(int	type,	    // type of dialog
	       char_u	*title,	    // title of dialog
	       char_u	*message,   // message text
	       char_u	*buttons,   // names of buttons
	       int	def_but,    // default button
	       char_u	*textfield, // text for textfield or NULL
	       int	ex_cmd UNUSED)
{
    GtkWidget	*dialog;
    GtkWidget	*entry = NULL;
    char_u	*text;
    int		response;
    GtkWidget	*alignment;
    DialogInfo  dialoginfo;

    dialog = create_message_dialog(type, title, message);
    dialoginfo.dialog = GTK_DIALOG(dialog);
    dialog_add_buttons(GTK_DIALOG(dialog), buttons);
# if !GTK_CHECK_VERSION(4,0,0)
    gtk_window_set_type_hint(GTK_WINDOW(dialog),
			     GDK_WINDOW_TYPE_HINT_POPUP_MENU);
# endif

    if (textfield != NULL)
    {
	entry = gtk_entry_new();
	gtk_widget_set_visible(entry, TRUE);

	// Make Enter work like pressing OK.
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

	text = CONVERT_TO_UTF8(textfield);
	gtk_entry_set_text(GTK_ENTRY(entry), (const char *)text);
	CONVERT_TO_UTF8_FREE(text);

# if GTK_CHECK_VERSION(3,0,0)
	gtk_widget_set_halign(GTK_WIDGET(entry), GTK_ALIGN_CENTER);
	gtk_widget_set_valign(GTK_WIDGET(entry), GTK_ALIGN_CENTER);
	gtk_widget_set_hexpand(GTK_WIDGET(entry), TRUE);
	gtk_widget_set_vexpand(GTK_WIDGET(entry), TRUE);

	alignment = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
# else
	alignment = gtk_alignment_new((float)0.5, (float)0.5,
						      (float)1.0, (float)1.0);
# endif
	gtk_box_append(GTK_BOX(alignment), entry);
	gtk_widget_set_visible(alignment, TRUE);

# if GTK_CHECK_VERSION(3,0,0)
	{
	    GtkWidget * const vbox
		= gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	    gtk_box_prepend(GTK_BOX(vbox), alignment);
	}
# else
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
			   alignment, TRUE, FALSE, 0);
# endif
	dialoginfo.noalt = FALSE;
    }
    else
	dialoginfo.noalt = TRUE;

# if GTK_CHECK_VERSION(4,0,0)
    // TODO: How should key presses be handled for the text inputs?
# else
    // Allow activation of mnemonic accelerators without pressing <Alt> when
    // there is no textfield.  Handle pressing Esc.
    g_signal_connect(G_OBJECT(dialog), "key-press-event",
			 G_CALLBACK(&dialog_key_press_event_cb), &dialoginfo);
# endif

    if (def_but > 0)
    {
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), def_but);
	dialoginfo.ignore_enter = FALSE;
    }
    else
	// No default button, ignore pressing Enter.
	dialoginfo.ignore_enter = TRUE;

    // Show the mouse pointer if it's currently hidden.
    gui_mch_mousehide(FALSE);

    response = gtk_dialog_run(GTK_DIALOG(dialog));

    // GTK_RESPONSE_NONE means the dialog was programmatically destroyed.
    if (response != GTK_RESPONSE_NONE)
    {
	if (response == GTK_RESPONSE_ACCEPT)	    // Enter pressed
	    response = def_but;
	if (textfield != NULL)
	{
	    text = (char_u *)gtk_entry_get_text(GTK_ENTRY(entry));
	    text = CONVERT_FROM_UTF8(text);

	    vim_strncpy(textfield, text, IOSIZE - 1);

	    CONVERT_FROM_UTF8_FREE(text);
	}
#if GTK_CHECK_VERSION(4,0,0)
	gtk_window_destroy(GTK_WINDOW(dialog));
#else
	gtk_widget_destroy(dialog);
#endif
    }

    return response > 0 ? response : 0;
}

#endif // FEAT_GUI_DIALOG

// FEAT_GUI_DIALOG }}}

// FEAT_BROWSE {{{

#if defined(FEAT_BROWSE) || defined(PROTO)
/*
 * Implementation of the file selector related stuff
 */

    static void
recent_func_log_func(const gchar *log_domain UNUSED,
		     GLogLevelFlags log_level UNUSED,
		     const gchar *message UNUSED,
		     gpointer user_data UNUSED)
{
    // We just want to suppress the warnings.
    // http://bugzilla.gnome.org/show_bug.cgi?id=664587
}

/*
 * Put up a file requester.
 * Returns the selected name in allocated memory, or NULL for Cancel.
 * saving,			select file to write
 * title			title for the window
 * dflt				default name
 * ext				not used (extension added)
 * initdir			initial directory, NULL for current dir
 * filter			file name filter
 */
    char_u *
gui_mch_browse(int saving,
	       char_u *title,
	       char_u *dflt,
	       char_u *ext UNUSED,
	       char_u *initdir,
	       char_u *filter)
{
# if GTK_CHECK_VERSION(3,20,0)
    GtkFileChooserNative	*fc;
# else
    GtkWidget			*fc;
# endif
    GError              *fc_error = NULL;
    char_u		dirbuf[MAXPATHL];
    GFile               *dirbuf_file = NULL;
    guint		log_handler;
    const gchar		*domain = "Gtk";

    title = CONVERT_TO_UTF8(title);

    // GTK has a bug, it only works with an absolute path.
    if (initdir == NULL || *initdir == NUL)
	mch_dirname(dirbuf, MAXPATHL);
    else if (vim_FullName(initdir, dirbuf, MAXPATHL - 2, FALSE) == FAIL)
	dirbuf[0] = NUL;
    // Always need a trailing slash for a directory.
    add_pathsep(dirbuf);

    // If our pointer is currently hidden, then we should show it.
    gui_mch_mousehide(FALSE);

    // Hack: The GTK file dialog warns when it can't access a new file, this
    // makes it shut up. http://bugzilla.gnome.org/show_bug.cgi?id=664587
    log_handler = g_log_set_handler(domain, G_LOG_LEVEL_WARNING,
						  recent_func_log_func, NULL);

    // We create the dialog each time, so that the button text can be "Open"
    // or "Save" according to the action.
# if GTK_CHECK_VERSION(3,20,0)
    fc = gtk_file_chooser_native_new(
# else
    fc = gtk_file_chooser_dialog_new(
# endif
	    (const gchar *)title,
	    GTK_WINDOW(gui.mainwin),
	    saving ? GTK_FILE_CHOOSER_ACTION_SAVE
					   : GTK_FILE_CHOOSER_ACTION_OPEN,
# if GTK_CHECK_VERSION(3,20,0)
	    saving ? _("_Save") : _("_Open"), _("_Cancel"));
# else
#  if GTK_CHECK_VERSION(3,10,0)
	    _("_Cancel"), GTK_RESPONSE_CANCEL,
	    saving ? _("_Save") : _("_Open"), GTK_RESPONSE_ACCEPT,
#  else
	    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	    saving ? GTK_STOCK_SAVE : GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#  endif
	    NULL);
# endif

# ifdef USE_GTK4
    dirbuf_file = g_file_new_for_path((const char *)dirbuf);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fc),
						       dirbuf_file,
						       &fc_error
						       );
    g_object_unref(dirbuf_file);
    dirbuf_file = NULL;
# else
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fc),
						       (const gchar *)dirbuf);
# endif

    if (filter != NULL && *filter != NUL)
    {
	int     i = 0;
	char_u  *patt;
	char_u  *p = filter;
	GtkFileFilter	*gfilter;

	gfilter = gtk_file_filter_new();
	patt = alloc(STRLEN(filter));
	while (p != NULL && *p != NUL)
	{
	    if (*p == '\n' || *p == ';' || *p == '\t')
	    {
		STRNCPY(patt, filter, i);
		patt[i] = '\0';
		if (*p == '\t')
		    gtk_file_filter_set_name(gfilter, (gchar *)patt);
		else
		{
		    gtk_file_filter_add_pattern(gfilter, (gchar *)patt);
		    if (*p == '\n')
		    {
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fc),
								     gfilter);
			if (*(p + 1) != NUL)
			    gfilter = gtk_file_filter_new();
		    }
		}
		filter = ++p;
		i = 0;
	    }
	    else
	    {
		p++;
		i++;
	    }
	}
	vim_free(patt);
    }
    if (saving && dflt != NULL && *dflt != NUL)
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fc), (char *)dflt);

    gui.browse_fname = NULL;
# if GTK_CHECK_VERSION(3,20,0)
    if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(fc)) == GTK_RESPONSE_ACCEPT)
# else
    if (gtk_dialog_run(GTK_DIALOG(fc)) == GTK_RESPONSE_ACCEPT)
# endif
    {
	char  *filename = NULL;
	GFile *selfile = NULL;

	selfile = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(fc));
	if (selfile != NULL)
	{
	    filename = g_file_get_path(selfile);
	    g_object_unref(selfile);
	}

	gui.browse_fname = (char_u *)g_strdup(filename);
	g_free(filename);
    }
# if GTK_CHECK_VERSION(3,20,0)
    g_object_unref(fc);
# else
    gtk_widget_destroy(GTK_WIDGET(fc));
# endif

    g_log_remove_handler(domain, log_handler);

    CONVERT_TO_UTF8_FREE(title);
    if (gui.browse_fname == NULL)
	return NULL;

    // shorten the file name if possible
    return vim_strsave(shorten_fname1(gui.browse_fname));
}

/*
 * Put up a directory selector
 * Returns the selected name in allocated memory, or NULL for Cancel.
 * title			title for the window
 * dflt				default name
 * initdir			initial directory, NULL for current dir
 */
    char_u *
gui_mch_browsedir(
	       char_u *title,
	       char_u *initdir)
{
    char_u		dirbuf[MAXPATHL];
    char_u		*p;
    GtkWidget		*dirdlg;	    // file selection dialog
    char_u		*dirname = NULL;
    GFile               *seldir = NULL;
    GError              *fc_error = NULL;

    title = CONVERT_TO_UTF8(title);

    dirdlg = gtk_file_chooser_dialog_new(
	    (const gchar *)title,
	    GTK_WINDOW(gui.mainwin),
	    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
# if GTK_CHECK_VERSION(3,10,0)
	    _("_Cancel"), GTK_RESPONSE_CANCEL,
	    _("_OK"), GTK_RESPONSE_ACCEPT,
# else
	    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	    GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
# endif
	    NULL);

    CONVERT_TO_UTF8_FREE(title);

    // if our pointer is currently hidden, then we should show it.
    gui_mch_mousehide(FALSE);

    // GTK appears to insist on an absolute path.
    if (initdir == NULL || *initdir == NUL
	       || vim_FullName(initdir, dirbuf, MAXPATHL - 10, FALSE) == FAIL)
	mch_dirname(dirbuf, MAXPATHL - 10);

    // Always need a trailing slash for a directory.
    // Also add a dummy file name, so that we get to the directory.
    add_pathsep(dirbuf);
    STRCAT(dirbuf, "@zd(*&1|");
    seldir = g_file_new_for_path(dirbuf);
    gtk_file_chooser_set_file(GTK_FILE_CHOOSER(dirdlg), seldir, &fc_error);
    g_object_unref(seldir);
    if (fc_error != NULL)
    {
	msg(fc_error->message);
	g_object_unref(fc_error);
	return NULL;
    }

    gtk_widget_set_visible(GTK_WIDGET(dirdlg), TRUE);

    // Run the dialog.
    if (gtk_dialog_run(GTK_DIALOG(dirdlg)) == GTK_RESPONSE_ACCEPT)
    {
	seldir = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dirdlg));
	if (seldir != NULL)
	{
	    dirname = g_file_get_path(seldir);
	    g_object_unref(seldir);
	}
    }

#if GTK_CHECK_VERSION(4,0,0)
    gtk_window_destroy(GTK_WINDOW(dirdlg));
#else
    gtk_widget_destroy(dirdlg);
#endif
    if (dirname == NULL)
	return NULL;

    // shorten the file name if possible
    p = vim_strsave(shorten_fname1(dirname));
    g_free(dirname);
    return p;
}


#endif	// FEAT_BROWSE

// FEAT_BROWSE }}}

