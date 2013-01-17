#include "config.h"
#include "DeepinMenuItem.h"
#include "DeepinMenu.h"
#include <CString.h>

namespace WebCore {
    DeepinMenuItem::~DeepinMenuItem()
    {
        if (m_type == 1 && m_submenu != NULL) 
            m_submenu->deref();
    }
    DeepinMenuItem::DeepinMenuItem(int type, unsigned short id, const String& title, DeepinMenu* menu)
        : m_title(title),
        m_id(0),
        m_type(type),
        m_submenu(NULL),
        m_enabled(true)
    {
        switch (m_type) {
            case 0:
                m_id = id;
                break;
            case 1:
                m_submenu = menu;
                m_submenu->ref();
                break;
            case 2:
                break;
        }
    }
}
