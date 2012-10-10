#ifndef __DEEPIN_MENU_ITEM__
#define __DEEPIN_MENU_ITEM__

#include "PlatformString.h"
#include <RefCounted.h>
#include <PassRefPtr.h>

namespace WebCore {
    class DeepinMenu;

    class DeepinMenuItem : public RefCounted<DeepinMenuItem> {
        public:
            static PassRefPtr<DeepinMenuItem> create(unsigned short id, const String& title, const String& iconURL) {
                return adoptRef(new DeepinMenuItem(id, title, iconURL));
            }
            static PassRefPtr<DeepinMenuItem> create(unsigned short id, DeepinMenu* sub_menu) {
                return adoptRef(new DeepinMenuItem(id, sub_menu));
            }

            const String& title() { return m_title; }
            void setTitle(const String& t) { m_title = t; }

            const String& iconURL() { return m_iconURL; }
            void setIconURL(const String& url) { m_iconURL = url; }

            unsigned short id() { return m_id; }

            int type() { return m_type; }

        private:
            DeepinMenuItem(unsigned short id, const String& title, const String& iconURL);
            DeepinMenuItem(unsigned short id, DeepinMenu* menu);

            String m_title;
            String m_iconURL;
            unsigned short m_id;
            int m_type;
            DeepinMenu *m_submenu;
    };
}


#endif
