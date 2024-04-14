/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

/*
 * In GTK4 all menus are expected to be represented by the GMenuModel
 * interface. Unfortunately for us, it is partially duplicative with role of
 * the VIM vimmenu_T data type.
 */

#ifndef __GTK4_MENU_H__
#define __GTK4_MENU_H__


# include <gtk/gtk.h>

# ifdef __cplusplus
extern "C" {
# endif

# define GTKVIM_TYPE_MENU				(gui_gtk_menu_get_type())
G_DECLARE_FINAL_TYPE(GtkVimMenu, gui_gtk_menu, GTKVIM, MENU, GMenuModel)

GType gui_gtk_menu_get_type(void);
GMenuModel *gui_gtk_menu_new(vimmenu_T *);
vimmenu_T *gui_gtk_menu_get_vim_menu(GtkVimMenu *);
void gui_gtk_menu_set_vim_menu(GtkVimMenu *, vimmenu_T *);
// TODO: void gui_gtk_menu_changed(vimmenu_T *);

# define GTKVIM_TYPE_MENU_ATTR_ITER		(gui_gtk_menu_attr_iter_get_type())
G_DECLARE_FINAL_TYPE(GtkVimMenuAttrIter, gui_gtk_menu_attr_iter, GTKVIM, MENU_ATTR_ITER, GMenuAttributeIter)
// TODO: The vimmenu_T pointer should probably be a construction property of GtkVimMenuAttrIter.

# define GTKVIM_TYPE_MENU_LINK_ITER		(gui_gtk_menu_link_iter_get_type())
G_DECLARE_FINAL_TYPE(GtkVimMenuLinkIter, gui_gtk_menu_link_iter, GTKVIM, MENU_LINK_ITER, GMenuLinkIter)

#endif // __GTK_MENU_H__
