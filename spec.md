# HotSpot File Spec

This specification defines a simple format for storing HotSpot texture targets.

### Structure

```
|- RectHeader_t
|- [For rect in rectCount]
|   |- Rect_t
```

### File Header

```cpp
// sizeof(RectHeader_t) = 14
struct RectHeader_t
{
    char        magic[4];   // "RECT"
    uint16_t    flags;      // No flags defined yet, but we may need them later
    IVec2_t     texSize;    // The texture size
    uint16_t    rectCount;  // Number of rects
    uint16_t    rectOffset; // The 0x offset to the first rect
}
```

### Rect Entry

```cpp
// sizeof(Rect_t) = 10
struct Rect_t
{
    uint16_t    flags;      // The rect-specific flags
    IVec2_t     mins;       // The bounding mins
    IVec2_t     maxs;       // The bounding maxes
}
```

### Vec2

```cpp
// sizeof(IVec2_t) = 4
struct IVec2_t
{
    uint16_t x;
    uint16_t y;
}
```
