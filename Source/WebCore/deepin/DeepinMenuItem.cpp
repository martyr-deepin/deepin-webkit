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
    DeepinMenuItem::DeepinMenuItem(const String& title, unsigned short id)
        : m_title(title),
        m_id(id),
        m_type(0)
    {
    }
    DeepinMenuItem::DeepinMenuItem(const String& title, DeepinMenu* menu)
        : m_title(title),
        m_type(1),
        m_submenu(menu)
    {
        m_submenu->ref();
    }
}
