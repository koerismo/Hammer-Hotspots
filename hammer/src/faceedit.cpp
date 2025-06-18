#include "faceedit.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "hotspot.h"

#define DEFAULT_TEXTURE_SCALE 0.25

// Set by module.h
JustifyTextureFunc f_JustifyTextureOriginal = NULL;
CalcTextureCoordsFunc f_CalcTextureCoords = NULL;

EditorTexture_t* GetFaceEditorTexture(Face_t* pFace) {
    // TODO: This is stupid
    #if GAME_P2CE
    return *(reinterpret_cast<EditorTexture_t**>(pFace + 0x1A0));
    #else
    return *(reinterpret_cast<EditorTexture_t**>(pFace + 0x1A0));
    #endif
}

void SetOriginalCallbacks(JustifyTextureFunc cb_justifyTexture, CalcTextureCoordsFunc cb_calcCoords) {
    f_JustifyTextureOriginal = cb_justifyTexture;
    f_CalcTextureCoords = cb_calcCoords;
}

// masm syntax debug:
// bp hamspot_x64!JustifyTexturePatched

// Called by module.h
void JustifyTexturePatched(Face_t* pFace, TextureJustify_t justifyMode, Extents_t extents) {
    DebugPrintF("Call hijacked! ptr=%i mode=%i\n", static_cast<void*>(pFace), justifyMode);

    if (justifyMode != TextureJustify_t::JustifyFit) {
        return f_JustifyTextureOriginal(pFace, justifyMode, extents);
    }

    DebugPrintF("JustifyTexture called! texture=\"%s\" bounds=[\n", pFace->texture.path, justifyMode);
    for (int i = 0; i < 6; i ++) {
        auto ex = extents[i];
        DebugPrintF("  %2.f %2.f %2.f\n", ex.x, ex.y, ex.z);
    }
    DebugPrintF("]\n");

    // TODO: Figure out actually correct offset for editor texture pointer
    // EditorTexture_t* eTex = GetFaceEditorTexture(pFace);
    // if (eTex) {
    //     DebugPrintF("TEX INFO: %ix%i %s\n", eTex->GetWidth(), eTex->GetHeight(), eTex->GetName());
    // }
    // else {
    //     DebugPrintF("No texinfo :(\n");
    // }

    // Fit full texture
    f_JustifyTextureOriginal(pFace, TextureJustify_t::JustifyFit, extents);
    float topLeftX = pFace->texture.axisU.w;
    float topLeftY = pFace->texture.axisV.w;
    float texScaleX = pFace->texture.scaleX;
    float texScaleY = pFace->texture.scaleY;
    float aspect = texScaleX / texScaleY;

    auto tempFile = new HotSpot::RectHeader;
    tempFile->rects.resize(1);
    HotSpot::Rect* tempRect = &tempFile->rects[0];
    tempRect->mins = { 0, 0 };
    tempRect->maxs = { 0, 0 };

    // int rectIndex = 0;
    float rectDiffA, rectDiffB;
    int rectIndex     = HotSpot::MatchRandomBestRect(tempFile, aspect, &rectDiffA);
    int rectIndexVert = HotSpot::MatchRandomBestRect(tempFile, 1/aspect, &rectDiffB);
    if (rectDiffB < rectDiffA) rectIndex = rectIndexVert;
    
    float iScaleX, iScaleY;
    HotSpot::GetInvScaleFactor(tempFile, rectIndex, &iScaleX, &iScaleY);

    DebugPrintF("Top corner: %4.f %4.f\nScale: %4.f %4.f\n", topLeftX, topLeftY, texScaleX, texScaleY);
    pFace->texture.scaleX *= iScaleX;
    pFace->texture.scaleY *= iScaleY;

    // Recalculate coords
    // f_CalcTextureCoords(pFace);

    // ...
    // f_JustifyTextureOriginal(pFace, justifyMode, extents);
}
