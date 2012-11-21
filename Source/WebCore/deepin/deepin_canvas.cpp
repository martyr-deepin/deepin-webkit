#include "config.h"
#include "HTMLCanvasElement.h"
#include "JSHTMLCanvasElement.h"
#include "ImageBuffer.h"
#include "GraphicsContext.h"
#include "PlatformContextCairo.h"
#include "FloatRect.h"
#include "JSValue.h"
#include "JSBase.h"
#include "APICast.h"
#include "CallFrame.h"
#include "APIShims.h"

#include <cairo.h>

extern "C" {
    using namespace WebCore;
    const cairo_user_data_key_t* _deepin_cairo_custom_key = static_cast<cairo_user_data_key_t*>(GINT_TO_POINTER(1032149));
    cairo_t* fetch_cairo_from_html_canvas(JSContextRef ctx, JSValueRef v)
    {
        JSC::ExecState* exec = toJS(ctx);
        JSC::APIEntryShim  entryShim(exec);
        HTMLCanvasElement* canvas = toHTMLCanvasElement(toJS(exec, v));
        if (canvas == NULL)
            return NULL;

        ImageBuffer* buf = canvas->buffer();
        if (buf == NULL)
            return NULL;
        GraphicsContext* gc = canvas->drawingContext();
        if (gc == NULL)
            return NULL;

        cairo_t *cr = gc->platformContext()->cr();
        if (cr != NULL) {
            cairo_reference(cr);
            canvas->ref();
            cairo_set_user_data(cr, _deepin_cairo_custom_key, canvas, NULL);
        }
        return cr;
    }

    void canvas_custom_draw_did(cairo_t *cr, const cairo_rectangle_t* rect)
    {
        if (cr == NULL)
            return;
        HTMLCanvasElement* canvas = static_cast<HTMLCanvasElement*>(cairo_get_user_data(cr, _deepin_cairo_custom_key));
        if (canvas == NULL)
            return;

        cairo_destroy(cr);

        if ( rect != NULL) {
            FloatRect f(*rect);
            canvas->didDraw(f);
        } else {
            FloatRect f(0, 0, canvas->width(), canvas->height());
            canvas->didDraw(f);
        }
        canvas->deref();
    }
}
