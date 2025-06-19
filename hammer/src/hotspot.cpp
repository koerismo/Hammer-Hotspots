#include "hotspot.h"

#include <stddef.h>
#include <vector>
#include <random>

namespace HotSpot {

/// If any additional rects are within this threshold,
/// they should be used to randomize the result.
const float aspectErrorMargin = 0.01f;

/// After the aspects are checked, the results
/// can be weighted by the world size error
/// With a value of 0.6, a 2x-small texture will have a weight of 62.5%
/// With a value of 8.0, a 2x-small texture will have a weight of 11.1%
const float worldSizeWeight = 5.0f;

Rect CreateRect(Vec2i mins, Vec2i maxs, uint16_t flags) {
    return { flags, mins, maxs };
}

RectHeader* ParseRectFile(void* data) {
    return NULL;
}

int MatchRandomBestRect(RectHeader* file, float targetAspect, float targetScale, float* resultDiff) {
    if (!file || !file->rects.size()) return -1;
    float logTargetScale = std::log2f(targetScale);

    std::vector<float> aspectErrors(file->rects.size());
    std::vector<float> scaleWeights(file->rects.size());
    float bestDiff = INFINITY;
    
    // Calculate the aspect ratio error of each rect
    for (int i=0; i<aspectErrors.size(); i++) {
        Rect rect = file->rects[i];
        float width = static_cast<float>(rect.maxs.x - rect.mins.x);
        float height = static_cast<float>(rect.maxs.y - rect.mins.y);
        float maxDim = (width > height ? width : height);
        if (height < FLT_EPSILON || width < FLT_EPSILON) continue;

        float aspect = width / height;
        float aspectDiff = fabs(aspect - targetAspect);
        aspectErrors[i] = aspectDiff;
        scaleWeights[i] = 1 / (1 + fabs(std::log2f(maxDim) - logTargetScale) * worldSizeWeight);
        if (aspectDiff < bestDiff) bestDiff = aspectDiff;
    }

    std::vector<int> matches;
    
    // Pick only the best matches within an error margin
    for (int i=0; i<aspectErrors.size(); i++) {
        if (aspectErrors[i] <= bestDiff + aspectErrorMargin)
            matches.push_back(i);
    }

    if (matches.size() == 0) return -1;
    if (matches.size() == 1) return matches[0];

    // Accumulate sum so we can proportionally weight by scale
    float weightSum = 0.0f;
    for (int i=0; i<matches.size(); i++) {
        weightSum += scaleWeights[matches[i]];
    }

    float value = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * weightSum;
    int result = -1;

    // Pick random value, accounting for weights
    for (int i=0; i<matches.size(); i++) {
        float weight = scaleWeights[matches[i]];
        if (value < weight) {
            result = matches[i];
            break;
        }
        value -= weight;
    }

    if (result != -1 && resultDiff != NULL) *resultDiff = aspectErrors[result];
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
