#ifndef __FLDSWITCHER_H__
#define __FLDSWITCHER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define FLD_TYPE_SWITCHER (fld_switcher_get_type())
G_DECLARE_DERIVABLE_TYPE (FldSwitcher, fld_switcher, FLD, SWITCHER, GtkBin)

struct _FldSwitcherClass
{
  GtkBinClass parent_class;

  GtkStack  *(*get_stack) (FldSwitcher *self);
  void       (*set_stack) (FldSwitcher *self,
                           GtkStack    *stack);
};

GtkWidget *fld_switcher_new ();
GtkStack  *fld_switcher_get_stack (FldSwitcher *self);
void       fld_switcher_set_stack (FldSwitcher *self,
                                   GtkStack    *stack);

G_END_DECLS

#endif /* __FLDSWITCHER_H__ */
