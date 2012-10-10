#ifndef _ITEMSELECTEDEVENT_h_
#define _ITEMSELECTEDEVENT_h_

#include "config.h"
#include "Event.h"
#include <CString.h>

namespace WebCore {
    class ItemSelectedEvent : public Event {
        public:
            static PassRefPtr<ItemSelectedEvent> create() {
                return adoptRef(new ItemSelectedEvent());
            }

            const AtomicString& title() { return m_title; }
            void setTitle(const AtomicString& title) { m_title = title; }
            void setID(int id) { m_id = id; }
            int id() { return m_id; }

            virtual const AtomicString& interfaceName() const { return eventNames().interfaceForItemSelectedEvent; }


        private:
            ItemSelectedEvent();
            AtomicString m_title;
            int m_id;
    };
}

#endif
