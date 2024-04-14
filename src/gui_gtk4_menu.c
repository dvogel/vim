/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

#include "vim.h"

#include "gui_gtk4_menu.h"
#include "gui_gtk_common.h"

# define GTKVIM_MENU_ATTR_ITER_LABEL  0
# define GTKVIM_MENU_ATTR_ITER_ACTION 1
# define GTKVIM_MENU_ATTR_ITER_TARGET 2
# define GTKVIM_MENU_ATTR_ITER_CUSTOM 3
struct _GtkVimMenuAttrIter
{
    GMenuAttributeIter   parent_instance;
    vimmenu_T           *menu;
    gint                 index;
};

struct _GtkVimMenuAttrIterClass
{
    GMenuAttributeIterClass    parent_class;
};

G_DEFINE_FINAL_TYPE(GtkVimMenuAttrIter, gui_gtk_menu_attr_iter, G_TYPE_MENU_ATTRIBUTE_ITER)


    static gboolean
attr_iter_get_next(GMenuAttributeIter *iter, const gchar **out_name, GVariant **out_value)
{
    GtkVimMenuAttrIter *self = GTKVIM_MENU_ATTR_ITER(iter);
    if (self->menu == NULL)
	return FALSE;

next:
    self->index++;

    // The documentation for MenuAttributeIter->get_next says:
    //     The caller of the method takes ownership
    //     of the returned data, and is responsible
    //     for freeing it.
    // However this does not mean exclusive ownership. It simply means that the
    // caller is expected to call g_variant_ref() and thus the implementation
    // of the iter should expected shared ownership and rely only on _ref() and
    // _unref().
    switch (self->index)
    {
	case GTKVIM_MENU_ATTR_ITER_LABEL:
	    if (self->menu->gtk_attr_label == NULL)
		goto next;

	    *out_name = G_MENU_ATTRIBUTE_LABEL;
	    *out_value = self->menu->gtk_attr_label;
	    break;
	case GTKVIM_MENU_ATTR_ITER_ACTION:
	    if (self->menu->gtk_attr_action == NULL)
		goto next;

	    *out_name = G_MENU_ATTRIBUTE_ACTION;
	    *out_value = self->menu->gtk_attr_action;
	    break;
	case GTKVIM_MENU_ATTR_ITER_TARGET:
	    if (self->menu->gtk_attr_target == NULL)
		goto next;

	    *out_name = G_MENU_ATTRIBUTE_TARGET;
	    *out_value = self->menu->gtk_attr_target;
	    break;
	case GTKVIM_MENU_ATTR_ITER_CUSTOM:
	    if (self->menu->gtk_attr_custom == NULL)
		goto next;

	    *out_name = "custom";
	    *out_value = self->menu->gtk_attr_custom;
	    break;
	default:
	    *out_name = NULL;
	    *out_value = NULL;
	    return FALSE;
    }
    return TRUE;
}

    static void
gui_gtk_menu_attr_iter_class_init(GtkVimMenuAttrIterClass *klass)
{
    GMenuAttributeIterClass *parent_klass = G_MENU_ATTRIBUTE_ITER_CLASS(klass);
    parent_klass->get_next = attr_iter_get_next;
}

    static void
gui_gtk_menu_attr_iter_init(GtkVimMenuAttrIter *self)
{
    self->index = -1;
}


struct _GtkVimMenuLinkIter
{
    GMenuLinkIter parent_instance;
    vimmenu_T *menu;
};

struct _GtkVimMenuLinkIterClass
{
    GMenuLinkIterClass parent_class;
};

G_DEFINE_FINAL_TYPE(GtkVimMenuLinkIter, gui_gtk_menu_link_iter, G_TYPE_MENU_LINK_ITER)

static void gui_gtk_menu_link_iter_class_init(GtkVimMenuLinkIterClass *);
static void gui_gtk_menu_link_iter_init(GtkVimMenuLinkIter *);

    static gboolean
link_iter_get_next(
	GMenuLinkIter  *iter,
	const gchar   **out_link,
	GMenuModel    **out_value)
{
    GtkVimMenuLinkIter *self = GTKVIM_MENU_LINK_ITER(iter);

    if (self->menu == NULL)
	return FALSE;

    if ((self->menu->children != NULL) && (self->menu->gobj_model_self != NULL))
    {
	if (out_link != NULL)
	    *out_link = G_MENU_LINK_SUBMENU;
	if (out_value != NULL)
	    *out_value = g_object_ref(G_MENU_MODEL(self->menu->gobj_model_self));
	self->menu = NULL;
	return TRUE;
    }

    return FALSE;
}

    static void
