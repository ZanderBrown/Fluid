#include "fld-switcher.h"

typedef struct
{
  GtkStack      *stack;
  GtkWidget     *switcher;
  GtkWidget     *label;
  GtkWidget     *selector;
  GMenu         *menu;
  GSimpleAction *action;
} FldSwitcherPrivate;

enum {
  PROP_0,
  PROP_STACK,
  N_PROPS
};

G_DEFINE_TYPE_WITH_CODE (FldSwitcher, fld_switcher, GTK_TYPE_BIN,
                         G_ADD_PRIVATE (FldSwitcher))

static GParamSpec *properties [N_PROPS];

static void
add_child (GtkWidget   *child,
           FldSwitcher *self);

static GtkStack *
fld_switcher_real_get_stack (FldSwitcher *self)
{
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  g_assert (FLD_IS_SWITCHER (self));

  return priv->stack;
}

static void
on_child_updated (GtkWidget   *child,
                  GParamSpec  *pspec,
                  FldSwitcher *self)
{
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  GValue pos = G_VALUE_INIT;
  g_value_init (&pos, G_TYPE_INT);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "position", &pos);
  gint position = g_value_get_int (&pos);

  g_menu_remove (priv->menu, position);
  add_child (child, self);
}

static void
add_child (GtkWidget   *child,
           FldSwitcher *self)
{
  g_assert (FLD_IS_SWITCHER (self));

  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  GValue title = G_VALUE_INIT;
  g_value_init (&title, G_TYPE_STRING);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "title", &title);
  const gchar *label = g_value_get_string (&title);

  GValue name = G_VALUE_INIT;
  g_value_init (&name, G_TYPE_STRING);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "name", &name);
  const gchar *target = g_value_get_string (&name);

  GValue pos = G_VALUE_INIT;
  g_value_init (&pos, G_TYPE_INT);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "position", &pos);
  gint position = g_value_get_int (&pos);

  g_menu_insert (priv->menu, position, label, g_strdup_printf ("switch.page::%s", target));

  g_signal_connect (child, "child-notify::title",
                    G_CALLBACK (on_child_updated), self);
  g_signal_connect (child, "child-notify::needs-attention",
                    G_CALLBACK (on_child_updated), self);
  g_signal_connect (child, "notify::visible",
                    G_CALLBACK (on_child_updated), self);
  g_signal_connect (child, "child-notify::position",
                    G_CALLBACK (on_child_updated), self);
}

static void
remove_child (GtkWidget   *child,
              FldSwitcher *self)
{
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  GValue pos = G_VALUE_INIT;
  g_value_init (&pos, G_TYPE_INT);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "position", &pos);
  gint position = g_value_get_int (&pos);

  g_signal_handlers_disconnect_by_func (child, on_child_updated, self);

  g_menu_remove (priv->menu, position);
}

static void
on_child_changed (GtkWidget       *widget,
                  GParamSpec      *pspec,
                  FldSwitcher     *self)
{
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  const gchar* target = gtk_stack_get_visible_child_name (GTK_STACK (priv->stack));

  g_action_change_state (G_ACTION (priv->action), g_variant_new_string (target));
}

static void
on_stack_child_added (GtkContainer    *container,
                      GtkWidget       *widget,
                      FldSwitcher     *self)
{
  add_child (widget, self);
}

static void
on_stack_child_removed (GtkContainer    *container,
                        GtkWidget       *widget,
                        FldSwitcher     *self)
{
  remove_child (widget, self);
}

static void
disconnect_stack_signals (FldSwitcher *self)
{
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  g_signal_handlers_disconnect_by_func (priv->stack, on_stack_child_added, self);
  g_signal_handlers_disconnect_by_func (priv->stack, on_stack_child_removed, self);
  g_signal_handlers_disconnect_by_func (priv->stack, on_child_changed, self);
  g_signal_handlers_disconnect_by_func (priv->stack, disconnect_stack_signals, self);
}

static void
connect_stack_signals (FldSwitcher *self)
{
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);
  g_signal_connect_after (priv->stack, "add",
                          G_CALLBACK (on_stack_child_added), self);
  g_signal_connect_after (priv->stack, "remove",
                          G_CALLBACK (on_stack_child_removed), self);
  g_signal_connect (priv->stack, "notify::visible-child",
                    G_CALLBACK (on_child_changed), self);
  g_signal_connect_swapped (priv->stack, "destroy",
                            G_CALLBACK (disconnect_stack_signals), self);
}

static void
fld_switcher_real_set_stack (FldSwitcher *self,
                             GtkStack    *stack)
{
  g_assert (FLD_IS_SWITCHER (self));
  g_assert (GTK_IS_STACK (stack));

  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);
  priv->stack = stack;

  gtk_stack_switcher_set_stack (GTK_STACK_SWITCHER (priv->switcher), stack);
  connect_stack_signals (self);

  gtk_container_foreach (GTK_CONTAINER (priv->stack), (GtkCallback)add_child, self);
}

