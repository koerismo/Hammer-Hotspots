#include "faceedit.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "hotspot.h"

#define DEFAULT_TEXTURE_SCALE 0.25f

// Set by module.h
JustifyTextureFunc f_JustifyTextureOriginal = NULL;
CalcTextureCoordsFunc f_CalcTextureCoords = NULL;

/// This is a helper function just so we can prototype before having the Face struct 100% compliant
EditorTexture_t* GetFaceEditorTexture(Face_t* pFace) {
    #if GAME_P2CE
    return *(EditorTexture_t**)((uint8*)pFace + 0x200);
    #else
    return *(EditorTexture_t**)((uint8*)pFace + 0x1A0);
    #endif
}

void SetOriginalCallbacks(JustifyTextureFunc cb_justifyTexture, CalcTextureCoordsFunc cb_calcCoords) {
    f_JustifyTextureOriginal = cb_justifyTexture;
    f_CalcTextureCoords = cb_calcCoords;
}

Vector Sub(Vector a, Vector b) { return { a.x-b.x, a.y-b.y, a.z-b.z }; }
Vector2 Sub(Vector2 a, Vector2 b) { return { a.x-b.x, a.y-b.y }; }

float Dot(Vector a, Vector b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float Dot(Vector2 a, Vector2 b) { return a.x * b.x + a.y * b.y; }

void GetTextureBounds(Texture_t &texture, Extents_t extents, Vector2 &mins, Vector2 &maxs) {
    bool zero = true;
    for (int i=0; i<6; i++) {
        float x = Dot(extents[i], *reinterpret_cast<Vector*>(&texture.axisU)) / texture.scaleX;
        float y = Dot(extents[i], *reinterpret_cast<Vector*>(&texture.axisV)) / texture.scaleY;
        if (x < mins.x || zero) { mins.x = x; }
        if (y < mins.y || zero) { mins.y = y; }
        if (x > maxs.x || zero) { maxs.x = x; }
        if (y > maxs.y || zero) { maxs.y = y; }
        zero = false;
    }
}

void FitTextureToRect(Face_t* pFace, Extents_t extents, const Vector2 &uvMins, const Vector2 &uvInvScale) {
    pFace->texture.scaleX = 1.0;
    pFace->texture.scaleY = 1.0;

    Vector2 mins, maxs;
    GetTextureBounds(pFace->texture, extents, mins, maxs);

    EditorTexture_t* pTex = GetFaceEditorTexture(pFace);
    if (!pTex) {
        DebugPrintF("Editor texture is nullptr!?\n");
        pFace->texture.scaleX = 0.25; // (maxs.x - mins.x) / 512.0f;
        pFace->texture.scaleY = 0.25; // (maxs.y - mins.y) / 512.0f;
        return;
    }

    Vector2 textureSize {
        static_cast<float>(pTex->textureMappingWidth),
        static_cast<float>(pTex->textureMappingHeight)
    };

    if (pTex->textureMappingWidth && pTex->textureMappingHeight) {
        pFace->texture.scaleX = (maxs.x - mins.x) / textureSize.x * uvInvScale.x;
        pFace->texture.scaleY = (maxs.y - mins.y) / textureSize.y * uvInvScale.y;
    }

    // Recalculate bounds after scaling
    GetTextureBounds(pFace->texture, extents, mins, maxs);

    // Align to top-left of UV rect
    pFace->texture.axisU.w = -mins.x + textureSize.x * uvMins.x;
    pFace->texture.axisV.w = -mins.y + textureSize.y * uvMins.y;
}

// masm syntax debug:
// bp hamspot_x64!JustifyTexturePatched

// Called by module.h
void JustifyTexturePatched(Face_t* pFace, TextureJustify_t justifyMode, Extents_t extents) {
    if (!pFace) return;

    if (justifyMode != TextureJustify_t::JustifyFit) {
        return f_JustifyTextureOriginal(pFace, justifyMode, extents);
    }

    DebugPrintF("JustifyTexture called! texture=\"%s\"\n", pFace->texture.path, justifyMode);
    // DebugPrintF("bounds=[\n");
    // for (int i = 0; i < 6; i ++) {
    //     auto ex = extents[i];
    //     DebugPrintF("  %.1f %.1f %.1f\n", ex.x, ex.y, ex.z);
    // }
    // DebugPrintF("]\n");

    // TODO: Figure out actually correct offset for editor texture pointer
    EditorTexture_t* eTex = GetFaceEditorTexture(pFace);
    if (!eTex) {
        DebugPrintF("Texture info is nullptr! This will disable scaling math.\n");
    }

    // else {
    //     DebugPrintF("Texture pointer is 0x%x\n", eTex);
    //     DebugPrintF("Texture dimensions: %ix%i\n", eTex->textureMappingWidth, eTex->textureMappingHeight);
    // }

    // TODO: Load and cache the rectheader from the selected material!
    auto tempFile = new HotSpot::RectHeader;
    tempFile->texSize = { 512, 512 };
    tempFile->rects.reserve(7);
    tempFile->rects.push_back(HotSpot::CreateRect({ 0, 0 }, { 256, 256 }));
    tempFile->rects.push_back(HotSpot::CreateRect({ 256, 0 }, { 384, 256 }));
    tempFile->rects.push_back(HotSpot::CreateRect({ 384, 0 }, { 448, 256 }));
    tempFile->rects.push_back(HotSpot::CreateRect({ 480, 0 }, { 496, 256 }));
    tempFile->rects.push_back(HotSpot::CreateRect({ 496, 0 }, { 512, 256 }));
    tempFile->rects.push_back(HotSpot::CreateRect({ 0, 256 }, { 512, 512 }));

    // Reset scaling so that GetTextureBounds works
    pFace->texture.scaleX = 1.0;
    pFace->texture.scaleY = 1.0;
    
    // Get aspect ratio of surface
    Vector2 surfMins, surfMaxs;
    GetTextureBounds(pFace->texture, extents, surfMins, surfMaxs);
    Vector2 surfSize = Sub(surfMaxs, surfMins);

    float targetScale = max(surfSize.x, surfSize.y) / DEFAULT_TEXTURE_SCALE;
    float targetAspect = surfSize.x / surfSize.y;

    DebugPrintF("Using aspect ratio of %2.2f (%2.2fx%2.2f)\n", targetAspect, surfSize.x, surfSize.y);

    // int rectIndex = 1;

    // bool rot90 = false;
    // float rectDiffA, rectDiffB;
    // int rectIndex     = HotSpot::MatchRandomBestRect(tempFile, aspect, &rectDiffA);
    // int rectIndexVert = HotSpot::MatchRandomBestRect(tempFile, 1/aspect, &rectDiffB);
    // if (rectDiffB < rectDiffA) rectIndex = rectIndexVert, rot90 = true;

    float rectDiff = -1;
    bool isRotated = false;
    int rectIndex = HotSpot::MatchRandomBestRect(tempFile, targetAspect, targetScale, &isRotated, &rectDiff);

    DebugPrintF("Chose rectangle %i (rotated=%i) with a diff of %.2f\n", rectIndex, isRotated, rectDiff);

    // Cancel everything if we can't match a rect. This means something has gone wrong.
    if (rectIndex == -1) return f_JustifyTextureOriginal(pFace, justifyMode, extents);

    // TODO: This doesn't work??
    // If we matched a rect that needs rotation, rotate the texture before fitting.
    // if (isRotated) {
    //     if (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) < 0.5)
    //         pFace->texture.rotation -= 90;
    //     else
    //         pFace->texture.rotation += 90;
    // }

    Vector2 uvMins, uvInvScale;
    HotSpot::GetOffsetAndInvScale(tempFile, rectIndex, &uvMins, &uvInvScale);

    // Do texture fit
    FitTextureToRect(pFace, extents, uvMins, uvInvScale);

    // Recalculate coords
    f_CalcTextureCoords(pFace);
}
