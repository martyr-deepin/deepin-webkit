#include "config.h"
#include "ContextMenu.h"

#include "DeepinMenu.h"
#include "DeepinMenuItem.h"
#include "ContextMenuItem.h"

#include <PassOwnPtr.h>
#include <Vector.h>

#include "CString.h"

namespace WebCore {
    DeepinMenu::DeepinMenu()
    {
    }
    PassOwnPtr<ContextMenu> DeepinMenu::contextMenu() 
    {
        ContextMenu *m = new ContextMenu();
        PassOwnPtr<ContextMenu> tmp_menu = adoptPtr(m);

        for (Vector<DeepinMenuItem*>::iterator itr=m_items.begin(); itr!=m_items.end(); ++itr) {
            switch ((*itr)->type()) {
                case 0:
                    {
                        ContextMenuItem menuitem(ActionType, 
                                static_cast<ContextMenuAction>(ContextMenuItemBaseApplicationTag+(*itr)->id()), 
                                (*itr)->title());
                        menuitem.setEnabled((*itr)->enabled());
                        tmp_menu->appendItem(menuitem);
                        break;
                    }
                case 1:
                    {
                        ContextMenuItem menuitem(SubmenuType, static_cast<ContextMenuAction>(0), (*itr)->title(), (*itr)->submenu()->contextMenu().get());
                        menuitem.setEnabled((*itr)->enabled());
                        tmp_menu->appendItem(menuitem);
                        break;
                    }
                case 2:
                    {
                        ContextMenuItem menuitem(SeparatorType, ContextMenuItemTagNoAction, String());
                        tmp_menu->appendItem(menuitem);
                        break;
                    }
            }

        }
        return tmp_menu;
    }

    void DeepinMenu::appendItem(DeepinMenuItem* item)
    {
        //FIXME: deref when remove;
        item->ref();
        m_items.append(item);
    }
    void DeepinMenu::insertItem(size_t position, DeepinMenuItem* item)
    {
    }
}

