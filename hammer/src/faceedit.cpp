#pragma once

#include "faceedit.h"

// Set by module.h
JustifyTextureFunc f_JustifyTextureOriginal = NULL;

void SetOriginalCallback(JustifyTextureFunc callback) {
    f_JustifyTextureOriginal = callback;
}

// Called by module.h
void JustifyTexturePatched(void* pFace, TextureJustify_t justifyMode, Extents_t extents) {
    printf("JustifyTexture called! mode=%i bounds=[\n", justifyMode);
    for (int i = 0; i < 6; i ++) {
        auto ex = extents[i];
        printf("  %2.f %2.f %2.f\n", ex.x, ex.y, ex.z);
    }
    printf("]\n");

    // ...
    f_JustifyTextureOriginal(pFace, justifyMode, extents);
}
