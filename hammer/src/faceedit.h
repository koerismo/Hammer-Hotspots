#pragma once

#include "interfaces.h"
#include "print.h"

typedef Vector Extents_t[6];

enum class TextureJustify_t {
	JustifyNone = 0,
	JustifyTop,
	JustifyBottom,
	JustifyLeft,
	JustifyCenter,
	JustifyRight,
	JustifyFit,
	JustifyMax,
};

/// `(void* pFace, TextureJustify_t justifyMode, Extents_t extents)`
typedef void (*JustifyTextureFunc)(Face_t*, TextureJustify_t, Extents_t);

// This is not hooked, we just use it to update the texture coords when hotspotting.
typedef void (*CalcTextureCoordsFunc)(Face_t*);

// Set by module.h
void SetOriginalCallbacks(JustifyTextureFunc cb_justifyTexture, CalcTextureCoordsFunc cb_calcCoords);

// Called by module.h
void JustifyTexturePatched(Face_t* pFace, TextureJustify_t justifyMode, Extents_t extents);
