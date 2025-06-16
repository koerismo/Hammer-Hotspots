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

struct Face_t {
    // 96 bytes of mystery padding
    // RE is my passion
    uint8       __unknown0[96];

    Texture_t     texture;
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

// Referenced from Alien Swarm SDK
class Material_t {
    virtual const char* GetName() const = 0;
    virtual const char* GetTextureGroupName() const = 0;
    // TODO: Finish transcribing these!
};

// Referenced from Kisak-Strike
class EditorTexture_t {
    virtual int GetPreviewImageWidth() const = 0;
    virtual int GetPreviewImageHeight() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual int GetMappingWidth() const = 0;
    virtual int GetMappingHeight() const = 0;
    virtual float GetDecalScale() const = 0;

    virtual const char* GetName() const = 0;
    virtual int GetShortName( char* szShortName ) const = 0;
    virtual int GetKeywords( char* szKeywords ) const = 0;
    virtual void Draw(void* thisHasALotOfArgsAndIAmNeverGoingToUseIt) const = 0;
    virtual int GetTextureFormat() const = 0;
    virtual int GetSurfaceAttributes() const = 0;
    virtual int GetSurfaceContents() const = 0;
    virtual int GetSurfaceValue() const = 0;
    virtual void* GetPalette() const = 0;
    virtual bool HasData() const = 0;
    virtual bool HasPalette() const = 0;
    virtual bool Load() const = 0;
    virtual void Reload() const = 0;
    virtual bool IsLoaded() const = 0;
    virtual const char* GetFileName() const = 0;
    virtual bool IsWater() const = 0;

    virtual int GetImageDataRGB(void* pData = NULL) = 0;
    virtual int GetImageDataRGBA(void* pData = NULL) = 0;

    virtual bool HasAlpha() const = 0;
    virtual bool IsDummy() const = 0;
    virtual int GetTextureID() const = 0;
    virtual void SetTextureID(int texId) = 0;
    virtual Material_t* GetMaterial(bool forceLoad=true) = 0;
};
