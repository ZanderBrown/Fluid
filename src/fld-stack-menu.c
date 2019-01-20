#include "fld-stack-menu.h"

typedef struct
{
  GtkStack      *stack;
  GArray        *items;
} FldStackMenuPrivate;

enum {
  PROP_0,
  PROP_STACK,
  N_PROPS
};

G_DEFINE_TYPE_WITH_CODE (FldStackMenu, fld_stack_menu, G_TYPE_MENU_MODEL,
                         G_ADD_PRIVATE (FldStackMenu))

static GParamSpec *properties [N_PROPS];

typedef struct
{
  GMenuModel *model;
  gint        position;
} ModEmit;

static gboolean
emit_modified (ModEmit *data)
{
  g_message ("Emit modifed");
  /* Hint this item was modifed it my marking one removed and another added */
  g_menu_model_items_changed (data->model, data->position, 1, 1);
  g_slice_free (ModEmit, data);
  return G_SOURCE_REMOVE;
}

static gboolean
emit_removed (ModEmit *data)
{
  g_message ("Emit removed");
  g_menu_model_items_changed (data->model, data->position, 1, 0);
  g_slice_free (ModEmit, data);
  return G_SOURCE_REMOVE;
}

static gboolean
emit_added (ModEmit *data)
{
  g_message ("Emit added");
  g_menu_model_items_changed (data->model, data->position, 0, 1);
  g_slice_free (ModEmit, data);
  return G_SOURCE_REMOVE;
}

static GHashTable *
build_item (FldStackMenu *self,
            GtkWidget    *child,
            gint         *position)
{
  g_assert (FLD_IS_STACK_MENU (self));

  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  GValue title = G_VALUE_INIT;
  g_value_init (&title, G_TYPE_STRING);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "title", &title);
  const gchar *label = g_value_get_string (&title);

  GValue hint = G_VALUE_INIT;
  g_value_init (&hint, G_TYPE_BOOLEAN);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "needs-attention", &hint);
  gboolean attention = g_value_get_boolean (&hint);

  if (attention)
    label = g_strdup_printf ("%s @", label);

  GValue name = G_VALUE_INIT;
  g_value_init (&name, G_TYPE_STRING);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "name", &name);
  const gchar *target = g_value_get_string (&name);

  g_message ("build_item for %s/%s", target, label);

  /* We can't control unnamed children, don't display it */
  if (!target)
    return NULL;

  GValue pos = G_VALUE_INIT;
  g_value_init (&pos, G_TYPE_INT);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "position", &pos);
  *position = g_value_get_int (&pos);

  GHashTable *item = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_variant_unref);
  g_hash_table_insert (item, g_strdup (G_MENU_ATTRIBUTE_ACTION), g_variant_ref_sink (g_variant_new_string ("switch.page")));
  g_hash_table_insert (item, g_strdup (G_MENU_ATTRIBUTE_TARGET), g_variant_ref_sink (g_variant_new_string (target)));
  g_hash_table_insert (item, g_strdup (G_MENU_ATTRIBUTE_LABEL), g_variant_ref_sink (g_variant_new_string (label)));

  return item;
}

static void
on_child_updated (GtkWidget   *child,
                  GParamSpec  *pspec,
                  FldStackMenu *self)
{
  g_message ("Child updated");
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  gint position = 0;
  GHashTable *item = build_item (self, child, &position);

  ModEmit *data = g_slice_new (ModEmit);
  data->model = G_MENU_MODEL (self);
  data->position = position;

  g_array_remove_index (priv->items, position);
  if (item)
    {
      g_array_insert_val (priv->items, position, item);
      g_idle_add ((GSourceFunc) emit_modified, data);
    }
  else
      g_idle_add ((GSourceFunc) emit_removed, data);
}

static void
on_position_updated (GtkWidget   *child,
                  GParamSpec  *pspec,
                  FldStackMenu *self)
{
  g_message ("Position stuff");
}

static void
add_child (GtkWidget   *child,
           FldStackMenu *self)
{
  g_assert (FLD_IS_STACK_MENU (self));

  g_message ("Add child");

  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  gint position = 0;
  GHashTable *item = build_item (self, child, &position);

  if (!item)
    return;

  g_array_insert_val (priv->items, position, item);

  g_signal_connect (child, "child-notify::title",
                    G_CALLBACK (on_child_updated), self);
  g_signal_connect (child, "child-notify::needs-attention",
                    G_CALLBACK (on_child_updated), self);
  g_signal_connect (child, "notify::visible",
                    G_CALLBACK (on_child_updated), self);
  g_signal_connect (child, "child-notify::position",
                    G_CALLBACK (on_position_updated), self);

  ModEmit *data = g_slice_new (ModEmit);
  data->model = G_MENU_MODEL (self);
  data->position = position;

  g_idle_add ((GSourceFunc) emit_added, data);
}

static void
remove_child (GtkWidget   *child,
              FldStackMenu *self)
{
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  g_message ("Remove child");

  GValue pos = G_VALUE_INIT;
  g_value_init (&pos, G_TYPE_INT);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "position", &pos);
  gint position = g_value_get_int (&pos);

  g_message ("Remove child (kinda) got pos %i", position);

  g_signal_handlers_disconnect_by_func (child, on_child_updated, self);
  g_signal_handlers_disconnect_by_func (child, on_position_updated, self);

  g_array_remove_index (priv->items, position);

  g_message ("Remove child removed");

  ModEmit *data = g_slice_new (ModEmit);
  data->model = G_MENU_MODEL (self);
  data->position = position;

  g_idle_add ((GSourceFunc) emit_removed, data);
}

