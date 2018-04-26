#pragma once
#include <cstddef>

constexpr std::size_t ANIMATION_FRAME_COUNT = 20;

constexpr std::size_t SPECIFIC_HOSTILE_ENTITIES_COUNT = 5;
constexpr std::size_t CONCURRENT_TREES_COUNT = 5;

constexpr std::size_t DESTRUCTION_SCARS_COUNT = 6;
constexpr std::size_t CONVEX_POLYS_COUNT = 10;
constexpr std::size_t CONVEX_POLY_VERTEX_COUNT = 8;

constexpr std::size_t RENDERING_POLYGON_VERTEX_COUNT = 20;
constexpr std::size_t RENDERING_POLYGON_INDEX_COUNT = 20;

constexpr std::size_t OWNER_FRICTION_GROUNDS_COUNT = 10;

constexpr std::size_t RECOIL_OFFSET_COUNT = 50;

constexpr std::size_t MAX_IMAGES_IN_ATLAS_COUNT = 1000;

// TODO: this will be view-bound, not logic-bound
constexpr std::size_t ONLY_PICK_THESE_ITEMS_COUNT = 20;

// TODO: this will be logic-bound, but still variable-sized (because in an inferred system)
constexpr std::size_t ITEMS_INSIDE_COUNT = 20;