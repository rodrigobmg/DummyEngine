
#include "Viewer.h"

#include <Ren/SW/SW.h>
#include <Sys/DynLib.h>

extern "C" DLL_EXPORT GameBase *CreateViewer(int w, int h, const char *local_dir) {
    return new Viewer(w, h, local_dir);
}

extern "C" DLL_EXPORT const void *GetRendererPixels(GameBase *viewer) {
    return swGetPixelDataRef(swGetCurFramebuffer());
}