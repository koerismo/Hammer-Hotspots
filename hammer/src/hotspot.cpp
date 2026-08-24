#include "hotspot.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>

namespace HotSpot {


// Aspect scores are raised to the power of ~6.0 when either dimension of the rect approaches 0
const float kPowCardinality = 6.0;
// Scale differences are raised to the power of 3.0
const float kPowScaleDiff = 3.0;


// perfect aspect = 100.0, worst-case approaches 0.0
const float kWeightDot = 100.0f;
// perfect scale = 0.0,
// 2x smaller/larger = 1^kPowScaleDiff * -2.0 = -2.0,
// 3x smaller/larger = 2^kPowScaleDiff * -2.0 = -16.0
const float kWeightScale = -2.0f;
// 1x1 tiling = 0.0, 12x12 tiling = -0.144
const float kWeightTiling = -0.001f;
// The error margin within which matches can be randomized.
const float kErrorMargin = 0.1f;


// Biases dot comparisons towards stricter results the closer they are to a cardinal axis.
// Returns a value from `1.0` (on diagonals) to `max_v` (on cardinals)
float GetCardinalityFactor(const Vec2f &nrm_dir, float max_v) {
    float basis = std::abs(nrm_dir.x) + std::abs(nrm_dir.y);
    return std::max(1.0, max_v - (basis - 1.0) * (max_v - 1.0) / std::numbers::sqrt2);
}

// Returns the score from attempting to stretch and scale the given rect to the given surface.
float GetBasicScore(const Vec2f &dims_surf, const Vec2f &dims_rect) {
    float len_surf, len_rect;
    Vec2f dir_surf = dims_surf.Normalized(&len_surf);
    Vec2f dir_rect = dims_rect.Normalized(&len_rect);

    float dot_prod = dir_surf.Dot(dir_rect);
    float scale_diff = std::abs(std::log2(len_rect) - std::log2(len_surf));
    float cardinality = GetCardinalityFactor(dir_rect, kPowCardinality);

    return (
        std::pow(dot_prod, cardinality) * kWeightDot +
        std::pow(scale_diff, kPowScaleDiff) * kWeightScale
    );
}

// Runs a tiling fit on the given dimensions.
float GetTiledScoreOnAxis(
    const Vec2f &dims_surf,
    const Vec2f &dims_rect,
    uint8_t major_axis,
    bool use_major,
    bool use_minor,
    Vec2i* out_tiling
) {
    const uint8_t minor_axis = 1 - major_axis;    

    float major_axis_scale = dims_surf.xy[major_axis] / dims_rect.xy[major_axis];
    int major_axis_count = use_major
        ? std::max<int>(1, std::lround(major_axis_scale))
        : 1;

    float major_axis_tiled_scale = major_axis_scale / major_axis_count;
    int minor_axis_count = use_minor
        ? std::max<int>(1, std::lround(dims_surf.xy[minor_axis] / dims_rect.xy[minor_axis] / major_axis_tiled_scale))
        : 1;

    out_tiling->xy[major_axis] = major_axis_count;
    out_tiling->xy[minor_axis] = minor_axis_count;

    Vec2f dims_rect_tiled{dims_rect.x * out_tiling->x, dims_rect.y * out_tiling->y};
    return GetBasicScore(dims_surf, dims_rect_tiled);
}

// Calculates two tiling fits (one for each leading axis) and returns the best one.
void GetTiledScore(
    const Vec2f &dims_surf,
    const Rect &rect,
    float* out_score,
    Vec2i* out_tiling
) {
    Vec2f dims_rect(rect.GetWidth(), rect.GetHeight());

    bool can_tile_x = rect.CanTileX();
    bool can_tile_y = rect.CanTileY();
    bool use_minor_axis = can_tile_x && can_tile_y;

    uint8_t major_axis =
        use_minor_axis ? (dims_surf.y > dims_surf.x) : can_tile_y;

    Vec2i tiling_1, tiling_2;
    float score_1, score_2;

    score_1 = GetTiledScoreOnAxis(dims_surf, dims_rect, major_axis, true, use_minor_axis, &tiling_1);
    score_2 = GetTiledScoreOnAxis(dims_surf, dims_rect, 1 - major_axis, use_minor_axis, true, &tiling_2);

    *out_tiling = score_2 > score_1 ? tiling_2 : tiling_1;
}

// Uses all available rect flags to calculate a best-case score, orientation, and tiling for the provided rect.
void GetScore(
    const Vec2f &dims_surf,
    const Rect &rect,
    float* out_score,
    bool* out_rotated,
    Vec2i* out_tiling
) {
    if (rect.CanTileX() || rect.CanTileY()) {
        GetTiledScore(dims_surf, rect,  out_score, out_tiling);

        if (rect.CanRotate()) {
            float out_r_score;
            Vec2i out_r_tiling;
            GetTiledScore({ dims_surf.y, dims_surf.x },rect, &out_r_score, &out_r_tiling);

            if (out_r_score > *out_score) {
                *out_score = out_r_score;
                *out_tiling = out_r_tiling;
                *out_rotated = true;
            }
        } else {
            *out_rotated = false;
        }

    } else {
        *out_tiling = Vec2i(1, 1);
        *out_rotated =
            (dims_surf.y > dims_surf.x) != (rect.GetHeight() > rect.GetWidth());

        Vec2f dims_rect(rect.GetWidth(), rect.GetHeight());

        *out_score = GetBasicScore(
            *out_rotated ? dims_surf.Swapped() : dims_surf,
            dims_rect
        );
    }
}

// Finds a random best rect within `kErrorMargin` for `dims_surf` and returns the fitting info.
int FitRectToSurface(
    std::vector<Rect> rects,
    Vec2f &dims_surf,
    RectFitResult* out_result
) {
    std::vector<RectFitResult> fit_results(rects.size());
    float best_score = -kFloatInf;
    int best_index = -1;

    for (int i = 0; i < rects.size(); i++) {
        float score;
        bool rotated;
        Vec2i tiling;

        RectFitResult& result = fit_results[i];
        GetScore(dims_surf, rects[i], &result.score, &result.rotated, &result.tiling);
        if (result.score > best_score) {
            best_score = result.score;
            best_index = i;
        }
    }

    std::vector<int> best_results;

    for (int i = 0; i < rects.size(); i++) {
        RectFitResult& result = fit_results[i];
        if (result.score < best_score - kErrorMargin) continue;
        best_results.push_back(i);
    }

    if (best_results.size() == 0)
        return -1;

    int result_idx = best_results[std::rand() % best_results.size()];
    RectFitResult& result = fit_results[result_idx];
    *out_result = result;

    return result_idx;
}

void GetOffsetAndInvScale(RectFile* file, int idx, Vector2* out_offset, Vector2* out_inv_scale) {
    Rect* rect = &file->rects[idx];
    out_offset->x =  static_cast<float>(rect->mins.x) / static_cast<float>(file->tex_size.x);
    out_offset->y =  static_cast<float>(rect->mins.y) / static_cast<float>(file->tex_size.y);
    out_inv_scale->x = static_cast<float>(file->tex_size.x) / static_cast<float>(rect->GetWidth());
    out_inv_scale->y = static_cast<float>(file->tex_size.y) / static_cast<float>(rect->GetHeight());
    return;
}

} // namespace HotSpot