static void
fld_switcher_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  FldSwitcher *self = FLD_SWITCHER (object);

  switch (prop_id)
    {
    case PROP_STACK:
      g_value_set_object (value, fld_switcher_get_stack (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
fld_switcher_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  FldSwitcher *self = FLD_SWITCHER (object);

  switch (prop_id)
    {
    case PROP_STACK:
      fld_switcher_set_stack (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
fld_switcher_get_preferred_width (GtkWidget *widget,
                                  gint      *minimum_width,
                                  gint      *natural_width)
{
  FldSwitcher *self = FLD_SWITCHER (widget);
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  gtk_widget_get_preferred_width (priv->switcher, NULL, natural_width);
  gtk_widget_get_preferred_width (priv->selector, minimum_width, NULL);
}

static void
fld_switcher_size_allocate (GtkWidget     *widget,
                            GtkAllocation *alloc)
{
  FldSwitcher *self = FLD_SWITCHER (widget);
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  int width = 0;
  gtk_widget_get_preferred_width (priv->switcher, NULL, &width);

  GtkWidget *curr = gtk_bin_get_child (GTK_BIN (self));

  if (width <= alloc->width)
    {
      if (!GTK_IS_STACK_SWITCHER (curr))
        {
          gtk_container_remove (GTK_CONTAINER (self), curr);
          gtk_widget_set_visible (priv->switcher, TRUE);
          gtk_container_add (GTK_CONTAINER (self), priv->switcher);
        }
    }
  else
    {
      if (!GTK_IS_MENU_BUTTON (curr))
        {
          gtk_container_remove (GTK_CONTAINER (self), curr);
          gtk_widget_set_visible (priv->selector, TRUE);
          gtk_container_add (GTK_CONTAINER (self), priv->selector);
        }
    }
  GTK_WIDGET_CLASS (fld_switcher_parent_class)->size_allocate (widget, alloc);
}

static void
fld_switcher_class_init (FldSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = fld_switcher_get_property;
  object_class->set_property = fld_switcher_set_property;

  widget_class->size_allocate = fld_switcher_size_allocate;
  widget_class->get_preferred_width = fld_switcher_get_preferred_width;

  klass->get_stack = fld_switcher_real_get_stack;
  klass->set_stack = fld_switcher_real_set_stack;

  /**
   * FldSwitcher:stack:
   *
   * The #GtkStack controlled by the #FldSwitcher
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
on_state_changed (GSimpleAction *act,
                  GVariant      *param,
                  FldSwitcher   *self)
{
  g_return_if_fail (FLD_IS_SWITCHER (self));
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);

  const gchar *id = g_variant_get_string (param, NULL);
  GtkWidget *child = gtk_stack_get_child_by_name (GTK_STACK (priv->stack), id);

  GValue title = G_VALUE_INIT;
  g_value_init (&title, G_TYPE_STRING);
  gtk_container_child_get_property (GTK_CONTAINER (priv->stack), child, "title", &title);
  const gchar *label = g_value_get_string (&title);

  gtk_label_set_label (GTK_LABEL (priv->label), label);

  gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), id);

  g_simple_action_set_state (act, param);
}

static void
fld_switcher_init (FldSwitcher *self)
{
  FldSwitcherPrivate *priv = fld_switcher_get_instance_private (self);
  priv->stack = NULL;

  priv->switcher = GTK_WIDGET (g_object_ref (G_OBJECT (gtk_stack_switcher_new ())));
  gtk_widget_set_visible (priv->switcher, TRUE);
  gtk_container_add (GTK_CONTAINER (self), priv->switcher);

  priv->selector = GTK_WIDGET (g_object_ref (G_OBJECT (gtk_menu_button_new ())));
  gtk_button_set_relief (GTK_BUTTON (priv->selector), GTK_RELIEF_NONE);
  gtk_widget_set_halign (GTK_WIDGET (priv->selector), GTK_ALIGN_CENTER);

  GtkWidget *con = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  priv->label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (con), priv->label, FALSE, FALSE, 0);
  GtkWidget *img = gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_MENU);
  gtk_box_pack_end (GTK_BOX (con), img, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (priv->selector), con);
  gtk_widget_show_all (con);

  GSimpleActionGroup *group = g_simple_action_group_new ();

  priv->action = g_simple_action_new_stateful ("page",
                                               G_VARIANT_TYPE_STRING,
                                               g_variant_new_string (""));
  g_action_map_add_action (G_ACTION_MAP (group), G_ACTION (priv->action));
  g_signal_connect (G_OBJECT (priv->action), "change-state", G_CALLBACK (on_state_changed), self);
  gtk_widget_insert_action_group (priv->selector, "switch", G_ACTION_GROUP (group));

  priv->menu = g_menu_new ();
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (priv->selector), G_MENU_MODEL (priv->menu));
}

/**
 * fld_switcher_get_stack:
 * @self: a #FldSwitcher
 *
 * Returns: (transfer none): The #GtkStack controlled by the #FldSwitcher or %NULL
 * Since: 0.1.0
 */
GtkStack *
fld_switcher_get_stack (FldSwitcher *self)
{
  g_return_val_if_fail (FLD_IS_SWITCHER (self), FALSE);

  return FLD_SWITCHER_GET_CLASS (self)->get_stack (self);
}

/**
 * fld_switcher_set_stack:
 * @self: a #FldSwitcher
 * @stack: The #GtkStack controlled by the #FldSwitcher
 *
 * Since: 0.1.0
 */
void
fld_switcher_set_stack (FldSwitcher *self,
                        GtkStack    *stack)
{
  g_return_if_fail (FLD_IS_SWITCHER (self));

  FLD_SWITCHER_GET_CLASS (self)->set_stack (self, stack);

  g_object_notify (G_OBJECT (self), "stack");
}

