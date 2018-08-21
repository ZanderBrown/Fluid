#include "fld-window.h"

/* Ideally this would be coming from GtkSettings or similar */
#define SNAP_AT 360
#define COMPACT_STYLE_CLASS "compact"

/**
 * SECTION:fld-application-window
 * @title: FldApplicationWindow
 * @short_description: An application window for fluid applications
 *
 * The #FldApplicationWindow class provides a #GtkApplicationWindow subclass
 * that makes it easier to build fluid applications.
 * 
 * When a window is compact it aquires the 'compact' css class that styles
 * may be scoped to aswell as emitting 'notify::compact' that widgets may
 * connect to
 *
 * Since: 0.1.0
 */

typedef struct
{
  gboolean is_compact;
} FldApplicationWindowPrivate;

enum {
  PROP_0,
  PROP_COMPACT,
  N_PROPS
};

G_DEFINE_TYPE_WITH_CODE (FldApplicationWindow, fld_application_window, GTK_TYPE_APPLICATION_WINDOW,
                         G_ADD_PRIVATE (FldApplicationWindow))

static GParamSpec *properties [N_PROPS];

static gboolean
fld_application_window_real_get_compact (FldApplicationWindow *self)
{
  FldApplicationWindowPrivate *priv = fld_application_window_get_instance_private (self);

  g_assert (FLD_IS_APPLICATION_WINDOW (self));

  return priv->is_compact;
}

static void
fld_application_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  FldApplicationWindow *self = FLD_APPLICATION_WINDOW (object);

  switch (prop_id)
    {
    case PROP_COMPACT:
      g_value_set_boolean (value, fld_application_window_get_compact (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
fld_application_window_size_allocate (GtkWidget     *widget,
                                      GtkAllocation *alloc)
{
  FldApplicationWindow *self = FLD_APPLICATION_WINDOW (widget);
  FldApplicationWindowPrivate *priv = fld_application_window_get_instance_private (self);
  
  gboolean was = priv->is_compact;
  
  priv->is_compact = alloc->width <= SNAP_AT;
  
  if (was != priv->is_compact)
    {
      if (priv->is_compact)
        gtk_style_context_add_class (gtk_widget_get_style_context (widget), COMPACT_STYLE_CLASS);
      else
        gtk_style_context_remove_class (gtk_widget_get_style_context (widget), COMPACT_STYLE_CLASS);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_COMPACT]);
    }

  GTK_WIDGET_CLASS (fld_application_window_parent_class)->size_allocate (widget, alloc);
}

static void
fld_application_window_class_init (FldApplicationWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  object_class->get_property = fld_application_window_get_property;

  widget_class->size_allocate = fld_application_window_size_allocate;

  klass->get_compact = fld_application_window_real_get_compact;

  /**
   * FldApplicationWindow:compact:
   *
   * The "compact" property denotes if the window is in the compact
   * state. Listen to this property to make changes neccisary for
   * the compact state, purly stylistic changes should be applied
   * through css by scoping to .compact
   */
  properties [PROP_COMPACT] =
    g_param_spec_boolean ("compact",
                          "Compact",
                          "If the window is compact",
                          FALSE,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
fld_application_window_init (FldApplicationWindow *self)
{
  FldApplicationWindowPrivate *priv = fld_application_window_get_instance_private (self);
  priv->is_compact = FALSE;
}

/**
 * fld_application_window_get_compact:
 * @self: a #FldApplicationWindow
 *
 * Gets if the window is in the compact state.
 *
 * Returns: %TRUE if @self is compact, otherwise %FALSE.
 *
 * Since: 0.1.0
 */
gboolean
fld_application_window_get_compact (FldApplicationWindow *self)
{
  g_return_val_if_fail (FLD_IS_APPLICATION_WINDOW (self), FALSE);

  return FLD_APPLICATION_WINDOW_GET_CLASS (self)->get_compact (self);
}

GtkWidget *
fld_application_window_new ()
{
  return g_object_new (FLD_TYPE_APPLICATION_WINDOW, NULL);
}