gui_gtk_menu_link_iter_class_init(GtkVimMenuLinkIterClass *klass)
{
    GMenuLinkIterClass *parent_klass = G_MENU_LINK_ITER_CLASS(klass);
    parent_klass->get_next = link_iter_get_next;
}


    static void
gui_gtk_menu_link_iter_init(GtkVimMenuLinkIter *self)
{
    self->menu = NULL;
}

struct _GtkVimMenu
{
    GMenuModel parent_instance;
    vimmenu_T *menu;
};

struct _GtkVimMenuClass
{
    GMenuModelClass parent_class;
};

G_DEFINE_FINAL_TYPE(GtkVimMenu, gui_gtk_menu, G_TYPE_MENU_MODEL)

static void gui_gtk_menu_class_init(GtkVimMenuClass *);
static void gui_gtk_menu_init(GtkVimMenu *);

    static void
get_item_links(
	GMenuModel* model,
	gint item_index,
	GHashTable** links)
{
    *links = g_hash_table_new(g_str_hash, g_str_equal);

    GMenuLinkIter *iter = g_menu_model_iterate_item_links(model, item_index);
    while (g_menu_link_iter_next(iter) == TRUE)
    {
	const gchar *name = g_menu_link_iter_get_name(G_MENU_LINK_ITER(iter));
	GMenuModel *val = g_menu_link_iter_get_value(G_MENU_LINK_ITER(iter));
	g_hash_table_insert(
		*links,
		name,
		(gpointer)val);
    }
}

    static gint
get_n_items(GMenuModel* model)
{
    GtkVimMenu *self = GTKVIM_MENU(model);

    vimmenu_T *walk = self->menu->children;
    int cnt = 0;

    while (walk != NULL)
    {
	walk = walk->next;
    }

    return cnt;
}

    static gboolean
is_mutable(GMenuModel* model)
{
    return TRUE;
}

    static GMenuAttributeIter*
iterate_item_attributes(
	GMenuModel* model,
	gint item_index)
{
    GtkVimMenu *self = GTKVIM_MENU(model);

    vimmenu_T *walk = self->menu->children;
    for (int idx = 0; idx < item_index; idx++)
    {
	if (walk->next == NULL)
	    return NULL;

	walk = walk->next;
    }

    GtkVimMenuAttrIter *iter = g_object_new(GTKVIM_TYPE_MENU_ATTR_ITER, NULL);
    iter->menu = walk;

    return G_MENU_ATTRIBUTE_ITER(iter);
}

    static GMenuLinkIter*
iterate_item_links(
	GMenuModel* model,
	gint item_index)
{
    GtkVimMenu *self = GTKVIM_MENU(model);

    GtkVimMenuLinkIter *iter = g_object_new(GTKVIM_TYPE_MENU_LINK_ITER, NULL);

    vimmenu_T *walk = self->menu->children;
    for (int idx = 0; idx < item_index; idx++)
    {
	if (walk == NULL)
	    return NULL;
	walk = walk->next;
    }

    iter->menu = walk;

    return G_MENU_LINK_ITER(iter);
}

    static void
gui_gtk_menu_class_init(GtkVimMenuClass *klass)
{
    GMenuModelClass *parent_klass = G_MENU_MODEL_CLASS(klass);
    parent_klass->get_item_links = get_item_links;
    parent_klass->get_n_items = get_n_items;
    parent_klass->is_mutable = is_mutable;
    parent_klass->iterate_item_attributes = iterate_item_attributes;
    parent_klass->iterate_item_links = iterate_item_links;
}

    static void
gui_gtk_menu_init(GtkVimMenu *self)
{
    self->menu = NULL;
}

    vimmenu_T *
gui_gtk_menu_get_vim_menu(GtkVimMenu *self)
{
    return self->menu;
}

    void
gui_gtk_menu_set_vim_menu(GtkVimMenu *self, vimmenu_T *new_val)
{
    self->menu = new_val;
}
