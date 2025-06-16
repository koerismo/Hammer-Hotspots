#define _USE_MATH_DEFINES
#include <math.h>
#include "faceedit.h"

// Set by module.h
JustifyTextureFunc f_JustifyTextureOriginal = NULL;
CalcTextureCoordsFunc f_CalcTextureCoords = NULL;

void SetOriginalCallbacks(JustifyTextureFunc cb_justifyTexture, CalcTextureCoordsFunc cb_calcCoords) {
    f_JustifyTextureOriginal = cb_justifyTexture;
    f_CalcTextureCoords = cb_calcCoords;
}

// Called by module.h
void JustifyTexturePatched(Face_t* pFace, TextureJustify_t justifyMode, Extents_t extents) {
    if (justifyMode != TextureJustify_t::JustifyFit) {
        return f_JustifyTextureOriginal(pFace, justifyMode, extents);
    }

    // get scaled idiot
    f_JustifyTextureOriginal(pFace, TextureJustify_t::JustifyFit, extents);
    pFace->texture.scaleX *= 0.25;
    pFace->texture.scaleY *= 0.25;

    // Recalculate coords
    f_CalcTextureCoords(pFace);

    DebugPrintF("JustifyTexture called! texture=\"%s\" mode=%i bounds=[\n", pFace->texture.path, justifyMode);
    for (int i = 0; i < 6; i ++) {
        auto ex = extents[i];
        DebugPrintF("  %2.f %2.f %2.f\n", ex.x, ex.y, ex.z);
    }
    DebugPrintF("]\n");

    // ...
    // f_JustifyTextureOriginal(pFace, justifyMode, extents);
}
