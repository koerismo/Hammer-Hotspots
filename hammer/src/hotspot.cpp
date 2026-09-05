#include "hotspot.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>

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
    float scale_diff = std::abs(std::log2(len_rect) - std::log2(len_surf));
    float cardinality = GetCardinalityFactor(dir_rect, config_.pow_cardinality);

    return (
        std::pow(dot_prod, cardinality) * config_.weight_dot +
        std::pow(scale_diff, config_.pow_scale_diff) * config_.weight_scale
    );
}

// Runs a tiling fit on the given dimensions.
float RectFitter::GetTiledScoreOnAxis(
    const Vec2f &dims_surf,
    const Vec2f &dims_rect,
    uint8_t major_axis,
    bool use_major,
    bool use_minor,
    Vec2f* out_tiling
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
    Vec2f* out_tiling
) {
    Vec2f dims_rect(rect.GetWidth(), rect.GetHeight());

    bool can_tile_x = rect.CanTileX();
    bool can_tile_y = rect.CanTileY();
    bool use_minor_axis = can_tile_x && can_tile_y;

    uint8_t major_axis =
        use_minor_axis ? (dims_surf.y > dims_surf.x) : can_tile_y;

    Vec2f tiling_1, tiling_2;
    float score_1, score_2;

    score_1 = GetTiledScoreOnAxis(dims_surf, dims_rect, major_axis, true, use_minor_axis, &tiling_1);
    score_2 = GetTiledScoreOnAxis(dims_surf, dims_rect, 1 - major_axis, use_minor_axis, true, &tiling_2);

    *out_tiling = score_2 > score_1 ? tiling_2 : tiling_1;
}

// Scales the given rect on one axis and tiles it on the other to apply as a trim.
void RectFitter::GetTrimScore(
    const Vec2f &dims_surf,
    const Rect &rect,
    float* out_score,
    Vec2f* out_tiling
) {
    const double rect_width = rect.GetWidth();
    const double rect_height = rect.GetHeight();

    // We already check the validity of this surface when we call this method,
    // so it doesn't need to be checked here.

    if (rect_width) {
        const float scale = dims_surf.x / rect_width;
        out_tiling->x = 1;
        out_tiling->y = dims_surf.y / scale;
        *out_score = GetBasicScore(dims_surf, Vec2f(rect_width, out_tiling->y));
    } else {
        const float scale = dims_surf.y / rect_height;
        out_tiling->x = dims_surf.x / scale;
        out_tiling->y = 1;
        *out_score = GetBasicScore(dims_surf, Vec2f(out_tiling->x, rect_height));
    }

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
    }

    // Trim texture (fit on one dimension and slide on the other)
    else if (
            (rect.CanTileX() && !width && height) ||
            (rect.CanTileY() && !height && width)) {
        GetTrimScore(dims_surf, rect, &out_result->score, &out_result->tiling);
    }

    // Standard texture
    else {
        out_result->tiling = Vec2f(1, 1);
        out_result->score = GetBasicScore(dims_surf, Vec2f(rect.GetWidth(), rect.GetHeight()));
    }
}

int RectFitter::FitRectToSurface(
    std::vector<Rect> rects,
    Vec2f &surf_dims,
    RectFitResult* out_result
) {
    std::vector<RectFitResult> fit_results;
    fit_results.reserve(rects.size());
    
    float best_score = -kFloatInf;
    int best_index = -1;

    for (int rect_idx = 0; rect_idx < rects.size(); rect_idx++) {

        const auto add_fit_result = [
                &fit_results, &best_score,
                &best_index, rect_idx,
                rects, this
            ](const bool rotated, const Vec2f surf_rotated) -> void {
                fit_results.push_back(RectFitResult(rect_idx, rotated));
                RectFitResult& result = fit_results.back();

                GetScore(surf_rotated, rects[rect_idx], &result);

                if (result.score > best_score) {
                    best_score = result.score;
                    best_index = fit_results.size() - 1;
                }
            };

        add_fit_result(false, surf_dims);
        if (rects[rect_idx].CanRotate()) {
            add_fit_result(true, surf_dims.Swapped());
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
void RectFitter::GetFinalBounds(Rect& rect, Vec2f& tiling, Rect *out_bounds) {
    out_bounds->mins = rect.mins;

    out_bounds->maxs.x = rect.mins.x == rect.maxs.x
        ? rect.mins.x + tiling.x
        : rect.mins.x + rect.GetWidth() * tiling.x;
    
    out_bounds->maxs.y = rect.mins.y == rect.maxs.y
        ? rect.mins.y + tiling.y
        : rect.mins.y + rect.GetHeight() * tiling.y;
}

} // namespace HotSpot
