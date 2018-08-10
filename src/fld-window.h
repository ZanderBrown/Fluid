#ifndef __FLDWINDOW_H__
#define __FLDWINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define FLD_TYPE_APPLICATION_WINDOW (fld_application_window_get_type())
G_DECLARE_DERIVABLE_TYPE (FldApplicationWindow, fld_application_window, FLD, APPLICATION_WINDOW, GtkApplicationWindow)

struct _FldApplicationWindowClass
{
  GtkApplicationWindowClass parent_class;

  gboolean (*get_compact) (FldApplicationWindow *self);
};

GtkWidget *fld_application_window_new ();
gboolean   fld_application_window_get_compact (FldApplicationWindow *self);

G_END_DECLS

#endif /* __FLDWINDOW_H__ */
