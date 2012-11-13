#include "config.h"
#include "DeepinMenuItem.h"
#include "stdio.h"
#include <CString.h>

namespace WebCore {
    DeepinMenuItem::DeepinMenuItem(const String& title, unsigned short id)
        : m_title(title),
        m_id(id),
        m_type(0)
    {
        printf("deepinmenuitem with normal .item...\n");
    }
    DeepinMenuItem::DeepinMenuItem(const String& title, DeepinMenu* menu)
        : m_title(title),
        m_type(1),
        m_submenu(menu)
    {
        printf("deepinmenuitem with submenu\n");
    }
}
