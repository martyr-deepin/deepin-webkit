#include "config.h"
#include "ContextMenu.h"

#include "DeepinMenu.h"
#include "DeepinMenuItem.h"
#include "ContextMenuItem.h"

#include <PassOwnPtr.h>
#include <Vector.h>

#include "CString.h"

#include <stdio.h>

namespace WebCore {
    DeepinMenu::DeepinMenu()
    {
    }
    PassOwnPtr<ContextMenu> DeepinMenu::contextMenu() 
    {
        ContextMenu *m = new ContextMenu();
        PassOwnPtr<ContextMenu> tmp_menu = adoptPtr(m);
        for (Vector<DeepinMenuItem*>::iterator itr=m_items.begin(); itr!=m_items.end(); ++itr) {
            ContextMenuItem menuitem(ActionType, 
                    static_cast<ContextMenuAction>(ContextMenuItemBaseApplicationTag+(*itr)->id()), 
                    (*itr)->title());
            tmp_menu->appendItem(menuitem);
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
