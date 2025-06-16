#pragma once

#include <stdio.h>

struct Vector2 {
    float x, y;
};

struct Vector {
    float x, y, z;
};

struct Vector4 {
    float x, y, z, w;
};

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

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

struct Texture {
    char path[260];
    Vector4 axisU;
    Vector4 axisV;
    float rotation;
    float scaleX;
    float scaleY;
    uint8 __unknown1;
    uint8 __unknown2;
    int lightmapScale;
};

struct Face {
    Texture     texture;
    Vector*     verts;
    int         vertCount;
    
    // Plane
    Vector      planeNormal;
    float       __unknown1;
    Vector      planePoints;

    int         flags;
    void*       __unknown2;
    uint8       __unknown3;

    int         faceId;
    void*       editorTexture;
    uint16      dispHandle;
    Vector2     textureCoords;
    Vector2     lightmapCoords;

    bool        isCordonFace : 1;
    bool        ignoreLighting : 1;

    void*       tangents;
    uint32      smoothGroups;
};

/// @brief `(void* pFace, TextureJustify_t justifyMode, Extents_t extents)`
typedef int (*JustifyTextureFunc)(void*, TextureJustify_t, Extents_t);

// Set by module.h
void SetOriginalCallback(JustifyTextureFunc callback);

// Called by module.h
void JustifyTexturePatched(void* pFace, TextureJustify_t justifyMode, Extents_t extents);
