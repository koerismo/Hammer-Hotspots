#include "hotspot.h"

#include <stddef.h>
#include <vector>
#include <random>

namespace HotSpot {

/// If any additional rects are within this threshold,
/// they should be used to randomize the result.
const float aspectErrorMargin = 0.01;


RectHeader* ParseRectFile(void* data) {
    return NULL;
}

int MatchRandomBestRect(RectHeader* file, float targetAspect, float* resultDiff=NULL) {
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
    int result = std::rand() % matches.size();
    if (resultDiff != NULL) *resultDiff = distances[result];
    return result;
}

void GetInvScaleFactor(RectHeader* header, int i, float* out_x, float* out_y) {
    Rect* rect = &header->rects[i];
    *out_x = (rect->maxs.x - rect->mins.x) / header->texSize.x;
    *out_y = (rect->maxs.y - rect->mins.y) / header->texSize.y;
    return;
}

} // namespace HotSpot
