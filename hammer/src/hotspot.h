#pragma once

#include <type_traits>
#include <vector>
#include <limits>
#include "interfaces.h"

namespace HotSpot {

const float kFloatInf = std::numeric_limits<float>::infinity();
const float kFloatEpsilon = std::numeric_limits<float>::epsilon();

union Vec2i {
    Vec2i() : x(0), y(0) {};
    template<typename T> requires(std::is_arithmetic_v<T>)
    Vec2i(T x, T y) : x(x), y(y) {};

    Vec2i Swapped() const { return Vec2i(y, x); }

    uint16_t xy[2];
    struct { uint16_t x, y; };
};

union Vec2f {
    Vec2f() : x(0), y(0) {};
    template<typename T> requires(std::is_arithmetic_v<T>)
    Vec2f(T x, T y) : x(x), y(y) {};

    inline Vec2f Swapped() const { return Vec2f(y, x); }
    inline float Dot(Vec2f &alt) { return x * alt.x + y * alt.y; }
    inline Vec2f Normalized(float* p_length) const {
        double length = std::hypot(x, y);
        if (p_length != nullptr) *p_length = length;
        return Vec2f(x / length, y / length);
    }

    double xy[2];
    struct { double x, y; };
};

// Defines a matrix with 2 columns and 3 rows,
// stored as [column][row] coordinates.
union Mat3x2 {
    double values[6];
    struct {
        double x[2];
        double y[2];
        double z[2];
    };

    void Multiply(const Vec2f &input, Vec2f &output) {
        output.x = input.x * x[0] + input.y * y[0] + z[0];
        output.y = input.x * x[1] + input.y * y[1] + z[1];
    }
};

// HotspotRectFlags_t
enum class RectFlags_t : unsigned char {
    enable_rotation   = 0x1,  // Can this region be randomly rotated?
    enable_reflection = 0x2,  // Can this region be randomly horizontally flipped?
    alt_group         = 0x4,  // If true, this region belongs to the alternate group.
    tile_x            = 0x8,  // Can this region tile horizontally?
    tile_y            = 0x10, // Can this region tile vertically?
    tile_x_y          = tile_x | tile_y,
};

// HotspotRect_t
struct Rect {
    uint16 flags;
    Vec2f mins, maxs;

    inline double GetWidth() const { return maxs.x - mins.x; }
    inline double GetHeight() const { return maxs.y - mins.y; }

    inline bool CanRotate() const { return flags & static_cast<uint8_t>(RectFlags_t::enable_rotation); }
    inline bool CanReflect() const { return flags & static_cast<uint8_t>(RectFlags_t::enable_reflection); }
    inline bool CanTile() const { return flags & static_cast<uint8_t>(RectFlags_t::tile_x_y); }
    inline bool CanTileX() const { return flags & static_cast<uint8_t>(RectFlags_t::tile_x); }
    inline bool CanTileY() const { return flags & static_cast<uint8_t>(RectFlags_t::tile_y); }
    inline bool IsAltGroup() const { return flags & static_cast<uint8_t>(RectFlags_t::alt_group); }
};

struct RectFitResult {
    RectFitResult(int rect_idx, bool rotated) : rect_idx(rect_idx), rotated(rotated) {};

    int rect_idx;
    Vec2i tiling;
    bool rotated;
    float score;
};

struct RectFile {
    uint8_t flags;
    Vec2i tex_size;
    std::vector<Rect> rects;
};

struct WeightConfig {
    // Aspect scores are raised to the power of ~6.0 when either dimension of the rect approaches 0
    float pow_cardinality = 6.0;
    // perfect aspect = 100.0, worst-case approaches 0.0
    float weight_dot = 100.0;
    // perfect scale = 50.0,
    // 2x smaller/larger = 0.5 * (50.0) = 25.0,
    // 4x smaller/larger = 0.25 * (50.0) = 12.5
    float weight_scale = 50.0;
    // 1x1 tiling = 0.0, 12x12 tiling = -0.144
    float weight_tiling = -0.001;
    // The error margin within which matches can be randomized.
    float error_margin = 0.1;
};

class RectFitter {
public:
    RectFitter() {}
    RectFitter(WeightConfig config) : config_(config) {}

public:
    WeightConfig config_;

protected:
    // Biases dot comparisons towards stricter results the closer they are to a cardinal axis.
    // Returns a value from `1.0` (on diagonals) to `max_v` (on cardinals)
    float GetCardinalityFactor(const Vec2f &nrm_dir, float max_v);

    // Returns the score from attempting to stretch and scale the given rect to the given surface.
    float GetBasicScore(const Vec2f &dims_surf, const Vec2f &dims_rect);

    // Runs a tiling fit on the given dimensions.
    float GetTiledScoreOnAxis(
        const Vec2f &dims_surf,
        const Vec2f &dims_rect,
        uint8_t major_axis,
        bool use_major,
        bool use_minor,
        Vec2i *out_tiling);

    // Calculates two tiling fits (one for each leading axis) and returns the best one.
    void GetTiledScore(
        const Vec2f &dims_surf,
        const Rect &rect,
        float *out_score,
        Vec2i *out_tiling);

public:
    // Uses all available rect flags to calculate a best-case score,
    // orientation, and tiling for the provided rect.
    void GetScore(
        const Vec2f &dims_surf,
        const Rect &rect,
        RectFitResult *out_result);

    // Finds a random best rect within `kErrorMargin` for `dims_surf` and returns the fitting info.
    int FitRectToSurface(
        std::vector<Rect> rects,
        Vec2f &dims_surf,
        RectFitResult *out_result);

    void GetOffsetAndInvScale(
        RectFile *file,
        int idx,
        Vector2 *out_offset,
        Vector2 *out_inv_scale);

    // Applies tiling to the given rect and returns its final bounds.
    void GetFinalBounds(
        Rect& rect,
        Vec2i& tiling,
        double inset,
        Rect *out_bounds);

    // Returns the pixel coordinate transformation for the given rect and image size.
    void GetFinalTransform(
        Vec2f& tex_size,
        Rect& rect,
        Vec2i& tiling,
        double inset,
        int rotation,
        Mat3x2& out_matrix);
};

} // namespace HotSpot
