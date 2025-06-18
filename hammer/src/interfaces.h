#pragma once

#include <stddef.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

struct Vector2 {
    float x, y;
};

struct Vector {
    float x, y, z;
};

struct Vector4 {
    float x, y, z, w;
};

// sizeof(Texture_t) = 312
struct Texture_t {
    char        path[260];
    Vector4     axisU;
    Vector4     axisV;
    float       rotation;
    float       scaleX;
    float       scaleY;
    uint8       __unknown1;
    uint8       __unknown2;
    int         lightmapScale;
};

// Referenced from Alien Swarm SDK
class Material_t {
    virtual const char* GetName() const = 0;
    virtual const char* GetTextureGroupName() const = 0;
    // TODO: Finish transcribing these!
};

// Referenced from Kisak-Strike
// class EditorTexture_t {
// public:
//     virtual ~EditorTexture_t(void) {}
//     virtual int GetPreviewImageWidth() const = 0;
//     virtual int GetPreviewImageHeight() const = 0;
//     virtual int GetWidth() const = 0;
//     virtual int GetHeight() const = 0;
//     virtual int GetMappingWidth() const = 0;
//     virtual int GetMappingHeight() const = 0; // Verified in P2!
//     virtual float GetDecalScale() const = 0;

//     virtual const char* GetName() const = 0;
//     virtual int GetShortName( char* szShortName ) const = 0;
//     virtual int GetKeywords( char* szKeywords ) const = 0;
//     virtual void Draw(void* thisHasALotOfArgsAndIAmNeverGoingToUseIt) const = 0;
//     virtual int GetTextureFormat() const = 0;
//     virtual int GetSurfaceAttributes() const = 0;
//     virtual int GetSurfaceContents() const = 0;
//     virtual int GetSurfaceValue() const = 0;
//     virtual void* GetPalette() const = 0;
//     virtual bool HasData() const = 0;
//     virtual bool HasPalette() const = 0;
//     virtual bool Load() const = 0;
//     virtual void Reload() const = 0;
//     virtual bool IsLoaded() const = 0;
//     virtual const char* GetFileName() const = 0;
//     virtual bool IsWater() const = 0;

//     virtual int GetImageDataRGB(void* pData = NULL) = 0;
//     virtual int GetImageDataRGBA(void* pData = NULL) = 0;

//     virtual bool HasAlpha() const = 0;
//     virtual bool IsDummy() const = 0;
//     virtual int GetTextureID() const = 0;
//     virtual void SetTextureID(int texId) = 0;
//     virtual Material_t* GetMaterial(bool forceLoad=true) = 0;
// };

struct EditorTexture_t {
    char path[260];
    char __unknown0[268];
    uint32 textureMappingWidth;
    uint32 textureMappingHeight;
    char __unknown1[1];
    bool isLoaded;
    uint8 __unknown2;
    char __unknown3[37];
    void* pMaterial;
};

struct Face_t {
    #if GAME_P2CE
    // P2CE: 96 bytes of mystery padding, RE is my passion
    uint8       __unknown0[96];             // 96 - 0x00
    #else
    // P2: Only 20 bytes of mystery padding! Wowza!
    uint8       __unknown0[20];             // 20 - 0x00
    #endif

    Texture_t   texture;                    // 312
    Vector*     verts;                      // 8*
    int         vertCount;                  // 4

    // TODO: 8 bytes here need to GO
    float       __mysteryBytesAgain[2];     // 8
    
    // Plane
    Vector      planeNormal;                // 12
    float       planeDist;                  // 4
    Vector      planePoints[3];             // 36

    int         flags;                      // 4
    uint8       __unknown3;                 // 1
    void*       __unknown2;                 // 8*

    #if GAME_P2
    uint8       __mysteryPaddingTheSequel[11];  // 1 (this is to make editorTexture correct)
    #endif

    int                 faceId;             // 4
    EditorTexture_t*    editorTexture;      // 8 - 0x1A0 (+416) in Portal 2
    uint16              dispHandle;         // 2
    Vector2             *textureCoords;     // 8*
    Vector2             *lightmapCoords;    // 8*
    

    bool        isCordonFace : 1;
    bool        ignoreLighting : 1;

    void*       tangents;
    uint32      smoothGroups;
};
