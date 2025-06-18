#pragma once

#include <vector>
#include "interfaces.h"

namespace HotSpot {

typedef unsigned short uint16;

struct Vec2i {
    uint16 x, y;
};

struct Rect {
    uint16 flags;
    Vec2i mins, maxs;
};

struct FileRectHeader {
    char   magic[4];
    uint16 flags;
    Vec2i  texSize;
    uint16 rectCount;
    uint16 rectOffset;
};

struct RectHeader {
    uint16 flags;
    Vec2i  texSize;
    std::vector<Rect> rects;
};

Rect CreateRect(Vec2i mins, Vec2i maxs, uint16_t flags=0);

/// Parses the specified .rect file into a RectHeader
RectHeader* ParseRectFile(void* data);

/// Returns the index of a random rect match.
int MatchRandomBestRect(RectHeader* file, float targetAspect, float* resultDiff=NULL);

/// Returns the reciprocal of the rect's scaling.
void GetOffsetAndInvScale(RectHeader* header, int i, Vector2* vOffset, Vector2* vInvScale);

} // namespace HotSpot
