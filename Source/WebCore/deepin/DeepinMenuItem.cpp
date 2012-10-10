#include "config.h"
#include "DeepinMenuItem.h"
#include "stdio.h"
#include <CString.h>

namespace WebCore {
    DeepinMenuItem::DeepinMenuItem(unsigned short id, const String& title, const String& iconURL) 
        : m_title(title),
        m_iconURL(iconURL),
        m_id(id),
        m_type(0)
    {
    }
    DeepinMenuItem::DeepinMenuItem(unsigned short id, DeepinMenu* menu)
        : m_id(id),
        m_type(1),
        m_submenu(menu)
    {
    }
}
