#include "hotspot.h"

#include <stddef.h>
#include <vector>
#include <random>

namespace HotSpot {

/// If any additional rects are within this threshold,
/// they should be used to randomize the result.
const float aspectErrorMargin = 0.01f;

Rect CreateRect(Vec2i mins, Vec2i maxs, uint16_t flags) {
    return { flags, mins, maxs };
}

RectHeader* ParseRectFile(void* data) {
    return NULL;
}

int MatchRandomBestRect(RectHeader* file, float targetAspect, float* resultDiff) {
    if (!file || !file->rects.size()) return -1;

    std::vector<float> distances(file->rects.size());
    std::vector<int> matches;

    float bestDiff = INFINITY;

    for (int i=0; i<file->rects.size(); i++) {
        Rect rect = file->rects[i];
        float width = static_cast<float>(rect.maxs.x - rect.mins.x);
        float height = static_cast<float>(rect.maxs.y - rect.mins.y);
        if (height < FLT_EPSILON || width < FLT_EPSILON) continue;

        float aspect = width / height;
        float aspectDiff = fabs(aspect - targetAspect);
        distances[i] = aspectDiff;
        if (aspectDiff < bestDiff) bestDiff = aspectDiff;
    }

    for (int i=0; i<file->rects.size(); i++) {
        if (distances[i] <= bestDiff + aspectErrorMargin)
            matches.push_back(i);
    }

    // This should never happen, but just in case...
    if (matches.size() == 0) return -1;

    // Return random matching index
    int result = matches[std::rand() % matches.size()];
    if (resultDiff != NULL) *resultDiff = distances[result];
    return result;
}

void GetOffsetAndInvScale(RectHeader* header, int i, Vector2* vOffset, Vector2* vInvScale) {
    Rect* rect = &header->rects[i];
    vOffset->x =  static_cast<float>(rect->mins.x) / static_cast<float>(header->texSize.x);
    vOffset->y =  static_cast<float>(rect->mins.y) / static_cast<float>(header->texSize.y);
    vInvScale->x = static_cast<float>(header->texSize.x) / static_cast<float>(rect->maxs.x - rect->mins.x);
    vInvScale->y = static_cast<float>(header->texSize.y) / static_cast<float>(rect->maxs.y - rect->mins.y);
    return;
}

} // namespace HotSpot
