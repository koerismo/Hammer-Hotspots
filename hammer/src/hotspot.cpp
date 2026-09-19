#include "hotspot.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <utility>

namespace HotSpot {


// Biases dot comparisons towards stricter results the closer they are to a cardinal axis.
// Returns a value from `1.0` (on diagonals) to `max_v` (on cardinals)
float RectFitter::GetCardinalityFactor(const Vec2f &nrm_dir, float max_v) {
    float basis = std::abs(nrm_dir.x) + std::abs(nrm_dir.y);
    return std::max(1.0, max_v - (basis - 1.0) * (max_v - 1.0) / std::numbers::sqrt2);
}

// Returns the score from attempting to stretch and scale the given rect to the given surface.
float RectFitter::GetBasicScore(const Vec2f &dims_surf, const Vec2f &dims_rect) {
    float len_surf, len_rect;
    Vec2f dir_surf = dims_surf.Normalized(&len_surf);
    Vec2f dir_rect = dims_rect.Normalized(&len_rect);

    float dot_prod = dir_surf.Dot(dir_rect);
    float cardinality = GetCardinalityFactor(dir_rect, config_.pow_cardinality);
    float dot_weight = std::pow(dot_prod, cardinality);

    float scale_weight = 0.0;
    if (len_rect > kFloatEpsilon && len_surf > kFloatEpsilon) {
        scale_weight = len_rect > len_surf
            ? len_surf / len_rect
            : len_rect / len_surf;
    }

    return (
        dot_weight * config_.weight_dot +
        scale_weight * config_.weight_scale
    );
}

// Runs a tiling fit on the given dimensions.
float RectFitter::GetTiledScoreOnAxis(
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

    Vec2f dims_rect_tiled(dims_rect.x * out_tiling->x, dims_rect.y * out_tiling->y);
    return GetBasicScore(dims_surf, dims_rect_tiled);
}

// Calculates two tiling fits (one for each leading axis) and returns the best one.
void RectFitter::GetTiledScore(
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

// Uses tiling (when applicable) to calculate a best-case score for the provided rect.
void RectFitter::GetScore(
    const Vec2f &dims_surf,
    const Rect &rect,
    RectFitResult* out_result
) {
    const double width = rect.GetWidth();
    const double height = rect.GetHeight();

    // Tiled texture
    if (rect.CanTile() && width && height) {
        GetTiledScore(dims_surf, rect,  &out_result->score, &out_result->tiling);
    } else {
        out_result->tiling = Vec2i(1, 1);
        out_result->score = GetBasicScore(dims_surf, Vec2f(rect.GetWidth(), rect.GetHeight()));
    }
}

int RectFitter::FitRectToSurface(
    std::vector<Rect> rects,
    Vec2f &surf_dims,
    RectFitResult* out_result
) {
    int fit_count = rects.size();
    for (int i=0; i<rects.size(); i++) {
        if (rects[i].CanRotate()) fit_count ++;
    }

    std::vector<RectFitResult> fit_results;
    fit_results.reserve(fit_count);

    float best_score = -kFloatInf;
    int best_index = -1;

    auto PushFit = [&fit_results](int rect_idx, bool rotated) -> RectFitResult& {
        fit_results.push_back(RectFitResult(rect_idx, rotated));
        return fit_results.back();
    };

    for (int rect_idx = 0; rect_idx < rects.size(); rect_idx++) {
        // Initial fit
        {
            RectFitResult& result = PushFit(rect_idx, false);
            GetScore(surf_dims, rects[rect_idx], &result);
    
            if (result.score > best_score) {
                best_score = result.score;
                best_index = fit_results.size() - 1;
            }
        }

        // Rotated fit
        if (rects[rect_idx].CanRotate()) {
            RectFitResult& result = PushFit(rect_idx, true);
            GetScore(surf_dims.Swapped(), rects[rect_idx], &result);

            if (result.score > best_score) {
                best_score = result.score;
                best_index = fit_results.size() - 1;
            }
        }
    }

    std::vector<int> best_fits;

    for (int fit_idx = 0; fit_idx < fit_results.size(); fit_idx++) {
        RectFitResult& result = fit_results[fit_idx];
        if (result.score < best_score - config_.error_margin) continue;
        best_fits.push_back(fit_idx);
    }

    if (best_fits.size() == 0)
        return -1;

    int result_idx = best_fits[std::rand() % best_fits.size()];
    RectFitResult& result = fit_results[result_idx];
    *out_result = result;

    return result.rect_idx;
}

void RectFitter::GetOffsetAndInvScale(RectFile* file, int idx, Vector2* out_offset, Vector2* out_inv_scale) {
    Rect* rect = &file->rects[idx];
    out_offset->x =  static_cast<float>(rect->mins.x) / static_cast<float>(file->tex_size.x);
    out_offset->y =  static_cast<float>(rect->mins.y) / static_cast<float>(file->tex_size.y);
    out_inv_scale->x = static_cast<float>(file->tex_size.x) / static_cast<float>(rect->GetWidth());
    out_inv_scale->y = static_cast<float>(file->tex_size.y) / static_cast<float>(rect->GetHeight());
    return;
}

// Applies tiling to the given rect and returns its final bounds.
void RectFitter::GetFinalBounds(Rect& rect, Vec2i& tiling, double inset, Rect *out_bounds) {
    out_bounds->mins.x = rect.mins.x + inset;
    out_bounds->mins.y = rect.mins.y + inset;
    out_bounds->maxs.x = rect.mins.x + rect.GetWidth() * tiling.x - inset * 2.0;
    out_bounds->maxs.y = rect.mins.y + rect.GetHeight() * tiling.y - inset * 2.0;
}

// Returns the pixel coordinate transform for the given rect and image size.
void RectFitter::GetFinalTransform(
    Vec2f& tex_size,
    Rect& rect,
    Vec2i& tiling,
    double inset,
    int rotation,
    Mat3x2& m
) {
    // Calculate bounds
    const double scale_x = (rect.GetWidth() * tiling.x - inset * 2.0) / tex_size.x;
    const double scale_y = (rect.GetHeight() * tiling.y - inset * 2.0) / tex_size.y;
    const double offset_x = rect.mins.x + inset;
    const double offset_y = rect.mins.y + inset;

    // Create identity matrix
    m.x[0] = 1;  m.x[1] = 0;
    m.y[0] = 0;  m.y[1] = 1;
    m.z[0] = 0;  m.z[1] = 0;

    // Rotate
    if (rotation == 2) {
        m.x[0] = -1; m.y[1] = -1;
        m.z[0] =  1; m.z[1] =  1;
    } else if (rotation) {
        std::swap(m.x, m.y);
        if (rotation > 0)  { m.x[1] = -1; m.z[1] = 1; }
        else               { m.y[0] = -1; m.z[0] = 1; }
    }

    // Scale
    m.x[0] *= scale_x;
    m.y[0] *= scale_x;
    m.z[0] *= scale_x;

    m.x[1] *= scale_y;
    m.y[1] *= scale_y;
    m.z[1] *= scale_y;

    // Translate
    m.z[0] += offset_x;
    m.z[1] += offset_y;
}

} // namespace HotSpot
