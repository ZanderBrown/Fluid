#ifndef __FLDSTACKMENU_H__
#define __FLDSTACKMENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define FLD_TYPE_STACK_MENU (fld_stack_menu_get_type())
G_DECLARE_DERIVABLE_TYPE (FldStackMenu, fld_stack_menu, FLD, STACK_MENU, GMenuModel)

struct _FldStackMenuClass
{
  GMenuModelClass parent_class;

  GtkStack    *(*get_stack) (FldStackMenu *self);
  void         (*set_stack) (FldStackMenu *self,
                             GtkStack     *stack);
};

FldStackMenu *fld_stack_menu_new ();
GtkStack     *fld_stack_menu_get_stack (FldStackMenu *self);
void          fld_stack_menu_set_stack (FldStackMenu *self,
                                        GtkStack     *stack);

G_END_DECLS

#endif /* __FLDSTACKMENU_H__ */
