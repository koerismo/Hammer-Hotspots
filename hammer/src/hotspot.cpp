#include "hotspot.h"

#include <stddef.h>
#include <vector>
#include <random>

namespace HotSpot {

/// If any other rects are within this threshold,
/// they should be used to randomize the result.
const float aspectErrorMargin = 0.02f;

// (1.0 = allow 2x bigger or 2x smaller textures)
const float scaleErrorMargin = 0.20f;

Rect CreateRect(Vec2i mins, Vec2i maxs, uint16_t flags) {
    return { flags, mins, maxs };
}

RectContainer* ParseRectFile(void* data) {
    return NULL;
}

int MatchRandomBestRect(RectContainer* file, float targetAspect, float targetScale, bool* out_isRotated, float* out_aspectErr, float* out_scalingErr) {
    if (!file || !file->rects.size()) return -1;
    float logTargetScale = std::log2f(targetScale);
    
    size_t rectCount = file->rects.size();
    std::vector<bool>  rectsRotated(rectCount);
    std::vector<float> aspectErrors(rectCount);
    std::vector<float>  scaleErrors(rectCount);
    float bestAspectError = INFINITY;
    
    // Calculate the aspect ratio and scaling errors of each rect
    for (int i=0; i<rectCount; i++) {
        Rect rect = file->rects[i];
        float width = static_cast<float>(rect.maxs.x - rect.mins.x);
        float height = static_cast<float>(rect.maxs.y - rect.mins.y);
        float maxDim = (width > height ? width : height);
        if (height < FLT_EPSILON || width < FLT_EPSILON) continue;

        float aspect = width / height;
        rectsRotated[i] = rect.CanRotate() && ((aspect > 1) != (targetAspect > 1));
        if (rectsRotated[i]) aspect = 1.0f / aspect;

        float aspectError = fabs(aspect - targetAspect);
        aspectErrors[i] = aspectError;
        scaleErrors[i] = fabs(std::log2f(maxDim) - logTargetScale);
        if (aspectError < bestAspectError) bestAspectError = aspectError;
    }

    std::vector<int> aspectMatches;
    float bestScaleError = INFINITY;
    
    // Pick only the best aspect matches within an error margin
    for (int i=0; i<rectCount; i++) {
        if (aspectErrors[i] > bestAspectError + aspectErrorMargin) continue;
        if (scaleErrors[i] < bestScaleError) bestScaleError = scaleErrors[i];
        aspectMatches.push_back(i);
    }

    std::vector<int> finalMatches;
    
    // Pick only the best scaling matches within an error margin
    for (int i=0; i<aspectMatches.size(); i++) {
        if (scaleErrors[aspectMatches[i]] > bestScaleError + scaleErrorMargin) continue;
        finalMatches.push_back(aspectMatches[i]);
    }

    if (finalMatches.size() == 0) return -1;
    int result = finalMatches[std::rand() % finalMatches.size()];

    *out_isRotated = rectsRotated[result];
    if (out_aspectErr  != NULL) *out_aspectErr  = aspectErrors[result];
    if (out_scalingErr != NULL) *out_scalingErr = scaleErrors[result];
    return result;
}

void GetOffsetAndInvScale(RectContainer* header, int i, Vector2* vOffset, Vector2* vInvScale) {
    Rect* rect = &header->rects[i];
    vOffset->x =  static_cast<float>(rect->mins.x) / static_cast<float>(header->texSize.x);
    vOffset->y =  static_cast<float>(rect->mins.y) / static_cast<float>(header->texSize.y);
    vInvScale->x = static_cast<float>(header->texSize.x) / static_cast<float>(rect->maxs.x - rect->mins.x);
    vInvScale->y = static_cast<float>(header->texSize.y) / static_cast<float>(rect->maxs.y - rect->mins.y);
    return;
}

} // namespace HotSpot
