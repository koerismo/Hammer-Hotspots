#pragma once

#include <vector>
#include "interfaces.h"

namespace HotSpot {

typedef unsigned short uint16;

struct Vec2i {
    uint16 x, y;
};

// HotspotHeader_t
struct RectResourceHeader {
    unsigned char  version;    // The resource version. Currently 0x1
    unsigned char  flags;      // Implementation-specific flags for editors.
    unsigned short rect_count; // The number of rect regions.
};

// HotspotRectFlags_t
enum class RectFlags_t : unsigned char {
    enable_rotation   = 0x1, // Can this region be randomly rotated?
    enable_reflection = 0x2, // Can this region be randomly horizontally flipped?
    alt_group         = 0x4, // If true, this region belongs to the alternate group.
};

// HotspotRect_t
struct Rect {
    uint16 flags;
    Vec2i mins, maxs;

    bool CanRotate() { return flags & static_cast<uint8>(RectFlags_t::enable_rotation); }
    bool CanReflect() { return flags & static_cast<uint8>(RectFlags_t::enable_reflection); }
    bool IsAltGroup() { return flags & static_cast<uint8>(RectFlags_t::alt_group); }
};

struct RectContainer {
    uint8 flags;
    Vec2i  texSize;
    std::vector<Rect> rects;
};

Rect CreateRect(Vec2i mins, Vec2i maxs, uint16_t flags=0);

/// Parses the specified .rect file into a RectHeader
RectContainer* ParseRectFile(void* data);

/// Returns the index of a random rect match.
int MatchRandomBestRect(RectContainer* file, float targetAspect, float targetScale, bool altGroup, bool* out_isRotated, float* out_aspectErr=NULL, float* out_scalingErr=NULL);

/// Returns the reciprocal of the rect's scaling.
void GetOffsetAndInvScale(RectContainer* header, int i, Vector2* vOffset, Vector2* vInvScale);

} // namespace HotSpot
