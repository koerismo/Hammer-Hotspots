#pragma once

#include <vector>

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

/// Parses the specified .rect file into a RectHeader
RectHeader* ParseRectFile(void* data);

/// Returns the index of a random rect match.
int MatchRandomBestRect(RectHeader* file, float targetAspect, float* resultDiff=NULL);

/// Returns the reciprocal of the rect's scaling.
void GetInvScaleFactor(RectHeader* header, int i, float* out_x, float* out_y);

} // namespace HotSpot
