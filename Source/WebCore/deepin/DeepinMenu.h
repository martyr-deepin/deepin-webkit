#ifndef __DEEPIN_MENU__
#define __DEEPIN_MENU__

#include <RefCounted.h>
#include <PassRefPtr.h>

#include "ContextMenu.h"

namespace WebCore {
    class ContextMenu;
    class DeepinMenuItem;
    class DeepinMenu : public RefCounted<DeepinMenu> {
        public:
            static PassRefPtr<DeepinMenu> create() {
                return adoptRef(new DeepinMenu());
            }
            void appendItem(DeepinMenuItem* item);
            void insertItem(size_t position, DeepinMenuItem* item);

            //void populateContextMenu(ContextMenu*);
            //void contextMenuItemSelected(ContextMenuItem*);
            //void contextMenuCleared();
            PassOwnPtr<ContextMenu> contextMenu();

        private:
            DeepinMenu();
            Vector<DeepinMenuItem*> m_items;
    };
}


#endif
