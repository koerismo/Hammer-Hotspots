#define _USE_MATH_DEFINES
#include <math.h>
#include "faceedit.h"

// Set by module.h
JustifyTextureFunc f_JustifyTextureOriginal = NULL;
CalcTextureCoordsFunc f_CalcTextureCoords = NULL;

EditorTexture_t* GetFaceEditorTexture(Face_t* pFace) {
    return *(reinterpret_cast<EditorTexture_t**>(pFace + 0x1A0)); // TODO: This is stupid
}

void SetOriginalCallbacks(JustifyTextureFunc cb_justifyTexture, CalcTextureCoordsFunc cb_calcCoords) {
    f_JustifyTextureOriginal = cb_justifyTexture;
    f_CalcTextureCoords = cb_calcCoords;
}

// Called by module.h
void JustifyTexturePatched(Face_t* pFace, TextureJustify_t justifyMode, Extents_t extents) {
    DebugPrintF("Call hijacked!\n");

    if (justifyMode != TextureJustify_t::JustifyFit) {
        return f_JustifyTextureOriginal(pFace, justifyMode, extents);
    }

    DebugPrintF("Calling custom things...\n");

    DebugPrintF("JustifyTexture called! texture=\"%s\" bounds=[\n", pFace->texture.path, justifyMode);
    for (int i = 0; i < 6; i ++) {
        auto ex = extents[i];
        DebugPrintF("  %2.f %2.f %2.f\n", ex.x, ex.y, ex.z);
    }
    DebugPrintF("]\n");

    EditorTexture_t* eTex = GetFaceEditorTexture(pFace);

    // if (etex) {
    //     DebugPrintF("TEX INFO: %ix%i %s\n", etex->GetWidth(), etex->GetHeight(), etex->GetName());
    // }
    // else {
    //     DebugPrintF("No texinfo :(\n");
    // }

    // get scaled idiot
    f_JustifyTextureOriginal(pFace, TextureJustify_t::JustifyFit, extents);
    pFace->texture.scaleX *= 0.25;
    pFace->texture.scaleY *= 0.25;

    // Recalculate coords
    f_CalcTextureCoords(pFace);

    // ...
    // f_JustifyTextureOriginal(pFace, justifyMode, extents);
}