static void
on_stack_child_added (GtkContainer    *container,
                      GtkWidget       *widget,
                      FldStackMenu     *self)
{
  add_child (widget, self);
}

static void
on_stack_child_removed (GtkContainer    *container,
                        GtkWidget       *widget,
                        FldStackMenu     *self)
{
  remove_child (widget, self);
}

static void
disconnect_stack_signals (FldStackMenu *self)
{
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  g_signal_handlers_disconnect_by_func (priv->stack, on_stack_child_added, self);
  g_signal_handlers_disconnect_by_func (priv->stack, on_stack_child_removed, self);
  g_signal_handlers_disconnect_by_func (priv->stack, disconnect_stack_signals, self);
}

static void
connect_stack_signals (FldStackMenu *self)
{
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);
  g_signal_connect_after (priv->stack, "add",
                          G_CALLBACK (on_stack_child_added), self);
  g_signal_connect_after (priv->stack, "remove",
                          G_CALLBACK (on_stack_child_removed), self);
  g_signal_connect_swapped (priv->stack, "destroy",
                            G_CALLBACK (disconnect_stack_signals), self);
}

static GtkStack *
fld_stack_menu_real_get_stack (FldStackMenu *self)
{
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  g_assert (FLD_IS_STACK_MENU (self));

  return priv->stack;
}

static void
fld_stack_menu_real_set_stack (FldStackMenu *self,
                               GtkStack    *stack)
{
  g_assert (FLD_IS_STACK_MENU (self));

  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);
  priv->stack = stack;

  connect_stack_signals (self);

  gtk_container_foreach (GTK_CONTAINER (priv->stack), (GtkCallback)add_child, self);
}

static void
fld_stack_menu_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  FldStackMenu *self = FLD_STACK_MENU (object);

  switch (prop_id)
    {
    case PROP_STACK:
      g_value_set_object (value, fld_stack_menu_get_stack (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
fld_stack_menu_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  FldStackMenu *self = FLD_STACK_MENU (object);

  switch (prop_id)
    {
    case PROP_STACK:
      fld_stack_menu_set_stack (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
fld_stack_menu_finalize (GObject *object)
{
  FldStackMenu *self = FLD_STACK_MENU (object);
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  g_array_free (priv->items, TRUE);

  G_OBJECT_CLASS (fld_stack_menu_parent_class)
    ->finalize (object);
}


static gboolean
fld_stack_menu_is_mutable (GMenuModel *model)
{
  /* We are always mutable */
  return TRUE;
}

static gint
fld_stack_menu_get_n_items (GMenuModel *model)
{
  FldStackMenu *self = FLD_STACK_MENU (model);
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  return priv->items->len;
}

static void
fld_stack_menu_get_item_attributes (GMenuModel  *model,
                                    gint         position,
                                    GHashTable **table)
{
  FldStackMenu *self = FLD_STACK_MENU (model);
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);

  *table = g_hash_table_ref (g_array_index (priv->items, GHashTable*, position));
}

static void
fld_stack_menu_get_item_links (GMenuModel  *model,
                               gint         position,
                               GHashTable **table)
{
  /* We don't support links */
  *table = g_hash_table_new (g_str_hash, g_str_equal);
}


static void
fld_stack_menu_class_init (FldStackMenuClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GMenuModelClass *model_class = G_MENU_MODEL_CLASS (klass);

  object_class->get_property = fld_stack_menu_get_property;
  object_class->set_property = fld_stack_menu_set_property;
  object_class->finalize = fld_stack_menu_finalize;

  model_class->is_mutable = fld_stack_menu_is_mutable;
  model_class->get_n_items = fld_stack_menu_get_n_items;
  model_class->get_item_attributes = fld_stack_menu_get_item_attributes;
  model_class->get_item_links = fld_stack_menu_get_item_links;

  klass->get_stack = fld_stack_menu_real_get_stack;
  klass->set_stack = fld_stack_menu_real_set_stack;

  /**
   * FldStackMenu:stack:
   *
   * The #GtkStack controlled by the #FldStackMenu
   */
  properties [PROP_STACK] =
    g_param_spec_object ("stack",
                         "Stack",
                         "The stack being controlled",
                         GTK_TYPE_STACK,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
fld_stack_menu_init (FldStackMenu *self)
{
  FldStackMenuPrivate *priv = fld_stack_menu_get_instance_private (self);
  priv->stack = NULL;
  priv->items = g_array_new (FALSE, FALSE, sizeof (GHashTable*));
  g_array_set_clear_func (priv->items, (GDestroyNotify) g_hash_table_destroy);
}

/**
 * fld_stack_menu_get_stack:
 * @self: a #FldStackMenu
 *
 * Returns: (transfer none): The #GtkStack controlled by the #FldStackMenu or %NULL
 * Since: 0.1.0
 */
GtkStack *
fld_stack_menu_get_stack (FldStackMenu *self)
{
  g_return_val_if_fail (FLD_IS_STACK_MENU (self), FALSE);

  return FLD_STACK_MENU_GET_CLASS (self)->get_stack (self);
}

/**
 * fld_stack_menu_set_stack:
 * @self: a #FldStackMenu
 * @stack: The #GtkStack controlled by the #FldStackMenu
 *
 * Since: 0.1.0
 */
void
fld_stack_menu_set_stack (FldStackMenu *self,
                          GtkStack     *stack)
{
  g_return_if_fail (FLD_IS_STACK_MENU (self));

  FLD_STACK_MENU_GET_CLASS (self)->set_stack (self, stack);

  g_object_notify (G_OBJECT (self), "stack");
}

FldStackMenu *
fld_stack_menu_new ()
{
  return g_object_new (FLD_TYPE_STACK_MENU, NULL);
}

